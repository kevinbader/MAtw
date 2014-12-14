#!/usr/bin/env python
# encoding: utf-8

from collections import OrderedDict as odict
import contextlib
import gc
import math
import os
import re
import subprocess

import matplotlib as mpl
mpl.use('Cairo')
import matplotlib.pyplot as plt
import mpl_toolkits.axes_grid.anchored_artists
import numpy as np
import pandas as pd
import seaborn as sns
import sqlite3 as sqlite


"""The path to the output files, relative to the LaTeX project root directory."""
OUTPUT_DIRNAME = 'plots'
# LATEX_INCLUDE_ALL_FILE = 'correlation-figures.tex'


SHORT_NAMES = {'Popsize': 'Population size',
               'TournamentSize': 'Tournament size',
               'LsFraction': 'LS fraction',
               'IlsOuterNonimpr': 'Max nonimpr.\\@ outer ILS steps',
               'IlsInnerNonimpr': 'Max nonimpr.\\@ inner ILS steps',
               'treewidth': 'Treewidth',
               'PartnerFraction': 'Partner population-fraction',
               'OpponentFraction': 'Opponent population-fraction'}
TEXT_NAMES = {'Popsize': 'population size',
              'TournamentSize': 'tournament size',
              'LsFraction': 'localsearch fraction',
              'IlsOuterNonimpr': "ILS' maximum number of nonimproving outer steps",
              'IlsInnerNonimpr': "ILS' maximum number of nonimproving inner steps",
              'treewidth': 'treewidth',
              'PartnerFraction': 'partner population-fraction',
              'OpponentFraction': 'opponent population-fraction'}
COLOR = {'Popsize': lambda: sns.color_palette(n_colors=8)[0],
         'TournamentSize': lambda: sns.color_palette(n_colors=8)[1],
         'LsFraction': lambda: sns.color_palette(n_colors=8)[2],
         'IlsOuterNonimpr': lambda: sns.color_palette(n_colors=8)[3],
         'IlsInnerNonimpr': lambda: sns.color_palette(n_colors=8)[4],
         'Crossover': lambda: sns.color_palette(n_colors=8)[5],
         'PartnerFraction': lambda: sns.color_palette(n_colors=8)[6],
         'OpponentFraction': lambda: sns.color_palette(n_colors=8)[7]}


def short_name(var):
    if var in SHORT_NAMES:
        return SHORT_NAMES[var]
    else:
        return var


def text_name(var):
    if var in TEXT_NAMES:
        return TEXT_NAMES[var]
    else:
        return short_name(var)


RANGES = {'MA3': odict((('Popsize', [10] + [i for i in range(500, 2001, 500)]),
                        ('LsFraction', [i / 100 for i in range(0, 101, 20)]),
                        ('IlsOuterNonimpr', [1] + [i for i in range(50, 201, 50)]))),
          'MA2': odict((('Popsize', [10] + [i for i in range(500, 2001, 500)]),
                        ('TournamentSize', [i for i in range(1, 6)]),
                        ('Crossover', [i / 100 for i in range(0, 101, 20)]),
                        ('LsFraction', [i / 100 for i in range(0, 101, 20)]),
                        ('IlsOuterNonimpr', [1] + [i for i in range(50, 201, 50)]),
                        ('IlsInnerNonimpr', [1] + [i for i in range(25, 101, 25)]))),
          'MA1': odict((('Popsize', [10] + [i for i in range(500, 2001, 500)]),
                        ('PartnerFraction', [i / 100 for i in range(0, 101, 20)]),
                        ('OpponentFraction', [i / 100 for i in range(0, 101, 20)]),
                        ('IlsOuterNonimpr', [1] + [i for i in range(50, 201, 50)]),
                        ('IlsInnerNonimpr', [1] + [i for i in range(25, 101, 25)])))}

