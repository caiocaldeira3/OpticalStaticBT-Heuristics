import sys

def generate_edges(n):
    edges = set()

    # First set of edges: (i, n/2 + i) for i in [0, n/2]
    for i in range(0, n//2):
        edges.add((i, n//2 + i))

    # Second set of edges: (i+1, n/2 + i) for i in [0, n/2-1]
    for i in range(0, n//2 -1):
        edges.add((i + 1, n//2 + i))

    return edges

n = int(sys.argv[1])
edges = generate_edges(n)

filename = f"{n}-worst-case"

with open(f"weights/{filename}", "w") as file:
    file.write(f"{n}\n")
    for edge in sorted(edges):
        file.write(f"{edge[0]} {edge[1]} {1}\n")

print(f"Edges saved to {filename}")
