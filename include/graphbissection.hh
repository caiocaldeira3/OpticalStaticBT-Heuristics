#pragma once


#include <set>
#include <memory>
#include <vector>
#include <algorithm>

#include <core/util.hh>
#include <core/bisectionRunRecord.hh>


namespace basic {

struct CostGain_t {
    double costGain;
    std::pair<uint32_t, uint32_t> vertices;
};

bool compareCostGainDecreasing (const CostGain_t& a, const CostGain_t& b) {
    return a.costGain > b.costGain;
}

double computeCostGainBasic (
    uint32_t leftVertex, uint32_t rightVertex,
    const std::vector<std::vector<double>>& demandMatrix,
    const std::vector<uint32_t>& vertices,
    const VectorLimits_t& leftLimits, const VectorLimits_t& rightLimits
) {
    double costGain = 0;

    for (uint32_t idx = leftLimits.leftLimit; idx < leftLimits.rightLimit; idx++) {
        uint32_t othLeft = vertices[idx];
        if (othLeft == leftVertex || othLeft == rightVertex)
            continue;

        costGain -= demandMatrix[leftVertex][othLeft];
        costGain -= demandMatrix[othLeft][leftVertex];
        costGain += demandMatrix[rightVertex][othLeft];
        costGain += demandMatrix[othLeft][rightVertex];

    }

    for (uint32_t idx = rightLimits.leftLimit; idx < rightLimits.rightLimit; idx++) {
        uint32_t othRight = vertices[idx];
        if (othRight == leftVertex || othRight == rightVertex)
            continue;

        costGain -= demandMatrix[rightVertex][othRight];
        costGain -= demandMatrix[othRight][rightVertex];
        costGain += demandMatrix[leftVertex][othRight];
        costGain += demandMatrix[othRight][leftVertex];
    }

    return costGain;
}

void graphReordering (
    const std::vector<std::vector<double>>& demandMatrix, std::vector<uint32_t>& vertices,
    const VectorLimits_t& vectorLimits, uint32_t maxDepth, bool parallelize, BisectionRunRecord& record,
    uint32_t maxIterations = 20
) {
    if (maxDepth == 0 || vectorLimits.rightLimit - vectorLimits.leftLimit <= 2)
        return;

    uint32_t numIterations = 0;
    uint32_t mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
    VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
    VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

    while (numIterations++ < maxIterations) {
        std::vector<CostGain_t> costGains;
        std::set<uint32_t> swappedVertices;

        // #pragma omp parallel for collapse(2) shared(costGains) schedule(dynamic)
        for (uint32_t leftIdx = leftLimits.leftLimit; leftIdx < leftLimits.rightLimit; leftIdx++) {
            for (uint32_t rightIdx = rightLimits.leftLimit; rightIdx < rightLimits.rightLimit; rightIdx++) {
                uint32_t leftVertex = vertices[leftIdx];
                uint32_t rightVertex = vertices[rightIdx];
                double costGain = computeCostGainBasic(
                    leftVertex, rightVertex, demandMatrix, vertices, leftLimits, rightLimits
                );

                #pragma omp critical
                {
                    costGains.push_back({ costGain, { leftIdx, rightIdx }});
                }
            }
        }

        uint32_t bestCostGainIdx = std::max_element(
            costGains.begin(), costGains.end(), compareCostGainDecreasing
        ) - costGains.begin();

        if (costGains[bestCostGainIdx].costGain <= 0)
            break;

        uint32_t leftIdx = costGains[bestCostGainIdx].vertices.first;
        uint32_t rightIdx = costGains[bestCostGainIdx].vertices.second;

        std::swap(vertices[leftIdx], vertices[rightIdx]);

        record.recordSwappedPairs(1);
        record.recordCostGain(costGains[bestCostGainIdx].costGain);
    }

    record.recordIterationCount(numIterations);

    if (parallelize) {
        throw std::runtime_error("Parallelization not implemented yet due to race condition on logger.");

        #pragma omp single nowait
        {
            for (const auto& limits : { leftLimits, rightLimits }) {
                #pragma omp task
                graphReordering(
                    demandMatrix, vertices, limits, maxDepth - 1,
                    parallelize, record, maxIterations
                );
            }
            #pragma omp taskwait
        }

    } else {
        graphReordering(
            demandMatrix, vertices, leftLimits, maxDepth - 1,
            parallelize, record, maxIterations
        );
        graphReordering(
            demandMatrix, vertices, rightLimits, maxDepth - 1,
            parallelize, record, maxIterations
        );

    }
}

}