SAMPLES = {'MA3': {'DSJR500.1c': 39,
                   'flat300_26_0': 32,
                   'fpsol2.i.3': 27,
                   'inithx.i.3': 25,
                   'mulsol.i.1': 33,
                   'zeroin.i.3': 31,
                   'le450_15c': 34,
                   'school1': 23,
                   'latin_square_10': 26,
                   'jean': 37,
                   'games120': 21,
                   'miles500': 23,
                   'queen10_10': 32,
                   'myciel6': 30},
           'MA2': {'DSJC500.9': 31,
                   'flat1000_76_0': 32,
                   'fpsol2.i.3': 24,
                   'inithx.i.3': 30,
                   'mulsol.i.2': 33,
                   'zeroin.i.3': 27,
                   'le450_5c': 31,
                   'school1_nsh': 20,
                   'latin_square_10': 20,
                   'jean': 35,
                   'games120': 22,
                   'miles1000': 28,
                   'queen12_12': 31,
                   'myciel6': 32},
           'MA1': {'DSJC1000.9': 31,
                   'flat300_26_0': 30,
                   'fpsol2.i.2': 24,
                   'inithx.i.3': 25,
                   'mulsol.i.2': 33,
                   'zeroin.i.3': 35,
                   'le450_15c': 38,
                   'school1': 20,
                   'latin_square_10': 25,
                   'anna': 31,
                   'games120': 23,
                   'miles500': 26,
                   'queen10_10': 41,
                   'myciel7': 35}}


class LatexFigure(object):

    def __init__(self, caption=None, label=None, tocCaption=None):
        self._items = []
        self.caption = caption
        self.label = label
        self.tocCaption = tocCaption

    def __str__(self):
        if self.caption:
            captionline = [r'\caption']
            if self.tocCaption:
                captionline.append('[%s]' % self.tocCaption)
            captionline.append('{%s}\n' % self.caption)
            captionline = ''.join(captionline)
        else:
            captionline = ''
        if self.label:
            labelline = '\\label{%s}\n' % self.label
        else:
            labelline = ''
        return '\n'.join([r'\begin{figure}[h]\strictpagecheck\centering''\n',
                          '\n'.join([str(x) for x in self._items]), '\n',
                          captionline,
                          labelline,
                          r'\end{figure}'])

    def add(self, item):
        self._items.append(item)


class LatexIncludegraphics(object):

    def __init__(self, filename):
        self._code = r'\includegraphics[scale=0.85]{%s/%s}' % (OUTPUT_DIRNAME, filename)

    def __str__(self):
        return self._code


class LatexFigureContent(object):

    def __init__(self):
        self._items = []

    def __str__(self):
        return '\n'.join(['\n\\vskip 0.5em plus 0.5em minus 0em\n'.join([str(x)
                          for x in self._items])])

    def add(self, item):
        self._items.append(item)


class LatexSidecaption(object):

    def __init__(self, caption, label, tocCaption=None):
        self._items = []
        self.caption = caption
        self.label = label
        self.tocCaption = tocCaption

    def __str__(self):
        environ = ''.join([r'\begin{sidecaption}',
                           (r'[%s]' % self.tocCaption) if self.tocCaption is not None else '',
                           r'{%s}' % self.caption,
                           (r'[%s]' % self.label) if self.label is not None else ''])
        return '\n'.join(
            [environ,
             r' \centering\small',
             '\n\\vskip 0.5em plus 0.5em minus 0em\n'.join([str(x) for x in self._items]),
             r'\end{sidecaption}'])

    def add(self, item):
        self._items.append(item)


class LatexSidelegend(object):

    def __init__(self, caption):
        self._items = []
        self.caption = caption

    def __str__(self):
        return '\n'.join(
            [r'\begin{sidelegend}{%s}' % self.caption,
             r' \centering\small',
             '\n\\vskip 0.5em plus 0.5em minus 0em\n'.join([str(x) for x in self._items]),
             r'\end{sidelegend}'])

    def add(self, item):
        self._items.append(item)


@contextlib.contextmanager
def LatexFile(filename):
    figure = LatexFigure()
    yield figure
    with open(filename, 'w') as f:
        f.write(str(figure))
    print('wrote latex figure to %s' % filename)


