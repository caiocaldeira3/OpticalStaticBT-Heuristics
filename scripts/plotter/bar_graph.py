import argparse
import pandas as pd
import matplotlib.pyplot as plt

# Parse command-line argument
parser = argparse.ArgumentParser(description='Plot bar graph of total_cost per algorithm.')
parser.add_argument('filename', type=str, help='Path to the CSV file')
args = parser.parse_args()

# Load the CSV
df = pd.read_csv(args.filename)

# Plot bar graph
plt.figure(figsize=(10, 6))
plt.bar(df['algorithm'], df['total_cost'], color='skyblue')

# Labels and title
plt.xlabel('Algorithm')
plt.ylabel('Total Cost')
plt.title('Total Cost per Algorithm')
plt.xticks(rotation=45)
plt.tight_layout()
plt.grid(axis='y')

# Show the plot
plt.show()
