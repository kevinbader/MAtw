#!/usr/bin/env python
# encoding: utf-8
# 
# Copyright 2014 Kevin Bader.
# Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
# 

import sqlite3
import argparse
import math


def minimal_treewidth_per_instance_and_configuration(conn, configCols, runtime):
    """minimal treewidth per instance and configuration"""
    c = conn.cursor()
    c.execute("""
        SELECT Instance, %(config)s, MIN(treewidth)
        FROM runs
        WHERE Runtime = '%(runtime)s'
        GROUP BY Instance, %(config)s
        ORDER BY Instance ASC;"""
              % {"config": ', '.join(configCols), "runtime": runtime})
    return tuple([x[0] for x in c.description]), c.fetchall()


def best_config_per_instance(conn, configCols, runtime):
    """best configuration (lowest average treewidth) per instance"""
    c = conn.cursor()
    c.execute("""
        SELECT Instance, %(config)s, min_tw, avg_tw
        FROM (
            SELECT Instance, %(config)s,
                   MIN(treewidth) as min_tw, AVG(treewidth) AS avg_tw
            FROM runs
            WHERE Runtime = '%(runtime)s'
            GROUP BY Instance, %(config)s
             )
        GROUP BY Instance
        HAVING avg_tw = MIN(avg_tw);"""
              % {"config": ', '.join(configCols), "runtime": runtime})
    return tuple([x[0] for x in c.description]), c.fetchall()


def average_performances(conn, configCols, runtime):
    """average performance over all instances, sorted by score"""
    c1 = conn.cursor()
    c2 = conn.cursor()
    averages = {}
    nInstances = 0
    # for each instance...
    isFirst = True
    for instance in c1.execute("SELECT DISTINCT(Instance) FROM runs;"):
        nInstances += 1
        instance = instance[0]
        # ...select configs that have at least ten results for each instance
        for row in c2.execute("""
            SELECT %(config)s, avg(treewidth)
            FROM runs
            WHERE Instance = '%(instance)s' and Runtime = '%(runtime)s'
            GROUP BY %(config)s
            HAVING COUNT(Popsize) >= 10;"""
                              % {"config": ', '.join(configCols), "runtime": runtime,
                                 "instance": instance}):
            config = tuple(row)[:-1]
            if isFirst:
                averages[config] = []
            if config in averages:
                averages[config].append(tuple(row)[-1])
        isFirst = False
    # filter configs which have too few results:
    for config in list(averages):
        if len(averages[config]) != nInstances:
            del averages[config]
    # compute averages as the average of the instance averages:
    for config in list(averages):
        avgs = averages[config]
        avg = sum(avgs) / len(avgs)
        averages[config] = avg
    resultRows = []
    for config in sorted(averages, key=averages.get):
        resultRows.append(list(config) + [averages[config]])
    return tuple([x[0] for x in c2.description[:-1]] + ['score']), resultRows


def best_configuration_all_instances(conn, configCols, runtime):
    """the best configuration applied to all instances"""
    c = conn.cursor()
    otherHeader, bestConfig = average_performances(conn, configCols, runtime)
    if len(bestConfig) == 0:
        return [''], [['there is no best configuration yet']]
    bestConfig = bestConfig[0]
    assert otherHeader == tuple(configCols + ['score']),\
        'check indices below to reflect what is in average_performances;'\
        ' expected: "%s", got "%s"' % (tuple(configCols + ['score']), otherHeader)
    assert str(runtime) == str(bestConfig[0])
    result = []
    whereClause = []
    for index in range(len(configCols)):
        whereClause.append("%s = '%s'" % (configCols[index], bestConfig[index]))
    whereClause = ' AND '.join(whereClause)
    c.execute("""
SELECT Instance, %(config)s, min(treewidth) as min_treewidth,
       avg(treewidth) as avg_treewidth, count(treewidth) as 'nSamples',
       sum(treewidth) as sum_treewidths, sum(treewidth * treewidth) as sum_squared_treewidths
FROM runs
WHERE %(whereClause)s
GROUP BY Instance, Crossover, Popsize, IlsOuterNonimpr, TournamentSize, Mutation,
         IlsInnerNonimpr, LsFraction;"""
              % {"config": ', '.join(configCols), "whereClause": whereClause})

    while True:
        row = c.fetchone()
        if row is None:
            break
        result.append(list(tuple(row)[:-3]))
        # s = \sqrt{\frac{1}{n-1} \sum_{i=1}^{n}(X_i - \bar{X})^2} =
        #   = \sqrt{\frac{1}{n-1} (\sum_{i=1}^{n}X_i^2 - 2\bar{X}\sum_{i=1}^{n}X_i + \bar{X}*n)}
        sum_Xi = float(row['sum_treewidths'])
        sum_Xi2 = float(row['sum_squared_treewidths'])
        avg = float(row['avg_treewidth'])
        n = float(row['nSamples'])
        if n == 1:
            sampleStdDev = 0
        else:
            sampleVariance = (sum_Xi2 - 2 * avg * sum_Xi + avg ** 2 * n) / (n - 1)
            sampleStdDev = math.sqrt(sampleVariance)
        result[-1].append(sampleStdDev)
    header = tuple([x[0] for x in c.description[:-3]] + ['StdDev'])
    return header, result


