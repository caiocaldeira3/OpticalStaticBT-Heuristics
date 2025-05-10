#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include <argparse/argparse.hh>
#include <core/util.hh>
#include <core/bounds.hh>

int main (int argc, char* argv[]) {
    argparse::ArgumentParser parser("entropy_args");

    std::string inputName;
    parser.add_argument("input-name")
        .store_into(inputName)
        .help("the name of the input in weights folder");

    bool storeEntropy;
    parser.add_argument("-s")
        .default_value(false)
        .store_into(storeEntropy)
        .help("if the entropy should be stored");

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

    std::cout << "Joint Entropy (X, Y): " << computeJointEntropy(demandMatrix) << std::endl;

    std::cout << "Conditional Entropy (X|Y) and (Y|X):" << std::endl;
    ConditionalEntropy_T conditionalEntropy = computeConditionalEntropy(demandMatrix);
    std::cout << "\tE(X|Y): " << conditionalEntropy.entropyXgivenY << std::endl;
    std::cout << "\tE(Y|X): " << conditionalEntropy.entropyYgivenX << std::endl;
    std::cout << "\tE(X|Y) + E(Y|X): " << conditionalEntropy.entropyXgivenY + conditionalEntropy.entropyYgivenX << std::endl;

    std::cout << "Marginal Entropy (X) and (Y):" << std::endl;
    MarginalEntropy_T marginalEntropy = computeMarginalEntropy(demandMatrix);
    std::cout << "\tE(X): " << marginalEntropy.entropyX << std::endl;
    std::cout << "\tE(Y): " << marginalEntropy.entropyY << std::endl;
    std::cout << "\tE(X) + E(Y): " << marginalEntropy.entropyX + marginalEntropy.entropyY << std::endl;

    double mutualInformation = (
        marginalEntropy.entropyX + marginalEntropy.entropyY -
        conditionalEntropy.entropyXgivenY - conditionalEntropy.entropyYgivenX
    );
    std::cout << "Mutual Information (X; Y): " << mutualInformation << std::endl;

    if (storeEntropy) {


    }

}
