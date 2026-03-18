#pragma once

#include <string>

struct RunConfig {
    std::string algorithm;
    std::string datasetName;
    int         maxIterations = 0;
    int         maxDepth      = 0;
    std::string outputDirectory = "output";
};
