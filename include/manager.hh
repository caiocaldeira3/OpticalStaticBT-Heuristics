#pragma once

#include <util.hh>
#include <optbst.hh>
#include <graphbissection.hh>


struct Response_t {
    double cost;
    std::vector<int> graphOrdering;
};

Response_t testRawGraphBissection (int nVertices, const std::vector<std::vector<double>>& demandMatrix) {
    std::vector<int> vertices(nVertices);
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        vertices[vIdx] = vIdx;
    }

    VectorLimits_t vectorLimits = { 0, nVertices };
    int maxDepth = ceil(log(nVertices) / log(2)) + 1;

    graphReordering(demandMatrix, vertices, vectorLimits, INF);

    std::vector<std::vector<int>> tree(nVertices, std::vector<int>());
    buildBalancedBinaryTree(vertices, tree, vectorLimits, -1);

    return {
        NEWtreeCost(tree, demandMatrix),
        vertices
    };
}

Response_t testBissectionPlusOBST (
    int nVertices, const std::vector<std::vector<double>>& demandMatrix, bool bounded = false
) {
    std::vector<int> vertices(nVertices);
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        vertices[vIdx] = vIdx;
    }

    VectorLimits_t vectorLimits = { 0, nVertices };
    int maxDepth = (bounded ? ceil(log(nVertices) / log(2)) + 1 : INF);

    graphReordering(demandMatrix, vertices, vectorLimits, maxDepth);
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return {
        optimalBST(nVertices, reorderedDemand),
        vertices
    };
}
