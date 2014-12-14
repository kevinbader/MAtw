#!/bin/bash

base_dir=$(pwd)
build_dir=$base_dir/build
training_db_fn=$base_dir/validation_trainingset.sqlite.db
validation_db_fn=$base_dir/validation_validationset.sqlite.db
training_instances=$(cat ../instances/training.list | grep -P -v '^#')
training_instances_dir=$base_dir/../instances/training
validation_instances=$(cat ../instances/validation.list | grep -P -v '^#' | sed 's/$/.dgf/')
validation_instances_dir=$base_dir/../instances/validation
# run for one hour:
time_limit_s=3600
# run this many experiments for each configuration:
n_experiments=20

function setup_build() {
    mkdir -p $build_dir
    cd $build_dir
    cmake ../.. -DCMAKE_BUILD_TYPE=Release
    #make
    #cd $base_dir
}

# TODO ILS
# - lol: LS1: move in source (TabuSearch.cpp:805), swap in paper, swap in *MA
# - treewidth vs largest clique size: verwendet nysret wirklich die falsche größe?
#   tw = max clique size - 1; IHA verwendet max clique size. Fehler?
function setup_IHA() {
    exe=IHA
    outfile=$base_dir/${exe}.conf
    if [[ -e $outfile ]]; then
        echo "skipping $exe: won't overwrite $outfile"
        return
    fi
    cd $build_dir && make $exe
    for instance in $validation_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $validation_db_fn --seed $seed --time-limit $time_limit_s --instance $validation_instances_dir/$instance"
            workdir=$base_dir/workdirs/$exe/$instance/$seed
            echo "mkdir -p $workdir && cd $workdir && $build_dir/external/ILS/$cmd" >>$outfile
        done
    done
    for instance in $training_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $training_db_fn --seed $seed --time-limit $time_limit_s --instance $training_instances_dir/$instance"
            workdir=$base_dir/workdirs/$exe/$instance/$seed
            echo "mkdir -p $workdir && cd $workdir && $build_dir/external/ILS/$cmd" >>$outfile
        done
    done
    echo "$exe: run with $base_dir/../tools/executor/executor.py [-j CONCURRENCY] $outfile"
}

function setup_GAtw() {
    exe=GAtw
    outfile=$base_dir/${exe}.conf
    if [[ -e $outfile ]]; then
        echo "skipping $exe: won't overwrite $outfile"
        return
    fi
    cd $build_dir && make $exe
    for instance in $validation_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $validation_db_fn -x $seed --time-limit $time_limit_s -f $validation_instances_dir/$instance"
            workdir=$base_dir/workdirs/$exe/$instance/$seed
            echo "mkdir -p $workdir && cd $workdir && $build_dir/external/GA/$cmd" >>$outfile
        done
    done
    for instance in $training_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $training_db_fn -x $seed --time-limit $time_limit_s -f $training_instances_dir/$instance"
            workdir=$base_dir/workdirs/$exe/$instance/$seed
            echo "mkdir -p $workdir && cd $workdir && $build_dir/external/GA/$cmd" >>$outfile
        done
    done
    echo "$exe: run with $base_dir/../tools/executor/executor.py [-j CONCURRENCY] $outfile"
}

function setup_my_variant() {
    exe=$1
    params="$2"
    outfile=$base_dir/${exe}.conf
    if [[ -e $outfile ]]; then
        echo "skipping $exe: won't overwrite $outfile"
        return
    fi
    cd $build_dir && make $exe
    for instance in $validation_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $validation_db_fn --seed $seed $params $validation_instances_dir/$instance"
            echo "$build_dir/$cmd" >>$outfile
        done
    done
    for instance in $training_instances; do
        for seed in $(seq 1 $n_experiments); do
            cmd="$exe --db $training_db_fn --seed $seed $params $training_instances_dir/$instance"
            echo "$build_dir/$cmd" >>$outfile
        done
    done
    echo $exe:
    echo sample call: $(tail -n1 $outfile)
    echo run with $base_dir/../tools/executor/executor.py -j0 $outfile
}

echo "$(echo $training_instances | wc -w) training instances:"
echo "$(echo $training_instances | xargs)"
echo
echo "$(echo $validation_instances | wc -w) validation instances:"
echo "$(echo $validation_instances | xargs)"
echo
setup_build
setup_IHA
setup_GAtw
setup_my_variant SMMA_-1 "--duration $time_limit_s -I92 -l.7135 -N4" 
setup_my_variant WMA_-1 "--duration $time_limit_s -I15 -C.9162 -k3 -N82 -n94 -l.4020" 
setup_my_variant MMA_-1 "--duration $time_limit_s -I1941 -p.9171 -o.0072 -N193 -n84"
