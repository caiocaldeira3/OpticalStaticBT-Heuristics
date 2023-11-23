#pragma once

#include <vector>

void buildBalancedBST (int minV, int maxV, std::vector<int>& preds, int last = -1) {
    if (minV == maxV) {
        return;

    }

    int midV = (maxV + minV) / 2;
    preds[midV] = last;

    buildBalancedBST(minV, midV, preds, midV);
    buildBalancedBST(midV + 1, maxV, preds, midV);

}