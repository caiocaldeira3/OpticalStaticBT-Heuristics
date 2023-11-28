#pragma once

#include <set>
#include <queue>
#include <vector>


static const int INF = 0x3f3f3f3f;
typedef std::pair<int,int> query;

bool isValidBinaryTree (const std::vector<int>& pred, bool debug = false) {
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
        if (cV == -1) {
            if (debug)
                std::cout << "pred vertex " << cV << " after root was -1" << std::endl;

            return false;

        } else if (visited[cV]) {
            if (debug)
                std::cout << "tree with cycles" << std::endl;

            return false;

        } else if (tree[cV].size() > 2) {
            if (debug) {
                std::cout << cV << " -> ";
                for (int child: tree[cV]) {
                    std::cout << child << ", ";
                }

                std::cout << std::endl;
                std::cout << "vertex " << cV << " with more than 2 children" << std::endl;
            }

            return false;

        }

        visited[cV] = true;
        for (int nV: tree[cV]) {
            q.push(nV);
        }
    }

    for (bool vis: visited) {
        if (!vis) {
            if (debug)
                std::cout << "not all vertices were visited" << std::endl;

            return false;

        }
    }

    return true;
}

std::vector<std::vector<int>> buildOccurrences (
    int nVertices, const std::vector<query>& queries
) {
    std::vector<std::vector<int>> occ(nVertices, std::vector<int>(nVertices, 0));
    for (const auto [src, dst]: queries) {
        occ[src][dst]++;
        occ[dst][src]++;

    }

    return occ;
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

int getPreLCA (int idx, int odx, const std::vector<int>& pred) {
    std::queue<int> q;
    q.push(idx);
    q.push(odx);

    while (!q.empty()) {
        int nxt = q.front(); q.pop();
        if (pred[nxt] == idx) {
            return nxt;

        } else if (pred[nxt] == odx) {
            return nxt;

        } else if (pred[nxt] == -1) {
            continue;

        }

        q.push(pred[nxt]);
    }

    return -1;
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