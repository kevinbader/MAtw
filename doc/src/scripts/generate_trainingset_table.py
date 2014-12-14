#!/usr/bin/env python
# encoding: utf-8

import math
import subprocess
import sys

import numpy as NP
import sqlite3 as sqlite


db_fn = sys.argv[1]

VALIDATED_VARIANTS = ['MA1', 'MA2', 'MA3', 'IHA', 'GAtw']
# OTHER_VARIANTS = ['BB_tw',      # Bodlaender, Bachoore. 2005
#                   'QuickBB',    # Gogate, Dechter. 2005
#                   'TabuTW',     # Clautiax et al. 2003
#                   'ACS_ILS',    # Hammerl. 2009
#                   ]

OTHER_VARIANTS = {'BB_tw': {'anna': 12,  # Bodlaender, Bachoore. 2005
                            'david': 13,
                            'games120': 38,
                            'inithx.i.2': 35,  # opt 31
                            'inithx.i.3': 35,  # opt 31
                            'myciel6': 35,
                            'myciel7': 66,
                            'school1': 178,
                            'le450_5a': 304,
                            'DSJC125.1': 64,
                            'DSJC125.5': 109,
                            'DSJC250.1': 177},
                  # Gogate, Dechter. 2005
                  'QuickBB': {'anna': 12,
                              'david': 13,
                              'huck': 10,
                              'homer': 31,
                              'jean': 9,
                              'queen5_5': 18,
                              'queen6_6': 25,
                              'queen7_7': 35,
                              'queen8_8': 46,
                              'queen9_9': 59,
                              'queen10_10': 72,
                              'queen11_11': 89,
                              'queen12_12': 110,
                              'queen13_13': 125,
                              'queen14_14': 143,
                              'queen15_15': 167,
                              'queen16_16': 205,
                              'fpsol2.i.1': 66,  # opt 65
                              'fpsol2.i.2': 31,  # opt 30
                              'fpsol2.i.3': 31,  # opt 30
                              'inithx.i.1': 56,  # opt 54
                              'inithx.i.2': 31,  # opt 31
                              'inithx.i.3': 31,  # opt 31
                              'miles1000': 49,
                              'miles1500': 77,
                              'miles250': 9,
                              'miles500': 22,
                              'miles750': 37,
                              'mulsol.i.1': 50,
                              'mulsol.i.2': 32,
                              'mulsol.i.3': 32,
                              'mulsol.i.4': 32,
                              'mulsol.i.5': 31,
                              'myciel3': 5,
                              'myciel4': 10,
                              'myciel5': 19,
                              'myciel6': 35,
                              'myciel7': 54,
                              'le450_5a': 307,
                              'le450_5b': 309,
                              'le450_5c': 315,
                              'le450_5d': 303,
                              'le450_15b': 289,
                              'le450_15c': 372,
                              'le450_15d': 371,
                              'le450_25a': 255,
                              'le450_25b': 251,
                              'le450_25c': 349,
                              'le450_25d': 349,
                              'DSJC1000.1': 896,
                              'DSJC1000.5': 977,
                              'DSJC1000.9': 991,
                              'DSJC125.1': 64,
                              'DSJC125.5': 109,
                              'DSJC125.9': 119,
                              'DSJC250.1': 176,
                              'DSJC250.5': 231,
                              'DSJC250.9': 243,
                              'DSJC500.1': 409,
                              'DSJC500.5': 479,
                              'DSJC500.9': 492,
                              'DSJR500.1c': 485,
                              'DSJR500.5': 175},
                  # Clautiax et al. 2003
                  'TabuTW': {'anna': 12,
                             'david': 13,
                             'huck': 10,
                             'homer': 31,
                             'jean': 9,
                             'games120': 33,
                             'queen5_5': 18,
                             'queen6_6': 25,
                             'queen7_7': 35,
                             'queen8_8': 46,
                             'queen9_9': 58,
                             'queen10_10': 72,
                             'queen11_11': 88,
                             'queen12_12': 104,
                             'queen13_13': 122,
                             'queen14_14': 141,
                             'queen15_15': 163,
                             'queen16_16': 186,
                             'fpsol2.i.1': 66,  # opt 65
                             'fpsol2.i.2': 31,  # opt 30
                             'fpsol2.i.3': 31,  # opt 30
                             'inithx.i.1': 56,  # opt 54
                             'inithx.i.2': 35,  # opt 31
                             'inithx.i.3': 35,  # opt 31
                             'miles1000': 49,
                             'miles1500': 77,
                             'miles250': 9,
                             'miles500': 22,
                             'miles750': 36,
                             'mulsol.i.1': 50,
                             'mulsol.i.2': 32,
                             'mulsol.i.3': 32,
                             'mulsol.i.4': 32,
                             'mulsol.i.5': 31,
                             'myciel3': 5,
                             'myciel4': 10,
                             'myciel5': 19,
                             'myciel6': 35,
                             'myciel7': 66,
                             'school1': 188,
                             'school1_nsh': 162,
                             'zeroin.i.1': 50,
                             'zeroin.i.2': 32,
                             'zeroin.i.3': 32,
                             'le450_5a': 256,
                             'le450_5b': 254,
                             'le450_5c': 272,
                             'le450_5d': 278,
                             'le450_15a': 272,
                             'le450_15b': 270,
                             'le450_15c': 359,
                             'le450_15d': 360,
                             'le450_25a': 234,
                             'le450_25b': 233,
                             'le450_25c': 327,
                             'le450_25d': 336,
                             'DSJC125.1': 65,
                             'DSJC125.5': 109,
                             'DSJC125.9': 119,
                             'DSJC250.1': 173,
                             'DSJC250.5': 232,
                             'DSJC250.9': 243},
                  # Hammerl. 2009
                  'ACS_ILS': {'anna': 12,
                              'david': 13,
                              'huck': 10,
                              'homer': 30,
                              'jean': 9,
                              'games120': 37,
                              'queen5_5': 18,
                              'queen6_6': 25,
                              'queen7_7': 35,
                              'queen8_8': 46,
                              'queen9_9': 59,
                              'queen10_10': 73,
                              'queen11_11': 89,
                              'queen12_12': 109,
                              'queen13_13': 128,
                              'queen14_14': 150,
                              'queen15_15': 174,
                              'queen16_16': 201,
                              'fpsol2.i.1': 66,  # opt 65
                              'fpsol2.i.2': 31,  # opt 30
                              'fpsol2.i.3': 31,  # opt 30
                              'inithx.i.1': 56,  # opt 54
                              'inithx.i.2': 31,  # opt 31
                              'inithx.i.3': 31,  # opt 31
                              'miles1000': 50,
                              'miles1500': 77,
                              'miles250': 9,
                              'miles500': 25,
                              'miles750': 38,
                              'mulsol.i.1': 50,
                              'mulsol.i.2': 32,
                              'mulsol.i.3': 32,
                              'mulsol.i.4': 32,
                              'mulsol.i.5': 31,
                              'myciel3': 5,
                              'myciel4': 10,
                              'myciel5': 19,
                              'myciel6': 35,
                              'myciel7': 66,
                              'school1': 228,
                              'school1_nsh': 185,
                              'zeroin.i.1': 50,
                              'zeroin.i.2': 33,
                              'zeroin.i.3': 33,
                              'le450_5a': 304,
                              'le450_5b': 308,
                              'le450_5c': 309,
                              'le450_5d': 290,
                              'le450_15a': 288,
                              'le450_15b': 292,
                              'le450_15c': 368,
                              'le450_15d': 371,
                              'le450_25a': 249,
                              'le450_25b': 245,
                              'le450_25c': 346,
                              'le450_25d': 355,
                              'DSJC125.1': 63,
                              'DSJC125.5': 108,
                              'DSJC125.9': 119,
                              'DSJC250.1': 174,
                              'DSJC250.5': 231,
                              'DSJC250.9': 243},
                  'IHA': {'anna': 12,
                          'david': 13,
                          'huck': 10,
                          'homer': 31,
                          'jean': 9,
                          'games120': 32,
                          'queen5_5': 18,
                          'queen6_6': 25,
                          'queen7_7': 35,
                          'queen8_8': 45,
                          'queen9_9': 58,
                          'queen10_10': 72,
                          'queen11_11': 87,
                          'queen12_12': 103,
                          'queen13_13': 121,
                          'queen14_14': 140,
                          'queen15_15': 162,
                          'queen16_16': 186,
                          'fpsol2.i.1': 66,  # opt 65
                          'fpsol2.i.2': 31,  # opt 30
                          'fpsol2.i.3': 31,  # opt 30
                          'inithx.i.1': 56,  # opt 54
                          'inithx.i.2': 35,  # opt 31
                          'inithx.i.3': 35,  # opt 31
                          'miles1000': 49,
                          'miles1500': 77,
                          'miles250': 9,
                          'miles500': 22,
                          'miles750': 36,
                          'mulsol.i.1': 50,
                          'mulsol.i.2': 32,
                          'mulsol.i.3': 32,
                          'mulsol.i.4': 32,
                          'mulsol.i.5': 31,
                          'myciel3': 5,
                          'myciel4': 10,
                          'myciel5': 19,
                          'myciel6': 35,
                          'myciel7': 66,
                          'school1': 178,
                          'school1_nsh': 152,
                          'zeroin.i.1': 50,
                          'zeroin.i.2': 32,
                          'zeroin.i.3': 32,
                          'le450_5a': 244,
                          'le450_5b': 246,
                          'le450_5c': 266,
                          'le450_5d': 265,
                          'le450_15a': 262,
                          'le450_15b': 258,
                          'le450_15c': 350,
                          'le450_15d': 355,
                          'le450_25a': 216,
                          'le450_25b': 219,
                          'le450_25c': 322,
                          'le450_25d': 328,
                          'DSJC125.1': 60,
                          'DSJC125.5': 108,
                          'DSJC125.9': 119,
                          'DSJC250.1': 167,
                          'DSJC250.5': 229,
                          'DSJC250.9': 243,
                          },
                  'GAtw': {'anna': 12,
                           'david': 13,
                           'huck': 10,
                           'homer': 31,
                           'jean': 9,
                           'games120': 32,
                           'queen5_5': 18,
                           'queen6_6': 26,
                           'queen7_7': 35,
                           'queen8_8': 45,
                           'queen9_9': 58,
                           'queen10_10': 72,
                           'queen11_11': 87,
                           'queen12_12': 104,
                           'queen13_13': 121,
                           'queen14_14': 141,
                           'queen15_15': 162,
                           'queen16_16': 186,
                           'fpsol2.i.1': 66,  # opt 65
                           'fpsol2.i.2': 32,  # opt 30
                           'fpsol2.i.3': 31,  # opt 30
                           'inithx.i.1': 56,  # opt 54
                           'inithx.i.2': 35,  # opt 31
                           'inithx.i.3': 35,  # opt 31
                           'miles1000': 50,
                           'miles1500': 77,
                           'miles250': 10,
                           'miles500': 24,
                           'miles750': 37,
                           'mulsol.i.1': 50,
                           'mulsol.i.2': 32,
                           'mulsol.i.3': 32,
                           'mulsol.i.4': 32,
                           'mulsol.i.5': 31,
                           'myciel3': 5,
                           'myciel4': 10,
                           'myciel5': 19,
                           'myciel6': 35,
                           'myciel7': 66,
                           'school1': 185,
                           'school1_nsh': 157,
                           'zeroin.i.1': 50,
                           'zeroin.i.2': 32,
                           'zeroin.i.3': 32,
                           'le450_5a': 243,
                           'le450_5b': 248,
                           'le450_5c': 265,
                           'le450_5d': 265,
                           'le450_15a': 265,
                           'le450_15b': 265,
                           'le450_15c': 351,
                           'le450_15d': 353,
                           'le450_25a': 225,
                           'le450_25b': 227,
                           'le450_25c': 320,
                           'le450_25d': 327,
                           'DSJC125.1': 61,
                           'DSJC125.5': 109,
                           'DSJC125.9': 119,
                           'DSJC250.1': 169,
                           'DSJC250.5': 230,
                           'DSJC250.9': 243}}

