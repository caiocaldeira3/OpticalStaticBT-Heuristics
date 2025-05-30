#pragma once

#include <set>
#include <queue>
#include <vector>
#include <iostream>

#include <cmath>

#define DEBUG std::cout<<"---------------------------------------"<<std::endl

struct VectorLimits_t {
    int leftLimit;
    int rightLimit;
};

static const long long INF = 0x3f3f3f3f;
static const long long LINF = 0x3f3f3f3f3f3f3f3f;
static const double EPS = 1e-10;
typedef std::pair<int,int> query;

bool isClose (double a, double b, double eps = EPS) {
    return std::abs(a - b) < eps;
}

void computeDistances (
    int nVertices, const std::vector<std::vector<int>>& tree, std::vector<std::vector<int>>& distances
) {
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        std::queue<std::pair<int, std::pair<int, int>>> vis;
        vis.push({ 0, { vIdx, -1 } });

        while (!vis.empty()) {
            auto [cost, vertices] = vis.front(); vis.pop();
            auto [cV, lV] = vertices;
            distances[vIdx][cV] = cost;

            for (int nV: tree[cV]) {
                if (nV == lV)
                    continue;

                vis.push({ cost + 1, { nV, cV } });

            }
        }
    }
}

void buildBalancedBinaryTree (
    const std::vector<int>& vertices,
    std::vector<std::vector<int>>& tree, VectorLimits_t limits, int parent
) {
    if (limits.leftLimit == limits.rightLimit)
        return;

    int root = (limits.leftLimit + limits.rightLimit) / 2;
    if (parent != -1) {
        tree[parent].push_back(vertices[root]);
        tree[vertices[root]].push_back(parent);
    }

    buildBalancedBinaryTree(vertices, tree, { limits.leftLimit, root }, vertices[root]);
    buildBalancedBinaryTree(vertices, tree, { root + 1, limits.rightLimit }, vertices[root]);
}

std::vector<std::vector<double>> reconfigureDemandMatrix (
    const std::vector<int> graphNewOrder, const std::vector<std::vector<double>>& demandMatrix
) {
    int nVertices = graphNewOrder.size();
    std::vector<std::vector<double>> newDemandMatrix(nVertices, std::vector<double>(nVertices, 0.0));

    for (int src = 0; src < nVertices; src++) {
        for (int dst = 0; dst < nVertices; dst++) {
            newDemandMatrix[src][dst] = demandMatrix[graphNewOrder[src]][graphNewOrder[dst]];
        }
    }

    return newDemandMatrix;
}

double treeCost (
    const std::vector<std::vector<int>>& tree,
    const std::vector<std::vector<double>>& demandMatrix
) {
    int nVertices = tree.size();
    std::vector<std::vector<int>> distances(nVertices, std::vector<int>(nVertices, INF));
    computeDistances(nVertices, tree, distances);
    double totalCost = 0;

    for (int src = 0; src < nVertices; src++) {
        for (int dst = 0; dst < nVertices; dst++) {
            totalCost += distances[src][dst] * demandMatrix[src][dst];

        }
    }

    return totalCost;
}
