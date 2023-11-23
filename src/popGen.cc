#include "util.hh"
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

int main (int argc, char* argv[]) {
    srand(42);

    if (argc != 2) {
        std::cout << "number of vertices need to be provide" << std::endl;
        throw;

    }

    int nVertices = std::stoi(argv[1]);

    for (int popIdx = 0; popIdx < 1000; popIdx++) {
        std::vector<int> preds(nVertices, -1);
        std::vector<int> degree(nVertices, 0);
        int root;

        while (!isValidBinaryTree(preds)) {
            preds = std::vector<int>(nVertices, -1);
            root = rand() % nVertices;
            for (int idx = 0; idx < nVertices; idx++) {
                if (idx == root) continue;
                int pred = rand() % nVertices;

                while (degree[pred] >= 2) {
                    pred = rand() % nVertices;

                }

                preds[idx] = pred;
            }
        }

        std::ofstream outFile(
            "pop/" + std::to_string(nVertices) + "_" + std::to_string(popIdx) + ".txt"
        );

        outFile << nVertices << "," << root << std::endl;
        for (int node = 0; node < nVertices; node++) {
            if (node != 0) outFile << ",";

            outFile << preds[node];

        }

        outFile << std::endl;
    }
}