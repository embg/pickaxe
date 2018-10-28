python3 -m venv .
. venv/bin/activate
pip install -r requirements.txt

for f in $(find . -name '*.cnf'); do
    maplesat f | python depth_experiment.py > depth_experiment.csv
done
