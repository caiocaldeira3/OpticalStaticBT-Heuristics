#pragma once


#include <set>
#include <memory>
#include <vector>
#include <algorithm>

#include <core/util.hh>


namespace loggap {
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
        double costGain = 0;
        int vertex = vertices[vIdx];
        int nTo = toLimits.rightLimit - toLimits.leftLimit;
        int nFrom = fromLimits.rightLimit - fromLimits.leftLimit;

        for (int fromIdx = fromLimits.leftLimit; fromIdx < fromLimits.rightLimit; fromIdx++) {
            int fromVertex = vertices[fromIdx];
            double v2from = demandMatrix[vertex][fromVertex] + demandMatrix[fromVertex][vertex];
            if (fromVertex == vertex || isClose(v2from, 0))
                continue;

            NodeSectionInfo_t fromInfo = computeVertexInfo(fromIdx, demandMatrix, vertices, fromLimits, toLimits);

            costGain -= (
                fromInfo.sameSumWeight * log2(nFrom / (double) (fromInfo.sameNeighbors + 1))
                + fromInfo.othSumWeight * log2(nTo / (double) (fromInfo.othNeighBors + 1))
            );

            costGain -= (
                - (fromInfo.sameSumWeight - v2from) * log2(nFrom / (double) (fromInfo.sameNeighbors))
                + (fromInfo.othSumWeight + v2from) * log2(nTo / (double) (fromInfo.othNeighBors + 2))
            );
        }

        for (int toIdx = toLimits.leftLimit; toIdx < toLimits.rightLimit; toIdx++) {
            int toVertex = vertices[toIdx];
            double v2to = demandMatrix[vertex][toVertex] + demandMatrix[toVertex][vertex];
            if (toVertex == vertex || isClose(v2to, 0))
                continue;

            NodeSectionInfo_t toInfo = computeVertexInfo(toIdx, demandMatrix, vertices, toLimits, fromLimits);

            costGain += (
                toInfo.sameSumWeight * log2(nTo / (double) (toInfo.sameNeighbors + 1))
                + toInfo.othSumWeight * log2(nFrom / (double) (toInfo.othNeighBors + 1))
            );

            costGain += (
                - (toInfo.sameSumWeight - v2to) * log2(nTo / (double) (toInfo.sameNeighbors))
                + (toInfo.othSumWeight + v2to) * log2(nFrom / (double) (toInfo.othNeighBors + 2))
            );
        }

        return {costGain, vIdx};
    }

    void graphReordering (
        const std::vector<std::vector<double>>& demandMatrix, std::vector<int>& vertices,
        const VectorLimits_t& vectorLimits, int maxDepth, bool parallelize, int maxIterations = 20
    ) {
        if (maxDepth == 0 || vectorLimits.rightLimit - vectorLimits.leftLimit <= 1)
            return;

        int mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
        VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
        VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

        for (int cIter = 0; cIter < maxIterations; cIter++) {
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

                if (swappedVertices.find(leftGain.vIdx) != swappedVertices.end() ||
                    swappedVertices.find(leftGain.vIdx) != swappedVertices.end()) {
                    continue;
                } else if (leftGain.costGain + rightGain.costGain <= 0) {
                    break;
                }

                std::swap(vertices[leftGain.vIdx], vertices[rightGain.vIdx]);
                swappedVertices.insert(leftGain.vIdx);
                swappedVertices.insert(rightGain.vIdx);
            }

            if (swappedVertices.size() == 0)
                break;
        }

        if (parallelize) {
            #pragma omp single nowait
            {
                for (const auto& limits : { leftLimits, rightLimits }) {
                    #pragma omp task
                    graphReordering(demandMatrix, vertices, limits, maxDepth - 1, parallelize, limits.rightLimit - limits.leftLimit);
                }
                #pragma omp taskwait
            }

        } else {
            graphReordering(demandMatrix, vertices, leftLimits, maxDepth - 1, parallelize, leftLimits.rightLimit - leftLimits.leftLimit);
            graphReordering(demandMatrix, vertices, rightLimits, maxDepth - 1, parallelize, rightLimits.rightLimit - rightLimits.leftLimit);
        }
    }

}