def load_data(variant, instance):
    db = sqlite.connect('/home/kevin/uni/d2/code/final/smac-v2.08.00-master-731/uranus.%s.sqlite' % (variant,))
    # dbc = db.cursor()
    # for row in dbc.execute('select instance, count(treewidth) from runs where instance = "%s"' % (instance,)):
    #     print("'%s': %s," % row)
    # db.close()
    # return
    if variant == 'MA3':
        columns = ('Popsize', 'TournamentSize', 'Mutation', 'Crossover',
                   'LsFraction', 'IlsOuterNonimpr', 'IlsInnerNonimpr', 'treewidth')
    elif variant == 'MA2':
        columns = ('Popsize', 'TournamentSize', 'Crossover',
                   'LsFraction', 'IlsOuterNonimpr', 'IlsInnerNonimpr', 'treewidth')
    elif variant == 'MA1':
        columns = ('Popsize', 'PartnerFraction', 'OpponentFraction',
                   'IlsOuterNonimpr', 'IlsInnerNonimpr', 'treewidth')
    else:
        assert False, 'unknown variant "%s"' % (variant,)
    df = pd.read_sql_query('SELECT %s FROM runs WHERE Instance = "%s"' % (','.join(columns), instance,), db)
    db.close()
    return pd.DataFrame(df, dtype=np.float32)


def create_corrplot(df, variant, instance):
    filename = '%s-correlation-corrplot-%s-crop.pdf' % (variant, instance.replace('_', '-').replace('.', '-'))
    if os.path.exists(filename):
        return
    print(filename)
    cmap = sns.blend_palette(["#00008B", "#6A5ACD", "#F0F8FF",
                              "#FFE6F8", "#C71585", "#8B0000"], as_cmap=True)
    labelled_df = df[:]
    labelled_df.columns = [short_name(x) for x in df.columns]
    sns.corrplot(labelled_df, annot=False, diag_names=False, cmap=cmap)
    plt.tight_layout()
    plt.savefig(filename)
    plt.clf()
    plt.close('all')
    labelled_df = None
    gc.collect()
    subprocess.check_call(['pdfcrop', '--luatex', '--margins', '0 0 0 0', filename])


def create_regplots(df, variant, instance, latexfig):
    assert(df.columns[-1] == 'treewidth')
    instancelatexstr = r'\Instance{%s}' % instance.replace('_', r'\textunderscore{}')
    caption = (r"\gls{%s} applied to %s, %s samples"
               % (variant, instancelatexstr, SAMPLES[variant][instance]))
    label = ('%s-correlation-regplots-%s'
             % (variant, instance.replace('_', '-').replace('.', '-')))
    tocCaption = ('Parameter influence for %s when applied to %s'
                  % (variant, instancelatexstr))
    if isinstance(latexfig, str):
        latexfig_fn = latexfig
        do_write_latex_file = True
        latexfig = LatexFigure(caption, label, tocCaption)
        latexfig_content = LatexFigureContent()
        latexfig.add(latexfig_content)
    else:
        latexfig_content = LatexSidecaption(caption, label, tocCaption)
        # caption=(r"Scatter plots and linear regression models (\SI{95}{\percent}"
        #          r" confidence band), showing how \textbf{%s}'s parameters influence the"
        #          r" result when applied to the instance \textbf{%s} (%s samples)"
        #          % (variant, instancelatexstr, SAMPLES[variant][instance])),
        # caption=(r"Scatter plots and \gls{LOWESS} curves, showing how \textbf{\gls{%s}}'s"
        #          r" parameters influence the result when applied to instance \textbf{%s}"
        #          r" (%s samples)"
        latexfig.add(latexfig_content)
        do_write_latex_file = False
    ymin = math.floor(df.treewidth.min() / 10) * 10 - 1
    ymax = math.ceil(df.treewidth.max() / 10) * 10 + 1
    tunedVariables = list(RANGES[variant])
    labelled_df = df[:]
    labelled_df.columns = [short_name(x) for x in df.columns]
    with sns.axes_style('white'):
        nPlots = len(tunedVariables)
        nCols = 3
        nRows = math.ceil(nPlots / nCols)
        nPlotsLeft = nPlots
        for rowNo in range(nRows):
            filename = '%s-correlation-regplots-%s-%s.pdf' % (variant, instance.replace('_', '-').replace('.', '-'), rowNo)
            latexfig_content.add(LatexIncludegraphics(re.sub(r'\.pdf$', '-crop.pdf', filename)))
            print(filename)
            assert nPlotsLeft > 0
            if nPlotsLeft > nCols + 1:
                nColsThisRow = nCols
            elif nPlotsLeft == nCols + 1:
                nColsThisRow = nCols - 1
            else:
                nColsThisRow = nPlotsLeft
            f, axes = plt.subplots(1, nColsThisRow, sharey=True,
                                   figsize=(nColsThisRow * 2.11, 1.76), dpi=600)
            for colNo in range(nColsThisRow):
                assert nPlotsLeft > 0
                i = nPlots - nPlotsLeft
                print('(%s, %s/%s): %s of %s plots left -> plot index %s'
                      % (rowNo, colNo, nColsThisRow, nPlotsLeft, nPlots, i))
                variable = tunedVariables[i]
                p = sns.regplot(short_name(variable), short_name('treewidth'), labelled_df,
                                ax=axes[colNo], color=COLOR[variable](), lowess=True,
                                scatter_kws={"marker": "."}, line_kws={"linewidth": 1})
                if colNo > 0:
                    p.set_ylabel('')
                axes[colNo].set_ylim([ymin, ymax])
                ticks = RANGES[variant][variable]
                assert len(ticks) >= 4, 'need at least 4 ticks for variable %r' % variable
                axes[colNo].set_xlim(ticks[0], ticks[-1])
                axes[colNo].set_xticks(ticks[1:-1])
                nPlotsLeft -= 1
            f.tight_layout()
            f.subplots_adjust(wspace=.05, hspace=.05)
            plt.savefig(filename)
            plt.clf()
            plt.close('all')
            subprocess.check_call(['pdfcrop', '--luatex', '--margins', '0 0 0 0', filename],
                                  stdout=subprocess.DEVNULL)
        assert nPlotsLeft == 0
    labelled_df = None
    gc.collect()
    if do_write_latex_file:
        with open(latexfig_fn, 'w') as f:
            f.write(str(latexfig))
        print('wrote latex figure to %s' % filename)