def all_configurations_all_instances(conn, configCols, runtime):
    """all configurations applied to all instances"""
    c = conn.cursor()
    result = []
    c.execute("""
SELECT Instance, %(config)s, min(treewidth) as min_treewidth,
       avg(treewidth) as avg_treewidth, count(treewidth) as 'nSamples',
       sum(treewidth) as sum_treewidths, sum(treewidth * treewidth) as sum_squared_treewidths
FROM runs
WHERE Runtime = '%(runtime)s'
GROUP BY Instance, %(config)s;""" % {"config": ', '.join(configCols),
                                     "runtime": runtime})

    while True:
        row = c.fetchone()
        if row is None:
            break
        result.append(list(tuple(row)[:-3]))
        # s = \sqrt{\frac{1}{n-1} \sum_{i=1}^{n}(X_i - \bar{X})^2} =
        #   = \sqrt{\frac{1}{n-1} (\sum_{i=1}^{n}X_i^2 - 2\bar{X}\sum_{i=1}^{n}X_i + \bar{X}*n)}
        sum_Xi = float(row['sum_treewidths'])
        sum_Xi2 = float(row['sum_squared_treewidths'])
        avg = float(row['avg_treewidth'])
        n = float(row['nSamples'])
        if n == 1:
            sampleStdDev = 0
        else:
            sampleVariance = (sum_Xi2 - 2 * avg * sum_Xi + avg ** 2 * n) / (n - 1)
            sampleStdDev = math.sqrt(sampleVariance)
        result[-1].append(sampleStdDev)
    header = tuple([x[0] for x in c.description[:-3]] + ['StdDev'])
    return header, result


def get_identifying_columnames(conn):
    c = conn.cursor()
    c.execute("SELECT * FROM runs LIMIT 0")
    cols = [x[0] for x in c.description if x[0] not in ['id',
                                                        'Instance',
                                                        'Seed',
                                                        'treewidth']]
    return cols


def print_table(header, listoflists, title=None):
    if title is not None:
        print(title)
        print('^' * len(title))

    lengths = [len(c) for c in header]
    for c in range(len(header)):
        for r in range(len(listoflists)):
            if len(str(listoflists[r][c])) > lengths[c]:
                lengths[c] = len(str(listoflists[r][c]))
    sep = ' | '

    cols = []
    for i in range(len(header)):
        cols.append('%%-%ss' % lengths[i] % header[i])
    print(sep.join(cols))

    for list in listoflists:
        cols = []
        for i in range(len(header)):
            cols.append('%%-%ss' % lengths[i] % list[i])
        print(sep.join(cols))
    print()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('dbfile', metavar='file', help='path to sqlite db')
    parser.add_argument('--runtime', metavar='seconds', type=int, default=1000,
                        help='only records with this runtime (in seconds)'
                        ' are considered (defaults to 1000)')
    args = parser.parse_args()
    conn = sqlite3.connect(args.dbfile)
    conn.row_factory = sqlite3.Row
    configCols = get_identifying_columnames(conn)
    print('unique configurations given by: %s\n' % ', '.join(configCols))
    # print_table(*minimal_treewidth_per_instance_and_configuration(conn, configCols, args.runtime),
    #             title=minimal_treewidth_per_instance_and_configuration.__doc__)
    print_table(*best_config_per_instance(conn, configCols, args.runtime),
                title=best_config_per_instance.__doc__)
    print_table(*average_performances(conn, configCols, args.runtime),
                title=average_performances.__doc__)
    print_table(*best_configuration_all_instances(conn, configCols, args.runtime),
                title=best_configuration_all_instances.__doc__)
    print_table(*all_configurations_all_instances(conn, configCols, args.runtime),
                title=all_configurations_all_instances.__doc__)
    conn.close()


if __name__ == '__main__':
    main()
