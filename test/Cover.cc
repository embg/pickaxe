#include "Cover.h"
using namespace std;

void sort_index(Index& index) {
    for (auto&& pair : index)
        std::sort(pair.second.begin(), pair.second.end());
}

ItemVec intersect(vector<ItemVec*> in_vecs) {
    ItemVec result;
    
    if (in_vecs.size() == 1) {
        result = *in_vecs[0];
    } else {
        vector<ItemVec*> tmp_vecs;
        
        for (int i=0; i < in_vecs.size() - 1; i += 2) {
            tmp_vecs.push_back(new ItemVec);
            
            set_intersection(in_vecs[i]->begin(), in_vecs[i]->end(),
                                  in_vecs[i+1]->begin(), in_vecs[i+1]->end(),
                                  back_inserter(*tmp_vecs.back()));
        }
 
        if (in_vecs.size() % 2 == 1) 
            tmp_vecs.push_back(new ItemVec(*in_vecs.back()));
        
        result = intersect(tmp_vecs);

        for (auto&& vec : tmp_vecs)
            delete vec;
    }

    return result;
}

Index invert(const Index& original){
    Index inverted;

    // Initialize empty lists for all items
    for (auto&& pair : original)
        for (auto&& item : pair.second)
            inverted.emplace(make_pair(item, ItemVec()));

    // Populate the lists
    for (auto&& pair : original)
        for (auto&& item : pair.second)
            inverted[item].push_back(pair.first);

    sort_index(inverted);
    return inverted;
}

void Cover::reduce(int min_count, int min_len) {
    PatVec solution;
    
    // Maps pattern indices to their current coverage counts
    std::map<int, int> counts;
    
    // Index *L* is a heap over pattern indices *p* with *plen(p) = L*
    vector<PatVec> heaps;

    // Helper lambdas
    auto plen = [this](int p) { return (int)patterns[p].size(); };
    auto comp = [&counts](int a, int b) { return counts[a] > counts[b]; };
    
    // Resize *heaps* to longest pattern length
    int max_len = 0;
    for (auto&& pair : itemsets)
        max_len = max(max_len, plen(pair.first));
    heaps.resize(max_len + 1);

    // Initialize bookkeeping data structures
    for (auto&& pair : itemsets) {
        counts[pair.first] = pair.second.size();
        heaps[plen(pair.first)].push_back(pair.first); 
    }

    // Construct an inverted index -- maps items to the patterns that cover them
    Index itemsets_inverse = invert(itemsets);
    
    // Iterate through patterns in decreasing order of length
    for (int len = heaps.size() - 1; len >= min_len; len--) {
        PatVec heap = heaps[len];
        sort(heap.start(), heap.end(), comp); // heuristic... TODO
        
        // Iterate through the heap for this length
        for (auto&& pat : heap) {
            if (counts[pat] >= min_count) {
                solution.push_back(pat);

                // Subtract the items covered by *p* from other patterns' counts
                for (auto&& item : itemsets[pat])
                    for (auto&& pat : itemsets_inverse[item])
                        counts[pat]--;
            }
        }
    }

    // TODO: construct inverse index
    // Unfortunately, looks like I need to jump ship and move to Python (for the indep study)
    // I can probably give: (streaming) pattern stats for wide range of benchmarks, avg depth stats, MAYBE coverage
    // (which would require C++ translation from Python)

    // Still, this C++ stuff is a great start, at least the project has momentum now!
}
