import argparse
import pandas as pd
import matplotlib.pyplot as plt

# Argument parser setup
parser = argparse.ArgumentParser(description='Plot total_cost vs max_depth for each algorithm.')
parser.add_argument('filename', type=str, help='Path to the CSV file')
args = parser.parse_args()

# Load the CSV
df = pd.read_csv(args.filename)

# Group by dataset
datasets = df['data_set'].unique()

# Plot
plt.figure(figsize=(10, 6))

for dataset in datasets:
    dataset_df = df[df['data_set'] == dataset]
    # Sort by max_depth to ensure line is plotted in order
    dataset_df = dataset_df.sort_values('max_depth')
    plt.plot(dataset_df['max_depth'], dataset_df['total_cost'], label=dataset)

# Labels and legend
plt.xlabel('Max Depth')
# plt.ylabel('Total Cost')
# plt.title('Total Cost vs Max Depth per Dataset')
plt.ylabel('MLogA Cost')
plt.title('MLogA Cost vs Max Depth per Dataset')
plt.legend(title='Datasets')
plt.grid(True)
plt.tight_layout()

# Show the plot
plt.show()
