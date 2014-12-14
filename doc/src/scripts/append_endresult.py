#!/usr/bin/env python
# encoding: utf-8
import sys

ENDTIME = 1000000

filename = sys.argv[1]
with open(filename, "r") as f:
    lines = f.readlines()
last = lines[-1]
pair = tuple([int(x) for x in last.split()])
if pair[0] != ENDTIME:
    with open(filename, "a") as f:
        print("%s %s" % (ENDTIME, pair[1]), file=f)
    sys.exit(0)
sys.exit(1)
