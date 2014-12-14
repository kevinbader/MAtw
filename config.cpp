/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#include "config.h"
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <sstream>

namespace d2 {

namespace po = ::boost::program_options;

string variantname() {
    string name = variant();
    auto splitpos = name.find_first_of('_');
    if (splitpos != string::npos) {
        name = name.substr(0, splitpos);
    }
    return name;
}

#if IS_VARIANT(MA3)
string Config::to_string() const {
    std::stringstream ss;
    ss << "Configuration:" << endl
       << "  sqlite database                 : "
       << (dbfile.empty() ? "(not set; logging is disabled)" : dbfile.native()) << endl
       << "  seed                            : " << seed << endl
       << "  runtime                         : " <<
          chrono::duration_cast<chrono::seconds>(runningTime).count() << " seconds" << endl
       << "  instance                        : " << instance_fn.native() << endl
       << "  population size                 : " << populationSize << endl
       << "  mutation probability            : " << mutationProbability << endl
       << "  k-tournament size (selection)   : " << tournamentSize << endl
       << "  ILS outer max nonimproving steps: " << maxIlsOuterNonimprovements << endl
       << "  ILS inner max nonimproving steps: " << maxIlsInnerNonimprovements << endl
       << "  localsearch fraction            : " << localsearchFraction << endl
       ;
    return ss.str();
}
string Config::to_compact_string() const {
    std::stringstream ss;
    auto map = to_map();
    //ss << map["Instance"] << "_";
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        //if (it->first == "Instance")
            //continue;
        if (it != map.cbegin())
            ss << ",";
        ss << it->first << "=" << it->second;
    }
    return ss.str();
}
std::unordered_map<string, string> Config::to_map() const {
    return std::unordered_map<string, string>{
        {"Instance", instance_fn.stem().native()},
        {"Runtime",
         std::to_string(chrono::duration_cast<chrono::seconds>(runningTime).count())},
        {"Popsize", std::to_string(populationSize)},
        {"Crossover", "1.0"},  /* in case we'd like to introduce it again */
        {"Mutation", std::to_string(mutationProbability)},
        {"TournamentSize", std::to_string(tournamentSize)},
        {"IlsOuterNonimpr", std::to_string(maxIlsOuterNonimprovements)},
        {"IlsInnerNonimpr", std::to_string(maxIlsInnerNonimprovements)},
        {"LsFraction", std::to_string(localsearchFraction)},
        {"Seed", std::to_string(seed)}
    };
}
bool Config::operator== (const Config& other) const {
    return runningTime == other.runningTime
        and populationSize == other.populationSize
        and mutationProbability == other.mutationProbability
        and tournamentSize == other.tournamentSize
        and maxIlsOuterNonimprovements == other.maxIlsOuterNonimprovements
        and maxIlsInnerNonimprovements == other.maxIlsInnerNonimprovements
        and localsearchFraction == other.localsearchFraction
        and seed == other.seed;
}
#elif IS_VARIANT(MA2)
string Config::to_string() const {
    std::stringstream ss;
    ss << "Configuration:" << endl
       << "  sqlite database                 : "
       << (dbfile.empty() ? "(not set; logging is disabled)" : dbfile.native()) << endl
       << "  seed                            : " << seed << endl
       << "  runtime                         : " <<
          chrono::duration_cast<chrono::seconds>(runningTime).count() << " seconds" << endl
       << "  instance                        : " << instance_fn.native() << endl
       << "  population size                 : " << populationSize << endl
       << "  crossover probability           : " << crossoverProbability << endl
       << "  k-tournament size (selection)   : " << tournamentSize << endl
       << "  ILS outer max nonimproving steps: " << ilsMaxOuterNonimprovements << endl
       << "  ILS inner max nonimproving steps: " << ilsMaxInnerNonimprovements << endl
       << "  localsearch fraction            : " << localsearchFraction << endl
       ;
    return ss.str();
}
string Config::to_compact_string() const {
    std::stringstream ss;
    auto map = to_map();
    //ss << map["Instance"] << "_";
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        //if (it->first == "Instance")
            //continue;
        if (it != map.cbegin())
            ss << ",";
        ss << it->first << "=" << it->second;
    }
    return ss.str();
}
std::unordered_map<string, string> Config::to_map() const {
    return std::unordered_map<string, string>{
        {"Instance", instance_fn.stem().native()},
        {"Runtime",
         std::to_string(chrono::duration_cast<chrono::seconds>(runningTime).count())},
        {"Popsize", std::to_string(populationSize)},
        {"Crossover", std::to_string(crossoverProbability)},
        {"TournamentSize", std::to_string(tournamentSize)},
        {"IlsOuterNonimpr", std::to_string(ilsMaxOuterNonimprovements)},
        {"IlsInnerNonimpr", std::to_string(ilsMaxInnerNonimprovements)},
        {"LsFraction", std::to_string(localsearchFraction)},
        {"Seed", std::to_string(seed)}
    };
}
bool Config::operator== (const Config& other) const {
    return runningTime == other.runningTime
        and populationSize == other.populationSize
        and crossoverProbability == other.crossoverProbability
        and tournamentSize == other.tournamentSize
        and ilsMaxOuterNonimprovements == other.ilsMaxOuterNonimprovements
        and ilsMaxInnerNonimprovements == other.ilsMaxInnerNonimprovements
        and localsearchFraction == other.localsearchFraction
        and seed == other.seed;
}
#elif IS_VARIANT(MA1)
string Config::to_string() const {
    std::stringstream ss;
    ss << "Configuration:" << endl
       << "  sqlite database                 : "
       << (dbfile.empty() ? "(not set; logging is disabled)" : dbfile.native()) << endl
       << "  seed                            : " << seed << endl
       << "  runtime                         : " <<
          chrono::duration_cast<chrono::seconds>(runningTime).count() << " seconds" << endl
       << "  instance                        : " << instance_fn.native() << endl
       << "  population size                 : " << populationSize << endl
       << "  partner fraction                : " << partnerFraction << endl
       << "  opponent fraction               : " << opponentFraction << endl
       << "  ILS outer max nonimproving steps: " << ilsMaxOuterNonimprovements << endl
       << "  ILS inner max nonimproving steps: " << ilsMaxInnerNonimprovements << endl
       ;
    return ss.str();
}
string Config::to_compact_string() const {
    std::stringstream ss;
    auto map = to_map();
    //ss << map["Instance"] << "_";
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        //if (it->first == "Instance")
            //continue;
        if (it != map.cbegin())
            ss << ",";
        ss << it->first << "=" << it->second;
    }
    return ss.str();
}
std::unordered_map<string, string> Config::to_map() const {
    return std::unordered_map<string, string>{
        {"Instance", instance_fn.stem().native()},
        {"Runtime",
         std::to_string(chrono::duration_cast<chrono::seconds>(runningTime).count())},
        {"Popsize", std::to_string(populationSize)},
        {"PartnerFraction", std::to_string(partnerFraction)},
        {"OpponentFraction", std::to_string(opponentFraction)},
        {"IlsOuterNonimpr", std::to_string(ilsMaxOuterNonimprovements)},
        {"IlsInnerNonimpr", std::to_string(ilsMaxInnerNonimprovements)},
        {"Seed", std::to_string(seed)}
    };
}
bool Config::operator== (const Config& other) const {
    return runningTime == other.runningTime
        and populationSize == other.populationSize
        and partnerFraction == other.partnerFraction
        and opponentFraction == other.opponentFraction
        and ilsMaxOuterNonimprovements == other.ilsMaxOuterNonimprovements
        and ilsMaxInnerNonimprovements == other.ilsMaxInnerNonimprovements
        and seed == other.seed;
}
#endif  // VARIANT


