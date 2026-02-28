#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include <numeric>
#include <cmath>

#include <argparse/argparse.hh>
#include <core/logging.hh>
#include <treebuilders/optbst.hh>
#include <treebuilders/greedy.hh>
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

// module of a number using std::abs


double computeMLogACost(
    const std::vector<int>& vertices,
    const std::vector<std::vector<double>>& demandMatrix
) {
    // Iterate edges and compute the cost based on the MLogA algorithm
    double totalCost = 0.0;
    for (int i = 0; i < vertices.size(); ++i) {
        for (int j = i + 1; j < vertices.size(); ++j) {
            int src = vertices[i];
            int dst = vertices[j];
            totalCost += demandMatrix[src][dst] * std::log2(std::abs(i - j));
        }
    }
    return totalCost;
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

    OrderingLogger logger(options.outputDirectory);

    // create a vector of vertices with size numVertices and fill it with indices from 0 to numVertices - 1
    std::vector<int> vertices(numVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    std::cout << "Running algorithm: " << options.algorithm 
                << " with max depth: " << options.maxDepth
                << " and max iterations: " << options.maxIterations << std::endl;

    logger.setAlgorithm(options.algorithm);
    logger.setMaxDepth(options.maxDepth);
    logger.setMaxIterations(options.maxIterations);
    logger.setDatasetName(options.datasetName);

    double totalCost = basic::computeBalancedBinaryTreeCostAfterReordering(
        vertices, graph, demandMatrix, options.maxDepth, options.maxIterations, logger
    );
    logger.logTotalCost(totalCost);
    std::cout << "Total cost after reordering: " << totalCost << std::endl;

    // compute the cost of the MLogA algorithm
    double mlogACost = computeMLogACost(vertices, demandMatrix);
    std::cout << "MLogA cost: " << mlogACost << std::endl;
    logger.logMLogACost(mlogACost);

    logger.pushToFile();


}
