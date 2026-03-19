#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include <numeric>
#include <cmath>

#include <algorithm.hh>
#include <argparse/argparse.hh>
#include <core/bisectionRunRecord.hh>
#include <core/logLevel.hh>
#include <treebuilders/optbst.hh>
#include <treebuilders/greedy.hh>
#include <recursiveGraphBisection.hh>
#include <util/forwardIndex.hh>
#include <util/forwardIndexFactory.hh>

struct Options {
    std::string algorithm;
    size_t maxDepth;
    int maxIterations;
    std::string datasetName;
    std::string outputDirectory;
    bool verbose = false;
};

void parseArguments(int argc, char* argv[], Options& options) {
    argparse::ArgumentParser parser("main_args");

    parser.add_argument("--algorithm")
        .store_into(options.algorithm)
        .help("the name of the algorithm (mloga or loggap)");

    parser.add_argument("--max-depth")
        .store_into(options.maxDepth)
        .help("the max depth of recursion");

    parser.add_argument("--max-iterations")
        .default_value(20)
        .store_into(options.maxIterations)
        .help("max number of iterations per recursion level");

    parser.add_argument("--dataset-name")
        .store_into(options.datasetName)
        .help("the name of the dataset");

    parser.add_argument("--output-directory")
        .default_value("output")
        .store_into(options.outputDirectory)
        .help("the name of the output directory");

    parser.add_argument("--verbose")
        .flag()
        .store_into(options.verbose)
        .help("enable verbose (debug-level) output");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

}

std::vector<std::vector<double>>
loadDataset(const std::string& filename) {
    std::ifstream file(filename);

    std::cout << "Loading dataset from: " << filename << std::endl;
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    size_t numVertices = 0;
    size_t numRequests = 0;

    // Read first line with numVertices and numRequests
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        if (std::getline(ss, token, ',')) {
            numVertices = std::stoul(token);
        }
        if (std::getline(ss, token, ',')) {
            numRequests = std::stoul(token);
        }
    } else {
        throw std::runtime_error("File is empty or invalid format.");
    }

    // Initialize demand matrix with zeros
    std::vector<std::vector<double>> demandMatrix(numVertices, std::vector<double>(numVertices, 0.0));

    for (size_t i = 0; i < numRequests; ++i) {
        if (!std::getline(file, line)) {
            throw std::runtime_error("Not enough lines for the specified number of requests.");
        }
        
        std::stringstream ss(line);
        std::string token;
        int src = -1, dst = -1;

        if (std::getline(ss, token, ',')) {
            src = std::stoi(token);
        }
        if (std::getline(ss, token, ',')) {
            dst = std::stoi(token);
        }

        if (src >= 0 && dst >= 0 && src < numVertices && dst < numVertices) {
            demandMatrix[src][dst]++;
            demandMatrix[dst][src]++; // Assuming undirected graph
        } else {
            throw std::runtime_error("Invalid vertex index in: " + line);
        }
    }

    return demandMatrix;
}

pisa::verticeRange<std::vector<uint32_t>::iterator> createVerticeRange(
    std::vector<uint32_t>& vertices,
    const pisa::forwardIndex& fwdIndex,
    std::vector<double>& gains
) {
    return pisa::verticeRange(vertices.begin(), vertices.end(), std::cref(fwdIndex), std::ref(gains));
}

double computeBalancedBinaryTreeCostAfterReordering(
    std::vector<uint32_t> vertices,
    const std::vector<std::vector<double>>& demandMatrix
) {
    uint32_t nVertices = vertices.size();
    auto reassignedDemandMatrix = reconfigureDemandMatrix(vertices, demandMatrix);

    // Build tree with canonical 0..n-1 ordering (positions, not original IDs)
    std::vector<uint32_t> canonicalOrder(nVertices);
    std::iota(canonicalOrder.begin(), canonicalOrder.end(), 0);

    std::vector<std::vector<uint32_t>> tree(nVertices, std::vector<uint32_t>());
    buildBalancedBinaryTree(canonicalOrder, tree, {0, nVertices}, -1);

    return treeCost(tree, reassignedDemandMatrix);
}

int main (int argc, char* argv[]) {
    Options options;
    parseArguments(argc, argv, options);

    g_logLevel = options.verbose ? LogLevel::Debug : LogLevel::Info;

    const auto& demandMatrix = loadDataset(options.datasetName);
    log(LogLevel::Info) << "Loaded dataset with " << demandMatrix.size() << " vertices." << std::endl;

    uint32_t numVertices = demandMatrix.size();
    std::vector<uint32_t> vertices(numVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    log(LogLevel::Debug) << "Running algorithm: " << options.algorithm
                << " with max depth: " << options.maxDepth
                << " and max iterations: " << options.maxIterations << std::endl;

    pisa::forwardIndex fwdIndex;
    if (options.algorithm == "loggap") {
        log(LogLevel::Info) << "Creating forward index for LogGap..." << std::endl;
        fwdIndex = pisa::createLogGapForwardIndex(demandMatrix);
    } else if (options.algorithm == "mloga") {
        log(LogLevel::Info) << "Creating forward index for MLOGA..." << std::endl;
        fwdIndex = pisa::createMlogaForwardIndex(demandMatrix);
    } else {
        throw std::runtime_error("Unknown algorithm: " + options.algorithm);
    }
    std::vector<double> gains(numVertices, 0.0);
    auto verticesRange = createVerticeRange(vertices, fwdIndex, gains);

    pisa::recursiveGraphBisection(verticesRange, options.maxDepth, options.maxIterations, options.maxDepth - 6, nullptr);

    double totalCost = computeBalancedBinaryTreeCostAfterReordering(vertices, demandMatrix);
    log(LogLevel::Info) << "Total cost after reordering: " << totalCost << std::endl;
}
