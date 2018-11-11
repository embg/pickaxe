export MROOT=/home/elliot/pickaxe/maple
cd maple/core
make
cd ../../
cp maple/core/maplesat solver/bin/maplesat
zip -r solver.zip solver
