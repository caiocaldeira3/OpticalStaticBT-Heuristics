#pragma once

#include <vector>


struct ConditionalEntropy_T {
    double entropyXgivenY;
    double entropyYgivenX;
};

struct MarginalEntropy_T {
    double entropyX;
    double entropyY;
};

double computeJointEntropy (
    const std::vector<std::vector<double>>& demandMatrix
) {
    double jointEntropy = 0;
    for (auto& row: demandMatrix) {
        for (double demand: row) {
            if (demand > 0)
                jointEntropy += demand * log2(demand);
        }
    }

    return -jointEntropy;
}

ConditionalEntropy_T computeConditionalEntropy (
    const std::vector<std::vector<double>>& demandMatrix
) {
    int nVertices = demandMatrix.size();
    std::vector<double> rowSums(nVertices, 0);
    std::vector<double> colSums(nVertices, 0);
    ConditionalEntropy_T conditionalEntropy{0, 0};
    double demand;

    for (int src = 0; src < nVertices; src++) {
        for (int dst = 0; dst < demandMatrix[src].size(); dst++) {
            rowSums[src] += demandMatrix[src][dst];
            colSums[dst] += demandMatrix[src][dst];

        }
    }

    for (int src = 0; src < nVertices; src++) {
        for (int dst = 0; dst < demandMatrix[src].size(); dst++) {
            demand = demandMatrix[src][dst];
            if (demand > 0) {
                conditionalEntropy.entropyYgivenX -= demand * log2(demand / rowSums[src]);
                conditionalEntropy.entropyXgivenY -= demand * log2(demand / colSums[dst]);
            }
        }
    }

    return conditionalEntropy;
}

MarginalEntropy_T computeMarginalEntropy (
    const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<double> rowSums(demandMatrix.size(), 0);
    std::vector<double> colSums(demandMatrix[0].size(), 0);
    MarginalEntropy_T marginalEntropy{0, 0};

    for (int i = 0; i < demandMatrix.size(); i++) {
        for (int j = 0; j < demandMatrix[i].size(); j++) {
            rowSums[i] += demandMatrix[i][j];
            colSums[j] += demandMatrix[i][j];
        }
    }

    for (int i = 0; i < rowSums.size(); i++) {
        if (rowSums[i] > 0) {
            marginalEntropy.entropyX -= rowSums[i] * log2(rowSums[i]);
        }

        if (colSums[i] > 0) {
            marginalEntropy.entropyY -= colSums[i] * log2(colSums[i]);
        }
    }

    return marginalEntropy;
}
