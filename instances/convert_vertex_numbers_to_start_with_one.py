#!/usr/bin/env python
# encoding: utf-8
import sys
import re


fn = sys.argv[1]
assert(fn.endswith('.orig'))
with open(fn, 'r') as r:
    with open(fn[:-5], 'w') as w:
        for line in r.readlines():
            line = line.strip()
            if line and line[0] == 'e':
                m = re.match(r'e (\d+) (\d+)', line)
                assert(m is not None)
                line = "e %d %d" % (int(m.group(1)) + 1, int(m.group(2)) + 1)
            elif line and line[0] == 'n':
                m = re.match(r'n (\d+) (.+)', line)
                assert(m is not None)
                line = "n %d %s" % (int(m.group(1)) + 1, m.group(2))
            print(line, file=w)
