import numpy as np

import matplotlib.pyplot as plt

output_path = "output"
algorithms = ["greedy", "greedy-log-gap-bissection", "obst-only", "obst-bissection", "obst-log-gap-bissection", "balanced-tree-on-bissection", "log_gap_raw"]
inputs = ["256-worst-case"]
n_tests = 10

def read_results(algorithm, input):
    with open(f"{output_path}/{input}/{algorithm}_costs.out", "r") as file:
        return [float(line.strip()) for line in file]

for input in inputs:
    results = {algorithm: read_results(algorithm, input) for algorithm in algorithms}

    means = {algorithm: np.mean(data) for algorithm, data in results.items()}
    std_devs = {algorithm: np.std(data) for algorithm, data in results.items()}

    fig, ax = plt.subplots()
    bar_width = 0.35
    index = np.arange(len(algorithms))

    bars = ax.bar(index, means.values(), bar_width, yerr=std_devs.values(), capsize=5, label='Algorithms')

    ax.set_xlabel('Algorithms')
    ax.set_ylabel('Cost')
    ax.set_title(f'Algorithm Performance with Standard Deviation on {input}')
    ax.set_xticks(index)
    ax.set_xticklabels(algorithms)
    ax.legend()

    plt.show()
