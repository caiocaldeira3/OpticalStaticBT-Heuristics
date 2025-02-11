#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>

#include "argparse/argparse.hh"
#include "manager.hh"

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    argparse::ArgumentParser parser("main_args");

    std::string inputName;
    parser.add_argument("input-name")
        .store_into(inputName)
        .help("the name of the input in weights folder");

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
    fs::create_directories(baseFolderName + "raw/");
    fs::create_directories(baseFolderName + "obst-bissection/");

    {
        std::ofstream rawCostFile(baseFolderName + "raw/" + std::to_string(testNumber) );

        const clock_t rawBeginTime = std::clock();
        Response_t rawBissecResponse = testRawGraphBissection(nVertices, demandMatrix);
        double rawSec = double(std::clock() - rawBeginTime) / CLOCKS_PER_SEC;

        std::cout << "Raw Bissection Cost: " << rawBissecResponse.cost << std::endl;
        std::cout << "Raw Time Spent: " << rawSec << std::endl;

        std::ofstream rawCostsFile(
            baseFolderName + "raw_costs.out", std::ios_base::app
        );
        rawCostsFile << rawBissecResponse.cost << std::endl;

        std::ofstream rawTimeSpent(
            baseFolderName + "raw_time_spent.out", std::ios_base::app
        );
        rawTimeSpent << rawSec << std::endl;

        std::ofstream orderingFile(
            baseFolderName + "orderings/ordering_" + std::to_string(testNumber) + ".out"
        );
        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            orderingFile << rawBissecResponse.graphOrdering[vIdx] << " ";
        }
        orderingFile << std::endl;
    }

    {
        std::ofstream obstCostFile(baseFolderName + "obst-bissection/" + std::to_string(testNumber) );

        const clock_t obstBeginTime = std::clock();
        Response_t obstBissecResponse = testBissectionPlusOBST(nVertices, demandMatrix);
        double obstSec = double(std::clock() - obstBeginTime) / CLOCKS_PER_SEC;

        std::cout << "OBST Bissection Cost: " << obstBissecResponse.cost << std::endl;
        std::cout << "OBST Time Spent: " << obstSec << std::endl;

        std::ofstream obstCostsFile(
            baseFolderName + "obst-bissection_costs.out", std::ios_base::app
        );
        obstCostsFile << obstBissecResponse.cost << std::endl;

        std::ofstream obstTimeSpent(
            baseFolderName + "obst-bissection_time_spent.out", std::ios_base::app
        );
        obstTimeSpent << obstSec << std::endl;

        std::ofstream orderingFile(
            baseFolderName + "orderings/ordering_" + std::to_string(testNumber) + ".out"
        );
        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            orderingFile << obstBissecResponse.graphOrdering[vIdx] << " ";
        }
        orderingFile << std::endl;
    }
}



