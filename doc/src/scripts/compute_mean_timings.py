#!/usr/bin/env python
# encoding: utf-8
import sys
import os.path


stepsize = 100  # ms
maxsize = 1000000  # ms

def read_data(filenames):
    bins = [[] for i in range(int(maxsize / stepsize))]
    n_samples = len(filenames)
    for fn in filenames:
        with open(fn, "r") as f:
            lines = f.readlines()
        pairs = []
        for line in lines:
            if line[0].isdigit():
                pairs.append(tuple([int(x) for x in line.split()]))
        pairs_index = 0
        bin_no = 0
        last_treewidth = None
        while bin_no < len(bins):
            time = (bin_no + 1) * stepsize
            if pairs_index < len(pairs) and pairs[pairs_index][0] < time:
                while pairs_index < len(pairs) and pairs[pairs_index][0] < time:
                    treewidth = pairs[pairs_index][1]
                    #print("step %s at %s ms - %s is before with tw %s" % (bin_no, time, pairs[pairs_index][0], treewidth))
                    bins[bin_no].append(treewidth)
                    last_treewidth = treewidth
                    pairs_index += 1
            else:
                bins[bin_no].append(last_treewidth)
            bin_no += 1
        for bin in bins:
            assert bin
    return bins

def make_avg(bins):
    for i in range(len(bins)):
        nSamples = len(bins[i])
        bins[i] = sum(bins[i]) / nSamples


if len(sys.argv) < 2:
    print("usage: %s FILE.." % sys.argv[0], file=sys.stderr)
    print("expects paths to result files that belong to a single instance and outputs an average", file=sys.stderr)
    sys.exit(1)
filenames = sys.argv[1:]
bins = read_data(filenames)
make_avg(bins)
for i in range(len(bins)):
    print(("%%%ss %%s" % len(str(maxsize))) % (i * stepsize + int(stepsize / 2), bins[i]))
