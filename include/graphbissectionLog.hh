#pragma once

#include <set>
#include <memory>
#include <vector>
#include <algorithm>
#include <numeric>

#include <core/util.hh>
#include <core/logging.hh>
#include <convertgraph.hh>


namespace basic {

struct Gain {
    double gain;
    int vertexIdx;
};

bool compareCostGainDecreasing (const Gain& a, const Gain& b) {
    return a.gain > b.gain;
}

double computeMoveGain(convertgraph::bipartiteGraph& graph, 
                        int vertex,
                        std::vector<int>& vertices,
                        int a_begin, int a_end,
                        int b_begin, int b_end) {
    double gain = 0.0;

    // std::cout << "Computing move gain for vertex: " << vertex << std::endl;

    int n_Da = a_end - a_begin;
    int n_Db = b_end - b_begin;

    // std::cout << "n_Da: " << n_Da << ", n_Db: " << n_Db << std::endl;

    for (int t = 0; t < graph.size(); ++t) {
        // std::cout << "Checking graph at term: " << t << std::endl;
        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        // std::cout << "Vertex " << vertex << " found in graph at term " << t << std::endl;

        int da = std::count_if(vertices.begin() + a_begin, vertices.begin() + a_end, [&](int v) { 
            return graph[t].find(v) != graph[t].end(); 
        });
        int db = std::count_if(vertices.begin() + b_begin, vertices.begin() + b_end, [&](int v) { 
            return graph[t].find(v) != graph[t].end(); 
        });

        // std::cout << "da: " << da << ", db: " << db << std::endl;

        double addGainA = (da * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (db * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = ((da - 1) * std::log2((n_Da / (double)da)));
        double removeGainB = ((db + 1) * std::log2((n_Db / (double)(db + 2))));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        // std::cout << "Current gain: " << gain << std::endl;
    }

    if (std::isnan(gain) || std::isinf(gain)) {
        throw std::runtime_error("Cost gain is NaN or Inf.");
    }

    return gain;
}

double computeMoveGainWeighted(convertgraph::bipartiteGraph& graph, 
                                int vertex, 
                                std::vector<int>& vertices,
                                int a_begin, int a_end,
                                int b_begin, int b_end) {
    double gain = 0.0;

    std::cout << "Computing move gain for vertex: " << vertex << std::endl;

    int n_Da = a_end - a_begin;
    int n_Db = b_end - b_begin;

    std::cout << "n_Da: " << n_Da << ", n_Db: " << n_Db << std::endl;

    for (int t = 0; t < graph.size(); ++t) {
        std::cout << "Checking graph at term: " << t << std::endl;

        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        std::cout << "Vertex " << vertex << " found in graph at term " << t << std::endl;

        int da = std::count_if(vertices.begin() + a_begin, vertices.begin() + a_end, [&](int v) { 
            return graph[t].find(v) != graph[t].end(); 
        });
        int db = std::count_if(vertices.begin() + b_begin, vertices.begin() + b_end, [&](int v) { 
            return graph[t].find(v) != graph[t].end(); 
        });

        double w_a = std::accumulate(vertices.begin() + a_begin, vertices.begin() + a_end, 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });
        double w_b = std::accumulate(vertices.begin() + b_begin, vertices.begin() + b_end, 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });
        std::cout << "da: " << da << ", db: " << db << std::endl;

        double addGainA = (w_a * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (w_b * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = (w_a - graph[t].at(vertex)) * std::log2(n_Da / (double)(da));
        double removeGainB = (w_b + graph[t].at(vertex)) * std::log2(n_Db / (double)(db + 2));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        std::cout << "Current gain: " << gain << std::endl;
    }

    if (std::isnan(gain) || std::isinf(gain)) {
        throw std::runtime_error("Cost gain is NaN or Inf.");
    }

    return gain;
}

double computeMoveGainEntropy(convertgraph::bipartiteGraph& graph, 
                              int vertex, 
                              std::vector<int>& vertices,
                              int a_begin, int a_end,
                              int b_begin, int b_end) {
    double gain = 0.0;

    double W_a = 0, W_b = 0;
    for (int t = 0; t < graph.size(); ++t) {
        for (int v = a_begin; v < a_end; ++v) {
            W_a += graph[t].find(v) != graph[t].end() ? graph[t].at(v) : W_a;
        }

        for (int v = b_begin; v < b_end; ++v) {
            W_b += graph[t].find(v) != graph[t].end() ? graph[t].at(v) : W_a;
        }
    }    

    std::cout << "Computing move gain for vertex: " << vertex << std::endl;

    for (int t = 0; t < graph.size(); ++t) {
        std::cout << "Checking graph at term: " << t << std::endl;

        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        std::cout << "Vertex " << vertex << " found in graph at term " << t << std::endl;

        std::cout << "W_a: " << W_a << ", W_b: " << W_b << std::endl;

        double w_a = std::accumulate(vertices.begin() + a_begin, vertices.begin() + a_end, 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });
        double w_b = std::accumulate(vertices.begin() + b_begin, vertices.begin() + b_end, 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });

        std::cout << "w_a: " << w_a << ", w_b: " << w_b << std::endl;

        double addGainA = (w_a * std::log2(W_a / (double)(w_a + 1)));
        double addGainB = isClose(W_a, 0) ? 0 : (w_b * std::log2(W_b / (double)(w_b + 1)));
        double removeGainA = (w_a - graph[t].at(vertex)) * std::log2(W_a / (double)(w_a));
        double removeGainB = isClose(W_b, 0) ? 0 : (w_b + graph[t].at(vertex)) * std::log2(W_b / (double)(w_b + graph[t].at(vertex)));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        std::cout << "Current gain: " << gain << std::endl;
    }

    if (std::isnan(gain) || std::isinf(gain)) {
        throw std::runtime_error("Cost gain is NaN or Inf.");
    }

    return gain;
}

template<typename Func>
void recursiveBisection(convertgraph::bipartiteGraph& graph, 
                        std::vector<int>& vertices,
                        int begin, int end,
                        int d,
                        int maxDepth,
                        int maxIterations,
                        OrderingLogger& logger,
                        Func computeGainFunc) {
     
    int size = end - begin;
    
    if (size <= 1 || d >= maxDepth) {
        return; // Base case: no further bisection possible
    }

    int mid = size / 2 + begin;

    std::vector<Gain> gainsA, gainsB;

    std::cout << "Initialize computing move gains for vertices in D_a and D_b" << std::endl;
    int iterationsWithSwap = 0;

    // Perform local optimization with a fixed number of iterations
    for (int it = 0; it < maxIterations; ++it) {

        // Compute gains for vertices in D_a
        for (int i = begin; i < mid; ++i) {
            int v = vertices[i];
            gainsA.push_back({computeGainFunc(graph, v, vertices, begin, mid, mid, end), i});
        }

        // Compute gains for vertices in D_b
        for (int i = mid; i < end; ++i) {
            int v = vertices[i];
            gainsB.push_back({computeGainFunc(graph, v, vertices, mid, end, begin, mid), i});
        }

        // Sort gains in decreasing order
        std::sort(gainsA.begin(), gainsA.end(), compareCostGainDecreasing);
        std::sort(gainsB.begin(), gainsB.end(), compareCostGainDecreasing);

        // Perform swaps based on positive gains
        bool swapNeeded = false;
        for (int i = 0; i < std::min(gainsA.size(), gainsB.size()); ++i) {
            if (gainsA[i].gain + gainsB[i].gain <= 0) { break; }
            std::swap(vertices[gainsA[i].vertexIdx], vertices[gainsB[i].vertexIdx]);
            swapNeeded = true;
        }

        if (!swapNeeded) {
            std::cout << "No swaps needed, exiting early." << std::endl;
            break; // No swaps needed, we are done
        } else {
            iterationsWithSwap++;
        }
    }

    std::cout << "begin: " << begin << " end: " << end << " mid: " << mid << " iterations with swap: " << iterationsWithSwap << std::endl;
    
    // Recurse on the two halves
    recursiveBisection(graph, vertices, begin, mid, d + 1, maxDepth, maxIterations, logger, computeGainFunc);
    recursiveBisection(graph, vertices, mid, end, d + 1, maxDepth, maxIterations, logger, computeGainFunc);

    return;
}

double computeBalancedBinaryTreeCostAfterReordering(std::vector<int>& vertices, 
                                                    convertgraph::bipartiteGraph& graph, 
                                                    std::vector<std::vector<double>>& demandMatrix,
                                                    int maxDepth,
                                                    int maxIterations,
                                                    OrderingLogger& logger) {
    int nVertices = vertices.size();
                                                        
    recursiveBisection(graph, vertices, 0 /*begin*/, nVertices/*end*/, 0 /*current level*/, maxDepth, maxIterations, logger, basic::computeMoveGain);

    std::vector<std::vector<int>> tree(nVertices, std::vector<int>());
    buildBalancedBinaryTree(vertices, tree, {0, nVertices}, -1);

    double totalCost = treeCost(tree, demandMatrix);
    logger.logTotalCost(totalCost);

    return totalCost;
}


} // namespace basic
