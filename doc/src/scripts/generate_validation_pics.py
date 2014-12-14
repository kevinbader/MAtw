#!/usr/bin/env python
# encoding: utf-8

import gc
import math
import os
import subprocess

import matplotlib as mpl
mpl.use('Cairo')
mpl.rc('font', **{'family': 'sans-serif', 'sans-serif': ['Computer Modern Sans serif']})
mpl.rcParams['text.usetex'] = True
mpl.rcParams['text.latex.unicode'] = True
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import sqlite3 as sqlite


COLOR = {'IHA': lambda: sns.color_palette(n_colors=8)[0],
         'GAtw': lambda: sns.color_palette(n_colors=8)[1],
         'MA1': lambda: sns.color_palette(n_colors=8)[2],
         'MA2': lambda: sns.color_palette(n_colors=8)[3],
         'MA3': lambda: sns.color_palette(n_colors=8)[4]}
VARIANTS = ['MA1', 'MA2', 'MA3', 'IHA', 'GAtw']
children_PIDs = []


def create_distandviolin(db, instance):
    query = ('select variant, min(treewidth) as tw'
             ' from validationresults'
             ' where instance="%s" group by variant, seed' % instance)
    # dbDf = pd.DataFrame(pd.read_sql_query(query, db), dtype=np.float32)
    dbDf = pd.read_sql_query(query, db)
    nSamples = 20

    def do_checks():
        read_variants = dbDf.variant.drop_duplicates().values
        variants = ['IHA', 'GAtw', 'MA1', 'MA2', 'MA3']
        assert list(map(lambda x: x in read_variants, variants)) == [True] * len(variants), 'expected the variants %r instead of %r' % (variants, read_variants)
        assert len(variants) == len(read_variants), 'expected the variants %r instead of %r' % (variants, read_variants)
        assert dbDf.tw.count() % len(variants) == 0, 'are there the same number of samples for each variant?'
        assert dbDf.tw.count() / len(variants) == nSamples, 'expected 20 samples'

    do_checks()

    def create_work_dataframe():
        validationDf = pd.DataFrame(index=np.arange(nSamples), columns=VARIANTS)
        for variant in VARIANTS:
            validationDf[variant] = dbDf.loc[(dbDf.variant == variant)].tw.values
        return validationDf

    validationDf = create_work_dataframe()
    try:
        create_distplots(validationDf, instance)
    except Exception as e:
        print('%s: dist plots failed (%r)' % (instance, e))
    try:
        create_violinplot(validationDf, instance)
    except Exception as e:
        print('%s: violin plot failed (%r)' % (instance, e))


def create_distplots(df, instance):
    for variant in VARIANTS:
        filename = 'validation-validationset-%s-%s-dist.pdf' % (instance.replace('_', '-'), variant)
        if os.path.exists(filename):
            print('skipping distplot for %s (file exists: %s)' % (instance, filename))
        else:
            sns.distplot(df[variant].values, rug=True, hist=False)
            plt.savefig(filename)
            plt.clf()
            plt.close('all')
            gc.collect()


def create_violinplot(df, instance):
    filename = 'validation-validationset-%s-violin.pdf' % instance.replace('_', '-')
    if os.path.exists(filename):
        print('skipping violinplot for %s (file exists: %s)' % (instance, filename))
    else:
        with sns.axes_style('whitegrid'):
            order = VARIANTS[:]
            order.reverse()
            fig, ax = plt.subplots(1, figsize=(3.76, 2.82), dpi=600)  # 4.8x3.6
            sns.violinplot(df, order=order, vert=False, fig=fig, ax=ax, alpha=0.75)
            sns.despine(left=True, bottom=True)
            ax.set_xlabel('treewidth')
            fig.tight_layout()
            plt.savefig(filename)
            plt.clf()
            plt.close('all')
            gc.collect()


