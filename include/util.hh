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
