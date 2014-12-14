/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include <cmath>
#include <ctime>
#include <limits>
#include <random>
#include <cstdio>
#include <queue>
#include <boost/optional.hpp>

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


population_t& competitions(std::default_random_engine& randgen, population_t& generation,
                           const vector<unsigned>& treewidths,
                           unsigned nOpponents/*, double temperature*/) {
    const auto populationSize = generation.size();
    assert(nOpponents < populationSize);
    auto random01 = std::uniform_real_distribution<float>{0, 1};
    auto challenging = vector<boost::optional<unsigned>>(populationSize, boost::none);
    auto isChallenged = vector<bool>(populationSize);
    auto choose_opponent = [&](unsigned forIndex) -> boost::optional<unsigned>{
        /**
         * The opponents are around the individual. If nOpponents is even,
         * exactly half of them are on either side; otherwise, floor(nOpponents)
         * come before and ceil(nOpponents) come after.
         */
        unsigned startIndex;
        if (nOpponents % 2 == 0) {
            if (forIndex >= nOpponents / 2) {
                startIndex = forIndex - nOpponents / 2;
            }
            else {
                startIndex = populationSize - (nOpponents / 2 - forIndex);
            }
        }
        else {
            if (forIndex >= nOpponents / 2) {
                startIndex = forIndex - nOpponents / 2;
            }
            else {
                startIndex = populationSize - (nOpponents / 2 - forIndex);
            }
        }
        /* first check that there is any opponent that can be challenged: */
        auto isChallengePossible = false;
        const auto forIndexTmp = startIndex < forIndex
                                 ? forIndex : forIndex + populationSize;
        for (auto i = 0u; i < nOpponents; ++i) {
            unsigned opponentIndex;
            if (startIndex + i < forIndexTmp) {
                opponentIndex = (startIndex + i) % populationSize;
            }
            else {
                opponentIndex = (startIndex + i + 1) % populationSize;
            }
            if (not isChallenged[opponentIndex]
                and challenging[opponentIndex] != forIndex)
            {
                isChallengePossible = true;
                break;
            }
        }
        if (not isChallengePossible) {
            return boost::none;
        }
        /* now find one to challenge */
        auto randomOpponent
            = std::uniform_int_distribution<unsigned>(0u, nOpponents - 1);
        unsigned opponentIndex;
        do {
            auto offset = randomOpponent(randgen);
            if (startIndex + offset < forIndexTmp) {
                opponentIndex = (startIndex + offset) % populationSize;
            }
            else {
                opponentIndex = (startIndex + offset + 1) % populationSize;
            }
        }
        while (isChallenged[opponentIndex] or challenging[opponentIndex] == forIndex);
        return opponentIndex;
    };
    auto resolve_challenge = [&randgen, &random01, &treewidths]
                             (unsigned challengerIndex, unsigned challengedIndex){
        // TODO might use a temperature here; deterministic for now
        if (treewidths[challengerIndex] < treewidths[challengedIndex]) {
            return true;
        }
        else {
            return false;
        }
    };

    /* find out who's challenging whom */
    for (auto i = 0u; i < populationSize; ++i) {
        boost::optional<unsigned> opponentIndex = choose_opponent(i);
        if (opponentIndex) {
            challenging[i] = *opponentIndex;
            isChallenged[*opponentIndex] = true;
        }
    }

    /* resolve challenges */
    auto replacements = std::vector<std::pair<unsigned, ordering_t>>();
    for (auto i = 0u; i < populationSize; ++i) {
        if (challenging[i]) {
            const auto& challengerIndex = i;
            const auto& challengedIndex = *challenging[i];
            bool isSuccessful = resolve_challenge(challengerIndex, challengedIndex);
            if (isSuccessful) {
                replacements.push_back(std::make_pair(challengedIndex,
                                                      generation[challengerIndex]));
            }
        }
    }

    /* apply changes */
    for (const auto& pair: replacements) {
        generation[pair.first] = std::move(pair.second);
    }

    return generation;
}

