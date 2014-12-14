#!/bin/bash

virtualenv --system-site-packages venv
source venv/bin/activate
pip install cython git+https://github.com/statsmodels/statsmodels.git
for pkg in numpy scipy matplotlib pandas patsy statsmodels seaborn; do
    pip install $pkg
done
