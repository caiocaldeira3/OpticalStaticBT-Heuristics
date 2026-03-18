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
#include <core/logging.hh>
#include <core/log_level.hh>
#include <treebuilders/optbst.hh>
#include <treebuilders/greedy.hh>
#include <convertgraph.hh>
#include <recursiveGraphBisection.hh>

struct Options {
    std::string algorithm;
    int maxDepth;
    int maxIterations;
    std::string datasetName;
    std::string outputDirectory;
    bool verbose = false;
};

void parseArguments(int argc, char* argv[], Options& options) {
    argparse::ArgumentParser parser("main_args");

    parser.add_argument("--algorithm")
        .store_into(options.algorithm)
        .help("the name of the algorithm");

    parser.add_argument("--max-depth")
        .store_into(options.maxDepth)
        .help("the max depth of recursion");

    parser.add_argument("--max-iterations")
        .default_value(20)
        .store_into(options.maxIterations)
        .help("max number of iterations per recursion level");

    parser.add_argument("--dataset-name")
        .store_into(options.datasetName)
        .help("the name of the input in weights folder");

    parser.add_argument("--output-directory")
        .default_value("output")
        .store_into(options.outputDirectory)
        .help("the name of the input in weights folder");

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

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    Options options;
    parseArguments(argc, argv, options);

    g_logLevel = options.verbose ? LogLevel::Debug : LogLevel::Info;

    const auto& demandMatrix = loadDataset(options.datasetName);
    std::cout << "Loaded dataset with " << demandMatrix.size() << " vertices." << std::endl;

    // create if and else for the value of options.algorithm and call the appropriate function, if it mloga or mloggapa
    convertgraph::bipartiteGraph graph;
    if (options.algorithm == "mloga") {
        graph = convertgraph::convertGraphToBipartiteGraphMLogA(demandMatrix);
    } else if (options.algorithm == "mloggapa") {
        graph = convertgraph::convertGraphToBipartiteGraphMLogGapA(demandMatrix);
    } else {
        std::cerr << "Error: Unknown algorithm '" << options.algorithm << "'." << std::endl;
        return 1;
    }

    // create output directory if it does not exist
    std::filesystem::create_directories(options.outputDirectory);

    OrderingLogger logger(options.outputDirectory);

    // create a vector of vertices with size numVertices and fill it with indices from 0 to numVertices - 1
    int numVertices = static_cast<int>(demandMatrix.size());
    std::vector<int> vertices(numVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    std::cout << "Running algorithm: " << options.algorithm 
                << " with max depth: " << options.maxDepth
                << " and max iterations: " << options.maxIterations << std::endl;

    logger.setAlgorithm(options.algorithm);
    logger.setMaxDepth(options.maxDepth);
    logger.setMaxIterations(options.maxIterations);
    logger.setDatasetName(options.datasetName);

    double totalCost = graphBisection::computeBalancedBinaryTreeCostAfterReordering(
        vertices, graph, demandMatrix, options.maxDepth, options.maxIterations, logger
    );
    logger.logTotalCost(totalCost);
    std::cout << "Total cost after reordering: " << totalCost << std::endl;

    // compute the cost of the MLogA algorithm
    double mlogACost = algorithm::computeMLogACost(vertices, demandMatrix);
    std::cout << "MLogA cost: " << mlogACost << std::endl;
    logger.logMLogACost(mlogACost);

    logger.pushToFile();


}
