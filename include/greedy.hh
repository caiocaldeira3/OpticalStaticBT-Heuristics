#pragma once

#include <map>
#include <vector>
#include <cstdlib>
#include <assert.h>
#include <algorithm>
#include "util.hh"


std::vector<int> rank;

class CompareRank {
    public:

    bool operator() (const std::pair<int, int> x, const std::pair<int, int> y) const {
        return rank[x.first] > rank[y.first];

    }
};

void insertVertexGreedily (
    int vIdx, int& totalCost, std::map<int, int>& leafes, std::vector<std::vector<int>>& distances,
    const std::vector<std::vector<int>>& occs, std::vector<int>& pred, std::vector<int>& rank
) {
    if (pred[vIdx] != INF)
        return;

    else if (leafes.size() == 0) {
        rank[vIdx] = 0;
        pred[vIdx] = -1;
        leafes[vIdx] = 2;

        return;

    }

    int pMin = INF;
    int pIdx = -1;

    for (const auto [leaf, degree]: leafes) {
        int cCost = 0;

        for (int dst = 0; dst < occs[vIdx].size(); dst++) {
            if (pred[dst] == INF)
                continue;

            cCost += (distances[leaf][dst] + 1) * occs[vIdx][dst];

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
    for (int dst = 0; dst < occs[vIdx].size(); dst++) {
        if (pred[dst] == INF || vIdx == dst)
            continue;

        distances[vIdx][dst] = distances[pIdx][dst] + 1;
        distances[dst][vIdx] = distances[dst][pIdx] + 1;
    }
}

std::vector<int> greedyConstructor (
    std::vector<query> queries, const int nVertices, int& totalCost
) {
    std::vector<std::vector<int>> distances(nVertices, std::vector<int>(nVertices, 0));
    std::vector<std::vector<int>> occ(nVertices, std::vector<int>(nVertices, 0));
    std::vector<int> pred(nVertices, INF);
    rank = std::vector<int>(nVertices, INF);
    std::map<int, int> leafes;
    totalCost = 0;

    for (const auto [src, dst]: queries) {
        occ[src][dst]++;
        occ[dst][src]++;

    }

    for (auto [src, dst]: queries) {
        if (pred[src] == INF && pred[dst] == INF && rand() % 2 == 1) {
            std::swap(src, dst);

        }

        insertVertexGreedily(src, totalCost, leafes, distances, occ, pred, rank);
        insertVertexGreedily(dst, totalCost, leafes, distances, occ, pred, rank);

    }


    std::priority_queue<
        std::pair<int, int>, std::vector<std::pair<int, int>>, CompareRank
    > avLeafes;

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (pred[vIdx] == INF)
            continue;

        auto it = leafes.find(vIdx);
        if (it == leafes.end())
            continue;

        avLeafes.push({ it->first, it->second });
    }

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (pred[vIdx] != INF)
            continue;

        std::pair<int, int> leaf = avLeafes.top(); avLeafes.pop();
        if (leaf.second == 2) {
            avLeafes.push({ leaf.first, 1 });

        }

        pred[vIdx] = leaf.first;
        rank[vIdx] = rank[leaf.first];
        avLeafes.push({ vIdx, 2 });

    }

    assert (isValidBinaryTree(pred));

    return pred;
}