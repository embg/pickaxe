# Build venv
virtualenv -p /usr/bin/python3 venv;
. venv/bin/activate;
pip3 install -r requirements.txt;

# Run experiment
echo "RAW_AVG, FRAC_AVG, RAW_STD, FRAC_STD" > depth_experiment.csv;
for f in $(find . -name '*.cnf'); do
    maplesat f | python depth_experiment.py >> depth_experiment.csv;
done

# Destroy venv
. deactivate;
rm -r venv;