Config_t parse(const int argc, const char* argv[]) {
    try {
        auto c = std::make_shared<Config>();

        const auto dbfileDefault = []{
            std::stringstream ss;
            ss << variant() << "-" << std::to_string(RELEASE) << ".sqlite";
            return fs::path{ss.str()};
        }();

        po::options_description generalOptions {"General Options"};
        generalOptions.add_options()
            ("help,h", "show usage")
            ("db",
             po::value<fs::path>(&c->dbfile)->default_value(dbfileDefault),
             "Path to an sqlite database. If the file does not exist, it will be created.")
            ("nodb", po::bool_switch(&c->nodb), "Disable using an sqlite database.")
            ("seed,s",
             po::value<unsigned>(&c->seed)
             ->default_value(1u),
             "Seed used to initialize the program-wide random-numbers generator.")
            ("duration,d",
             po::value<unsigned>()
             ->default_value(1000)
             ->notifier([&c](unsigned num){ c->runningTime = chrono::seconds{num}; }),
             "Time the algorithm should run (cputime) in seconds (>0).")
            ("populationsize,I",
             po::value<unsigned>()
             ->required()
             ->notifier([&c](unsigned num){
                 if (num == 0)
                     throw _make_out_of_range_error("populationsize", "> 0", num);
#if IS_VARIANT(MA3)
                 if (num % 2 != 0)
                     throw _make_out_of_range_error("populationsize", "an even number", num);
#endif
#if IS_VARIANT(MA3) or IS_VARIANT(MA2)
                 if (num < c->tournamentSize)
                     throw _make_out_of_range_error("populationSize", "> k-tournament size",
                                                    num);
#endif
                 c->populationSize = num;
                 }),
#if IS_VARIANT(MA3)
             "Size of the population. Expected to be even, and >= k-tournament size."
#elif IS_VARIANT(MA2)
             "Size of the population. Expected to be >= k-tournament size."
#elif IS_VARIANT(MA1)
             "Size of the population. Expected to be >0."
#endif
             )
            ;

        po::options_description hiddenOptions;
        hiddenOptions.add_options()
            ("instance",
             po::value<fs::path>(&c->instance_fn)
             ->required()
             ->notifier([&c](const fs::path& path){
                if (not fs::exists(path))
                    throw std::logic_error("given instance file does not exist");
                c->instance_fn = path;
                }),
             "Path to a DIMACS file");

        po::options_description variantOptions {"Variant Options"};
        variantOptions.add_options()
#if IS_VARIANT(MA3)
            /*
             *("crossoverFraction,C",
             * po::value<float>()
             * ->required()
             * ->notifier([&c](float num){
             *     if (num < 0.f or 1.f < num)
             *         throw _make_out_of_range_error("crossoverFraction",
             *                                       "real between 0 and 1", num);
             *     c->crossoverFraction = num;
             *     }),
             * "Fraction of the population for which crossover is used to build"
             * " the following generation.")
             */
            /*
             *("mutationProbability,m",
             * po::value<float>()
             * ->required()
             * ->notifier([&c](float num){
             *     if (num < 0.f or 1.f < num)
             *         throw _make_out_of_range_error("mutationProbability",
             *                                       "real between 0 and 1", num);
             *     c->mutationProbability = num;
             *     }),
             * "Fraction of the population on which mutation is applied.")
             */
            /*
             *("tournamentSize,k",
             * po::value<unsigned>(&c->tournamentSize)->required(),
             * "Size (k) of the k-Tournament selection operator.")
             */
            ("maxIlsOuterNonimprovements,N",
             po::value<unsigned>(&c->maxIlsOuterNonimprovements)->required(),
             "Number of nonimproving steps the ILS procedure encounters before giving up.")
            /*
             *("maxIlsInnerNonimprovements,n",
             * po::value<unsigned>(&c->maxIlsInnerNonimprovements)->required(),
             * "Number of nonimproving steps the LS procedure, which is called by the ILS"
             * " procedure, encounters before giving up.")
             */
            ("localsearchFraction,l",
             po::value<float>()
             ->required()
             ->notifier([&c](float num){
                 if (num < 0.f or 1.f < num)
                     throw _make_out_of_range_error("localsearchFraction",
                                                   "real between 0 and 1", num);
                 c->localsearchFraction = num;
                 }),
             "Fraction of the population that is at each iteration randomly selected for"
             " localsearch.")
            ;
#elif IS_VARIANT(MA2)
            ("crossoverProbability,C",
             po::value<float>()
             ->required()
             ->notifier([&c](float num){
                 if (num < 0.f or 1.f < num)
                     throw _make_out_of_range_error("crossoverProbability",
                                                    "real between 0 and 1", num);
                 c->crossoverProbability = num;
                 }),
             "In each generation, probability for an individual to be created by crossover"
             " rather than mutation.")
            ("tournamentSize,k",
             po::value<unsigned>(&c->tournamentSize)->required(),
             "Size (k) of the k-Tournament selection operator.")
            ("ilsMaxOuterNonimprovements,N",
             po::value<unsigned>(&c->ilsMaxOuterNonimprovements)->required(),
             "Number of nonimproving steps the ILS procedure encounters before giving up.")
            ("ilsMaxInnerNonimprovements,n",
             po::value<unsigned>(&c->ilsMaxInnerNonimprovements)->required(),
             "Number of nonimproving steps the LS procedure, which is called by the ILS"
             " procedure, encounters before giving up.")
            ("localsearchFraction,l",
             po::value<float>()
             ->required()
             ->notifier([&c](float num){
                 if (num < 0.f or 1.f < num)
                     throw _make_out_of_range_error("localsearchFraction",
                                                    "real between 0 and 1", num);
                 c->localsearchFraction = num;
                 }),
             "Fraction of the population that is at each iteration selected for localsearch;"
             " the selection is done by fitness values, descending.")
            ;
#elif IS_VARIANT(MA1)
            ("partnerFraction,p",
             po::value<float>()
             ->required()
             ->notifier([&c](float num){
                 if (num < 0.f or 1.f < num)
                     throw _make_out_of_range_error("partnerFraction",
                                                    "real between 0 and 1", num);
                 c->partnerFraction = num;
                 }),
             "The number of partners that each individual has, expressed as a fraction"
             " of the population size.")
            ("opponentFraction,o",
             po::value<float>()
             ->required()
             ->notifier([&c](float num){
                 if (num < 0.f or 1.f < num)
                     throw _make_out_of_range_error("opponentFraction",
                                                    "real between 0 and 1", num);
                 c->opponentFraction = num;
                 }),
             "The number of opponents that each individual has, expressed as a fraction"
             " of the population size.")
            ("ilsMaxOuterNonimprovements,N",
             po::value<unsigned>(&c->ilsMaxOuterNonimprovements)->required(),
             "Number of nonimproving steps the ILS procedure encounters before giving up.")
            ("ilsMaxInnerNonimprovements,n",
             po::value<unsigned>(&c->ilsMaxInnerNonimprovements)->required(),
             "Number of nonimproving steps the LS procedure, which is called by the ILS"
             " procedure, encounters before giving up.")
            ;
#endif  // VARIANT

        po::options_description miscOptions{"Miscellaneous"};
        miscOptions.add_options()
            ("dot",
             po::bool_switch(&c->doDotOutputOnly),
             //->default_value(false)->implicit_value(true),
             "Output the graph in dot format and exit.");

        po::options_description cliOptions;
        cliOptions
            .add( generalOptions )
            .add( variantOptions )
            .add( miscOptions )
            ;

        po::positional_options_description positionalOptions;
        positionalOptions.add( "instance", 1 );

        po::options_description allOptions;
        allOptions.add(cliOptions).add(hiddenOptions);

        po::variables_map vm;
        po::store( po::command_line_parser( argc, argv )
                    .options( allOptions )
                    .positional( positionalOptions )
                    .run(), vm );

        if (vm.count("help") > 0 or vm.count("instance") == 0) {
            std::stringstream ss;
            ss << "usage: " << argv[0] << " [options] DIMACS-FILE" << endl;
            cliOptions.print(ss);
            throw std::logic_error(ss.str());
        }
        po::notify( vm );
        return c;
    }
    catch(const std::logic_error& e) {
        throw ConfigParseException(e.what());
    }
}



}  // ns d2
