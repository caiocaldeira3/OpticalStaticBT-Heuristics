import random
import sys

import numpy as np

if len(sys.argv) != 4:
    print("the script must be called with the probabilities ")

    raise Exception

weight_name = sys.argv[1]
temporal = float(sys.argv[2])
error = float(sys.argv[3])

n_vertices = int(input())
n_messages = int(input())

n_pairs = ((n_vertices - 1) * n_vertices) // 2
weights = {}

so_far = 0
with open(f"weights/{weight_name}") as fo:
    for line in fo.readlines():
        src, dst, weight = line.split(",")
        src = int(src)
        dst = int(dst)
        weight = int(weight)
        so_far += weight

        weights[(min(src, dst), max(src, dst))] = weight

n_tests = 30
for test in range(n_tests):
    og_probs = []
    pairs = []
    for src in range(n_vertices):
        for dst in range(src + 1, n_vertices):
            pairs.append((len(pairs), src, dst))
            og_probs.append(
                max(0, weights.get((src, dst), 1) +
                np.random.choice([error, -error]))
            )

    og_probs = np.array(og_probs) / np.sum(og_probs)
    cur_probs = og_probs.copy()
    seq = []

    same_as_last = 0
    last = -1

    for _ in range(n_messages):
        chosen, src, dst = random.choices(pairs, weights=cur_probs)[0]

        cur_probs = og_probs.copy() * (1 - temporal)
        cur_probs[chosen] += temporal

        if chosen == last:
            same_as_last += 1

        last = chosen
        seq.append((src, dst))

    with open(f"input/{weight_name}-{test}.txt", "w") as fo:
        fo.write(f"{n_vertices} {n_messages}\n")
        for src, dst in seq:
            fo.write(f"{src} {dst}\n")
