import argparse
import pandas as pd
import numpy as np

def compute_weight_matrix(csv_path):
    # Read the CSV file into a pandas DataFrame
    df = pd.read_csv(csv_path)
    
    # Ensure the CSV has 'src', 'dst', and 'weight' columns
    if 'src' not in df.columns or 'dst' not in df.columns or 'weight' not in df.columns:
        raise ValueError("CSV file must contain 'src', 'dst', and 'weight' columns")

    # Group by 'src' and 'dst', then sum the weights for each pair
    grouped_df = df.groupby(['src', 'dst']).agg({'weight': 'sum'}).reset_index()

    # Pivot the table to create a matrix with 'src' as rows and 'dst' as columns
    weight_matrix = grouped_df.pivot(index='src', columns='dst', values='weight').fillna(0)

    return weight_matrix

def compute_joint_entropy(matrix):
    # Convert the weight matrix to a probability matrix
    total_weight = matrix.values.sum()
    prob_matrix = matrix / total_weight
    
    # Flatten the probability matrix to work with individual probabilities
    probs = prob_matrix.values.flatten()
    
    # Remove zero probabilities to avoid log(0)
    probs = probs[probs > 0]
    
    # Calculate the joint entropy
    joint_entropy = -np.sum(probs * np.log2(probs))
    
    return joint_entropy

def compute_conditional_entropy(weight_matrix):
    # Calculate the total weight
    total_weight = weight_matrix.values.sum()

    # Compute the joint probability matrix
    joint_prob_matrix = weight_matrix / total_weight

    # Compute the marginal probabilities for src (rows) and dst (columns)
    src_marginal_probs = joint_prob_matrix.sum(axis=1)
    dst_marginal_probs = joint_prob_matrix.sum(axis=0)

    # Initialize conditional entropy
    conditional_entropy = 0

    # Iterate over the matrix to calculate conditional entropy
    for src in joint_prob_matrix.index:
        for dst in joint_prob_matrix.columns:
            joint_prob = joint_prob_matrix.at[src, dst]
            if joint_prob > 0:
                cond_prob = joint_prob / src_marginal_probs[src]
                conditional_entropy -= joint_prob * np.log2(cond_prob)

    return conditional_entropy

def compute_conditional_entropy_swapped(weight_matrix):
    # Calculate the total weight
    total_weight = weight_matrix.values.sum()

    # Compute the joint probability matrix
    joint_prob_matrix = weight_matrix / total_weight

    # Compute the marginal probabilities for dst (rows after swap) and src (columns after swap)
    dst_marginal_probs = joint_prob_matrix.sum(axis=0)  # Now it's rows
    src_marginal_probs = joint_prob_matrix.sum(axis=1)  # Now it's columns

    # Initialize conditional entropy
    conditional_entropy = 0

    # Iterate over the matrix to calculate conditional entropy H(X|Y)
    for dst in joint_prob_matrix.columns:  # Swapped
        for src in joint_prob_matrix.index:  # Swapped
            joint_prob = joint_prob_matrix.at[src, dst]
            if joint_prob > 0:
                cond_prob = joint_prob / dst_marginal_probs[dst]
                conditional_entropy -= joint_prob * np.log2(cond_prob)

    return conditional_entropy

def compute_H_XY(csv_path):
    weight_matrix = compute_weight_matrix(csv_path)

    # Compute joint entropy H(X,Y)
    H_XY = compute_joint_entropy(weight_matrix)
    print("Joint Entropy H(X,Y):", H_XY)

    return H_XY

def compute_lower_bound(csv_path):
    weight_matrix = compute_weight_matrix(csv_path)
    #print("Weight Matrix:")
    #print(weight_matrix)

    conditional_entropy = compute_conditional_entropy(weight_matrix)
    print("Conditional Entropy H(Y|X):", conditional_entropy)

    conditional_entropy_swapped = compute_conditional_entropy_swapped(weight_matrix)
    print("Conditional Entropy H(X|Y):", conditional_entropy_swapped)

    # Sum of both conditional entropies
    total_conditional_entropy = conditional_entropy + conditional_entropy_swapped
    print("H(X|Y) + H(Y|X):", total_conditional_entropy)

    return total_conditional_entropy

def compute_marginal_entropy(matrix):
    total_weight = matrix.values.sum()
    joint_prob_matrix = matrix / total_weight

    # Marginal probabilities
    marginal_x = joint_prob_matrix.sum(axis=1)  # Marginal distribution of X (sum over rows)
    marginal_y = joint_prob_matrix.sum(axis=0)  # Marginal distribution of Y (sum over columns)

    # Entropy calculation
    entropy_x = -np.sum(marginal_x * np.log2(marginal_x + np.finfo(float).eps))  # Add epsilon for numerical stability
    entropy_y = -np.sum(marginal_y * np.log2(marginal_y + np.finfo(float).eps))  # Add epsilon for numerical stability

    return entropy_x, entropy_y

def compute_upper_bound(csv_path):
    weight_matrix = compute_weight_matrix(csv_path)
    #print("Weight Matrix:")
    #print(weight_matrix)

    # Compute marginal entropies
    H_X, H_Y = compute_marginal_entropy(weight_matrix)
    print("Marginal Entropy H(X):", H_X)
    print("Marginal Entropy H(Y):", H_Y)

    # Sum of marginal entropies
    total_entropy = H_X + H_Y
    print("H(X) + H(Y):", total_entropy)

    return total_entropy


if __name__ == "__main__":
    # Setup argparse to handle command line arguments
    parser = argparse.ArgumentParser(description='Compute conditional entropies from CSV file.')
    parser.add_argument('csv_file', type=str, help='Path to the CSV file containing source-destination-weight data')

    args = parser.parse_args()

    csv_path = args.csv_file
    print(csv_path)
    joint_entropy = compute_H_XY(csv_path)
    print("\n")
    lower_bound = compute_lower_bound(csv_path)
    print("\n")
    upper_bound = compute_upper_bound(csv_path)
