#pragma once

#include <set>
#include <memory>
#include <vector>
#include <algorithm>

#include <core/util.hh>
#include <core/bisectionRunRecord.hh>


namespace mloggapa {
    struct CostGain_t {
        double costGain;
        uint32_t vIdx;
    };

    struct NodeSectionInfo_t {
        uint32_t sameNeighbors;
        uint32_t othNeighBors;
        double sameSumWeight;
        double othSumWeight;
    };

    bool compareCostGainDecreasing (const CostGain_t& a, const CostGain_t& b) {
        return a.costGain > b.costGain;
    }

    NodeSectionInfo_t computeVertexInfo (
        uint32_t vIdx, const std::vector<std::vector<double>>& demandMatrix,
        const std::vector<uint32_t>& vertices,
        const VectorLimits_t& fromLimits, const VectorLimits_t& toLimits
    ) {
        NodeSectionInfo_t info = { 0, 0, 0, 0 };
        uint32_t vertex = vertices[vIdx];

        for (uint32_t othIdx = fromLimits.leftLimit; othIdx < fromLimits.rightLimit; othIdx++) {
            uint32_t othVertex = vertices[othIdx];
            double weight = demandMatrix[vertex][othVertex];
            if (othVertex == vertex || isClose(weight, 0))
                continue;

            info.sameNeighbors++;
            info.sameSumWeight += weight;
        }

        for (uint32_t othIdx = toLimits.leftLimit; othIdx < toLimits.rightLimit; othIdx++) {
            uint32_t othVertex = vertices[othIdx];
            double weight = demandMatrix[vertex][othVertex];
            if (othVertex == vertex || isClose(weight, 0))
                continue;

            info.othNeighBors++;
            info.othSumWeight += weight;
        }

        return info;
    }

    CostGain_t computeCostGain (
        uint32_t vIdx, const std::vector<std::vector<double>>& demandMatrix,
        const std::vector<uint32_t>& vertices, const std::vector<NodeSectionInfo_t>& nodeSectionInfo,
        const VectorLimits_t& fromLimits, const VectorLimits_t& toLimits
    ) {
        double costGain = 0;
        uint32_t nVertices = vertices.size();
        uint32_t vertex = vertices[vIdx];
        uint32_t nTo = toLimits.rightLimit - toLimits.leftLimit;
        uint32_t nFrom = fromLimits.rightLimit - fromLimits.leftLimit;

        for (uint32_t tIdx = 0; tIdx < nVertices; tIdx++) {
            if (demandMatrix[tIdx][vertex] == 0) {
                continue;
            }

            NodeSectionInfo_t vInfo = nodeSectionInfo[tIdx];

            costGain += (
                vInfo.sameSumWeight * log2(nTo / (double) (vInfo.sameNeighbors + 1))
                + vInfo.othSumWeight * log2(nFrom / (double) (vInfo.othNeighBors + 1))
            );

            costGain += (
                - (vInfo.sameSumWeight) * log2((nTo - 1) / (double) (vInfo.sameNeighbors + 1))
                - (vInfo.othSumWeight) * log2((nFrom + 1) / (double) (vInfo.othNeighBors + 1))
            );
        }

        if (std::isnan(costGain) || std::isinf(costGain)) {
            throw std::runtime_error("Cost gain is NaN or Inf.");
        }

        return {costGain, vIdx};
    }

    void graphReordering (
        const std::vector<std::vector<double>>& demandMatrix, std::vector<uint32_t>& vertices,
        const VectorLimits_t& vectorLimits, uint32_t maxDepth, bool parallelize, BisectionRunRecord& record,
        uint32_t maxIterations = 20
    ) {
        if (maxDepth == 0 || vectorLimits.rightLimit - vectorLimits.leftLimit <= 3)
            return;

        uint32_t numVertices = vertices.size();
        std::vector<NodeSectionInfo_t> partitionInfo(numVertices);
        uint32_t numIterations = 0;
        uint32_t mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
        VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
        VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

        while (numIterations++ < maxIterations) {
            for (uint32_t tIdx = 0; tIdx < numVertices; tIdx++) {
                partitionInfo[tIdx] = computeVertexInfo(
                    tIdx, demandMatrix, vertices, leftLimits, rightLimits
                );
            }

            uint32_t numSwapped = 0;
            double totalCostGain = 0;
            std::vector<CostGain_t> leftGains, rightGains;
            std::set<uint32_t> swappedVertices;

            for (uint32_t leftIdx = leftLimits.leftLimit; leftIdx < leftLimits.rightLimit; leftIdx++) {
                leftGains.push_back(computeCostGain(
                    leftIdx, demandMatrix, vertices, partitionInfo,
                    leftLimits, rightLimits
                ));
            }

            for (uint32_t rightIdx = rightLimits.leftLimit; rightIdx < rightLimits.rightLimit; rightIdx++) {
                rightGains.push_back(computeCostGain(
                    rightIdx, demandMatrix, vertices, partitionInfo,
                    rightLimits, leftLimits
                ));
            }

            std::sort(leftGains.begin(), leftGains.end(), compareCostGainDecreasing);
            std::sort(rightGains.begin(), rightGains.end(), compareCostGainDecreasing);

            for (uint32_t gainIdx = 0; gainIdx < std::min(leftGains.size(), rightGains.size()); gainIdx++) {
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

            record.recordSwappedPairs(numSwapped);
            record.recordCostGain(totalCostGain);

            if (swappedVertices.size() == 0)
                break;
        }

        record.recordIterationCount(numIterations);

        if (parallelize) {
            throw std::runtime_error("Parallelization not implemented yet due to race condition on logger.");

            #pragma omp single nowait
            {
                for (const auto& limits : { leftLimits, rightLimits }) {
                    #pragma omp task
                    graphReordering(
                        demandMatrix, vertices, limits, maxDepth - 1, parallelize,
                        record, maxIterations
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
                parallelize,  record, maxIterations
            );
        }
    }

    void bipartiteGraphReordering (
        const std::vector<std::vector<double>>& demandMatrix, std::vector<uint32_t>& vertices,
        const VectorLimits_t& vectorLimits, uint32_t maxDepth, bool parallelize, BisectionRunRecord& record,
        uint32_t maxIterations = 20
    ) {
        uint32_t nVertices = vertices.size();
        std::vector<std::vector<double>> demandMatrixCopy(nVertices, std::vector<double>(nVertices, 0.0));
        for (uint32_t i = 0; i < vertices.size(); i++) {
            for (uint32_t j = 0; j < vertices.size(); j++) {
                demandMatrixCopy[i][j] = (
                    demandMatrix[vertices[i]][vertices[j]] + demandMatrix[vertices[j]][vertices[i]]
                );
            }
        }

        graphReordering(demandMatrixCopy, vertices, vectorLimits, maxDepth, parallelize, record, maxIterations);
    }
}

