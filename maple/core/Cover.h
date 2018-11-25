#ifndef Minisat_Cover_h
#define Minisat_Cover_h

#include <vector>
#include <map>
#include <algorithm>

class Cover {
 public:
    Cover(){}
    ~Cover(){}

    std::vector<std::vector<int>> patterns;
    std::map<int, std::vector<int>> itemsets;
    
    void clear() {
        patterns.clear();
        itemsets.clear();
    }
    void sort_itemsets() {
        for (auto&& pair : itemsets)
            std::sort(pair.second.begin(), pair.second.end());
    }
    void reduce();
};

#endif
