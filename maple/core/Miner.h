#ifndef Minisat_Miner_h
#define Minisat_Miner_h

#include "core/Cover.h"
#include "core/Solver.h"

#include <cstdio>
#include <string>

class Miner {
  public:
    Miner(Minisat::Solver* sol);
    ~Miner();
    
    //void attachClause(); // TODO: implement callback
    //void removeClause(); // TODO: implement callback (want to save clause if it's hidden in a super)
    
    Cover build_cover(int start, int batch_size, float min_sup, int min_len);
        
  private:
    Minisat::Solver* solver;
    
    std::string in_filename;
    std::string out_filename;
    
    /* Helpers for build_cover() */
    void write_mafia_input(int start, int batch_size);
    void call_mafia(float min_sup);
    Index build_base_index(int start, int batch_size);
    Cover read_mafia_output(Index& base_index);

};
#endif
