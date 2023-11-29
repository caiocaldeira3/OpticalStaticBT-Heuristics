#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include "huffman.hh"
#include "greedy.hh"
#include "bbst.hh"
#include "optbst.hh"
#include "genetic.hh"

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    if (argc < 2) {
        std::cout << "filename inputName" << std::endl;
        throw;

    }

    std::string inputName = argv[1];
    int slash = inputName.find("/");
    int dash = inputName.find("-");
    int dot = inputName.find(".");
    int testNumber = std::stoi(inputName.substr(dash + 1, dot - dash - 1));
    std::string locality = inputName.substr(slash, dash - slash);

    bool shuffle = false;
    std::string shuffleArg;
    if (argc == 3 && "shuffle" == std::string(argv[2])) {
        shuffle = true;
        shuffleArg = "shuffle";
    } else {
        shuffle = false;
        shuffleArg = "static";
    }

    std::ifstream iFile(inputName);
    int nVertices, nMessages, gTotalCost = 0, hTotalCost = 0, optBstTotalCost = 0, genTotalCost = 0;
    iFile >> nVertices >> nMessages;

    std::vector<query> queries(nMessages);
    for (auto& [src, dst]: queries) {
        iFile >> src >> dst;

    }

    if (shuffle) {
        std::shuffle(queries.begin(), queries.end(), std::default_random_engine(seed));

    }

    std::string baseFolderName = (
        "output/" + std::to_string(nVertices) + "/" + std::to_string(nMessages) + "/" +
        locality + "/" + shuffleArg + "/"
    );

    namespace fs = std::filesystem;
    fs::create_directories(baseFolderName + "huffman/");
    fs::create_directories(baseFolderName + "greedy/");
    fs::create_directories(baseFolderName + "bbst/");
    fs::create_directories(baseFolderName + "optbst/");
    fs::create_directories(baseFolderName + "genetic/");

    std::ofstream gPredsFile(
        baseFolderName + "greedy/" + std::to_string(testNumber) + ".out"
    );

    const clock_t gBeginTime = std::clock();
    std::vector<int> gTree = greedyConstructor(queries, nVertices, gTotalCost);
    double gSec = double(std::clock() - gBeginTime) / CLOCKS_PER_SEC;

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        gPredsFile << gTree[vIdx] << std::endl;
    }

    std::ofstream gCostsFile(
        baseFolderName + "greedy_costs.out", std::ios_base::app
    );
    gCostsFile << gTotalCost << std::endl;

    std::ofstream gTimeSpent(
        baseFolderName + "greedy_time_spent.out", std::ios_base::app
    );
    gTimeSpent << gSec << std::endl;

    assert (gTotalCost == treeCost(gTree, queries));

    std::ofstream hPredFile(
        baseFolderName + "huffman/" + std::to_string(testNumber) + ".out"
    );

    const clock_t hBeginTime = std::clock();
    std::vector<int> hTree = huffmanHeuristic(queries, nVertices, hTotalCost);
    double hSec = double(std::clock() - hBeginTime) / CLOCKS_PER_SEC;

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        hPredFile << hTree[vIdx] << std::endl;
    }

    std::ofstream hCostsFile(
        baseFolderName + "huffman_costs.out", std::ios_base::app
    );
    hCostsFile << hTotalCost << std::endl;

    assert (hTotalCost == treeCost(hTree, queries));

    std::ofstream hTimeSpent(
        baseFolderName + "huffman_time_spent.out", std::ios_base::app
    );
    hTimeSpent << hSec << std::endl;

    std::vector<int> bTree(nVertices);
    buildBalancedBST(0, nVertices, bTree);

    std::ofstream bPredsFile(
        baseFolderName + "bbst/" + std::to_string(nVertices) + "_bbst.out"
    );
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        bPredsFile << bTree[vIdx] << std::endl;
    }

    std::ofstream bCostsFile(
        baseFolderName + "bbst_costs.out", std::ios_base::app
    );
    bCostsFile << treeCost(bTree, queries) << std::endl;

    std::ofstream optBstPredFile(
        baseFolderName + "optbst/" + std::to_string(testNumber) + ".out"
    );

    const clock_t optBstBeginTime = std::clock();
    std::vector<int> optBstTree = optimalBST(queries, nVertices, optBstTotalCost);
    double optBstSec = double(std::clock() - optBstBeginTime) / CLOCKS_PER_SEC;

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        optBstPredFile << optBstTree[vIdx] << std::endl;
    }

    std::ofstream optBstCostFile(
        baseFolderName + "optbst_costs.out", std::ios_base::app
    );
    optBstCostFile << optBstTotalCost << std::endl;

    assert (optBstTotalCost == treeCost(optBstTree, queries));

    std::ofstream optBstTimeSpent(
        baseFolderName + "optbst_time_spent.out", std::ios_base::app
    );
    optBstTimeSpent << optBstSec << std::endl;

    std::vector<std::vector<int>> othResponses = { hTree, gTree, bTree, optBstTree };

    if (!shuffle) {
        std::ofstream geneticPredFile(
            baseFolderName + "genetic/" + std::to_string(testNumber) + ".out"
        );

        int numGen;
        const clock_t genBeginTime = std::clock();
        std::vector<int> geneticTree = geneticAlgorithm(
            1000, 100, nVertices, genTotalCost, numGen, queries, othResponses=othResponses
        );
        double genSec = double(std::clock() - genBeginTime) / CLOCKS_PER_SEC;

        for (int vIdx = 0; vIdx < nVertices; vIdx++) {
            geneticPredFile << geneticTree[vIdx] << std::endl;
        }

        std::ofstream geneticCostFile(
            baseFolderName + "genetic_costs.out", std::ios_base::app
        );
        geneticCostFile << genTotalCost << std::endl;

        std::ofstream genTimeSpent(
            baseFolderName + "genetic_time_spent.out", std::ios_base::app
        );
        genTimeSpent << genSec << std::endl;

        std::ofstream genNumFile(
            baseFolderName + "genetic_generation_cost.out", std::ios_base::app
        );
        genNumFile << numGen << std::endl;
    }
}