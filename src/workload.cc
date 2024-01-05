#include <map>
#include <fstream>
#include <chrono>
#include <string>
#include <random>
#include <iostream>
#include <stdlib.h>


int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    if (argc < 4) {
        std::cout << "the script must be called with the probabilities" << std::endl;
        throw;

    }

    std::string weightName = argv[1];
    double temporalProb = atof(argv[2]);
    double error = atof(argv[3]);

    int nVertices, nMessages;
    std::map<std::pair<int,int>, int> weights;
    std::cin >> nVertices >> nMessages;

    std::ifstream weightFile("weights/" + weightName);

    int src, dst, weight;
    while (weightFile >> src >> dst >> weight) {
        weights[{std::min(src, dst), std::max(src, dst)}] = weight;

    }

    std::vector<std::pair<int, int>> pairs;
    for (int src = 0; src < nVertices; src++) {
        for (int dst = src + 1; dst < nVertices; dst++) {
            pairs.push_back({src, dst});

        }
    }

    for (int testIdx = 0; testIdx < 30; testIdx++) {
        double sum = 0;
        std::vector<double> ogProb, roundProb;
        for (int src = 0; src < nVertices; src++) {
            for (int dst = src + 1; dst < nVertices; dst++) {
                auto wIt = weights.find({src, dst});

                std::uniform_real_distribution<> errorDis(
                    -error, error
                );

                double weight = (
                    wIt == weights.end() ? 1.0 : wIt->second
                ) + errorDis(gen);

                ogProb.push_back(weight);

                sum += weight;
            }
        }

        for (double& w: ogProb)
            w /= sum;

        roundProb = ogProb;
        std::ofstream tFile(
            "input/" + weightName + "-" + std::to_string(testIdx) + ".txt"
        );

        tFile << nVertices << " " << nMessages << std::endl;

        int lastIdx = -1;
        for (int mIdx = 0; mIdx < nMessages; mIdx++) {
            std::discrete_distribution<int> pDis(
                std::begin(roundProb), std::end(roundProb)
            );

            int nextIdx = pDis(gen);
            tFile << pairs[nextIdx].first << " " << pairs[nextIdx].second << std::endl;

            if (nextIdx != lastIdx) {
                roundProb = ogProb;
                for (int pairIdx = 0; pairIdx < roundProb.size(); pairIdx++) {
                    roundProb[pairIdx] *= (1.0 - temporalProb);
                    if (pairIdx == nextIdx) {
                        roundProb[pairIdx] += temporalProb;

                    }
                }
            }

        }
    }
}