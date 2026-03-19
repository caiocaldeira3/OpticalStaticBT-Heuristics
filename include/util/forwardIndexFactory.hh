#pragma once

#include <cstdint>
#include <vector>

#include <core/util.hh>
#include <util/forwardIndex.hh>

namespace pisa {

inline forwardIndex createLogGapForwardIndex(const std::vector<std::vector<double>>& demandMatrix) {
    uint32_t n = demandMatrix.size();
    std::vector<std::vector<uint32_t>> docTerms(n);
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t j = 0; j < n; ++j) {
            if (i != j && !isClose(demandMatrix[i][j], 0.0)) {
                docTerms[i].push_back(j);
            }
        }
    }
    return forwardIndex(docTerms, n);
}

inline forwardIndex createMlogaForwardIndex(const std::vector<std::vector<double>>& demandMatrix) {
    uint32_t n = demandMatrix.size();
    std::vector<std::vector<uint32_t>> docTerms(n);
    uint32_t edgeId = 0;
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t j = i; j < n; ++j) {
            double weight = demandMatrix[i][j] + (i != j ? demandMatrix[j][i] : 0.0);
            if (!isClose(weight, 0.0)) {
                docTerms[i].push_back(edgeId);
                if (i != j) {
                    docTerms[j].push_back(edgeId);
                }
                edgeId++;
            }
        }
    }
    return forwardIndex(docTerms, edgeId);
}

} // namespace pisa
