#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>

#include <argparse/argparse.hh>
#include <manager.hh>
#include <graphbissection.hh>
#include <loggraphbissection.hh>
#include <mlogagraphbissection.hh>

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
    fs::create_directories(baseFolderName + "orderings/");

    std::vector<int> vertices, orderedVertices, mlogaOrderedVertices, logGapOrderedVertices;
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        vertices.push_back(vIdx);
        orderedVertices.push_back(vIdx);
        mlogaOrderedVertices.push_back(vIdx);
        logGapOrderedVertices.push_back(vIdx);
    }

    if (false) {
        VectorLimits_t vectorLimits = { 0, nVertices };
        int maxDepth = (bounded ? ceil(log(nVertices) / log(2)) + 1 : INF);

        const clock_t orderBeginTime = std::clock();
        basic::graphReordering(demandMatrix, orderedVertices, vectorLimits, maxDepth, parallelize, nVertices);
        double orderEndTime = double(std::clock() - orderBeginTime) / CLOCKS_PER_SEC;
        std::cout << "Basic Ordering Time Spent: " << orderEndTime << std::endl;

        std::ofstream orderingFile(
            baseFolderName + "orderings/basic_ordering_" + std::to_string(testNumber) + ".out"
        );
        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            orderingFile << orderedVertices[vIdx] << " ";
        }
        orderingFile << std::endl;
    }

    if (false) {
        VectorLimits_t vectorLimits = { 0, nVertices };
        int maxDepth = (bounded ? ceil(log(nVertices) / log(2)) + 1 : INF);

        const clock_t orderBeginTime = std::clock();
        mloga::graphReordering(demandMatrix, mlogaOrderedVertices, vectorLimits, maxDepth, parallelize, nVertices);
        double orderEndTime = double(std::clock() - orderBeginTime) / CLOCKS_PER_SEC;
        std::cout << "LogGap Ordering Time Spent: " << orderEndTime << std::endl;

        std::ofstream orderingFile(
            baseFolderName + "orderings/mloga_ordering_" + std::to_string(testNumber) + ".out"
        );
        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            orderingFile << mlogaOrderedVertices[vIdx] << " ";
        }
        orderingFile << std::endl;
    }

    if (false) {
        VectorLimits_t vectorLimits = { 0, nVertices };
        int maxDepth = (bounded ? ceil(log(nVertices) / log(2)) + 1 : INF);

        const clock_t orderBeginTime = std::clock();
        loggap::graphReordering(demandMatrix, logGapOrderedVertices, vectorLimits, maxDepth, parallelize, nVertices);
        double orderEndTime = double(std::clock() - orderBeginTime) / CLOCKS_PER_SEC;
        std::cout << "LogGap Ordering Time Spent: " << orderEndTime << std::endl;

        std::ofstream orderingFile(
            baseFolderName + "orderings/log_gap_ordering_" + std::to_string(testNumber) + ".out"
        );
        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            orderingFile << logGapOrderedVertices[vIdx] << " ";
        }
        orderingFile << std::endl;
    }

    if (false){
        std::cout << "Raw Bissection" << std::endl;

        const clock_t rawBeginTime = std::clock();
        double rawBissecResponse = testGraphOrder(orderedVertices, demandMatrix);
        double rawSec = double(std::clock() - rawBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tRaw Bissection Cost: " << rawBissecResponse << std::endl;
        std::cout << "\tRaw Time Spent: " << rawSec << std::endl;

        std::ofstream rawCostsFile(
            baseFolderName + "balanced-tree-on-bissection_costs.out", std::ios_base::app
        );
        rawCostsFile << rawBissecResponse << std::endl;

        std::ofstream rawTimeSpent(
            baseFolderName + "balanced-tree-on-bissection_spent.out", std::ios_base::app
        );
        rawTimeSpent << rawSec << std::endl;
    }

    if (false) {
        std::cout << "MLogA Raw Bissection" << std::endl;

        const clock_t rawBeginTime = std::clock();
        double rawBissecResponse = testGraphOrder(mlogaOrderedVertices, demandMatrix);
        double rawSec = double(std::clock() - rawBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tRaw MLogA Bissection Cost: " << rawBissecResponse << std::endl;
        std::cout << "\tRaw MLogA Time Spent: " << rawSec << std::endl;

        std::ofstream rawCostsFile(
            baseFolderName + "mloga_raw_costs.out", std::ios_base::app
        );
        rawCostsFile << rawBissecResponse << std::endl;

        std::ofstream rawTimeSpent(
            baseFolderName + "mloga_raw_time_spent.out", std::ios_base::app
        );
        rawTimeSpent << rawSec << std::endl;
    }

    if (false) {
        std::cout << "LogGap Raw Bissection" << std::endl;

        const clock_t rawBeginTime = std::clock();
        double rawBissecResponse = testGraphOrder(logGapOrderedVertices, demandMatrix);
        double rawSec = double(std::clock() - rawBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tRaw Log Gap Bissection Cost: " << rawBissecResponse << std::endl;
        std::cout << "\tRaw Log Gap Time Spent: " << rawSec << std::endl;

        std::ofstream rawCostsFile(
            baseFolderName + "log_gap_raw_costs.out", std::ios_base::app
        );
        rawCostsFile << rawBissecResponse << std::endl;

        std::ofstream rawTimeSpent(
            baseFolderName + "log_gap_raw_time_spent.out", std::ios_base::app
        );
        rawTimeSpent << rawSec << std::endl;
    }

    if (false) {
        std::cout << "OBST Bissection" << std::endl;

        const clock_t obstBeginTime = std::clock();
        double obstBissecResponse = testOBST(orderedVertices, demandMatrix);
        double obstSec = double(std::clock() - obstBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tOBST Bissection Cost: " << obstBissecResponse << std::endl;
        std::cout << "\tOBST Time Spent: " << obstSec << std::endl;

        std::ofstream obstCostsFile(
            baseFolderName + "obst-bissection_costs.out", std::ios_base::app
        );
        obstCostsFile << obstBissecResponse << std::endl;

        std::ofstream obstTimeSpent(
            baseFolderName + "obst-bissection_time_spent.out", std::ios_base::app
        );
        obstTimeSpent << obstSec << std::endl;
    }

    if (false) {
        std::cout << "OBST LogGap Bissection" << std::endl;

        const clock_t obstBeginTime = std::clock();
        double obstBissecResponse = testOBST(mlogaOrderedVertices, demandMatrix);
        double obstSec = double(std::clock() - obstBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tOBST LogGap Bissection Cost: " << obstBissecResponse << std::endl;
        std::cout << "\tOBST LogGap Time Spent: " << obstSec << std::endl;

        std::ofstream obstCostsFile(
            baseFolderName + "obst-log-gap-bissection_costs.out", std::ios_base::app
        );
        obstCostsFile << obstBissecResponse << std::endl;

        std::ofstream obstTimeSpent(
            baseFolderName + "obst-log-gap-bissection_time_spent.out", std::ios_base::app
        );
        obstTimeSpent << obstSec << std::endl;
    }

    if (false) {
        std::cout << "OBST LogGap Bissection" << std::endl;

        const clock_t obstBeginTime = std::clock();
        double obstBissecResponse = testOBST(logGapOrderedVertices, demandMatrix);
        double obstSec = double(std::clock() - obstBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tOBST LogGap Bissection Cost: " << obstBissecResponse << std::endl;
        std::cout << "\tOBST LogGap Time Spent: " << obstSec << std::endl;

        std::ofstream obstCostsFile(
            baseFolderName + "obst-log-gap-bissection_costs.out", std::ios_base::app
        );
        obstCostsFile << obstBissecResponse << std::endl;

        std::ofstream obstTimeSpent(
            baseFolderName + "obst-log-gap-bissection_time_spent.out", std::ios_base::app
        );
        obstTimeSpent << obstSec << std::endl;
    }

    if (false) {
        std::cout << "OBST Only" << std::endl;

        const clock_t obstOnlyBeginTime = std::clock();
        double obstOnlyResponse = testOBST(vertices, demandMatrix);
        double obstOnlySec = double(std::clock() - obstOnlyBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tOBST Only Cost: " << obstOnlyResponse << std::endl;
        std::cout << "\tOBST Only Time Spent: " << obstOnlySec << std::endl;

        std::ofstream obstOnlyCostsFile(
            baseFolderName + "obst-only_costs.out", std::ios_base::app
        );
        obstOnlyCostsFile << obstOnlyResponse << std::endl;

        std::ofstream obstOnlyTimeSpent(
            baseFolderName + "obst-only_time_spent.out", std::ios_base::app
        );
        obstOnlyTimeSpent << obstOnlySec << std::endl;
    }

    if (false) {
        std::cout << "Greedy + Bissection" << std::endl;

        const clock_t greedyBissectionBeginTime = std::clock();
        double greedyBissectionResponse = testGreedy(orderedVertices, demandMatrix);
        double greedyBissectionSec = double(std::clock() - greedyBissectionBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tGreedy + Bissection Cost: " << greedyBissectionResponse << std::endl;
        std::cout << "\tGreedy + Bissection Time Spent: " << greedyBissectionSec << std::endl;

        std::ofstream greedyBissectionCostsFile(
            baseFolderName + "greedy-bissection_costs.out", std::ios_base::app
        );
        greedyBissectionCostsFile << greedyBissectionResponse << std::endl;

        std::ofstream greedyBissectionTimeSpent(
            baseFolderName + "greedy-bissection_time_spent.out", std::ios_base::app
        );
        greedyBissectionTimeSpent << greedyBissectionSec << std::endl;
    }

    if (false) {
        std::cout << "Greedy + MLogA Bissection" << std::endl;

        const clock_t greedyBissectionBeginTime = std::clock();
        double greedyBissectionResponse = testGreedy(mlogaOrderedVertices, demandMatrix);
        double greedyBissectionSec = double(std::clock() - greedyBissectionBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tGreedy + MLogA Bissection Cost: " << greedyBissectionResponse << std::endl;
        std::cout << "\tGreedy + MLogA Bissection Time Spent: " << greedyBissectionSec << std::endl;

        std::ofstream greedyBissectionCostsFile(
            baseFolderName + "greedy-log-gap-bissection_costs.out", std::ios_base::app
        );
        greedyBissectionCostsFile << greedyBissectionResponse << std::endl;

        std::ofstream greedyBissectionTimeSpent(
            baseFolderName + "greedy-log-gap-bissection_time_spent.out", std::ios_base::app
        );
        greedyBissectionTimeSpent << greedyBissectionSec << std::endl;
    }

    if (false) {
        std::cout << "Greedy + LogGap Bissection" << std::endl;

        const clock_t greedyBissectionBeginTime = std::clock();
        double greedyBissectionResponse = testGreedy(logGapOrderedVertices, demandMatrix);
        double greedyBissectionSec = double(std::clock() - greedyBissectionBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tGreedy + LogGap Bissection Cost: " << greedyBissectionResponse << std::endl;
        std::cout << "\tGreedy + LogGap Bissection Time Spent: " << greedyBissectionSec << std::endl;

        std::ofstream greedyBissectionCostsFile(
            baseFolderName + "greedy-log-gap-bissection_costs.out", std::ios_base::app
        );
        greedyBissectionCostsFile << greedyBissectionResponse << std::endl;

        std::ofstream greedyBissectionTimeSpent(
            baseFolderName + "greedy-log-gap-bissection_time_spent.out", std::ios_base::app
        );
        greedyBissectionTimeSpent << greedyBissectionSec << std::endl;
    }

    {
        std::cout << "Greedy Only" << std::endl;
        const clock_t greedyBeginTime = std::clock();
        double greedyResponse = testGreedy(vertices, demandMatrix);
        double greedySec = double(std::clock() - greedyBeginTime) / CLOCKS_PER_SEC;

        std::cout << "\tGreedy Cost: " << greedyResponse << std::endl;
        std::cout << "\tGreedy Time Spent: " << greedySec << std::endl;

        std::ofstream greedyCostsFile(
            baseFolderName + "greedy_costs.out", std::ios_base::app
        );
        greedyCostsFile << greedyResponse << std::endl;

        std::ofstream greedyTimeSpent(
            baseFolderName + "greedy_time_spent.out", std::ios_base::app
        );
        greedyTimeSpent << greedySec << std::endl;
    }
}
