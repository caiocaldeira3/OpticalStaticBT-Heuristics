#pragma once

#include <fstream>
#include <iostream>
#include <numeric>

class OrderingLogger {
public:
    OrderingLogger() = default;

    // Create constructor that receives filename and create file with header: algorithm, max_iterations, max_depth, total_cost, average_cost_gain, average_swapped_pairs, average_num_iterations
    OrderingLogger(std::string _outputDirectory) : outputDirectory(std::move(_outputDirectory)) {}

    OrderingLogger(int _maxIterations) : maxIterations(_maxIterations) {}

    OrderingLogger(std::string algorithm, int _maxIterations, int _maxDepth, std::string _outputDirectory)
        : algorithm(algorithm), maxIterations(_maxIterations), maxDepth(_maxDepth), outputDirectory(std::move(_outputDirectory)) {}

    void logSwappedPairs (int numSwapped) {
        swappedPairsPerIteration.push_back(numSwapped);
    }

    void logNumIterations (int numIterations) {
        numIterationPerRecursion.push_back(numIterations);
    }

    void logCostGain (double costGain) {
        costGainPerIteration.push_back(costGain);
    }

    void logTotalCost (double totalCost) {
        tCost = totalCost;
    }

    void logMLogACost (double mlogacost) {
        mLogACost = mlogacost;
    }

    void setFileName (const std::string& fileName) {
        outputDirectory = fileName;
    }

    void setAlgorithm (const std::string& algo) {
        algorithm = algo;
    }

    void setDatasetName (const std::string& name) {
        datasetName = name;
    }

    void setMaxIterations (int maxIter) {
        maxIterations = maxIter;
    }

    void setMaxDepth (int maxD) {
        maxDepth = maxD;
    }

    double getAverageCostGainPerRecursion () const {
        if (costGainPerIteration.empty()) return 0;
        double total = std::accumulate(
            costGainPerIteration.begin(),
            costGainPerIteration.end(), 0.0
        );
        return total / costGainPerIteration.size();
    }

    double getAverageSwappedPairsPerIteration () const {
        if (swappedPairsPerIteration.empty()) return 0;
        double total = std::accumulate(
            swappedPairsPerIteration.begin(),
            swappedPairsPerIteration.end(), 0.0
        );
        return total / swappedPairsPerIteration.size();
    }

    double getAverageNumIterationsPerRecursion () const {
        if (numIterationPerRecursion.empty()) return 0;
        double total = std::accumulate(
            numIterationPerRecursion.begin(),
            numIterationPerRecursion.end(), 0.0
        );
        return total / numIterationPerRecursion.size();
    }

    int getNumberOfMaxIterationsOccurences () const {
        int maxOccurences = 0;
        for (int i = 0; i < numIterationPerRecursion.size(); i++) {
            if (numIterationPerRecursion[i] == maxIterations) {
                maxOccurences++;
            }
        }
        return maxOccurences;
    }

    // Function to append information to an existing file in csv format with the following header: algorithm, max_iterations, max_depth, total_cost, average_cost_gain, average_swapped_pairs, average_num_iterations
    void pushToFile() const {
        std::ofstream outFile(outputDirectory + "/result.csv", std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file." << std::endl;
            return;
        }

        outFile << datasetName << ","
                << algorithm << ","
                << maxIterations << "," 
                << maxDepth << ","
                << tCost << "," 
                << mLogACost << "\n";

        outFile.close();
    }

    // clear all logged data
    void clear() {
        maxIterations = 0;
        maxDepth = 0;
        algorithm = "no specified";
        swappedPairsPerIteration.clear();
        numIterationPerRecursion.clear();
        costGainPerIteration.clear();
        tCost = 0.0;
    }

private:
    std::vector<int> swappedPairsPerIteration;
    std::vector<int> numIterationPerRecursion;
    std::vector<double> costGainPerIteration;

    int currIteration;
    int maxIterations;
    int maxDepth;

    double tCost = 0.0;
    double mLogACost = 0.0;

    std::string algorithm = "no specified"; // Name of the algorithm used
    std::string datasetName = "default"; // Name of the dataset used
    std::string outputDirectory = "output"; // Directory to save the results
};
