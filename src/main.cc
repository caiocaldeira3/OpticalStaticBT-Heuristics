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

    namespace fs = std::filesystem;
    fs::create_directories("output/" + locality + "/" + shuffleArg + "/huffman/");
    fs::create_directories("output/" + locality + "/" + shuffleArg + "/greedy/");
    fs::create_directories("output/" + locality + "/" + shuffleArg + "/bbst/");
    fs::create_directories("output/" + locality + "/" + shuffleArg + "/optbst/");
    fs::create_directories("output/" + locality + "/" + shuffleArg + "/genetic/");

    std::ofstream gPredsFile(
        "output/" + locality + "/" + shuffleArg + "/greedy/" + std::to_string(testNumber) + ".out"
    );
    std::vector<int> gTree = greedyConstructor(queries, nVertices, gTotalCost);

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        gPredsFile << gTree[vIdx] << std::endl;
    }

    std::ofstream gCostsFile(
        "output/" + locality + "/" + shuffleArg + "/greedy_costs.out", std::ios_base::app
    );
    gCostsFile << gTotalCost << std::endl;

    assert (gTotalCost == treeCost(gTree, queries));

    std::ofstream hPredFile(
        "output/" + locality + "/" + shuffleArg + "/huffman/" + std::to_string(testNumber) + ".out"
    );
    std::vector<int> hTree = huffmanHeuristic(queries, nVertices, hTotalCost);

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        hPredFile << hTree[vIdx] << std::endl;
    }

    std::ofstream hCostsFile(
        "output/" + locality + "/" + shuffleArg + "/huffman_costs.out", std::ios_base::app
    );
    hCostsFile << hTotalCost << std::endl;

    assert (hTotalCost == treeCost(hTree, queries));

    std::vector<int> bTree(nVertices);
    buildBalancedBST(0, nVertices, bTree);

    std::ofstream bPredsFile(
        "output/" + locality + "/" + shuffleArg + "/bbst/" + std::to_string(nVertices) + "_bbst.out"
    );
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        bPredsFile << bTree[vIdx] << std::endl;
    }

    std::ofstream bCostsFile(
        "output/" + locality + "/" + shuffleArg + "/bbst_costs.out", std::ios_base::app
    );
    bCostsFile << treeCost(bTree, queries) << std::endl;

    std::ofstream optBstPredFile(
        "output/" + locality + "/" + shuffleArg + "/optbst/" + std::to_string(testNumber) + ".out"
    );
    std::vector<int> optBstTree = optimalBST(queries, nVertices, optBstTotalCost);

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        optBstPredFile << optBstTree[vIdx] << std::endl;
    }

    std::ofstream optBstCostFile(
        "output/" + locality + "/" + shuffleArg + "/optbst_costs.out", std::ios_base::app
    );
    optBstCostFile << optBstTotalCost << std::endl;

    assert (optBstTotalCost == treeCost(optBstTree, queries));

    std::ofstream geneticPredFile(
        "output/" + locality + "/" + shuffleArg + "/genetic/" + std::to_string(testNumber) + ".out"
    );
    std::vector<int> geneticTree = geneticAlgorithm(300, 100, nVertices, genTotalCost, queries);

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        geneticPredFile << geneticTree[vIdx] << std::endl;
    }

    std::ofstream geneticCostFile(
        "output/" + locality + "/" + shuffleArg + "/genetic_costs.out", std::ios_base::app
    );
    geneticCostFile << genTotalCost << std::endl;

}