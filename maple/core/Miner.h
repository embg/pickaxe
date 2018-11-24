#ifndef Minisat_Miner_h
#define Minisat_Miner_h

#include "core/Index.h"
#include <cstdio>
#include <string>

namespace Minisat {
    class Solver;
};

class Miner {
  public:
    Miner(int batch, float sup, Minisat::Solver &sol);
    ~Miner();
    
    void attachClause(); // TODO: implement callback
    void removeClause(); // TODO: implement callback (want to save clause if it's hidden in a super)
    
  private:
    Minisat::Solver &solver;
    Index index;
    
    int num_unprocessed;
    int batch_size;
    float min_sup; // TODO

    std::string in_filename;
    std::string out_filename;
    
    void process();
    
    /* Helpers for process() */
    void call_mafia();
    void write_mafia_input();
    void read_mafia_output();
};

#endif
