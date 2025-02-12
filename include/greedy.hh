#pragma once

#include <map>
#include <vector>
#include <cstdlib>
#include <assert.h>
#include <algorithm>
#include <util.hh>


std::vector<int> rank;

class CompareRank {
    public:

    bool operator() (const std::pair<int, int> x, const std::pair<int, int> y) const {
        return rank[x.first] > rank[y.first];

    }
};

void insertVertexGreedily (
    int vIdx, double& totalCost, std::map<int, int>& leafes, std::vector<std::vector<int>>& distances,
    const std::vector<std::vector<double>>& demandMatrix, std::vector<int>& pred, std::vector<int>& rank
) {
    if (pred[vIdx] != INF)
        return;

    else if (leafes.size() == 0) {
        rank[vIdx] = 0;
        pred[vIdx] = -1;
        leafes[vIdx] = 2;

        return;

    }

    double pMin = LINF;
    double pIdx = -1;

    for (const auto [leaf, degree]: leafes) {
        double cCost = 0;

        for (int dst = 0; dst < demandMatrix[vIdx].size(); dst++) {
            if (pred[dst] == INF)
                continue;

            cCost += (distances[leaf][dst] + 1) * demandMatrix[vIdx][dst];
            cCost += (distances[leaf][dst] + 1) * demandMatrix[dst][vIdx];

        }

        if (cCost < pMin) {
            pMin = cCost;
            pIdx = leaf;
        }
    }

    auto it = leafes.find(pIdx);
    if (it->second == 1) {
        leafes.erase(it);

    } else {
        it->second--;

    }

    totalCost += pMin;
    leafes[vIdx] = 2;
    pred[vIdx] = pIdx;
    rank[vIdx] = rank[pIdx] + 1;
    for (int dst = 0; dst < demandMatrix[vIdx].size(); dst++) {
        if (pred[dst] == INF || vIdx == dst)
            continue;

        distances[vIdx][dst] = distances[pIdx][dst] + 1;
        distances[dst][vIdx] = distances[dst][pIdx] + 1;
    }
}

double greedyConstructor (
    int nVertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<int>> distances(nVertices, std::vector<int>(nVertices, 0));
    std::vector<int> pred(nVertices, INF);
    rank = std::vector<int>(nVertices, INF);
    std::map<int, int> leafes;
    std::set<int> insertedVertices;

    double totalCost = 0;

    std::vector<std::pair<double, std::pair<int, int>>> queries;
    for (int src = 0; src < nVertices; src++) {
        for (int dst = 0; dst < nVertices; dst++) {
            queries.push_back({ -demandMatrix[src][dst], { src, dst } });

        }
    }
    std::sort(queries.begin(), queries.end());

    for (const auto& query : queries) {
        auto [src, dst] = query.second;
        insertVertexGreedily(src, totalCost, leafes, distances, demandMatrix, pred, rank);
        insertVertexGreedily(dst, totalCost, leafes, distances, demandMatrix, pred, rank);

        insertedVertices.insert(src);
        insertedVertices.insert(dst);
        if (insertedVertices.size() == nVertices)
            break;
    }

    return totalCost;
}
