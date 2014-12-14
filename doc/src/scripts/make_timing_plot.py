#!/usr/bin/env python
# encoding: utf-8
import sys
import os

def get_best_previous_result(instance, prevResultsFn):
    with open(prevResultsFn, "r") as f:
        line = None
        while line != "":
            line = f.readline()
            cols = line.strip().split(",")
            if cols[0] == instance:
                return min([int(x) for x in cols[3:] if x.isdigit()])
    assert(False)

if len(sys.argv) != 3:
    print("usage: %s [instance-name] [prev-results file]" % sys.argv[0])
    sys.exit(1)
instance = sys.argv[1]
prevResultsFn = sys.argv[2]
prev_best = get_best_previous_result(instance, prevResultsFn)

instance_fn = "./timings_gails/%s" % instance
latex_gails = r"""
      \pgfplotsinvokeforeach{1,...,10} {
         \addplot[const plot, color=blue, opacity=0.4]
         table {%(instance_fn)s.#1};
      }
      \addplot[smooth, very thick, color=blue] table {%(instance_fn)s};
      \addlegendentry{GA-ILS}
""" % {"instance_fn": instance_fn} if os.path.isfile(instance_fn) else "% no data available for GA-ILS"

instance_fn = "./timings_memetic/%s" % instance
latex_memetic = r"""
      \pgfplotsinvokeforeach{1,...,10} {
         \addplot[const plot, color=red, opacity=0.4]
         table {%(instance_fn)s.#1};
      }
      \addplot[smooth, very thick, color=red] table {%(instance_fn)s};
      \addlegendentry{Memetic}
""" % {"instance_fn": instance_fn} if os.path.isfile(instance_fn) else "% no data available for Memetic"

latex_prev_best = r"""
      \addplot[dotted, color=brown]
      coordinates {(0,%(prev_best)s) (1000000,%(prev_best)s)};
""" % {"prev_best": prev_best}

latex = r"""
\begin{tikzpicture}
   \begin{axis}[
         title=%(title)s,
         xlabel={runtime in seconds},
         xmin=0,
         xmax=1000000, %% ms
         scaled x ticks=manual:{}{\pgfmathparse{#1/1000}},
         xticklabel style={/pgf/number format/fixed,
                           /pgf/number format/precision=1},
         ylabel={treewidth},
         width=0.7\textwidth,
      ]
      %(gails)s
      %(memetic)s
      %(prev_best)s
   \end{axis}
\end{tikzpicture}
""" % {"title": instance.replace("_", r"\texttt{\textunderscore}"),
       "gails": latex_gails,
       "memetic": latex_memetic,
       "prev_best": latex_prev_best}
print(latex)
