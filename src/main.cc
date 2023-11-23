#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "greedy.hh"
#include "bbst.hh"


int main (int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "filename inputName" << std::endl;
        throw;

    }

    std::string inputName = argv[1];
    int slash = inputName.find("/");
    int dash = inputName.find("-");
    int dot = inputName.find(".");
    int testNumber = std::stoi(inputName.substr(dash + 1, dot - dash - 1));
    std::string locality = inputName.substr(slash, dash - slash);

    std::ifstream iFile(inputName);

    int nVertices, nMessages, totalCost = 0;
    iFile >> nVertices >> nMessages;

    std::vector<query> queries(nMessages);
    for (auto& [src, dst]: queries) {
        iFile >> src >> dst;

    }

    std::vector<int> gTree = greedyConstructor(queries, nVertices, totalCost);

    namespace fs = std::filesystem;
    fs::create_directories("output/" + locality + "/greedy/");
    fs::create_directories("output/" + locality + "/bbst/");

    std::ofstream gPredsFile("output/" + locality + "/greedy/" + std::to_string(testNumber) + ".out");
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (vIdx != 0) {
            gPredsFile << ",";
        }

        gPredsFile << gTree[vIdx];
    }
    gPredsFile << std::endl;

    std::ofstream gCostsFile("output/" + locality + "/greedy_costs.out", std::ios_base::app);
    gCostsFile << totalCost << std::endl;

    std::vector<int> bTree(nVertices);
    buildBalancedBST(0, nVertices, bTree);

    std::ofstream bPredsFile("output/" + locality + "/bbst/" + std::to_string(nVertices) + "_bbst.out");
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (vIdx != 0) {
            bPredsFile << ",";
        }

        bPredsFile << bTree[vIdx];
    }
    bPredsFile << std::endl;

    std::ofstream bCostsFile("output/" + locality + "/bbst_costs.out", std::ios_base::app);
    bCostsFile << treeCost(bTree, queries) << std::endl;

}