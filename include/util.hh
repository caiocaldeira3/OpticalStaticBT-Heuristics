#pragma once
#include <vector>
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