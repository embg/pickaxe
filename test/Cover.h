#ifndef Minisat_Cover_h
#define Minisat_Cover_h

#include <vector>
#include <queue>
#include <map>
#include <algorithm>

// Main data structures
typedef std::vector<int> ItemVec;
typedef std::vector<int> PatVec;
typedef std::map<int, std::vector<int>> Index;

class Cover {
 public:
    Cover(){}
    ~Cover(){}

    std::vector<ItemVec> patterns;
    Index itemsets;
    
    void reduce(int min_count, int min_len);
    void clear() {
        patterns.clear();
        itemsets.clear();
    }
    void print();
};

// Helper functions
void sort_index(Index& index);
ItemVec intersect(std::vector<ItemVec*> in_vecs);
Index invert(const Index& original);

#endif
