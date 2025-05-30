#pragma once


class OrderingLogger {
public:
    OrderingLogger(int _maxIterations) : maxIterations(_maxIterations) {}

    void logSwappedPairs (int numSwapped) {
        swappedPairsPerIteration.push_back(numSwapped);
    }

    void logNumIterations (int numIterations) {
        numIterationPerRecursion.push_back(numIterations);
    }

    void logCostGain (double costGain) {
        costGainPerIteration.push_back(costGain);
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

private:
    std::vector<int> swappedPairsPerIteration;
    std::vector<int> numIterationPerRecursion;
    std::vector<double> costGainPerIteration;

    int maxIterations;
};
