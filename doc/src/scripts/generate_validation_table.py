#!/usr/bin/env python
# encoding: utf-8

import math
import sys

import scipy.stats as STATS
import numpy as NP
import sqlite3 as sqlite
import statsmodels.stats.multicomp

import LSD


db_fn = sys.argv[1]
instance = sys.argv[2]

VARIANTS = ['MA1', 'MA2', 'MA3', 'IHA', 'GAtw']


def main():
    def n_digits(num):
        if num <= 1:
            return 1
        return math.ceil(math.log(num) / math.log(10))

    db = sqlite.connect(db_fn)
    dbc = db.cursor()
    rows = []
    integer_digits = {'best': 0,
                      'best_time': 0,
                      'mean': 0,
                      'stddev': 0}
    allvals = []
    allvals_dict = {}
    for variant in VARIANTS:
        query = ("select tw from (select min(treewidth) as tw from validationresults where variant='%(variant)s' and instance='%(instance)s' group by seed)")
        result = dbc.execute(query % {'variant': variant, 'instance': instance})
        vals = NP.array([row[0] for row in result])
        min, mean, stddev = vals.min(), vals.mean(), vals.std()
        # print('%s: vals=%r' % (variant, vals), file=sys.stderr)
        W, p = STATS.shapiro(vals)
        print('%s: normal distribution? shapiro-wilk: W=%s (p=%s) %s@5%% %s@2%%' % (variant, W, p, 'no' if W <= .905 else 'yes', 'no' if W <= .884 else 'yes'), file=sys.stderr)
        z, p = STATS.skewtest(vals)
        print('%s: normal distribution? skew test: (z=%s) p=%s => %s' % (variant, z, p, 'no' if p < .5 else 'yes'), file=sys.stderr)
        allvals.append(vals)
        allvals_dict[variant] = vals

        query = ("select min(runtime_s)"
                 " from validationresults"
                 " where variant='%(variant)s' and instance='%(instance)s' and treewidth='%(treewidth)s'")
        result = dbc.execute(query % {'variant': variant, 'instance': instance, 'treewidth': min})
        best_time = [row[0] for row in result][0]
        # print("%s: best=%s @ %ss, avg=%s +- %s" % (variant, min, best_time, mean, stddev), file=sys.stderr)
        row = {'variant': variant,
               'best': min,
               'best_time': round(best_time, 1),
               'mean': round(mean, 1),
               'stddev': round(stddev, 1)}
        rows.append(row)
        integer_digits['best'] = max(integer_digits['best'], n_digits(row['best']))
        integer_digits['best_time'] = max(integer_digits['best_time'], n_digits(row['best_time']))
        integer_digits['mean'] = max(integer_digits['mean'], n_digits(row['mean']))
        integer_digits['stddev'] = max(integer_digits['stddev'], n_digits(row['stddev']))
    db.close()
    T, p = STATS.bartlett(*allvals)
    print('equal variances? bartlett: T=%s (p=%s) [vs Chi-Quadrat_{k-1=%s, alpha=.5}]' % (T, p, len(allvals) - 1), file=sys.stderr)
    W, p = STATS.levene(*allvals, center='mean')
    print('equal variances? levene (mean): (W=%s) p=%s' % (W, p), file=sys.stderr)
    W, p = STATS.levene(*allvals, center='median')
    print('equal variances? levene (median): (W=%s) p=%s' % (W, p), file=sys.stderr)
    F, p = STATS.f_oneway(*allvals)
    print('equal means? one-way ANOVA: F=%s, p=%s [vs F_{k-1=%s,n-k=%s}]' % (F, p, len(allvals) - 1, sum([len(x) for x in allvals]) - len(allvals)), file=sys.stderr)
    try:
        W, p = STATS.kruskal(*allvals)
        print('equal means? kruskal wallis: W=%s, p=%s' % (W, p), file=sys.stderr)
    except Exception as e:
        print(e)
    lsd = LSD.LSD(allvals, .05)
    print('LSD: %r' % lsd, file=sys.stderr)
    print(statsmodels.stats.multicomp.pairwise_tukeyhsd(NP.array(allvals).ravel(), NP.array([[x] * 20 for x in VARIANTS]).ravel(), alpha=.10), file=sys.stderr)
    print(statsmodels.stats.multicomp.pairwise_tukeyhsd(NP.array(allvals).ravel(), NP.array([[x] * 20 for x in VARIANTS]).ravel(), alpha=.05), file=sys.stderr)

    def welch(var1, var2):
        res = STATS.ttest_ind(allvals_dict[var1], allvals_dict[var2], equal_var=False)
        print('%4s vs %s  t,p=%r => \t%s @a=10%%, %s @a=5%%'
              % (var1, var2, res, 'NE' if res[1] < .01116 else ' E', 'NE' if res[1] < .00568 else ' E'), file=sys.stderr)

    print('pairwise Welch\'s t-test with Bonferroni correction:', file=sys.stderr)
    welch('IHA', 'MA1')
    welch('IHA', 'MA2')
    welch('IHA', 'MA3')
    welch('GAtw', 'MA1')
    welch('GAtw', 'MA2')
    welch('GAtw', 'MA3')
    welch('MA1', 'MA2')
    welch('MA1', 'MA3')
    welch('MA2', 'MA3')

    def mannwhitneyu(var1, var2):
        try:
            res = STATS.mannwhitneyu(allvals_dict[var1], allvals_dict[var2])
            print('%4s vs %s  u,p=%r => \t%s @a=10%%, %s @a=5%%'
                  % (var1, var2, res, 'NE' if res[1] < .01116 else ' E', 'NE' if res[1] < .00568 else ' E'), file=sys.stderr)
        except Exception as e:
            print('%4s vs %s  failed: %r' % (var1, var2, e))

    print('pairwise Mann-Whitney U test with Bonferroni correction:', file=sys.stderr)
    mannwhitneyu('IHA', 'MA1')
    mannwhitneyu('IHA', 'MA2')
    mannwhitneyu('IHA', 'MA3')
    mannwhitneyu('GAtw', 'MA1')
    mannwhitneyu('GAtw', 'MA2')
    mannwhitneyu('GAtw', 'MA3')
    mannwhitneyu('MA1', 'MA2')
    mannwhitneyu('MA1', 'MA3')
    mannwhitneyu('MA2', 'MA3')

    #latex = [r'\begin{sidefigure}{caption={Results for instance \Instance{%(instanceTexEsc)s}},label={fig:%(instanceFileEsc)s-results},place={htbp}}''\n'
             #r'   \begin{center}''\n'
    latex = [r'\begin{table}[hbtp]''\n'
             r'   \caption{Results for instance \Instance{%(instanceTexEsc)s}}''\n'
             r'   \label{fig:%(instanceFileEsc)s-results}''\n'
             r'   \centering\small''\n'
             r'      \begin{tabular}{l S[table-format=%(best)s] S[table-format=%(best_time)s.1]%%''\n'
             r'                      S[table-format=%(mean)s.1,table-number-alignment=right] @{$\,\pm\,$} S[table-format=%(stddev)s.1,table-number-alignment=left]''\n'
             r'                      S[table-format=2]} \toprule''\n'
             r'         & \multicolumn{2}{c}{\header{Best}} & \multicolumn{2}{c}{\header{Average}} & \\ \cmidrule(lr){2-3}\cmidrule(lr){4-5}''\n'
             r'         & \header{treewidth} & \header{seconds} & \multicolumn{2}{c}{\header{treewidth}} & \header{samples} \\ \midrule'
             % dict(integer_digits.items() | dict(instanceTexEsc=instance.replace('_', r'\textunderscore{}'), instanceFileEsc=instance.replace('_', '-')).items())]
    for row in rows:
        latex.append(' ' * (3 * 3) + ' & '.join([row['variant'], str(row['best']), str(row['best_time']), str(row['mean']), str(row['stddev']), "20"]) + r'\\')
    latex.append(r'         \bottomrule''\n'
                 r'      \end{tabular}''\n'
                 r'\end{table}')
                 #r'   \end{center}''\n'
                 #r'\end{sidefigure}')

    with open('validation-validationset-%s-results.tex' % instance.replace('_', '-'), 'w') as f:
        print('\n'.join(latex), file=f)


if __name__ == '__main__':
    main()
