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
Cover Miner::read_mafia_output(Cover& base_cover){
    Cover cover;
    std::ifstream file(out_filename);

    std::string line;
    while (getline(file, line)) {
        // Parse the line
        std::istringstream line_ss(line);
        int item;
        auto pattern = vector<int>();
        
        while (line_ss >> item)
            pattern.push_back(item);
        cover.patterns.push_back(pattern);

        // Collect relevant itemsets from base_cover
        auto pattern_itemsets = vector<vector<int>*>();
        for (auto&& i : pattern)
            pattern_itemsets.push_back(&base_cover.itemsets[i]);
        
        // Add the intersection of the base_cover itemsets to cover
        int pattern_idx = cover.patterns.size() - 1;
        cover.itemsets.insert(std::make_pair(
            pattern_idx, intersect(pattern_itemsets))
        );
    }

    file.close();
    return cover;
}

vector<int> intersect(vector<vector<int>*> in_vecs) {
    vector<int> result;
    
    if (in_vecs.size() == 1) {
        result = *in_vecs[0];
    } else {
        vector<vector<int>*> tmp_vecs;
        
        for (int i=0; i < in_vecs.size() - 1; i += 2) {
            tmp_vecs.push_back(new vector<int>);
            
            std::set_intersection(in_vecs[i]->begin(), in_vecs[i]->end(),
                                  in_vecs[i+1]->begin(), in_vecs[i+1]->end(),
                                  std::back_inserter(*tmp_vecs.back()));
        }
        
        if (in_vecs.size() % 2 == 1) 
            tmp_vecs.push_back(new vector<int>(*in_vecs.back()));
        
        result = intersect(tmp_vecs);

        for (auto&& vec : tmp_vecs)
            delete vec;
    }

    return result;
}

// Build single-variable cover for *solver->learnts[start : start + batch_size]*
Cover Miner::build_base_cover(int start, int batch_size) {
    Cover base_cover;
    
    // First pass -- initialize itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];

        for (int j = 0; j < c.size(); j++)
            if (base_cover.itemsets.count(toInt(c[j])) == 0)
                base_cover.itemsets.insert(std::make_pair(toInt(c[j]),
                                                          vector<int>()));
    }

    // Second pass -- populate itemsets
    for (int i = start; i < start + batch_size; i++) {
        Minisat::CRef cr = solver->learnts[i];
        Minisat::Clause& c = solver->ca[cr];
        
        for (int j = 0; j < c.size(); j++)
            base_cover.itemsets[toInt(c[j])].push_back(i);
    }

    base_cover.sort_itemsets();
    return base_cover;
}

// Run mafia on last *batch_size* learnt clauses, parse output, build cover, compute cover.
Cover Miner::build_cover(int start, int batch_size, float min_sup) {
    assert(solver->learnts.size() >= batch_size);             
    
    write_mafia_input(start, batch_size);
    call_mafia(min_sup);

    Cover base_cover = build_base_cover(start, batch_size);
    Cover mafia_cover = read_mafia_output(base_cover);
    
    mafia_cover.reduce();    
    return mafia_cover;
}