def create_regplots_partial(df, variant, instance, latex):
    assert False, 'TODO: check create_regplots'
    labelled_df = df[:]
    ymin = math.floor(df.treewidth.min() / 10) * 10 - 1
    ymax = math.ceil(df.treewidth.max() / 10) * 10 + 1

    def plot_for_variable(variable, confoundingVariables, color):
        filename = '%s-correlation-regplots-%s-%s-crop.pdf' % (variant, instance.replace('_', '-').replace('.', '-'), variable)
        latex.add_sidelegend('\ldots for \emph{%s} when controlling for'
                             ' confounding variables' % text_name(variable),
                             re.sub(r'\.pdf$', '-crop.pdf', filename))
        if os.path.exists(filename):
            return
        print(filename)
        with sns.axes_style('white'):
            f, axes = plt.subplots(1, len(confoundingVariables), sharey=True, figsize=(len(confoundingVariables) * 3, 2.5))
            for i in range(len(confoundingVariables)):
                p = sns.regplot(short_name(variable), short_name('treewidth'), labelled_df,
                                x_partial=short_name(confoundingVariables[i]), ax=axes[i], color=color,  # color="#4C72B0",
                                scatter_kws={"marker": "."}, line_kws={"linewidth": 1})
                if i > 0:
                    p.set_ylabel('')
                axes[i].set_ylim([ymin, ymax])
                ticks = tunedVariables[variable]
                assert len(ticks) >= 4
                axes[i].set_xlim(ticks[0], ticks[-1])
                axes[i].set_xticks(ticks[1:-1])
                axes[i].add_artist(
                    mpl_toolkits.axes_grid.anchored_artists.AnchoredText('confounder: %s' % short_name(confoundingVariables[i]),
                                                                         loc=2, frameon=False, prop=dict(size=8)))
            f.tight_layout()
            f.subplots_adjust(wspace=.05, hspace=.05)
        plt.savefig(filename)
        plt.clf()
        plt.close('all')
        f, axes = None, None
        gc.collect()
        subprocess.check_call(['pdfcrop', '--luatex', '--margins', '0 0 0 0', filename],
                              stdout=subprocess.DEVNULL)

    tunedVariables = RANGES[variant]
    i = 0
    for variable in tunedVariables:
        confoundingVariables = [x for x in df.columns if x not in ['treewidth', variable] and x in tunedVariables]
        labelled_df.columns = [short_name(x) for x in df.columns]
        plot_for_variable(variable, confoundingVariables, sns.color_palette()[i])
        i += 1
    labelled_df = None
    gc.collect()


