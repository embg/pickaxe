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
    
    int build_cover(Cover& cover, int start, int batch_size, float min_sup);
        
  private:
    Minisat::Solver* solver;
    
    std::string in_filename;
    std::string out_filename;
    
    /* Helpers for build_cover() */
    void build_base_cover(Cover& base_cover, int start, int batch_size);
    void write_mafia_input(int start, int batch_size);
    void call_mafia(float min_sup);
    void read_mafia_output(Cover& cover, Cover& base_cover);

};

#endif
