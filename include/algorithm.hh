#pragma once

#include <cmath>
#include <vector>

namespace algorithm
{

    double computeMLogACost(
        const std::vector<int>& vertices,
        const std::vector<std::vector<double>>& demandMatrix
    ) {
        // Iterate edges and compute the cost based on the MLogA algorithm
        double totalCost = 0.0;
        for (int i = 0; i < vertices.size(); ++i) {
            for (int j = i + 1; j < vertices.size(); ++j) {
                int src = vertices[i];
                int dst = vertices[j];
                totalCost += demandMatrix[src][dst] * std::log2(std::abs(i - j));
            }
        }
        return totalCost;
    }
        
} // namespace algorithm
    