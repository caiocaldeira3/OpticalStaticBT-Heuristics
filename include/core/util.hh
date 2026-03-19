#pragma once

#include <set>
#include <queue>
#include <vector>
#include <iostream>

#include <cmath>

#define DEBUG std::cout<<"---------------------------------------"<<std::endl

struct VectorLimits_t {
    uint32_t leftLimit;
    uint32_t rightLimit;
};

static const long long INF = 0x3f3f3f3f;
static const long long LINF = 0x3f3f3f3f3f3f3f3f;
static const double EPS = 1e-10;
typedef std::pair<int,int> query;

inline bool isClose (double a, double b, double eps = EPS) {
    return std::abs(a - b) < eps;
}

inline void computeDistances (
    uint32_t nVertices, const std::vector<std::vector<uint32_t>>& tree, std::vector<std::vector<uint32_t>>& distances
) {
    for (uint32_t vIdx = 0; vIdx < nVertices; vIdx++) {
        std::queue<std::pair<uint32_t, std::pair<uint32_t, uint32_t>>> vis;
        vis.push({ 0, { vIdx, -1 } });

        while (!vis.empty()) {
            auto [cost, vertices] = vis.front(); vis.pop();
            auto [cV, lV] = vertices;
            distances[vIdx][cV] = cost;

            for (uint32_t nV: tree[cV]) {
                if (nV == lV)
                    continue;

                vis.push({ cost + 1, { nV, cV } });

            }
        }
    }
}

inline void buildBalancedBinaryTree (
    const std::vector<uint32_t>& vertices,
    std::vector<std::vector<uint32_t>>& tree, VectorLimits_t limits, uint32_t parent
) {
    if (limits.leftLimit == limits.rightLimit)
        return;

    uint32_t root = (limits.leftLimit + limits.rightLimit) / 2;
    if (parent != -1) {
        tree[parent].push_back(vertices[root]);
        tree[vertices[root]].push_back(parent);
    }

    buildBalancedBinaryTree(vertices, tree, { limits.leftLimit, root }, vertices[root]);
    buildBalancedBinaryTree(vertices, tree, { root + 1, limits.rightLimit }, vertices[root]);
}

inline std::vector<std::vector<double>> reconfigureDemandMatrix (
    const std::vector<uint32_t> graphNewOrder, const std::vector<std::vector<double>>& demandMatrix
) {
    uint32_t nVertices = graphNewOrder.size();
    std::vector<std::vector<double>> newDemandMatrix(nVertices, std::vector<double>(nVertices, 0.0));

    for (uint32_t src = 0; src < nVertices; src++) {
        for (uint32_t dst = 0; dst < nVertices; dst++) {
            newDemandMatrix[src][dst] = demandMatrix[graphNewOrder[src]][graphNewOrder[dst]];
        }
    }

    return newDemandMatrix;
}

inline double treeCost (
    const std::vector<std::vector<uint32_t>>& tree,
    const std::vector<std::vector<double>>& demandMatrix
) {
    uint32_t nVertices = tree.size();
    std::vector<std::vector<uint32_t>> distances(nVertices, std::vector<uint32_t>(nVertices, INF));
    computeDistances(nVertices, tree, distances);
    double totalCost = 0;

    for (uint32_t src = 0; src < nVertices; src++) {
        for (uint32_t dst = 0; dst < nVertices; dst++) {
            totalCost += distances[src][dst] * demandMatrix[src][dst];

        }
    }

    return totalCost;
}
