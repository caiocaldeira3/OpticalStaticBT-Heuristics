#pragma once

#include <assert.h>

struct IntervalRoot_t{
    double cost;
    int root;
};

std::vector<std::vector<double>> buildAggregateDemand (
    int nVertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> nodeWeight(nVertices, std::vector<double>(nVertices, 0));
    std::vector<std::vector<double>> aggDemand(nVertices, std::vector<double>(nVertices, 0));

    for (int i = 0; i < nVertices; i++) {
        for (int j = 0; j < nVertices; j++) {
            nodeWeight[i][j] = (
                demandMatrix[i][j] + demandMatrix[j][i] + (j == 0 ? 0 : nodeWeight[i][j - 1])
            );
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

std::vector<std::vector<double>> buildAggregateDemandN4 (
    int nVertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> aggDemand(nVertices, std::vector<double>(nVertices, 0));

    for (int lIdx = 0; lIdx < nVertices; lIdx++) {
        for (int rIdx = lIdx; rIdx < nVertices; rIdx++) {
            for (int k = lIdx; k <= rIdx; k++) {
                for (int i = 0; i < lIdx; i++) {
                    aggDemand[lIdx][rIdx] += demandMatrix[i][k] + demandMatrix[k][i];
                }
                for (int i = rIdx + 1; i < nVertices; i++) {
                    aggDemand[lIdx][rIdx] += demandMatrix[i][k] + demandMatrix[k][i];
                }
            }
        }
    }

    return aggDemand;
}

IntervalRoot_t buildOptimalBST (
    int nVertices, const std::vector<std::vector<double>>& aggr,
    std::vector<std::vector<IntervalRoot_t>>& intervals
) {
    for (int vIdx = 0; vIdx < nVertices; vIdx++)
        intervals[vIdx][vIdx] = IntervalRoot_t{ 0, vIdx };

    for (int delta = 1; delta < nVertices; delta++) {
        for (int lIdx = 0; lIdx < nVertices; lIdx++) {
            int rIdx = lIdx + delta;
            if (rIdx >= nVertices)
                continue;

            for (int x = lIdx; x <= rIdx; x++) {
                double cost = 0;
                if (x != rIdx)
                    cost += intervals[x + 1][rIdx].cost + aggr[x+1][rIdx];

                if (x != lIdx)
                    cost += intervals[lIdx][x-1].cost + aggr[lIdx][x-1];

                if (cost < intervals[lIdx][rIdx].cost) {
                    intervals[lIdx][rIdx] = { cost, x };
                }

            }
        }
    }

    return intervals[0][nVertices - 1];
}

double optimalBST (
    int nVertices, const std::vector<std::vector<double>>& demandMatrix,
    bool verbose = false
) {
    std::vector<std::vector<IntervalRoot_t>> intervals(
        nVertices, std::vector<IntervalRoot_t>(nVertices, IntervalRoot_t{ INF, -1 }
    ));
    std::vector<std::vector<double>> agg = buildAggregateDemand(nVertices, demandMatrix);
    std::vector<int> pred(nVertices);
    std::queue<std::pair<int, std::pair<int, int>>> q;

    IntervalRoot_t root = buildOptimalBST(nVertices, agg, intervals);

    if (verbose) {
        q.push({ -1, { 0, nVertices - 1 } });

        while (!q.empty()) {
            auto [ pIdx, inter ] = q.front(); q.pop();
            IntervalRoot_t intResp = intervals[inter.first][inter.second];

            pred[intResp.root] = pIdx;

            if (intResp.root != inter.first) {
                q.push({ intResp.root, { inter.first, intResp.root - 1 } });

            }

            if (intResp.root != inter.second) {
                q.push({ intResp.root, { intResp.root + 1, inter.second }});

            }
        }

        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            std::cout << pred[vIdx] << " ";
        }
    }

    return root.cost;
}