population_t& cooperations(std::default_random_engine& randgen, population_t& population,
                           const vector<unsigned>& treewidths,
                           unsigned nPartners/*, double temperature*/) {
    const auto nPopulation = population.size();
    assert(nPartners < nPopulation);
    auto random01 = std::uniform_real_distribution<float>{0, 1};
    auto proposingTo = vector<boost::optional<unsigned>>(nPopulation, boost::none);
    auto isBeingProposed = vector<bool>(nPopulation);
    auto choose_partner = [&](unsigned forIndex) -> boost::optional<unsigned>{
        /**
         * The partners are around the individual that's opposite of the actual
         * individual.
         */
        const auto flooredMidNomod = forIndex + nPopulation / 2;
        unsigned startIndex;
        if (nPopulation % 2 == 0) {
            startIndex = (flooredMidNomod - (nPartners - 1) / 2) % nPopulation;
        }
        else {
            startIndex = (flooredMidNomod - (nPartners - 2) / 2) % nPopulation;
        }
        /* first check that there is any opponent that can be challenged: */
        auto isProposalPossible = false;
        for (auto i = 0u; i < nPartners; ++i) {
            auto partnerIndex = (startIndex + i) % nPopulation;
            if (not isBeingProposed[partnerIndex]
                and proposingTo[partnerIndex] != forIndex)
            {
                isProposalPossible = true;
                break;
            }
        }
        if (not isProposalPossible) {
            return boost::none;
        }
        /* now find one to propose to */
        auto randomPartner
            = std::uniform_int_distribution<unsigned>(0u, nPartners - 1);
        unsigned partnerIndex;
        do {
            partnerIndex = (startIndex + randomPartner(randgen)) % nPopulation;
        }
        while (isBeingProposed[partnerIndex] or proposingTo[partnerIndex] == forIndex);
        return partnerIndex;
    };
    auto resolve_proposal = [&randgen, &random01, &population, &treewidths]
                            (unsigned sourceIndex, unsigned targetIndex)
                            -> boost::optional<ordering_t> {
        // TODO might use a temperature here; deterministic and always, for now
        ordering_t child;
        crossover::POS_single(population[sourceIndex], population[targetIndex],
                              &child, randgen);
        return std::move(child);
    };

    /* find out who's proposing to whom */
    for (auto i = 0u; i < nPopulation; ++i) {
        boost::optional<unsigned> partnerIndex = choose_partner(i);
        if (partnerIndex) {
            proposingTo[i] = *partnerIndex;
            isBeingProposed[*partnerIndex] = true;
        }
    }

    /* resolve challenges */
    auto replacements = std::vector<std::pair<unsigned, ordering_t>>();
    for (auto i = 0u; i < nPopulation; ++i) {
        if (proposingTo[i]) {
            const auto& sourceIndex = i;
            const auto& targetIndex = *proposingTo[i];
            boost::optional<ordering_t> child = resolve_proposal(sourceIndex, targetIndex);
            if (child) {
                replacements.push_back(std::make_pair(targetIndex, std::move(*child)));
            }
        }
    }

    /* apply changes */
    for (const auto& pair: replacements) {
        population[pair.first] = std::move(pair.second);
    }

    return population;
}

unsigned run_algorithm(const Config_t config, Graph& graph, Recorder& recorder,
                       float *const cputime_s = nullptr) {
    const auto populationSize = config->populationSize;
    const auto nPartners = (unsigned)floor((populationSize - 1) * config->partnerFraction);
    const auto nOpponents = (unsigned)floor((populationSize - 1) * config->opponentFraction);
    const auto ilsMaxOuterNonimprovements = config->ilsMaxOuterNonimprovements;
    const auto ilsMaxInnerNonimprovements = config->ilsMaxInnerNonimprovements;
    std::default_random_engine randgen{config->seed};
    std::uniform_real_distribution<float> random01(0.f, 1.f);

    auto population = generate_random_population(randgen,
                                                 config->populationSize,
                                                 graph.nVertices());
    ordering_t best;
    auto bestTreewidth = graph.nVertices();

    auto firstClocktick = clock();
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

    auto record_best = [&population, &best, &bestTreewidth, &recorder]
                       (unsigned elitistIndex, unsigned elitistTreewidth) {
        const auto currentTick = clock();
        if (elitistTreewidth < bestTreewidth) {
            recorder.add(currentTick, elitistTreewidth, RecordType::IMPROVEMENT);
            best = population[elitistIndex];
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
    };

    const std::chrono::steady_clock::duration runningTime = config->runningTime;
    /*
     *const double normalizedRunningTimeSecs
     *    = (double)std::chrono::duration_cast<std::chrono::seconds>( runningTime ).count()
     *      / 1000.;
     *auto temperature = [&firstClocktick, &runningTime, &normalizedRunningTimeSecs] {
     *    double realElapsedSecs = (clock() - firstClocktick) / CLOCKS_PER_SEC;
     *    double virtElapsedSecs = realElapsedSecs / normalizedRunningTimeSecs;
     *    return 1000. * exp( -virtElapsedSecs / 130. );  // 130 is empirically guessed magic constant
     *};
     */

    auto treewidths = vector<unsigned>(populationSize);
    const auto lastClocktick = firstClocktick
        + CLOCKS_PER_SEC * chrono::duration_cast<chrono::seconds>(runningTime).count();
    auto apply_localsearch_to_population = [&](unsigned *const elitistIndex,
                                               unsigned *const elitistTreewidth) {
        for (auto i = 0u; i < populationSize; ++i) {
            auto treewidth = LS::ILS(&population[i], graph, randgen,
                                     ilsMaxOuterNonimprovements,
                                     ilsMaxInnerNonimprovements, lastClocktick);
            treewidths[i] = treewidth;
            if (i == 0) {
                *elitistIndex = 0;
                *elitistTreewidth = treewidth;
            }
            else if (treewidth < *elitistTreewidth) {
                *elitistIndex = i;
                *elitistTreewidth = treewidth;
            }
        }
    };
    while (clock() < lastClocktick) {
#ifndef NDEBUG
        cout << "\u1690" << std::flush;
#endif
        unsigned elitistIndex = 0;                   // to satisfy compiler only
        unsigned elitistTreewidth = populationSize;  // to satisfy compiler only
        apply_localsearch_to_population(&elitistIndex, &elitistTreewidth);
        record_best(elitistIndex, elitistTreewidth);

#ifndef NDEBUG
        cout << "\u1691" << std::flush;
#endif
        population = competitions(randgen, population, treewidths, nOpponents);

#ifndef NDEBUG
        cout << "\u1692" << std::flush;
#endif
        apply_localsearch_to_population(&elitistIndex, &elitistTreewidth);
        record_best(elitistIndex, elitistTreewidth);

#ifndef NDEBUG
        cout << "\u1693" << std::flush;
#endif
        population = cooperations(randgen, population, treewidths, nPartners);
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

