import numpy as np

import matplotlib.pyplot as plt
import os

output_path = "output"
# define ordering flags and tree builders to match nested folder structure
ordering_flags = ["noop", "onehop"]
tree_builders = ["raw", "greedy", "obst"]
inputs = ["256-worst-case", "200-40-40"]
# n_tests no longer used directly

os.makedirs("plots/", exist_ok=True)

def read_results(algorithm, input):
    # open nested files under flag and builder directory
    flag, builder = algorithm
    with open(f"{output_path}/{input}/{flag}/{builder}_costs.out", "r") as file:
        return [float(line.strip()) for line in file]

for input in inputs:
    save_dir = f"plots/{input}"
    os.makedirs(save_dir, exist_ok=True)

    # iterate through each tree builder to plot costs per ordering flag
    for builder in tree_builders:
        # collect results for each ordering flag
        results = {flag: read_results((flag, builder), input) for flag in ordering_flags}
        means = {flag: np.mean(data) for flag, data in results.items()}
        std_devs = {flag: np.std(data) for flag, data in results.items()}

        fig, ax = plt.subplots()
        bar_width = 0.35
        index = np.arange(len(ordering_flags))

        ax.bar(index, means.values(), bar_width, yerr=std_devs.values(), capsize=5)
        ax.set_xlabel('Ordering Algorithm')
        ax.set_ylabel('Cost')
        ax.set_title(f'{builder.capitalize()} Tree Builder Costs on {input}')
        ax.set_xticks(index)
        ax.set_xticklabels(ordering_flags)

        fig.savefig(f"{save_dir}/{builder}_cost.png")
        plt.close(fig)

# plot ordering metrics (one figure per metric)
metrics_names = ["avg-iterations", "avg-swapped-pairs", "avg-cost-gain"]

def read_metrics(flag, input):
    # reads metrics.out, expects header then rows per test
    with open(f"{output_path}/{input}/{flag}/metrics.out", "r") as file:
        lines = file.read().strip().splitlines()
    header = lines[0].split(",")
    rows = [list(map(float, line.split(","))) for line in lines[1:]]
    return {header[i]: [row[i] for row in rows] for i in range(len(header))}

for input in inputs:
    save_dir = f"plots/{input}"
    os.makedirs(save_dir, exist_ok=True)

    for metric in metrics_names:
        # collect metric values per ordering flag
        results = {flag: read_metrics(flag, input)[metric] for flag in ordering_flags}
        means = [np.mean(vals) for vals in results.values()]
        stds = [np.std(vals) for vals in results.values()]

        fig, ax = plt.subplots()
        index = np.arange(len(ordering_flags))
        ax.bar(index, means, 0.35, yerr=stds, capsize=5)
        ax.set_xlabel('Ordering Algorithm')
        ax.set_ylabel(metric)
        ax.set_title(f'{metric.replace("-", " ").capitalize()} on {input}')
        ax.set_xticks(index)
        ax.set_xticklabels(ordering_flags)

        fig.savefig(f"{save_dir}/{metric}.png")
        plt.close(fig)


