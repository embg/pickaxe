#include "Cover.h"

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

vector<ItemVec> learnts;

void call_mafia(float min_sup) {
    std::ostringstream command;
    command << "../mafia"
            << " " << "-mfi"
            << " " << min_sup
            << " " << "-ascii"
            << " " << "mafia.in"
            << " " << "mafia.out";
    std::system(command.str().c_str());
}

// Write Mafia in_file for *solver->learnts[start : start + batch_size]*
void write_mafia_input(int start, int batch_size) {
    std::ofstream file("mafia.in");
    
    for (int i = start; i < start + batch_size; i++) {
        ItemVec c = learnts[i];

        for (int j = 0; j < c.size(); j++) {
            file << c[j];
            if (j < c.size() - 1)
                file << " ";
        }
        file << "\n";
    }

    file.close();
}

// Parse Mafia out_file into *cover* using *base_cover*
Cover read_mafia_output(Cover& base_cover){
    Cover cover;
    std::ifstream file("mafia.out");

    std::string line;
    while (getline(file, line)) {
        // Parse the line
        std::istringstream line_ss(line);
        int item;
        auto pattern = ItemVec();
        
        while (line_ss >> item)
            pattern.push_back(item);
        cover.patterns.push_back(pattern);

        // Collect relevant itemsets from base_cover
        auto pattern_itemsets = vector<ItemVec*>();
        for (auto&& i : pattern)
            pattern_itemsets.push_back(&base_cover.itemsets[i]);
        
        // Add the intersection of the base_cover itemsets to cover
        int pattern_idx = cover.patterns.size() - 1;
        cover.itemsets.insert({pattern_idx, intersect(pattern_itemsets)});
    }

    file.close();
    return cover;
}

// Build single-variable cover for *solver->learnts[start : start + batch_size]*
Cover build_base_cover(int start, int batch_size) {
    Cover base_cover;
    
    // First pass -- initialize itemsets
    for (int i = start; i < start + batch_size; i++) {
        ItemVec& c = learnts[i];
        for (int j = 0; j < c.size(); j++)
            base_cover.itemsets.emplace(std::make_pair(c[j], ItemVec()));
    }

    // Second pass -- populate itemsets
    for (int i = start; i < start + batch_size; i++) {
        ItemVec& c = learnts[i];
        for (int j = 0; j < c.size(); j++)
            base_cover.itemsets[c[j]].push_back(i);
    }

    sort_index(base_cover.itemsets);
    return base_cover;
}

// Run mafia on last *batch_size* learnt clauses, parse output, build cover, compute cover.
Cover build_cover(int start, int batch_size, float min_sup, int min_len) {
    assert(learnts.size() >= batch_size);             
        
    write_mafia_input(start, batch_size);
    call_mafia(min_sup);

    Cover base_cover = build_base_cover(start, batch_size);
    Cover mafia_cover = read_mafia_output(base_cover);
    
    mafia_cover.reduce((int) min_sup * batch_size, min_len);    
    return mafia_cover;
}

int main() {
    learnts = {
        {1, 3, 5, 7},
        {1, 3, 6, 5},
        {2, 3, 5, 10, 12, 14},
        {42},
        {2},
        {1,6,5,14},
        {1,3,14},
        {1,2,3}
    };

    int start = 0;
    int batch_size = learnts.size();
    float min_sup = 2./8;
    int min_len = 1;
    
    Cover cover = build_cover(start, batch_size, min_sup, min_len);

    Index test_idx = {
        {0, {1,2}},
        {1, {3,4}},
        {2, {0,2}}
    };
    
    //    assert(false);
}