PREV_RESULT = {'anna': 12,
               'david': 13,
               'huck': 10,
               'homer': 31,
               'jean': 9,
               'games120': 32,
               'queen5_5': 18,
               'queen6_6': 25,
               'queen7_7': 35,
               'queen8_8': 45,
               'queen9_9': 58,
               'queen10_10': 72,
               'queen11_11': 87,
               'queen12_12': 103,
               'queen13_13': 121,
               'queen14_14': 140,
               'queen15_15': 162,
               'queen16_16': 186,
               'fpsol2.i.1': 66,  # opt 65
               'fpsol2.i.2': 31,  # opt 30
               'fpsol2.i.3': 31,  # opt 30
               'inithx.i.1': 56,  # opt 54
               'inithx.i.2': 31,  # opt 31
               'inithx.i.3': 31,  # opt 31
               'miles1000': 49,
               'miles1500': 77,
               'miles250': 9,
               'miles500': 22,
               'miles750': 36,
               'mulsol.i.1': 50,
               'mulsol.i.2': 32,
               'mulsol.i.3': 32,
               'mulsol.i.4': 32,
               'mulsol.i.5': 31,
               'myciel3': 5,
               'myciel4': 10,
               'myciel5': 19,
               'myciel6': 35,
               'myciel7': 54,
               'school1': 178,
               'school1_nsh': 152,
               'zeroin.i.1': 50,
               'zeroin.i.2': 32,
               'zeroin.i.3': 32,
               'le450_5a': 243,
               'le450_5b': 246,
               'le450_5c': 265,
               'le450_5d': 265,
               'le450_15a': 262,
               'le450_15b': 258,
               'le450_15c': 350,
               'le450_15d': 353,
               'le450_25a': 216,
               'le450_25b': 219,
               'le450_25c': 320,
               'le450_25d': 327,
               'DSJC125.1': 60,
               'DSJC125.5': 108,
               'DSJC125.9': 119,
               'DSJC250.1': 167,
               'DSJC250.5': 229,
               'DSJC250.9': 243,
               # 'flat1000_50_0': 50,  # optimum
               # 'flat1000_60_0': 60,  # optimum
               # 'flat1000_76_0': 76,  # optimum
               # 'flat300_20_0': 20,  # optimum
               # 'flat300_26_0': 26,  # optimum
               # 'flat300_28_0': 28,  # optimum
               }


