#pragma once

#include <set>
#include <memory>
#include <vector>
#include <algorithm>

#include <core/util.hh>
#include <core/logging.hh>


namespace onehop {
    struct CostGain_t {
        double costGain;
        int vIdx;
    };

    struct NodeSectionInfo_t {
        int sameNeighbors;
        int othNeighBors;
        double sameSumWeight;
        double othSumWeight;
    };

    bool compareCostGainDecreasing (const CostGain_t& a, const CostGain_t& b) {
        return a.costGain > b.costGain;
    }

    NodeSectionInfo_t computeVertexInfo (
        int vIdx, const std::vector<std::vector<double>>& demandMatrix,
        const std::vector<int>& vertices,
        const VectorLimits_t& fromLimits, const VectorLimits_t& toLimits
    ) {
        NodeSectionInfo_t info = { 0, 0, 0, 0 };
        int vertex = vertices[vIdx];

        for (int othIdx = fromLimits.leftLimit; othIdx < fromLimits.rightLimit; othIdx++) {
            int othVertex = vertices[othIdx];
            double weight = demandMatrix[vertex][othVertex] + demandMatrix[othVertex][vertex];
            if (othVertex == vertex || isClose(weight, 0))
                continue;

            info.sameNeighbors++;
            info.sameSumWeight += weight;
        }

        for (int othIdx = toLimits.leftLimit; othIdx < toLimits.rightLimit; othIdx++) {
            int othVertex = vertices[othIdx];
            double weight = demandMatrix[vertex][othVertex] + demandMatrix[othVertex][vertex];
            if (othVertex == vertex || isClose(weight, 0))
                continue;

            info.othNeighBors++;
            info.othSumWeight += weight;
        }

        return info;
    }

    CostGain_t computeCostGain (
        int vIdx, const std::vector<std::vector<double>>& demandMatrix,
        const std::vector<int>& vertices,
        const VectorLimits_t& fromLimits, const VectorLimits_t& toLimits
    ) {
        double costGain;
        int vertex = vertices[vIdx];
        int nTo = toLimits.rightLimit - toLimits.leftLimit;
        int nFrom = fromLimits.rightLimit - fromLimits.leftLimit;

        NodeSectionInfo_t vInfo = computeVertexInfo(vIdx, demandMatrix, vertices, fromLimits, toLimits);

        costGain = (
            vInfo.sameSumWeight * log2(nTo / (double) (vInfo.sameNeighbors + 1))
            + vInfo.othSumWeight * log2(nFrom / (double) (vInfo.othNeighBors + 1))
        );

        costGain += (
            - (vInfo.sameSumWeight) * log2((nTo - 1) / (double) (vInfo.sameNeighbors + 1))
            - (vInfo.othSumWeight) * log2((nFrom + 1) / (double) (vInfo.othNeighBors + 1))
        );

        if (std::isnan(costGain) || std::isinf(costGain)) {
            throw std::runtime_error("Cost gain is NaN or Inf.");
        }

        return {costGain, vIdx};
    }

    void graphReordering (
        const std::vector<std::vector<double>>& demandMatrix, std::vector<int>& vertices,
        const VectorLimits_t& vectorLimits, int maxDepth, bool parallelize, OrderingLogger& logger,
        int maxIterations = 20
    ) {
        if (maxDepth == 0 || vectorLimits.rightLimit - vectorLimits.leftLimit <= 3)
            return;

        int numIterations = 0;
        int mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
        VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
        VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

        while (numIterations++ < maxIterations) {
            int numSwapped = 0;
            double totalCostGain = 0;
            std::vector<CostGain_t> leftGains, rightGains;
            std::set<int> swappedVertices;

            for (int leftIdx = leftLimits.leftLimit; leftIdx < leftLimits.rightLimit; leftIdx++) {
                leftGains.push_back(computeCostGain(leftIdx, demandMatrix, vertices, leftLimits, rightLimits));
            }

            for (int rightIdx = rightLimits.leftLimit; rightIdx < rightLimits.rightLimit; rightIdx++) {
                rightGains.push_back(computeCostGain(rightIdx, demandMatrix, vertices, rightLimits, leftLimits));
            }

            std::sort(leftGains.begin(), leftGains.end(), compareCostGainDecreasing);
            std::sort(rightGains.begin(), rightGains.end(), compareCostGainDecreasing);

            for (int gainIdx = 0; gainIdx < std::min(leftGains.size(), rightGains.size()); gainIdx++) {
                CostGain_t leftGain = leftGains[gainIdx];
                CostGain_t rightGain = rightGains[gainIdx];

                if (
                    swappedVertices.find(leftGain.vIdx) != swappedVertices.end() ||
                    swappedVertices.find(leftGain.vIdx) != swappedVertices.end()
                ) {
                    continue;
                } else if (leftGain.costGain + rightGain.costGain <= 0) {
                    break;
                }

                totalCostGain += leftGain.costGain + rightGain.costGain;
                numSwapped++;

                std::swap(vertices[leftGain.vIdx], vertices[rightGain.vIdx]);
                swappedVertices.insert(leftGain.vIdx);
                swappedVertices.insert(rightGain.vIdx);
            }

            logger.logSwappedPairs(numSwapped);
            logger.logCostGain(totalCostGain);

            if (swappedVertices.size() == 0)
                break;
        }

        logger.logNumIterations(numIterations);

        if (parallelize) {
            throw std::runtime_error("Parallelization not implemented yet due to race condition on logger.");

            #pragma omp single nowait
            {
                for (const auto& limits : { leftLimits, rightLimits }) {
                    #pragma omp task
                    graphReordering(
                        demandMatrix, vertices, limits, maxDepth - 1, parallelize,
                        logger, maxIterations
                    );
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
                parallelize,  logger, maxIterations
            );
        }
    }

}

