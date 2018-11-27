#include "core/Cover.h"

#include <set>
#include <iterator>
#include <assert.h>
#include <iostream>

void sort_index(Index& index) {
    for (auto&& pair : index)
        std::sort(pair.second.begin(), pair.second.end());
}

ItemVec intersect(std::vector<ItemVec*> in_vecs) {
    ItemVec result;
    
    if (in_vecs.size() == 1) {
        result = *in_vecs[0];
    } else {
        std::vector<ItemVec*> tmp_vecs;
        
        for (int i=0; i < in_vecs.size() - 1; i += 2) {
            tmp_vecs.push_back(new ItemVec);
            
            std::set_intersection(in_vecs[i]->begin(), in_vecs[i]->end(),
                                  in_vecs[i+1]->begin(), in_vecs[i+1]->end(),
                                  std::back_inserter(*tmp_vecs.back()));
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

// Sort pattern indices in decreasing order of length
PatVec patterns_by_length(const std::vector<ItemVec>& patterns) {
    PatVec result;
    
    for (int i = 0; i < patterns.size(); i++)
        result.push_back(i);

    auto comp = [&](int a, int b){ return patterns[a].size() > patterns[b].size(); };
    std::sort(result.begin(), result.end(), comp);

    return result;
}

// Index [i] of the result lists all p \in [begin, end) with counts[p] == i
std::vector<PatVec> patterns_by_count(PatVec::iterator begin,
                                      PatVec::iterator end,
                                      std::map<int, int> counts) {
    std::vector<PatVec> queue;

    for (auto pat = begin; pat != end; pat++) {
        if (queue.size() <= counts[*pat])
            queue.resize(counts[*pat] + 1);
        queue[counts[*pat]].push_back(*pat);
    }

    return queue;
}

void Cover::reduce(int min_count, int min_len) {
    
    // We'll need to track how many items are covered by each pattern.
    std::map<int, int> counts;
    for (auto&& pair : itemsets)
        counts[pair.first] = pair.second.size();

    // We'll also need to track which patterns/items are covering/covered.
    std::map<int, bool> is_covering, is_covered;
    for (auto&& pair : itemsets){
        is_covering[pair.first] = false;
        for (auto&& item : pair.second)
            is_covered[item] = false;
    }        
        
    // Inverted index for *itemsets* -- maps items to the patterns that cover them.
    Index itemsets_inverse = invert(itemsets);

    // Pattern indices in decreasing order of length
    PatVec patterns_sorted = patterns_by_length(patterns);
    
    // Finds uniform pattern-length block endpoints in *patterns_sorted*
    auto find_block_end = [&](PatVec::iterator begin) {
        auto end = begin;
        while (++end != patterns_sorted.end())
            if (patterns[*begin].size() != patterns[*end].size())
                break;
        return end;
    };

    // Iterate through pattern indices in decreasing order of length
    PatVec::iterator begin, end;
    begin = end = patterns_sorted.begin();
    while (end != patterns_sorted.end()) {
        begin = end;
        end = find_block_end(begin);
        
        if (patterns[*begin].size() < min_len)
            break;

        // Iterate through the block [begin, end) in decreasing order of support
        std::vector<PatVec> queue = patterns_by_count(begin, end, counts);
        for (int count = queue.size() - 1; count >= min_count; count--) {
            for (auto&& pat : queue[count]) {
                assert(!is_covering[pat]);

                // Check if *pat* needs to be moved lower in the queue
                if (counts[pat] < count) {
                    assert(counts[pat] >= 0);
                    if (counts[pat] >= min_count)
                        queue[counts[pat]].push_back(pat);
                    continue;
                }

                // Add *pat* to the partial solution
                is_covering[pat] = true;

                // Subtract the items covered by *pat* from other patterns' counts
                for (auto&& item : itemsets[pat]) {
                    // We must only update *counts* once per item
                    if (is_covered[item]) continue;

                    // Mark *item* as covered and update *counts*
                    is_covered[item] = true;
                    for (auto&& other_pat : itemsets_inverse[item])
                        counts[other_pat]--;
                }
            }
        }
    }
    
    // Remove non-covering patterns from this->itemsets
    for (auto it = itemsets.begin(); it != itemsets.end();) {
        if (!is_covering[it->first])
            it = itemsets.erase(it);
        else
            ++it;
    }
}

void Cover::print() {
    using namespace std;
   
    cout << "*** Cover ***" << endl;
    for (auto&& pair : itemsets) {
        cout << "Pattern: ";
        for (auto&& x : patterns[pair.first])
            cout << x << " ";
        cout << endl;

        cout << "Covered clauses: ";
        for (auto&& x : pair.second)
            cout << x << " ";
        cout << endl;
    }
}
