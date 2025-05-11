#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include <numeric>

#include <argparse/argparse.hh>
#include <core/manager.hh>
#include <graphbissection.hh>
#include <loggraphbissection.hh>
#include <mlogagraphbissection.hh>
#include <onehopgraphbissection.hh>

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    argparse::ArgumentParser parser("main_args");

    std::string inputName;
    parser.add_argument("input-name")
        .store_into(inputName)
        .help("the name of the input in weights folder");

    bool bounded;
    parser.add_argument("bounded")
        .default_value(false)
        .store_into(bounded)
        .help("if the bissection algorithm should be bounded");

    bool parallelize;
    parser.add_argument("-p")
        .default_value(false)
        .store_into(parallelize)
        .help("if the bissection algorithm should be parallelized");

    int testNumber;
    parser.add_argument("--test-number")
        .default_value(0)
        .store_into(testNumber)
        .help("the test number");

    // Add command-line arguments for each algorithm
    std::string enabledAlgorithms;
    parser.add_argument("--algorithms")
        .default_value(std::string(""))
        .store_into(enabledAlgorithms)
        .help("Comma-separated list of algorithms to run (e.g., basic,mloga,loggap,onehop)");

    try {
        parser.parse_args(argc, argv);

    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::ifstream iFile("weights/" + inputName);
    int nVertices;
    iFile >> nVertices;

    int src, dst;
    double flowSize;
    std::vector<std::vector<double>> demandMatrix(nVertices, std::vector<double>(nVertices, 0.0));
    while (iFile >> src >> dst >> flowSize) {
        demandMatrix[src][dst] = flowSize;
    }

    std::string baseFolderName = ("output/" + inputName + "/");
    namespace fs = std::filesystem;

    std::set<std::string> algorithmsToRun = { "noop" };
    if (!enabledAlgorithms.empty()) {
        std::stringstream ss(enabledAlgorithms);
        std::string algorithm;
        while (std::getline(ss, algorithm, ',')) {
            algorithmsToRun.insert(algorithm);
        }
    }

    std::vector<int> vertices(nVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    std::vector<Ordering_t> allOrderAlgs = {
        { "noop",  "No Reordering", noop, vertices },
        { "basic",  "Basic Reordering", basic::graphReordering, vertices },
        { "mloga",  "MLogA Reordering", mloga::graphReordering, vertices },
        { "loggap", "LogGap Reordering", loggap::graphReordering, vertices },
        { "onehop", "OneHop Reordering", onehop::graphReordering, vertices },
        // …add more as needed…
    };

    for (auto& orderingAlg: allOrderAlgs) {
        if (algorithmsToRun.count(orderingAlg.flag)) {
            fs::create_directories(baseFolderName + orderingAlg.flag + "/");
            fs::create_directories(baseFolderName + orderingAlg.flag + "/orderings/");
            fs::create_directories(baseFolderName + orderingAlg.flag + "/metrics/");

            runOrdering(
                orderingAlg.flag, orderingAlg.label,
                orderingAlg.func, orderingAlg.vertices,
                demandMatrix, bounded, parallelize, nVertices,
                baseFolderName, testNumber
            );
        }
    }

    for (auto& orderingAlg: allOrderAlgs) {
        if (algorithmsToRun.count(orderingAlg.flag)) {
            runTreeBuilder(
                orderingAlg.flag, orderingAlg.label,
                "raw", testGraphOrder,
                orderingAlg.vertices, demandMatrix,
                bounded, parallelize, nVertices,
                baseFolderName, testNumber
            );

            runTreeBuilder(
                orderingAlg.flag, orderingAlg.label,
                "greedy", testGreedy,
                orderingAlg.vertices, demandMatrix,
                bounded, parallelize, nVertices,
                baseFolderName, testNumber
            );

            runTreeBuilder(
                orderingAlg.flag, orderingAlg.label,
                "obst", testOBST,
                orderingAlg.vertices, demandMatrix,
                bounded, parallelize, nVertices,
                baseFolderName, testNumber
            );
        }
    }
}
