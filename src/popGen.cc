#include "util.hh"
#include <iostream>
#include <string>
#include <random>
#include <map>
#include <chrono>
#include <fstream>
#include <assert.h>
#include <filesystem>

int main (int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "number of vertices need to be provide" << std::endl;
        throw;

    }

    int nVertices = std::stoi(argv[1]);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> vDis(0, nVertices - 1);
    namespace fs = std::filesystem;
    fs::create_directories("pop/" + std::to_string(nVertices) + "/");

    for (int popIdx = 0; popIdx < 1000; popIdx++) {
        std::vector<int> preds(nVertices, -1);
        std::map<int, int> leafes;
        std::set<int> remV;
        int root = vDis(gen);
        leafes[root] = 2;

        for (int idx = 0; idx < nVertices; idx++) {
            if (idx == root) continue;
            remV.insert(idx);

        }

        while (!remV.empty()) {
            std::uniform_int_distribution<> lDis(0, leafes.size() - 1);
            std::uniform_int_distribution<> rDis(0, remV.size() - 1);
            auto leafIt = leafes.begin();
            auto remIt = remV.begin();

            std::advance(leafIt, lDis(gen));
            std::advance(remIt, rDis(gen));

            preds[*remIt] = leafIt->first;
            leafes[*remIt] = 2;
            remV.erase(remIt);

            if (leafIt->second == 1) {
                leafes.erase(leafIt);

            } else {
                leafIt->second--;

            }
        }

        assert (isValidBinaryTree(preds));

        std::ofstream outFile(
            "pop/" + std::to_string(nVertices) + "/" + std::to_string(popIdx) + ".txt"
        );

        for (int node = 0; node < nVertices; node++) {
            outFile << preds[node] << std::endl;

        }
    }
}