def process_instance(instance, variant):
    latexfigure_fn = ('%s-correlation-figure-%s.tex'
                      % (variant, instance.replace('_', '-').replace('.', '-')))
    if os.path.exists(latexfigure_fn):
        return
    print(latexfigure_fn)
    data = load_data(variant, instance)
    # create_corrplot(data, variant, instance)
    # with LatexFile(latexfigure_fn) as latexfig:
    #      # r'Scatter plots and linear regression models for %s when applied to'
    #      # r' \Instance{%s} (based on %s runs)'
    #      # % (variant, instance.replace('_', r'\textunderscore{}'), SAMPLES[variant][instance]),
    #      # '%s-correlation-regplots-%s'
    #      # % (variant, instance.replace('_', '-').replace('.', '-'))) as latex:
    #      create_regplots(data, variant, instance, latexfig)
    #      # create_regplots_partial(data, variant, instance, latexfig)
    create_regplots(data, variant, instance, latexfigure_fn)
    with open('%s-correlation-figures.tex' % variant, 'a') as f:
        f.write('\\input{%s/%s}\n' % (OUTPUT_DIRNAME, latexfigure_fn))


def main():
    # use latex fonts
    mpl.rc('font', **{'family': 'sans-serif', 'sans-serif': ['Computer Modern Sans serif']})
    mpl.rc('text', usetex=True)
    sns.set_context('paper')
    sns.set_palette(sns.color_palette('colorblind') + ['#872D62', '#BA8417'], n_colors=8)
    # with open('/home/kevin/uni/d2/code/final/instances/training.list', 'r') as f:
    #     lines = f.readlines()
    # instances = [line.strip()[:-4] for line in lines if not line.strip().startswith('#')]
    # these instances are those of their kind which had been involved in the most training runs:
    DSJ = {'MA3': ['DSJR500.1c'],
           'MA2': ['DSJC500.9'],
           'MA1': ['DSJC1000.9']}
    FLAT = {'MA3': ['flat300_26_0'],
            'MA2': ['flat1000_76_0'],
            'MA1': ['flat300_26_0']}
    REG = {'MA3': ['fpsol2.i.3', 'inithx.i.3', 'mulsol.i.1', 'zeroin.i.3'],
           'MA2': ['fpsol2.i.3', 'inithx.i.3', 'mulsol.i.2', 'zeroin.i.3'],
           'MA1': ['fpsol2.i.2', 'inithx.i.3', 'mulsol.i.2', 'zeroin.i.3']}
    LEI = {'MA3': ['le450_15c'],
           'MA2': ['le450_5c'],
           'MA1': ['le450_15c']}
    SCH = {'MA3': ['school1'],
           'MA2': ['school1_nsh'],
           'MA1': ['school1']}
    LAT = ['latin_square_10']
    SGB = {'MA3': ['jean', 'games120', 'miles500', 'queen10_10'],
           'MA2': ['jean', 'games120', 'miles1000', 'queen12_12'],
           'MA1': ['anna', 'games120', 'miles500', 'queen10_10']}
    MYC = {'MA3': ['myciel6'],
           'MA2': ['myciel6'],
           'MA1': ['myciel7']}
    # assert not os.path.exists('correlation-figures.tex')
    for variant in ['MA3', 'MA2', 'MA1']:
        print('---%s---' % variant)
        instances = DSJ[variant] + FLAT[variant] + REG[variant] + LEI[variant] + SCH[variant] + LAT + SGB[variant] + MYC[variant]
        for instance in instances:
            process_instance(instance, variant)
        # with open(LATEX_INCLUDE_ALL_FILE, 'a') as f:
        #     f.write('\\clearpage\n')


if __name__ == '__main__':
    main()
