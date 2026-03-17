#!/usr/bin/env python3
"""Batch runner for the ``run`` executable

This helper builds a list of parameter combinations and invokes the
compiled ``bin/run`` program for each set.  It is intended to ease
experiments by automating calls to the C++ driver in ``run.cc``.

Usage examples
--------------

1. run with a hard‑coded grid of values::

    python3 scripts/batch_run.py

2. provide custom lists on the command line::

    python3 scripts/batch_run.py \
        --algorithms mloga mloggapa \
        --depths 2 4 6 \
        --iterations 10 20 \
        --datasets weights/test weights/high_locality \
        --output-dir results

The script will create the output directory if necessary and print the
command before executing it.  If a subprocess returns a nonzero exit
status the script stops immediately.

The parameters accepted by the ``run`` binary are:

* ``--algorithm`` – name of the algorithm (``mloga`` or ``mloggapa``).
* ``--max-depth`` – recursion depth for the reordering.
* ``--max-iterations`` – iterations per level (default 20 in the C++ code).
* ``--dataset-name`` – file path to the input dataset.
* ``--output-directory`` – directory to store logs produced by ``run``.

"""

import argparse
import itertools
import os
import subprocess
import sys


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Batch-run the `bin/run` executable")
    p.add_argument("--algorithms", nargs="+", default=["mloga", "mloggapa"],
                   help="list of algorithm names to try")
    p.add_argument("--depths", nargs="+", type=int, default=[1, 2],
                   help="max depths to test")
    p.add_argument("--iterations", nargs="+", type=int, default=[20],
                   help="max iterations values to test")
    p.add_argument("--datasets", nargs="+", default=["weights/test"],
                   help="dataset names or paths to feed to the program")
    p.add_argument("--output-dir", default="output_batch",
                   help="base directory where each run will write its logs")
    p.add_argument("--dry-run", action="store_true",
                   help="print commands but do not execute")
    return p.parse_args()


def main():
    args = parse_args()

    bin_path = os.path.abspath(os.path.join(os.getcwd(), "bin", "run"))
    if not os.path.isfile(bin_path):
        sys.exit(f"error: program not found at {bin_path}, compile the project first")

    os.makedirs(args.output_dir, exist_ok=True)

    combinations = list(itertools.product(
        args.algorithms,
        args.depths,
        args.iterations,
        args.datasets,
    ))

    for alg, depth, iters, dataset in combinations:
        # build an output subdirectory for this run so logs don't clobber
        name = f"{alg}_d{depth}_i{iters}_{os.path.basename(dataset)}"
        outdir = os.path.join(args.output_dir, name)
        os.makedirs(outdir, exist_ok=True)

        cmd = [bin_path,
               "--algorithm", alg,
               "--max-depth", str(depth),
               "--max-iterations", str(iters),
               "--dataset-name", dataset,
               "--output-directory", outdir]

        print("Executing: ", " ".join(cmd))
        if not args.dry_run:
            ret = subprocess.run(cmd)
            if ret.returncode != 0:
                sys.exit(f"run failed with exit code {ret.returncode}")


del __all__

if __name__ == "__main__":
    main()
