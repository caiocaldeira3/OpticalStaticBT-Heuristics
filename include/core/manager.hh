#pragma once

#include <core/util.hh>
#include <core/logging.hh>
#include <treebuilders/optbst.hh>
#include <treebuilders/greedy.hh>

struct Response_t {
    double cost;
    std::vector<int> graphOrdering;
};

double testGraphOrder (
    const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    int nVertices = vertices.size();

    std::vector<std::vector<int>> tree(nVertices, std::vector<int>());
    buildBalancedBinaryTree(vertices, tree, {0, nVertices}, -1);

    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return treeCost(tree, reorderedDemand);
}

double testOBST (
    const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return optimalBST(vertices.size(), reorderedDemand);
}

double testGreedy (
    const std::vector<int>& vertices, const std::vector<std::vector<double>>& demandMatrix
) {
    std::vector<std::vector<double>> reorderedDemand = reconfigureDemandMatrix(vertices, demandMatrix);

    return greedyConstructor(vertices.size(), reorderedDemand);
}

template<typename Func>
void runTreeBuilder (
    const std::string& flag, const std::string& label,
    const std::string& treeBuilder,
    Func treeBuilderFn, std::vector<int>& vertices,
    const std::vector<std::vector<double>>& demandMatrix,
    bool bounded, bool parallelize, int nVertices,
    const std::string& baseFolder, int testNumber
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
        std::vector<int>&,
        VectorLimits_t,int,bool,OrderingLogger&,int
    )> func;
    std::vector<int> vertices;
};

inline constexpr auto noop = [](auto&&...) {};

template<typename Func>
void runOrdering (
    const std::string& flag, const std::string& label,
    Func reorderFn, std::vector<int>& orderVec,
    const std::vector<std::vector<double>>& demandMatrix,
    bool bounded, bool parallelize, int nVertices,
    const std::string& baseFolder, int testNumber, int maxIterations = 20
) {
    OrderingLogger logger(maxIterations);

    std::cout << label << std::endl;
    VectorLimits_t limits{0, nVertices};
    int maxDepth = (bounded ?
        static_cast<int>(std::ceil(std::log(nVertices)/std::log(2))) + 1: INF
    );
    const clock_t beginTime = std::clock();
    reorderFn(demandMatrix, orderVec, limits, maxDepth, parallelize, logger, maxIterations);
    double secs = double(std::clock() - beginTime) / CLOCKS_PER_SEC;
    std::cout << "\tTime Spent: " << secs << std::endl;

    // write the ordering itself
    std::ofstream orderingFile(
        baseFolder + flag + "/orderings/" + std::to_string(testNumber) + ".out"
    );
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        orderingFile << orderVec[vIdx] << " ";
    }
    orderingFile << std::endl;

    std::ofstream orderingMetricsFile(
        baseFolder + flag + "/metrics/" + std::to_string(testNumber) + ".out"
    );
    orderingMetricsFile << "max-it-occ,avg-iterations,avg-swapped-pairs,avg-cost-gain" << std::endl;
    orderingMetricsFile << logger.getNumberOfMaxIterationsOccurences() << ","
        << logger.getAverageNumIterationsPerRecursion() << ","
        << logger.getAverageSwappedPairsPerIteration() << ","
        << logger.getAverageCostGainPerRecursion() << std::endl;
}
