#include "core/Solver.h"
#include "core/Miner.h"

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

Miner::Miner(Minisat::Solver* sol, int batch, float sup)
    : solver(sol)
    , batch_size(batch)
    , min_sup(sup)
{
    std::ostringstream suffix_ss;
    suffix_ss << getpid() << "_" << gettid();
    in_filename = "/home/elliot/pickaxe/in_" + suffix_ss.str();
    out_filename = "/home/elliot/pickaxe/out_" + suffix_ss.str();    
}

Miner::~Miner() {}

// Callback for Solver::attachClause
void Miner::attachClause() {
    num_unprocessed += 1;
    if (num_unprocessed == batch_size)
        process();
}

// Start Mafia (blocks for input on the pipe in_filename)
void Miner::call_mafia() {
    std::ostringstream command;
    command << "./mafia"
            << " " << "-mfi"
            << " " << min_sup
            << " " << "-ascii"
            << " " << in_filename
            << " " << out_filename;
    std::system(command.str().c_str());
}

// Write Mafia in_file for *solver.learnts[-batch_size:]*
void Miner::write_mafia_input() {
    std::ofstream file(in_filename);
    
    int start = solver->learnts.size() - batch_size;
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];

        for (int j = 0; j < c.size(); j++) {
            file << c[j].x;
            if (j < c.size() - 1) {
                file << " ";
            }
        }
        file << "\n";
    }

    file.close();
}

// Read Mafia out_file into *index*
void Miner::read_mafia_output(){
    std::ifstream file(out_filename);
    
    index.clear();
    std::string line;
    while (getline(file, line)) {
        int item;
        std::vector<int> itemset;

        std::istringstream line_ss(line);
        while (line_ss >> item)
            itemset.push_back(item);
        
        index.add(itemset);
        itemset.clear();
    }

    file.close();
}

// Run mafia, parse output, build index, compute cover
void Miner::process() {
    if (solver->learnts.size() >= batch_size) {
        write_mafia_input();
        call_mafia();
        read_mafia_output();
        index.reduce();
        remove((in_filename).c_str());
        remove((out_filename).c_str());
    }
    num_unprocessed = 0;
}
