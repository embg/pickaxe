#ifndef Minisat_Index_h
#define Minisat_Index_h

#include <vector>

class Index {
 public:
    Index(){};
    ~Index(){};
    
    void clear();
    void add(std::vector<int> &itemset);
    void reduce();
    
    // private:
    //    std::vector<std::vector<int>> itemsets;
};

#endif
