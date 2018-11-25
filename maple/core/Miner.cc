#include "core/Solver.h"
#include "core/Miner.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream> 
#include <fstream>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

Miner::Miner(Minisat::Solver* sol) : solver(sol)
{
    std::ostringstream suffix_ss;
    suffix_ss << getpid() << "_" << gettid();
    in_filename = "/home/elliot/pickaxe/in_" + suffix_ss.str();
    out_filename = "/home/elliot/pickaxe/out_" + suffix_ss.str();    
}

Miner::~Miner() {
    // Delete the tempfiles
    remove((in_filename).c_str());
    remove((out_filename).c_str());
}

// Callback for Solver::attachClause
// void Miner::attachClause() {
//     num_unprocessed += 1;
//     if (num_unprocessed == batch_size)
//         process();
// }

// Start Mafia (blocks for input on the pipe in_filename)
void Miner::call_mafia(float min_sup) {
    std::ostringstream command;
    command << "./mafia"
            << " " << "-mfi"
            << " " << min_sup
            << " " << "-ascii"
            << " " << in_filename
            << " " << out_filename;
    std::system(command.str().c_str());
}

// Write Mafia in_file for *solver->learnts[start : start + batch_size]*
void Miner::write_mafia_input(int start, int batch_size) {
    std::ofstream file(in_filename);
    
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];

        for (int j = 0; j < c.size(); j++) {
            file << c[j].x;
            if (j < c.size() - 1)
                file << " ";
        }
        file << "\n";
    }

    file.close();
}

// Parse Mafia out_file into *cover* using *base_cover*
void Miner::read_mafia_output(Cover& cover, Cover& base_cover){
    std::ifstream file(out_filename);
    cover.clear();
    
    std::string line;
    while (getline(file, line)) {
        int item;
        std::vector<int> itemset;

        std::istringstream line_ss(line);
        while (line_ss >> item)
            itemset.push_back(item);
        
        //        cover.add(itemset); // TODO: MERGE ITEMSET INTO COVER USING BASE_COVER 
        itemset.clear();
    }

    file.close();
}

// Build single-variable cover for *solver->learnts[start : start + batch_size]*
void Miner::build_base_cover(Cover& base_cover, int start, int batch_size) {
    base_cover.clear();
    
    // First pass -- initialize itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];

        for (int j = 0; j < c.size(); j++)
            if (base_cover.itemsets.count(toInt(c[j])) == 0)
                base_cover.itemsets.insert(std::make_pair(toInt(c[j]), std::vector<int>()));
    }

    // Second pass -- populate itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];
        
        for (int j = 0; j < c.size(); j++)
            base_cover.itemsets[toInt(c[j])].push_back(i);
    }

    base_cover.sort_itemsets();
}

// Run mafia on last *batch_size* learnt clauses, parse output, build cover, compute cover.
int Miner::build_cover(Cover& cover, int start, int batch_size, float min_sup) {
    if (solver->learnts.size() >= batch_size) {
        Cover base_cover;
        build_base_cover(base_cover, start, batch_size);
        
        write_mafia_input(start, batch_size);
        call_mafia(min_sup);        
        read_mafia_output(cover, base_cover);

        cover.reduce();
        return 1;
    }
    return 0;
}
