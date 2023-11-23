#pragma once
#include <vector>
#include <set>
#include <queue>


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