def normalized_measurements(df, bins, start_value=None):
    assert not df.empty, 'got empty DataFrame'
    assert len(df.columns) == 4 and 'variant' in df.columns and 'runtime' in df.columns and 'treewidth' in df.columns and 'seed' in df.columns, "DataFrame doesn't look as expected"
    # the average for a bin is the average of the seed-averages.
    # a seed-average is the average of the seed-specific values at the beginning and the end of a bin.
    # the value at the beginning of a bin is not the last seed-average but the last seed-specific measurement,
    # so we remember this value for each seed:
    last_treewidth = None
    ret = pd.DataFrame(index=bins[:-1], columns=df.columns)
    for i in range(len(bins) - 1):
        # print(ret)
        # print('processing bin %s: %s--%s secs' % (i, bins[i], bins[i+1] if i < len(bins) else "inf"))
        selection_window = (df.runtime >= bins[i]) & (df.runtime < bins[i + 1])
        # print(df.loc[selection_window])
        timepoint = bins[i] + (bins[i + 1] - bins[i]) / 2.
        if selection_window.any():
            # we have measurements in this window. we ignore all data points but the last one:
            # print('window %s: max=%s, min=%s' % (i, df.loc[selection_window, 'treewidth'].max(), df.loc[selection_window, 'treewidth'].min()))
            next_last_treewidth = df.loc[selection_window, 'treewidth'].min()
            assert last_treewidth is None or next_last_treewidth <= last_treewidth, "window is [%s,%s], but treewidth values should be descending :(" % (last_treewidth, next_last_treewidth)
            if i == len(bins) - 1:
                # in the last bin we use the final treewidth:
                mean = df.loc[selection_window, 'treewidth'].min()
            elif last_treewidth is None:
                # first datapoint found; we use that value instead of the average:
                mean = df.loc[selection_window, 'treewidth'].max()
            else:
                # use the average of start and end values of this bin:
                mean = (last_treewidth + next_last_treewidth) / 2
            # print('%4s: %s | <%s values> | %s => %s' % (i, last_treewidth, len(df.loc[selection_window, 'treewidth']), next_last_treewidth, mean))
            # print('found %s rows. mean: %s' % (df.loc[sel, 'runtime'].count(), mean))
            ret.loc[bins[i]] = df.loc[selection_window].head(1).values[0]
            ret.loc[bins[i], ('runtime', 'treewidth',)] = (timepoint, mean,)
            last_treewidth = next_last_treewidth
        else:
            if last_treewidth is None:
                # print('no rows found so far, so skipping bin')
                # sys.stdout.write('.')
                ret.loc[bins[i]] = df.head(1).values[0]
                ret.loc[bins[i], 'runtime'] = timepoint
                if start_value is None:
                    ret.loc[bins[i], 'treewidth'] = np.nan
                else:
                    ret.loc[bins[i], 'treewidth'] = start_value
            else:
                ret.loc[bins[i]] = ret.loc[bins[i - 1]]
                ret.loc[bins[i], ('runtime', 'treewidth',)] = (timepoint, last_treewidth,)
    return ret


def create_timing_plots(db, instance):
    dbDf = pd.read_sql_query('select variant, runtime_s as runtime, treewidth, seed'
                             ' from validationresults where instance = "%s"' % instance,
                             db)
    filename = 'validation-validationset-%s-timings.pdf' % instance.replace('_', '-')
    if os.path.exists(filename):
        print('skipping timing plots for %s (file exists: %s)' % (instance, filename))
    else:
        max_treewidth = dbDf.treewidth.max()
        max_time = dbDf.runtime.max()
        bins_limit = max_time + 1
        logspace_base = 10
        logspace_end = math.ceil(math.log(bins_limit) / math.log(logspace_base))
        logspace_n_samples = bins_limit / 36
        bins = np.array([x - 1 for x in np.logspace(0, logspace_end, num=logspace_n_samples, base=logspace_base) if x < bins_limit]
                        + [bins_limit])
        seeds = dbDf.seed.drop_duplicates().values
        frames = []
        for variant in VARIANTS:
            for seed in seeds:
                # print('---frame for %s:%s\n%r' % (variant, seed, dbDf.loc[(dbDf.variant == variant) & (dbDf.seed == seed)]))
                frames.append(normalized_measurements(dbDf.loc[(dbDf.variant == variant) & (dbDf.seed == seed)],
                                                      bins,
                                                      start_value=max_treewidth))
        runs = pd.concat(frames)
        with sns.axes_style('ticks'):
            f, axes = plt.subplots(1, 2, sharey=False, figsize=(5.4, 2.28), dpi=600)
            # by default the 68% confidence interval is plotted, which corresponds to the standard error of the estimator (ci=68)
            sns.tsplot(runs, time='runtime', value='treewidth', unit='seed', condition='variant', ax=axes[0])
            sns.tsplot(runs, time='runtime', value='treewidth', unit='seed', condition='variant', ax=axes[1])
            # axes[0].set_title('solution quality over time, linear time-scale')
            axes[0].set_xlim([0, max_time])
            axes[0].set_xticks(list(range(0, int(max_time+1), 600)))
            second_largest_treewidth = dbDf.treewidth.drop_duplicates().sort(ascending=False, inplace=False)[2]
            axes[0].set_ylim([0, second_largest_treewidth])
            axes[0].legend().remove()
            # axes[1].set_title('solution quality over time, logarithmic time-scale')
            axes[1].set_xscale('log')
            axes[1].set_xlim([0, max_time])
            axes[1].set_ylim([0, max_treewidth * 1.01])
            axes[1].legend().set_title(None)
            sns.despine()
            f.tight_layout()
            plt.savefig(filename)
            plt.clf()
            plt.close('all')
        gc.collect()
        subprocess.check_call(['pdfcrop', '--luatex', '--margins', '0 0 10 0', filename], stdout=subprocess.DEVNULL)


