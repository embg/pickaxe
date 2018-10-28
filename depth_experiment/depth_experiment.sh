for f in $(find . -name '*.cnf'); do
    maplesat f | python tabulator.py > depth_experiment.csv
done
