import pandas as pd
import matplotlib.pyplot as plt

# Argument parser setup
parser = argparse.ArgumentParser(description='Plot total_cost vs max_depth for each algorithm.')
parser.add_argument('filename', type=str, help='Path to the CSV file')
args = parser.parse_args()

# Load the CSV
df = pd.read_csv(args.filename)

# Group by algorithm
algorithms = df['algorithm'].unique()

# Plot
plt.figure(figsize=(10, 6))

for algo in algorithms:
    algo_df = df[df['algorithm'] == algo]
    # Sort by max_depth to ensure line is plotted in order
    algo_df = algo_df.sort_values('max_depth')
    plt.plot(algo_df['max_depth'], algo_df['total_cost'], label=algo)

# Labels and legend
plt.xlabel('Max Depth')
plt.ylabel('Total Cost')
plt.title('Total Cost vs Max Depth per Algorithm')
plt.legend(title='Algorithm')
plt.grid(True)
plt.tight_layout()

# Show the plot
plt.show()
