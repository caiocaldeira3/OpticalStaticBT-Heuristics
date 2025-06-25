#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include <numeric>

#include <argparse/argparse.hh>
#include <core/logging.hh>
#include <convertgraph.hh>
#include <graphbissectionLog.hh>

struct Options {
    std::string algorithm;
    int maxDepth;
    int maxIterations;
    std::string datasetName;
    std::string outputDirectory;
};

void parseArguments(int argc, char* argv[], Options& options) {
    argparse::ArgumentParser parser("main_args");

    parser.add_argument("--algorithm")
        .store_into(options.algorithm)
        .help("the name of the input in weights folder");

    parser.add_argument("--max-depth")
        .store_into(options.maxDepth)
        .help("the test number");

    parser.add_argument("--max-iterations")
        .default_value(15)
        .store_into(options.maxIterations)
        .help("Dot-separated list of algorithms to run (e.g., basic.mloggap.onehop)");

    parser.add_argument("--dataset-name")
        .store_into(options.datasetName)
        .help("the name of the input in weights folder");

    parser.add_argument("--output-directory")   
        .default_value("output")
        .store_into(options.outputDirectory)
        .help("the name of the input in weights folder");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

}

bool loadDataset(const std::string& filename, int& numVertices,
                 std::vector<std::vector<double>>& demandMatrix) {
    std::ifstream file(filename);

    std::cout << "Loading dataset from: " << filename << std::endl;
    if (!file.is_open()) {
        return false; // File could not be opened
    }

    std::string line;
    size_t numEdges = 0;

    // Read first line with numVertices and numEdges
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        if (std::getline(ss, token, ',')) {
            numVertices = std::stoul(token);
        }
        if (std::getline(ss, token, ',')) {
            numEdges = std::stoul(token);
        }
    } else {
        throw std::runtime_error("File is empty or invalid format.");
    }

    // Initialize demand matrix with zeros
    demandMatrix.assign(numVertices, std::vector<double>(numVertices, 0.0));

    // Read each communication pair
    while (std::getline(file, line)) {
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

    return true;
}

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    Options options;
    parseArguments(argc, argv, options);

    std::vector<std::vector<double>> demandMatrix;
    int numVertices = 0;    
    if(!loadDataset(options.datasetName, numVertices, demandMatrix)) {
        std::cerr << "Error: Could not load dataset from " << options.datasetName << std::endl;
        return 1;
    }
    std::cout << "Loaded dataset with " << numVertices << " vertices." << std::endl;

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

    // create a vector of vertices with size numVertices and fill it with indices from 0 to numVertices - 1
    std::vector<int> vertices(numVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    OrderingLogger logger(options.outputDirectory);
    logger.createFile();

    for (int depth = 1; depth <= options.maxDepth; ++depth) {
        std::cout << "Running algorithm: " << options.algorithm 
                  << " with max depth: " << depth 
                  << " and max iterations: " << options.maxIterations << std::endl;

        logger.setAlgorithm(options.algorithm);
        logger.setMaxDepth(depth);
        logger.setMaxIterations(options.maxIterations);
        logger.setDatasetName(options.datasetName);

        double totalCost = basic::computeBalancedBinaryTreeCostAfterReordering(
            vertices, graph, demandMatrix, depth, options.maxIterations, logger
        );
        logger.logTotalCost(totalCost);
        std::cout << "Total cost after reordering: " << totalCost << std::endl;

        logger.pushToFile();
    }

}
