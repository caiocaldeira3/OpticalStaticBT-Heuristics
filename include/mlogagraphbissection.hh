#pragma once


#include <set>
#include <memory>
#include <vector>
#include <algorithm>

#include <core/util.hh>


namespace mloga {

struct CostGain_t {
    double costGain;
    std::pair<int, int> vertices;
};

bool compareCostGainDecreasing (const CostGain_t& a, const CostGain_t& b) {
    return a.costGain > b.costGain;
}

double computeCostGain (
    int leftIdx, int rightIdx,
    const std::vector<std::vector<double>>& demandMatrix,
    const std::vector<int>& vertices,
    const VectorLimits_t& leftLimits, const VectorLimits_t& rightLimits
) {
    int leftVertex = vertices[leftIdx];
    int rightVertex = vertices[rightIdx];
    double costGain = 0;

    for (int idx = leftLimits.leftLimit; idx < leftLimits.rightLimit; idx++) {
        int othLeft = vertices[idx];
        double leftWeight = demandMatrix[leftVertex][othLeft] + demandMatrix[othLeft][leftVertex];
        double rightWeight = demandMatrix[rightVertex][othLeft] + demandMatrix[othLeft][rightVertex];

        if (othLeft != leftVertex) {
            costGain += log2(abs(rightVertex - othLeft)) * leftWeight - log2(abs(leftVertex - othLeft)) * leftWeight;
            costGain += -log2(abs(rightVertex - othLeft)) * rightWeight + log2(abs(leftVertex - othLeft)) * rightWeight;
        }
    }

    for (int idx = rightLimits.leftLimit; idx < rightLimits.rightLimit; idx++) {
        int othRight = vertices[idx];
        double leftWeight = demandMatrix[leftVertex][othRight] + demandMatrix[othRight][leftVertex];
        double rightWeight = demandMatrix[rightVertex][othRight] + demandMatrix[othRight][rightVertex];

        if (othRight != rightVertex) {
            costGain += -log2(abs(leftVertex - othRight)) * leftWeight + log2(abs(rightVertex - othRight)) * leftWeight;
            costGain += log2(abs(leftVertex - othRight)) * rightWeight - log2(abs(rightVertex - othRight)) * rightWeight;
        }
    }

    return costGain;
}

void graphReordering (
    const std::vector<std::vector<double>>& demandMatrix, std::vector<int>& vertices,
    const VectorLimits_t& vectorLimits, int maxDepth, bool parallelize, OrderingLogger& logger,
    int maxIterations = 20
) {
    if (maxDepth == 0 || vectorLimits.rightLimit - vectorLimits.leftLimit <= 2)
        return;

    int numIterations = 0;

    int mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
    VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
    VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

    while (numIterations++ < maxIterations) {
        int numSwapped = 0;
        double totalCostGain = 0;

        std::vector<CostGain_t> costGains;

        // #pragma omp parallel for collapse(2) shared(costGains) schedule(dynamic)
        for (int leftIdx = leftLimits.leftLimit; leftIdx < leftLimits.rightLimit; leftIdx++) {
            for (int rightIdx = rightLimits.leftLimit; rightIdx < rightLimits.rightLimit; rightIdx++) {
                double costGain = computeCostGain(
                    leftIdx, rightIdx, demandMatrix, vertices, leftLimits, rightLimits
                );

                #pragma omp critical
                {
                    costGains.push_back({ costGain, { leftIdx, rightIdx }});
                }
            }
        }

        int bestCostGainIdx = std::max_element(
            costGains.begin(), costGains.end(), compareCostGainDecreasing
        ) - costGains.begin();

        if (costGains[bestCostGainIdx].costGain <= 0)
            break;

        int leftIdx = costGains[bestCostGainIdx].vertices.first;
        int rightIdx = costGains[bestCostGainIdx].vertices.second;

        std::swap(vertices[leftIdx], vertices[rightIdx]);

        logger.logSwappedPairs(1);
        logger.logCostGain(costGains[bestCostGainIdx].costGain);
    }

    logger.logNumIterations(numIterations);

    if (parallelize) {
        throw std::runtime_error("Parallelization not implemented yet due to race condition on logger.");

        #pragma omp single nowait
        {
            for (const auto& limits : { leftLimits, rightLimits }) {
                #pragma omp task
                graphReordering(demandMatrix, vertices, limits, maxDepth - 1, parallelize, logger, maxIterations);
            }
            #pragma omp taskwait
        }

    } else {
        graphReordering(
            demandMatrix, vertices, leftLimits, maxDepth - 1,
            parallelize, logger, maxIterations
        );
        graphReordering(
            demandMatrix, vertices, rightLimits, maxDepth - 1,
            parallelize, logger, maxIterations
        );

    }
}

}
