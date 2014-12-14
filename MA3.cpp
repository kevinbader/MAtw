/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <cmath>
#include <ctime>
#include <limits>
#include <random>
#include <cstdio>

#include <common.h>
#include <graph.h>
#include <population.h>
#include <crossover.h>
#include <localsearch.h>
#include <mutation.h>
#include <selection.h>
#include <recorder.h>
#include <dbwriter.h>

#include "config.h"

using namespace d2;


/*
 *const unsigned terminalCols = []{
 *    auto cols = 80u;
 *#ifdef TIOCGSIZE
 *    struct ttysize ts;
 *    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
 *    cols = ts.ts_cols;
 *#elif defined(TIOCGWINSZ)
 *    struct winsize ts;
 *    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
 *    cols = ts.ws_cols;
 *#endif
 *    return cols;
 *}();
 */


population_t select_next_generation(const population_t& current, Graph& graph,
                                    const Config_t config,
                                    std::default_random_engine& randgen) {
    const auto populationSize = config->populationSize;
    const auto k = config->tournamentSize;
    auto nextgen = population_t{};
    nextgen.reserve(populationSize);
    for (auto i = 0u; i < populationSize; ++i) {
        nextgen.push_back(*selection::k_tournament(current, k, graph, randgen));
    }
    return nextgen;
}

population_t recombine(const population_t& parents,
                       std::default_random_engine& randgen) {
    const auto populationSize = parents.size();
    auto children = population_t{};
    children.reserve(populationSize);

    for (auto i = 0u; i < populationSize; i += 2) {
        ordering_t child1, child2;
        crossover::POS(parents[i], parents[i+1], &child1, &child2, randgen);
        children.push_back(std::move(child1));
        children.push_back(std::move(child2));
    }

    assert(children.size() == populationSize);
    return children;
}

void mutate(population_t& population, const Config_t config,
            std::default_random_engine& randgen) {
    std::uniform_real_distribution<float> random01(0.f, 1.f);
    const float& mutationProbability = config->mutationProbability;
    for (auto individual: population) {
        if (random01(randgen) < mutationProbability) {
            mutation::ISM(&individual, randgen);
        }
    }
}

void improve_by_localsearch(population_t& population, Graph& graph, const Config_t config,
                            std::default_random_engine& randgen, const clock_t maxTick) {
    const auto populationSize = config->populationSize;
    const auto nLocalsearches = populationSize * config->localsearchFraction;
    auto randomIndividualDist
        = std::uniform_int_distribution<unsigned>(0u, populationSize - 1);
    auto randomIndividual = [&randgen, &randomIndividualDist]{
        return randomIndividualDist(randgen);
    };
    auto improvementMarkers = vector<bool>(populationSize, false);
    for (auto i = 0u; i < nLocalsearches; ++i) {
#ifndef NDEBUG
        cout << (i + 1) << std::flush;
#endif
        unsigned pos;
        do {
            pos = randomIndividual();
        }
        while (improvementMarkers[pos] == true);
        LS::ILS(&population[pos], graph, randgen,
                config->maxIlsOuterNonimprovements,
                config->maxIlsInnerNonimprovements,
                maxTick);
#ifndef NDEBUG
        for (auto j = std::ceil(std::log((float)(i+1) + 1.) / std::log(10.f)); j > 0; --j) {
            cout << "\b";
        }
#endif
    }
}

unsigned run_algorithm(const Config_t config, Graph& graph, Recorder& recorder,
                       float *const cputime_s = nullptr) {
    std::default_random_engine randgen{config->seed};
    std::uniform_real_distribution<float> random01(0.f, 1.f);

    auto population = generate_random_population(randgen,
                                                 config->populationSize,
                                                 graph.nVertices());
    ordering_t best;
    auto bestTreewidth = graph.nVertices();

    const auto firstClocktick = clock();
    recorder.add(firstClocktick, bestTreewidth, RecordType::IMPROVEMENT);

    /* find best individual */
    {
        const ordering_t * populationsBest;
        unsigned populationsBestTreewidth;
        find_best_individual(graph, population, &populationsBest, &populationsBestTreewidth);
        const auto currentTick = clock();
        assert (populationsBestTreewidth < bestTreewidth);
        best = *populationsBest;
        bestTreewidth = populationsBestTreewidth;
        recorder.add(currentTick, bestTreewidth, RecordType::IMPROVEMENT);
#ifndef NDEBUG
        cout << "(" << bestTreewidth << ") " << std::flush;
#endif
    }

    const auto lastClocktick = firstClocktick + CLOCKS_PER_SEC *
                         chrono::duration_cast<chrono::seconds>(config->runningTime).count();
    while (clock() < lastClocktick) {
#ifndef NDEBUG
        cout << "." << std::flush;
#endif
        population = select_next_generation(population, graph, config, randgen);
        population = recombine(population, randgen);
        mutate(population, config, randgen);
        improve_by_localsearch(population, graph, config, randgen, lastClocktick);

        const auto currentTick = clock();
        const ordering_t * elitist;
        unsigned elitistTreewidth;
        find_best_individual(graph, population, &elitist, &elitistTreewidth);
        if (elitistTreewidth < bestTreewidth) {
            recorder.add(currentTick, elitistTreewidth, RecordType::IMPROVEMENT);
            best = *elitist;
            bestTreewidth = elitistTreewidth;
#ifndef NDEBUG
            cout << " " << bestTreewidth << " " << std::flush;
#endif
        }
        else {
            if (elitistTreewidth != bestTreewidth) {
                recorder.add(currentTick, elitistTreewidth, RecordType::ITERATION);
            }
        }
    }
    const auto nClockticks = clock() - firstClocktick;
    if (cputime_s != nullptr) {
        *cputime_s = (float)nClockticks / CLOCKS_PER_SEC;
    }
#ifndef NDEBUG
    cout << endl << "finished after "
         << ((float)nClockticks / CLOCKS_PER_SEC) << " seconds (cputime)." << endl;
    cout << "found upper bound on treewidth: " << bestTreewidth << endl;
#endif
    return bestTreewidth;
}

int main(const int argc, const char* argv[]) {
    auto config = std::shared_ptr<const Config>();
    try {
        config = parse(argc, argv);
    }
    catch(const ConfigParseException& e) {
        cout << e.what() << endl;
        return 1;
    }

    auto graph = Graph::from_dimacs_file(config->instance_fn);
    if (config->doDotOutputOnly) {
        graph.print_dot(cout);
        return 0;
    }
    auto thevariant = variantname();
    unsigned treewidth;
    if (not config->nodb and db::has_previous_validationresult(config->dbfile, thevariant,
                                                               config->instance_fn,
                                                               config->seed, &treewidth)) {
#ifndef NDEBUG
        cout << "found previous result in " << config->dbfile << ": " << treewidth << endl;
#endif
        cerr << treewidth << endl;
        return 0;
    }
#ifndef NDEBUG
    cout << config->to_string();
#endif

    auto recorder = Recorder{};
    float cputime_s;
    treewidth = run_algorithm(config, graph, recorder, &cputime_s);

    if (not config->nodb) {
        db::write_validationresults(config->dbfile, thevariant,
                                    config->instance_fn,
                                    config->seed,
                                    cputime_s,
                                    *recorder.data(),
                                    config->to_compact_string());
    }
    cerr << treewidth << endl;
    return 0;
}
