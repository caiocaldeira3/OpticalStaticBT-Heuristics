#pragma once

#include <core/runConfig.hh>
#include <vector>
#include <string>

class BisectionRunRecord {
public:
    explicit BisectionRunRecord(RunConfig config) : config_(std::move(config)) {}

    // --- sample collection (called inside the algorithm) ---
    void recordSwappedPairs(int count) {
        swappedPairsSamples_.push_back(count);
    }

    void recordIterationCount(int count) {
        iterationCountSamples_.push_back(count);
    }

    void recordCostGain(double gain) {
        costGainSamples_.push_back(gain);
    }

    void recordTotalCost(double cost) {
        totalCost_ = cost;
    }
    
    void recordMLogACost(double cost) {
        mLogACost_ = cost;
    }

    // --- aggregate queries ---
    double averageCostGain() const {
        if (costGainSamples_.empty()) return 0.0;
        double total = std::accumulate(costGainSamples_.begin(), costGainSamples_.end(), 0.0);
        return total / costGainSamples_.size();
    }
    
    double averageSwappedPairs() const {
        if (swappedPairsSamples_.empty()) return 0.0;
        double total = std::accumulate(swappedPairsSamples_.begin(), swappedPairsSamples_.end(), 0.0);
        return total / swappedPairsSamples_.size();
    }
    
    double averageIterationCount() const {
        if (iterationCountSamples_.empty()) return 0.0;
        double total = std::accumulate(iterationCountSamples_.begin(), iterationCountSamples_.end(), 0.0);
        return total / iterationCountSamples_.size();
    }

    int maxIterationHitCount() const {
        int count = 0;
        for (int n : iterationCountSamples_) {
            if (n == config_.maxIterations)
                count++;
        }
        return count;
    }

    const RunConfig& config() const {
        return config_;
    }

    // --- persistence ---
    void appendToCsv() const {  // appends one row to <outputDirectory>/result.csv
        std::ofstream outFile(config_.outputDirectory + "/result.csv", std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file." << std::endl;
            return;
        }

        outFile << config_.datasetName << ","
                << config_.algorithm << ","
                << config_.maxIterations << ","
                << config_.maxDepth << ","
                << totalCost_ << ","
                << mLogACost_ << "\n";
    }
    
    void appendMetrics() const { // appends aggregates to <outputDirectory>/metrics.out
        const std::string path = config_.outputDirectory + "/metrics.out";

        // Write header if file does not yet exist or is empty
        bool writeHeader = false;
        {
            std::ifstream probe(path);
            writeHeader = !probe.is_open() || probe.peek() == std::ifstream::traits_type::eof();
        }

        std::ofstream outFile(path, std::ios::app);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open metrics file." << std::endl;
            return;
        }

        if (writeHeader) {
            outFile << "max-it-occ,avg-iterations,avg-swapped-pairs,avg-cost-gain\n";
        }

        outFile << maxIterationHitCount() << ","
                << averageIterationCount() << ","
                << averageSwappedPairs() << ","
                << averageCostGain() << "\n";
    }

    // --- lifecycle ---
    void reset() {  // clears only samples, preserves config
        costGainSamples_.clear();
        swappedPairsSamples_.clear();
        iterationCountSamples_.clear();
        totalCost_  = 0.0;
        mLogACost_  = 0.0;
    }

private:
    RunConfig config_;

    std::vector<double> costGainSamples_;
    std::vector<int>    swappedPairsSamples_;
    std::vector<int>    iterationCountSamples_;

    double totalCost_  = 0.0;
    double mLogACost_  = 0.0;
};
