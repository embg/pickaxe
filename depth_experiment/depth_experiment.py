import sys, npstreams
import numpy as np

cleaned = (line.rstrip().split() for line in sys.stdin)
filtered = (map(int, line[1:]) for line in cleaned if line[0] == 'NFNT')
stream = ((NF, NF / NT) for NF, NT in filtered)
# Correct up to here

stream_for_avgs, stream_for_stds = npstreams.itercopy(stream)
iavgs = npstreams.imean(stream_for_avgs, axis=1)
istds = npstreams.istd(stream_for_stds, axis=1)

avgs, stds = npstreams.last(zip(iavgs, istds))
print(*list(avgs), *list(stds))
# Incorrect



