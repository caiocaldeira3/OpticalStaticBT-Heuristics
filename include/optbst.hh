#pragma once

#include <util.hh>
#include <assert.h>


struct IntResp_t{
    int cost;
    int root;
};

std::vector<std::vector<int>> buildAggregateDemand (
    int nVertices, std::vector<std::vector<int>> weights
) {
    std::vector<std::vector<int>> nodeWeight(nVertices, std::vector<int>(nVertices, 0));
    std::vector<std::vector<int>> aggDemand(nVertices, std::vector<int>(nVertices, 0));
    std::vector<std::vector<int>> teste(nVertices, std::vector<int>(nVertices, 0));

    for (int i = 0; i < nVertices; i++) {
        for (int j = 0; j < nVertices; j++) {
            nodeWeight[i][j] = weights[i][j] + (j == 0 ? 0 : nodeWeight[i][j - 1]);
        }
    }

    for (int i = 0; i < nVertices; i++) {
        for (int j = 0; j < nVertices; j++) {
            for (int k = i; k <= j; k++) {
                aggDemand[i][j] += (
                    nodeWeight[k][nVertices - 1] - nodeWeight[k][j] + (i == 0 ? 0 : nodeWeight[k][i - 1])
                );
            }
        }
    }

    return aggDemand;
}

IntResp_t buildOptimalBST (
    int i, int j, std::vector<std::vector<int>> aggr, std::vector<std::vector<IntResp_t>>& intervals
) {
    int nVertice = j + 1;
    IntResp_t root{ INF, -1 };

    for (int i = 0; i < nVertice; i++)
        intervals[i][i] = IntResp_t{ 0, i };

    for (int delta = 1; delta < nVertice; delta++) {
        for (int i = 0; i < nVertice; i++) {
            int j = i + delta;
            if (j >= nVertice)
                continue;

            for (int x = i; x <= j; x++) {
                int cost = 0;
                if (x != j)
                    cost += intervals[x + 1][j].cost + aggr[x+1][j];

                if (x != i)
                    cost += intervals[i][x-1].cost + aggr[i][x-1];

                if (cost < intervals[i][j].cost) {
                    intervals[i][j] = { cost, x };
                }

            }
        }
    }

    return intervals[0][nVertice - 1];
}

std::vector<int> optimalBST (
    const std::vector<query>& queries, int nVertices, int& totalCost
) {
    std::vector<std::vector<int>> occ = buildOccurrences(nVertices, queries);
    std::vector<std::vector<int>> agg = buildAggregateDemand(nVertices, occ);
    std::vector<std::vector<IntResp_t>> intervals(
        nVertices, std::vector<IntResp_t>(nVertices, IntResp_t{ INF, -1 }
    ));
    std::vector<int> pred(nVertices);
    std::queue<std::pair<int, std::pair<int, int>>> q;

    IntResp_t root = buildOptimalBST(0, nVertices - 1, agg, intervals);
    totalCost = root.cost;

    q.push({ -1, { 0, nVertices - 1 } });

    while (!q.empty()) {
        auto [ pIdx, inter ] = q.front(); q.pop();
        IntResp_t intResp = intervals[inter.first][inter.second];

        pred[intResp.root] = pIdx;

        if (intResp.root != inter.first) {
            q.push({ intResp.root, { inter.first, intResp.root - 1 } });

        }

        if (intResp.root != inter.second) {
            q.push({ intResp.root, { intResp.root + 1, inter.second }});

        }

    }

    assert (isValidBinaryTree(pred));

    return pred;
}