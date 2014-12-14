#!/bin/bash

source ../scripts/venv/bin/activate

for i in $(cat /home/kevin/uni/d2/code/final/instances/validation.list | grep -v -P '^#'); do
    escaped=${i//_/-}
    ../scripts/generate_validation_table.py ~/uni/d2/code/final/validation/uranus.validation_validationset.sqlite.db $i 2>"validation-validationset-$escaped-results.txt"
done

../scripts/generate_trainingset_table.py ~/uni/d2/code/final/validation/uranus.validation_trainingset.sqlite.db
