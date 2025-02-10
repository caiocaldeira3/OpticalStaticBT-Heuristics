#pragma once


#include <set>
#include <memory>
#include <vector>
#include <algorithm>


struct CostGain_t {
    double costGain;
    std::pair<int, int> vertices;
};

struct VectorLimits_t {
    int leftLimit;
    int rightLimit;
};

bool compareCostGainDecreasing (const CostGain_t& a, const CostGain_t& b) {
    return a.costGain > b.costGain;
}

double computeCostGainBasic (
    int leftVertex, int rightVertex,
    const std::vector<std::vector<double>>& demandMatrix,
    const std::vector<int>& vertices, 
    const VectorLimits_t& leftLimits, const VectorLimits_t& rightLimits
) {
    double costGain = 0;

    for (int idx = leftLimits.leftLimit; idx < leftLimits.rightLimit; idx++) {
        int othLeft = vertices[idx];
        costGain -= demandMatrix[leftVertex][othLeft];
        costGain += demandMatrix[rightVertex][othLeft];
    }

    for (int idx = rightLimits.leftLimit; idx < rightLimits.rightLimit; idx++) {
        int othRight = vertices[idx];
        costGain -= demandMatrix[rightVertex][othRight];
        costGain += demandMatrix[leftVertex][othRight];
    }

    return costGain;
}

void graphReordering (
    const std::vector<std::vector<double>>& demandMatrix, std::vector<int>& vertices,
    const VectorLimits_t& vectorLimits, int& maxDepth, int maxIterations = 20
) {
    int mid = (vectorLimits.leftLimit + vectorLimits.rightLimit) / 2;
    VectorLimits_t leftLimits = { vectorLimits.leftLimit, mid };
    VectorLimits_t rightLimits = { mid, vectorLimits.rightLimit };

    for (int cIter = 0; cIter < maxIterations; cIter++) {
        std::vector<CostGain_t> costGains;
        std::set<int> swappedVertices;

        for (int leftIdx = leftLimits.leftLimit; leftIdx < leftLimits.rightLimit; leftIdx++) { 
            int leftVertex = vertices[leftIdx];
            for (int rightIdx = rightLimits.leftLimit; rightIdx < rightLimits.rightLimit; rightIdx++) {
                int rightVertex = vertices[rightIdx];
                double costGain = computeCostGainBasic(
                    leftVertex, rightVertex, demandMatrix, vertices, leftLimits, rightLimits
                );

                costGains.push_back({ costGain, { leftVertex, rightVertex }});
            }
        }

        std::sort(costGains.begin(), costGains.end(), compareCostGainDecreasing);

        for (CostGain_t& cg: costGains) {
            if (swappedVertices.find(cg.vertices.first) != swappedVertices.end() ||
                swappedVertices.find(cg.vertices.second) != swappedVertices.end()) {
                continue;
            } else if (cg.costGain <= 0) {
                break;
            }

            std::swap(vertices[cg.vertices.first], vertices[cg.vertices.second]);
            swappedVertices.insert(cg.vertices.first);
            swappedVertices.insert(cg.vertices.second);
        }

        if (swappedVertices.size() == 0)
            break;
    }

    graphReordering(demandMatrix, vertices, leftLimits, maxDepth, maxIterations);
    graphReordering(demandMatrix, vertices, rightLimits, maxDepth, maxIterations);
}