def prev_upper_bound(instance):
    ub = None
    for variant in list(OTHER_VARIANTS):
        if instance in OTHER_VARIANTS[variant]:
            if ub:
                ub = min(ub, OTHER_VARIANTS[variant][instance])
            else:
                ub = OTHER_VARIANTS[variant][instance]
    return ub


def main():
    def n_digits(num):
        if num <= 1:
            return 1
        return math.ceil(math.log(num) / math.log(10))

    db = sqlite.connect(db_fn)
    dbc = db.cursor()
    rows = []
    field_widths = {}
    for variant in VALIDATED_VARIANTS:
        field_widths['%s-min' % variant] = 0
    for variant in ['TabuTW', 'ACS_ILS', 'BB_tw', 'QuickBB']:
        field_widths['%s-min' % variant] = 3
    field_widths['V'] = field_widths['E'] = 0
    # instances are ordered according to their size, ascending
    instances = eval(subprocess.check_output(r"grep -v -P '^#' %s"
                                             r" | sort -t ' ' -k 2 -n"
                                             r" | sed -r 's/(.*)/(\"\1\")/'"
                                             r" | sed -r 's/ /\", \"/g'"
                                             r" | perl -pe 's/(.*)\n/$1, /g'"
                                             r" | sed -r 's/(.*)/[\1]/'"
                                             % '../../code/final/instances/training.list2', shell=True).decode())
    print('instances: %r' % instances)
    print()
    for instance_tuple in instances:
        instance_name = instance_tuple[0].rpartition('.')[0]
        instance_latex = instance_name.replace('_', r'\textunderscore{}')
        row = dict(name_raw=instance_name, name_tex=instance_latex, V=instance_tuple[1], E=instance_tuple[2])
        field_widths['V'] = max(field_widths['V'], n_digits(int(row['V'])))
        field_widths['E'] = max(field_widths['E'], n_digits(int(row['E'])))
        ub = prev_upper_bound(instance_name)
        if ub:
            row['rowmin'] = ub
            row['ub'] = ub
        for variant in VALIDATED_VARIANTS:
            query = ("select tw from (select min(treewidth) as tw from validationresults where variant='%(variant)s' and instance='%(instance)s' group by seed)"
                     % {'variant': variant, 'instance': instance_name})
            result = dbc.execute(query)
            vals = NP.array([row[0] for row in result])
            tw_min, tw_mean, tw_stddev = vals.min(), vals.mean(), vals.std()
            row[variant] = dict(min=str(tw_min))
            field_widths['%s-min' % variant] = max(field_widths['%s-min' % variant], n_digits(tw_min))
            if 'rowmin' in row:
                row['rowmin'] = min(row['rowmin'], tw_min)
            else:
                row['rowmin'] = tw_min
        rows.append(row)
    db.close()

    # latex = [r'\begin{sidefigure}{caption={Results for the training set},label={fig:validation-trainingset-results},place={htbp}}''\n'
    latex = [r'{\footnotesize''\n'
             r'\begin{longtable}[c]{l S[table-format=%(V)s] S[table-format=%(E)s]%%''\n'
             r'                     S[table-format=%(MA1-min)s] S[table-format=%(MA2-min)s] S[table-format=%(MA3-min)s]%%''\n'
             r'                     S[table-format=%(IHA-min)s] S[table-format=%(GAtw-min)s]''\n'
             r'                     S[table-format=%(TabuTW-min)s] S[table-format=%(ACS_ILS-min)s] S[table-format=%(BB_tw-min)s] S[table-format=%(QuickBB-min)s]}''\n'
             r'   \caption[Results on the benchmark instances of the\protect\newline Second DIMACS Implementation Challenge]{Results on the benchmark instances of the Second DIMACS Implementation Challenge}\label{tab:dimacs-results}\\ \toprule''\n'
             r'   & \headeR{1}{l}{0}{$|V|$} & \headeR{1}{l}{0}{$|E|$} & \headeR{1}{l}{60}{\glsname{MA1}} & \headeR{1}{l}{60}{\glsname{MA2}} & \headeR{1}{l}{60}{\glsname{MA3}} & \headeR{1}{l}{60}{\glsname{IHA}} & \headeR{1}{l}{60}{\glsname{GA-tw}} & \headeR{1}{l}{60}{\glsname{TabuTW}} & \headeR{1}{l}{60}{\glsname{ACSILS}} & \headeR{1}{l}{60}{\glsname{BBtw}} & \headeR{1}{l}{60}{\glsname{QuickBB}} \\ \midrule\endfirsthead'
             r'   \caption[]{(continued) Results on the benchmark instances of the Second DIMACS Implementation Challenge}\\ \toprule''\n'
             r'   & \headeR{1}{l}{0}{$|V|$} & \headeR{1}{l}{0}{$|E|$} & \headeR{1}{l}{60}{\glsname{MA1}} & \headeR{1}{l}{60}{\glsname{MA2}} & \headeR{1}{l}{60}{\glsname{MA3}} & \headeR{1}{l}{60}{\glsname{IHA}} & \headeR{1}{l}{60}{\glsname{GA-tw}} & \headeR{1}{l}{60}{\glsname{TabuTW}} & \headeR{1}{l}{60}{\glsname{ACSILS}} & \headeR{1}{l}{60}{\glsname{BBtw}} & \headeR{1}{l}{60}{\glsname{QuickBB}} \\ \midrule\endhead'
             % field_widths]

    def make_field(row, min_tw):
        assert 'rowmin' in row
        if int(min_tw) == row['rowmin']:
            field = r'\boldentry{%s}{%%s}' % field_widths['%s-min' % variant]
            if 'ub' in row and int(min_tw) < row['ub']:
                field %= r'\cellcolor{GreenYellow}%s'
        else:
            field = '%s'
        return field % min_tw

    for row in rows:
        if row['name_raw'] == 'latin_square_10':
            row['name_tex'] = r'latin\footnote{\Instance{latin\textunderscore{}square\textunderscore{}10}}'
        latexrow = [r'\Instance{%s}' % row['name_tex'], row['V'], row['E']]
        for variant in VALIDATED_VARIANTS:
            min_tw = row[variant]['min']
            latexrow.append(make_field(row, min_tw))
        for variant in ['TabuTW', 'ACS_ILS', 'BB_tw', 'QuickBB']:
            if row['name_raw'] in OTHER_VARIANTS[variant]:
                min_tw = OTHER_VARIANTS[variant][row['name_raw']]
                latexrow.append(make_field(row, min_tw))
            else:
                latexrow.append(r'\multicolumn{1}{c}{-}')
        latex.append(' & '.join(latexrow) + r'\\')
        # latex.append(' ' * (1 * 3) + ' & '.join([r'\Instance{%s}' % row['name'], row['V'], row['E']]
        #                                         + [r'\boldentry{%s}{%s}' % (field_widths['%s-min' % variant], r'\cellcolor{GreenYellow}%s' % row[variant]['min'] if 'prev_best' in row and int(row['prev_best']) != row['rowmin'] else row[variant]['min']) if int(row[variant]['min']) == row['rowmin']
        #                                            else row[variant]['min']
        #                                            for variant in VARIANTS]
        #                                         + [r'\boldentry{%s}{%s}' % (field_widths['prev_best'], row['prev_best']) if 'prev_best' in row and int(row['prev_best']) == row['rowmin']
        #                                            else (row['prev_best'] if 'prev_best' in row else r'\multicolumn{1}{c}{-}')]) + r'\\')
    latex.append(r'   \bottomrule''\n'
                 r'\end{longtable}}')
    # r'\end{sidefigure}')

    with open('validation-trainingset-results.tex', 'w') as f:
        print('\n'.join(latex), file=f)


if __name__ == '__main__':
    main()
