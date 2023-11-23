#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "include/greedy.hh"


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

    std::vector<int> tree = greedyConstructor(queries, nVertices, totalCost);

    namespace fs = std::filesystem;
    fs::create_directories("output/" + locality + "/");

    std::ofstream predFile("output/" + locality + "/" + std::to_string(testNumber) + ".out");
    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        if (vIdx != 0) {
            predFile << ",";
        }

        predFile << tree[vIdx];
    }
    predFile << std::endl;

    std::ofstream costsFile("output/" + locality + "_costs.out", std::ios_base::app);
    costsFile << totalCost << std::endl;

}