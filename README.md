MAtw
====

This repository contains my work on memetic algorithms for treewidth minimization, as submitted for the master's degree in Software Engineering & Internet Computing at Vienna University of Technology. The thesis itself, which includes information about the memetic algorithms, can be found in the `doc` directory.

If you're interested in the topic, or if any questions pop up, please don't hesitate to ask.


Building from Source
====================

Debug build:

    $ mkdir build && cd build && cmake .. && make

Optimized build:

    $ mkdir build_release && cd build_release && cmake .. -DCMAKE_BUILD_TYPE=Release && make


Running the Algorithms
======================

Usage information can be displayed with `--help`, e.g.,

    $ ./MA1_-1 --help


License
=======

Copyright (c) 2014 Kevin Bader.
Distributed under the GNU GPL v3 or later. For full terms see the file `LICENSE`.
