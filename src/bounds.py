import argparse
import pandas as pd
import numpy as np

def compute_demand_matrix (matrix_path: str) -> np.ndarray[np.ndarray]:
    with open(f"weights/{matrix_path}", 'r') as f:
        n_vertices = int(f.readline())
        matrix = np.zeros((n_vertices, n_vertices))
        total_weight = 0

        for line in f:
            src, dst, weight = line.split()
            src = int(src)
            dst = int(dst)
            weight = float(weight)

            matrix[src][dst] = weight
            total_weight += weight

    return matrix / total_weight

def compute_joint_entropy (matrix: np.ndarray[np.ndarray]) -> np.float64:
    probs = matrix.flatten()

    # Remove zero probabilities to avoid log(0)
    probs = probs[probs > 0]

    # Calculate the joint entropy
    joint_entropy = -np.sum(probs * np.log2(probs))

    return joint_entropy

def compute_conditional_entropy (matrix: np.ndarray[np.ndarray]) -> np.float64:
    # Compute the marginal probabilities for src (rows) and dst (columns)
    src_marginal_probs = matrix.sum(axis=1)
    # dst_marginal_probs = matrix.sum(axis=0) Check if necessary

    # Initialize conditional entropy
    conditional_entropy = 0

    # Iterate over the matrix to calculate conditional entropy
    for src in range(matrix.shape[0]):
        for dst in range(matrix.shape[1]):
            joint_prob = matrix[src, dst]
            if joint_prob > 0:
                cond_prob = joint_prob / src_marginal_probs[src]
                conditional_entropy -= joint_prob * np.log2(cond_prob)

    return conditional_entropy

def compute_conditional_entropy_swapped (matrix: np.ndarray[np.ndarray]) -> np.float64:
    # Compute the marginal probabilities for dst (rows after swap) and src (columns after swap)
    dst_marginal_probs = matrix.sum(axis=0)  # Now it's rows
    # src_marginal_probs = matrix.sum(axis=1)  # Now it's columns

    # Initialize conditional entropy
    conditional_entropy = 0

    # Iterate over the matrix to calculate conditional entropy H(X|Y)
    for dst in range(matrix.shape[1]):  # Swapped
        for src in range(matrix.shape[0]):  # Swapped
            joint_prob = matrix[src, dst]
            if joint_prob > 0:
                cond_prob = joint_prob / dst_marginal_probs[dst]
                conditional_entropy -= joint_prob * np.log2(cond_prob)

    return conditional_entropy

def compute_H_XY (matrix_path: str) -> np.float64:
    demand_matrix = compute_demand_matrix(matrix_path)

    # Compute joint entropy H(X,Y)
    H_XY = compute_joint_entropy(demand_matrix)
    print("Joint Entropy H(X,Y):", H_XY)

    return H_XY

def compute_lower_bound (csv_path: str) -> np.float64:
    demand_matrix = compute_demand_matrix(csv_path)

    conditional_entropy = compute_conditional_entropy(demand_matrix)
    print("Conditional Entropy H(Y|X):", conditional_entropy)

    conditional_entropy_swapped = compute_conditional_entropy_swapped(demand_matrix)
    print("Conditional Entropy H(X|Y):", conditional_entropy_swapped)

    # Sum of both conditional entropies
    total_conditional_entropy = conditional_entropy + conditional_entropy_swapped
    print("H(X|Y) + H(Y|X):", total_conditional_entropy)

    return total_conditional_entropy

def compute_marginal_entropy (matrix: np.ndarray[np.ndarray]) -> tuple[np.float64, np.float64]:
    # Marginal probabilities
    marginal_x = matrix.sum(axis=1)  # Marginal distribution of X (sum over rows)
    marginal_y = matrix.sum(axis=0)  # Marginal distribution of Y (sum over columns)

    # Entropy calculation
    entropy_x = -np.sum(marginal_x * np.log2(marginal_x + np.finfo(float).eps))  # Add epsilon for numerical stability
    entropy_y = -np.sum(marginal_y * np.log2(marginal_y + np.finfo(float).eps))  # Add epsilon for numerical stability

    return entropy_x, entropy_y

def compute_upper_bound (matrix_path) -> np.float64:
    demand_matrix = compute_demand_matrix(matrix_path)

    # Compute marginal entropies
    H_X, H_Y = compute_marginal_entropy(demand_matrix)
    print("Marginal Entropy H(X):", H_X)
    print("Marginal Entropy H(Y):", H_Y)

    # Sum of marginal entropies
    total_entropy = H_X + H_Y
    print("H(X) + H(Y):", total_entropy)

    return total_entropy


if __name__ == "__main__":
    # Setup argparse to handle command line arguments
    parser = argparse.ArgumentParser(description='Compute conditional entropies from demand matrix.')
    parser.add_argument('matrix_name', type=str, help='Name of the demand matrix file inside weights folder')

    args = parser.parse_args()

    matrix_name = args.matrix_name
    joint_entropy = compute_H_XY(matrix_name)
    print("\n")
    lower_bound = compute_lower_bound(matrix_name)
    print("\n")
    upper_bound = compute_upper_bound(matrix_name)
