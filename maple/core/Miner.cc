#include "core/Solver.h"
#include "core/Miner.h"

#include <algorithm>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream> 
#include <fstream>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#include <cassert>
#define gettid() syscall(SYS_gettid)
using std::vector;

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

void Miner::call_mafia(float min_sup) {
    std::ostringstream command;
    command << "../mafia >/dev/null"
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

// Parse Mafia out_file into *cover* using *base_index*
Cover Miner::read_mafia_output(Index& base_index){
    Cover cover;
    std::ifstream file(out_filename);

    std::string line;
    while (getline(file, line)) {
        // Parse the line
        std::istringstream line_ss(line);
        int item;

        cover.patterns.push_back(ItemVec());
        while (line_ss >> item)
            cover.patterns.back().push_back(item);

        // Collect relevant itemsets from base_index
        auto pattern_itemsets = vector<ItemVec*>();
        for (auto&& i : cover.patterns.back())
            pattern_itemsets.push_back(&base_index[i]);
        
        // Add the intersection of the base_index itemsets to cover
        int pattern_idx = cover.patterns.size() - 1;
        cover.itemsets.insert({pattern_idx, intersect(pattern_itemsets)});
    }

    file.close();
    return cover;
}

// Build single-variable cover for *solver->learnts[start : start + batch_size]*
Index Miner::build_base_index(int start, int batch_size) {
    Index base_index;
    
    // First pass -- initialize itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];

        for (int j = 0; j < c.size(); j++)
            base_index.emplace(std::make_pair(toInt(c[j]), ItemVec()));
    }

    // Second pass -- populate itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];
        
        for (int j = 0; j < c.size(); j++)
            base_index[toInt(c[j])].push_back(i);
    }

    sort_index(base_index);
    return base_index;
}

// Run mafia on last *batch_size* learnt clauses, parse output, build cover, compute cover.
Cover Miner::build_cover(int start, int batch_size, float min_sup, int min_len) {
    assert(solver->learnts.size() >= batch_size);             
        
    write_mafia_input(start, batch_size);
    call_mafia(min_sup);

    Index base_index = build_base_index(start, batch_size);
    Cover mafia_cover = read_mafia_output(base_index);
    
    mafia_cover.reduce((int)(min_sup * batch_size), min_len);    
    return mafia_cover;
}
