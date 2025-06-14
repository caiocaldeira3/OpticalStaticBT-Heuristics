#pragma once

#include <vector>
#include <unordered_map>

namespace convertgraph {

using bipartiteGraph = std::vector<std::unordered_map<int, double>>;

bipartiteGraph convertGraphToBipartiteGraphMLogA(const std::vector<std::vector<double>>& demandMatrix) {
    int nVertices = demandMatrix.size();
    bipartiteGraph bGraph;
    
    for (int i = 0; i < nVertices; ++i) {
        for (int j = i + 1; j < nVertices; ++j) {
            if (demandMatrix[i][j] != 0.0) {
                std::unordered_map<int, double> adjacentVertices;

                // Connect edge node to vertices i and j
                adjacentVertices[i] = demandMatrix[i][j];
                adjacentVertices[j] = demandMatrix[i][j];

                bGraph.push_back(adjacentVertices);
            }
        }
    }

    return bGraph;
}

bipartiteGraph convertGraphToBipartiteGraphMLogGapA(const std::vector<std::vector<double>>& demandMatrix) {
    int nVertices = demandMatrix.size();
    bipartiteGraph bGraph;
    
    for (int i = 0; i < nVertices; ++i) {
        std::unordered_map<int, double> adjacentVertices;

        for (int j = 0; j < nVertices; ++j) {
            if (i != j && demandMatrix[i][j] != 0.0) {
                adjacentVertices[j] = demandMatrix[i][j];                
            }
        }

        bGraph.push_back(adjacentVertices);
    }

    return bGraph;
}

}
