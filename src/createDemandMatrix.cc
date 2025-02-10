#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "argparse/argparse.hh"

int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    argparse::ArgumentParser parser("matrix_args");

    std::string dstPath;
    parser.add_argument("dst-path")
        .store_into(dstPath)
        .help("the path to the destination file");

    int nVertices;
    parser.add_argument("-n")
        .store_into(nVertices)
        .help("number of vertex in the graph");

    auto& srcGroup = parser.add_mutually_exclusive_group(true);
    int srcSize;
    double srcPct;
    srcGroup.add_argument("--src-size")
        .store_into(srcSize)
        .help("percentage of vertices that send data");

    srcGroup.add_argument("--src-pct")
        .store_into(srcPct)
        .help("percentage of vertices that send data");

    auto& dstGroup = parser.add_mutually_exclusive_group(true);
    int dstSize;
    double dstPct;
    dstGroup.add_argument("--dst-size")
        .store_into(dstSize)
        .help("percentage of vertices that receive data");

    dstGroup.add_argument("--dst-pct")
        .store_into(dstPct)
        .help("percentage of vertices that receive data");

    auto& flowType = parser.add_mutually_exclusive_group(true);
    flowType.add_argument("-u")
        .flag()
        .help("uniform flow distribution");

    try {
        parser.parse_args(argc, argv);
        if (parser.is_used("--src-pct")) {
            srcSize = nVertices * srcPct;

        }

        if (parser.is_used("--dst-pct")) {
            dstSize = nVertices * dstPct;

        }

    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::vector<int> srcShuffle;
    std::vector<int> dstShuffle;

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        srcShuffle.push_back(vIdx);
        dstShuffle.push_back(vIdx);
    }

    std::shuffle(srcShuffle.begin(), srcShuffle.end(), gen);
    std::shuffle(dstShuffle.begin(), dstShuffle.end(), gen);

    std::set<int> srcVertices;
    std::set<int> dstVertices;
    for (int idx = 0; idx < srcSize; idx++) {
        srcVertices.insert(srcShuffle[idx]);
    }

    for (int idx = 0; idx < dstSize; idx++) {
        dstVertices.insert(dstShuffle[idx]);
    }

    double srcFlowSize;
    if (parser.is_used("-u")) {
        srcFlowSize = 1.0 / srcSize;

    }

    std::cout << "Destination Path: " << dstPath << std::endl;
    std::cout << "Number of Vertices: " << nVertices << std::endl;
    std::cout << "Source Size: " << srcSize << std::endl;
    std::cout << "Destination Size: " << dstSize << std::endl;
    if (parser.is_used("--src-pct")) {
        std::cout << "Source Percentage: " << srcPct << std::endl;
    }
    if (parser.is_used("--dst-pct")) {
        std::cout << "Destination Percentage: " << dstPct << std::endl;
    }
    if (parser.is_used("-u")) {
        std::cout << "Uniform Flow Distribution selected" << std::endl;
    }
    std::cout << "Source Flow Size: " << srcFlowSize << std::endl;

    std::ofstream matrixFile("weights/" + dstPath);
    matrixFile << nVertices << std::endl;

    for (int src: srcVertices) {
        bool erased = dstVertices.erase(src);
        double flowSize = srcFlowSize / dstVertices.size();

        for (int dst: dstVertices) {
            matrixFile << src << "," << dst << "," << flowSize << std::endl;
        }

        if (erased) {
            dstVertices.insert(src);
        }
    }
}
