#include "core/Index.h"
#include <iostream>

using namespace std;

void Index::clear() {
    // Todo
}

void Index::add(std::vector<int> &itemset) {
    for (int i = 0; i < itemset.size(); i++)
        cout << itemset[i];
    cout << endl;
}

void Index::reduce() {
    // Todo
}
