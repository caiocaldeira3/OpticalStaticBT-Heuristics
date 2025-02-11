#pragma once

#include <util.hh>
#include <optbst.hh>
#include <greedy.hh>
#include <graphbissection.hh>


struct Response_t {
    double cost;
    std::vector<int> graphOrdering;
};

double testGraphOrder (const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix) {
    int nVertices = vertices.size();

    std::vector<std::vector<int>> tree(nVertices, std::vector<int>());
    buildBalancedBinaryTree(vertices, tree, {0, nVertices}, -1);

    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return treeCost(tree, reorderedDemand);
}

double testOBST (
    const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return optimalBST(vertices.size(), reorderedDemand);
}

double testGreedy (
    const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix, bool bounded = false
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return greedyConstructor(vertices.size(), reorderedDemand);
}
