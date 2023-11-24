#pragma once

#include <set>
#include <queue>
#include <vector>


static const int INF = 0x3f3f3f3f;
typedef std::pair<int,int> query;

bool isValidBinaryTree (const std::vector<int>& pred) {
    std::vector<std::vector<int>> tree = std::vector<std::vector<int>>(
        pred.size(), std::vector<int>()
    );
    std::vector<bool> visited = std::vector<bool>(pred.size(), false);

    int root = -1;
    for (int i = 0; i < pred.size(); i++) {
        if (pred[i] == -1) {
            root = i;

        } else {
            tree[pred[i]].push_back(i);

        }
    }

    std::queue<int> q = std::queue<int>();
    q.push(root);

    while (!q.empty()) {
        int cV = q.front(); q.pop();
        if (cV == -1 || visited[cV] || tree[cV].size() > 2) {
            return false;

        }

        visited[cV] = true;
        for (int nV: tree[cV]) {
            q.push(nV);
        }
    }

    for (bool vis: visited) {
        if (!vis)
            return false;

    }

    return true;
}

void insertVertexOnClosestToRoot (
    int vIdx, int& totalCost, std::vector<std::vector<int>>& distances,
    const std::vector<std::vector<int>>& occs, std::vector<int>& pred,
    std::priority_queue<
        std::pair<rankAndTB, leafsAndNode>,
        std::vector<std::pair<rankAndTB, leafsAndNode>>,
        CompareHuffmanRank
    >& leafes
) {
    if (pred[vIdx] != INF)
        return;

    else if (leafes.size() == 0) {
        pred[vIdx] = -1;
        leafes.push({{ 0, rand() }, { vIdx, 2 } });

        return;

    }

    auto [ rankTb, leafNode ] = leafes.top(); leafes.pop();
    if (leafNode.second > 1) {
        leafes.push({ rankTb, { leafNode.first, leafNode.second - 1 }});

    }


    leafes.push({{ rankTb.first + 1, rand() }, { vIdx, 2 }});
    pred[vIdx] = leafNode.first;

    for (int dst = 0; dst < occs[vIdx].size(); dst++) {
        if (pred[dst] == INF || vIdx == dst)
            continue;

        distances[vIdx][dst] = distances[leafNode.first][dst] + 1;
        distances[dst][vIdx] = distances[dst][leafNode.first] + 1;

        totalCost += distances[vIdx][dst] * occs[vIdx][dst];
    }
}

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

int treeCost (const std::vector<int>& preds, const std::vector<query>& queries) {
    int nVertices = preds.size();
    std::vector<std::vector<int>> distances(nVertices, std::vector<int>(nVertices, INF));
    std::vector<std::vector<int>> tree(nVertices);
    int root;
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (preds[vIdx] == -1) {
            root = vIdx;

        } else {
            tree[preds[vIdx]].push_back(vIdx);
            tree[vIdx].push_back(preds[vIdx]);

        }
    }

    computeDistances(nVertices, tree, distances);
    int totalCost = 0;

    for (query qry: queries) {
        totalCost += distances[qry.first][qry.second];

    }

    return totalCost;
}