def create_title_pic(db):
    dbDf = pd.read_sql_query('select variant, runtime_s as runtime, treewidth, seed'
                             ' from validationresults where instance = "rl5934"',
                             db)
    filename = 'titlepic.pdf'
    if os.path.exists(filename):
        print('skipping title pic (file exists: %s)' % (filename,))
        return
    max_treewidth = dbDf.treewidth.max()
    max_time = dbDf.runtime.max()
    bins_limit = max_time + 1
    logspace_base = 10
    logspace_end = math.ceil(math.log(bins_limit) / math.log(logspace_base))
    logspace_n_samples = bins_limit / 36
    bins = np.array([x - 1 for x in np.logspace(0, logspace_end, num=logspace_n_samples, base=logspace_base) if x < bins_limit]
                    + [bins_limit])
    seeds = dbDf.seed.drop_duplicates().values
    frames = []
    for variant in ['MA3']:
        for seed in seeds:
            # print('---frame for %s:%s\n%r' % (variant, seed, dbDf.loc[(dbDf.variant == variant) & (dbDf.seed == seed)]))
            frames.append(normalized_measurements(dbDf.loc[(dbDf.variant == variant) & (dbDf.seed == seed)],
                                                  bins,
                                                  start_value=max_treewidth))
    runs = pd.concat(frames)
    sns.set_palette(sns.color_palette('hls'))
    # by default the 68% confidence interval is plotted, which corresponds to the standard error of the estimator (ci=68)
    with sns.axes_style('white'):
        plt.figure(figsize=(12, 8), dpi=600)
        ax = sns.tsplot(runs, time='runtime', value='treewidth', unit='seed', condition='variant',
                        color=sns.color_palette()[0], err_style="unit_traces", legend=False)
        ax.set_ylim([0, 200])
        ax.set_xlim([0, 3600])
        plt.axis('off')
        plt.savefig(filename)
        plt.clf()
        plt.close('all')
    gc.collect()
    subprocess.check_call(['pdfcrop', '--luatex', '--margins', '0 0 0 0', filename], stdout=subprocess.DEVNULL)


def main():
    sns.set_context('paper')
    sns.set_palette(sns.color_palette('colorblind') + ['#872D62', '#BA8417'], n_colors=8)
    db = sqlite.connect('/home/kevin/uni/d2/code/final/validation/'
                        'uranus.validation_validationset.sqlite.db')
    dbc = db.cursor()
    query_result = dbc.execute('select distinct(instance) from validationresults')
    instances = [row[0] for row in query_result]
    # instances = ['link']
    print(instances)
    for instance in instances:
        create_distandviolin(db, instance)
        try:
            create_timing_plots(db, instance)
        except Exception as e:
            print('%s: timing plots failed (%r)' % (instance, e))
    # create_title_pic(db)
    db.close()


if __name__ == '__main__':
    main()
