import sys, re

# Read stdin to the "STATS:" line
stream = iter(sys.stdin)
for line in stream:
    if line.rstrip() == "STATS:":
        break
    else:
        print(line)

# Read the following data lines
# Format: "key: mean +/- std"
pattern = re.compile(r"(\w+): (\d+\.\d+) \+/- (\d+\.\d+)")
data = dict()
for line in stream:
    match = re.match(pattern, line)
    if match:
        key, mean, std = match.groups()
        data[key] = {'mean': mean, 'std': std}

# Write dict-of-dicts to stdout
print(data)
print(',')
