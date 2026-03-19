#pragma once

#include <core/util.hh>
#include <core/bisectionRunRecord.hh>
#include <treebuilders/optbst.hh>
#include <treebuilders/greedy.hh>

struct Response_t {
    double cost;
    std::vector<uint32_t> graphOrdering;
};

double testGraphOrder (
    const std::vector<uint32_t>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    uint32_t nVertices = vertices.size();

    std::vector<std::vector<uint32_t>> tree(nVertices, std::vector<uint32_t>());
    buildBalancedBinaryTree(vertices, tree, {0, nVertices}, -1);

    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return treeCost(tree, reorderedDemand);
}

double testOBST (
    const std::vector<uint32_t>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return optimalBST(vertices.size(), reorderedDemand);
}

double testGreedy (
    const std::vector<uint32_t>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return greedyConstructor(vertices.size(), reorderedDemand);
}

template<typename Func>
void runTreeBuilder (
    const std::string& flag, const std::string& label,
    const std::string& treeBuilder,
    Func treeBuilderFn, std::vector<uint32_t>& vertices,
    const std::vector<std::vector<double>>& demandMatrix,
    bool bounded, bool parallelize, uint32_t nVertices,
    const std::string& baseFolder, uint32_t testNumber
) {
    std::cout << treeBuilder + " " + label + " Bissection" << std::endl;
    const clock_t beginTime = std::clock();
    double response = treeBuilderFn(vertices, demandMatrix);
    double timeSpent = double(std::clock() - beginTime) / CLOCKS_PER_SEC;

    std::cout << "\tBissection Cost: " << response << std::endl;
    std::cout << "\tTime Spent: " << timeSpent << std::endl;

    std::ofstream oneHopCostsFile(
        baseFolder + flag + "/" + treeBuilder + "_costs.out",
        std::ios_base::app
    );
    oneHopCostsFile << response << std::endl;

    std::ofstream oneHopTimeSpent(
        baseFolder + flag + "/" + treeBuilder + "_time_spent.out",
        std::ios_base::app
    );
    oneHopTimeSpent << timeSpent << std::endl;
}

struct Ordering_t {
    std::string flag, label;
    std::function<void(
        const std::vector<std::vector<double>>&,
        std::vector<uint32_t>&,
        VectorLimits_t,uint32_t,bool,BisectionRunRecord&,uint32_t
    )> func;
    std::vector<uint32_t> vertices;
};

inline constexpr auto noop = [](auto&&...) {};

template<typename Func>
void runOrdering (
    const std::string& flag, const std::string& label,
    Func reorderFn, std::vector<uint32_t>& orderVec,
    const std::vector<std::vector<double>>& demandMatrix,
    bool bounded, bool parallelize, uint32_t nVertices,
    const std::string& baseFolder, uint32_t testNumber, int maxIterations = 20
) {
    BisectionRunRecord record(RunConfig{.maxIterations = maxIterations, .outputDirectory = baseFolder + flag});

    std::cout << label << std::endl;
    VectorLimits_t limits{0, nVertices};
    uint32_t maxDepth = (bounded ?
        static_cast<uint32_t>(std::ceil(std::log(nVertices)/std::log(2))) + 1: LINF
    );
    const clock_t beginTime = std::clock();
    reorderFn(demandMatrix, orderVec, limits, maxDepth, parallelize, record, maxIterations);
    double secs = double(std::clock() - beginTime) / CLOCKS_PER_SEC;
    std::cout << "\tTime Spent: " << secs << std::endl;

    // write the ordering itself
    std::ofstream orderingFile(
        baseFolder + flag + "/orderings/" + std::to_string(testNumber) + ".out"
    );
    for (uint32_t vIdx = 0; vIdx < nVertices; vIdx++) {
        orderingFile << orderVec[vIdx] << " ";
    }
    orderingFile << std::endl;

    record.appendMetrics();
}
