#!/bin/bash

if [[ $# -ne 1 || ! -d "$1" ]]; then
    echo -e "usage: $0 VARIANT_FOLDER\nwhere VARIANT_FOLDER is one of\n$(ls -1 | grep -P '^[A-Z][^-]*-[0-9]+/$' | sed -r 's/(.*)\/$/\1/')"
	exit 1
fi
variant="$1"
scenario="$variant/conf/scenario.txt"
echo "variant: $variant"

#date=$(date +'%F')
commit=$(git log --format="%H" -n 1)
#build_dir="$variant/build_${date}_${commit}"
build_dir="$variant/build_${commit}"
mkdir -p "$build_dir" || exit 2
cd "$build_dir"
cmake ../../.. -DCMAKE_BUILD_TYPE=Release || exit 3
make -j32 || exit 4
cd ../..
mkdir -p "$variant/workdir" || exit 5
echo "verifying scenario $scenario"
./util/verify-scenario --scenarios "$scenario"
read -p "exit code $? (<CR> to continue)" NOTHING
#./smac --scenario-file "$scenario" --rungroup "$variant" --seed 1 --abort-on-crash true --retryTargetAlgorithmRunCount 1 --validation-cores 10 --logAllProcessOutput true
./smac --scenario-file "$scenario" --rungroup "$variant" --seed 1 --abort-on-crash true --retryTargetAlgorithmRunCount 1 --validation false --logAllProcessOutput true
