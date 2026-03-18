#pragma once

#include <set>
#include <memory>
#include <vector>
#include <algorithm>
#include <numeric>

#include <core/util.hh>
#include <core/logging.hh>
#include <core/log_level.hh>
#include <convertgraph.hh>


namespace graphBisection {

struct Gain {
    double gain;
    int vertexIdx;
};

bool compareCostGainDecreasing (const Gain& a, const Gain& b) {
    return a.gain > b.gain;
}

double computeMoveGainPrevious(convertgraph::bipartiteGraph& graph, 
                        int vertex, 
                        std::vector<int>& D_a, 
                        std::vector<int>& D_b) {
    double gain = 0.0;

    log(LogLevel::Debug) << "Computing move gain for vertex: " << vertex << "\n";

    for (int t = 0; t < graph.size(); ++t) {
        log(LogLevel::Debug) << "Checking graph at term: " << t << "\n";
        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        log(LogLevel::Debug) << "Vertex " << vertex << " found in graph at term " << t << "\n";

        int n_Da = D_a.size();
        int n_Db = D_b.size();

        int da = std::count_if(D_a.begin(), D_a.end(), [&](int v) { return graph[t].find(v) != graph[t].end(); });
        int db = std::count_if(D_b.begin(), D_b.end(), [&](int v) { return graph[t].find(v) != graph[t].end(); });

        log(LogLevel::Debug) << "da: " << da << ", db: " << db << "\n";

        double addGainA = (da * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (db * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = ((da - 1) * std::log2((n_Da / (double)da)));
        double removeGainB = ((db + 1) * std::log2((n_Db / (double)(db + 2))));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        log(LogLevel::Debug) << "Current gain: " << gain << "\n";
    }

    if (std::isnan(gain) || std::isinf(gain)) {
        throw std::runtime_error("Cost gain is NaN or Inf.");
    }

    return gain;
}

double computeMoveGain(convertgraph::bipartiteGraph& graph, 
                        int vertex,
                        std::vector<int>& vertices,
                        int a_begin, int a_end,
                        int b_begin, int b_end) {
    double gain = 0.0;

    log(LogLevel::Debug) << "Computing move gain for vertex: " << vertex << "\n";

    int n_Da = a_end - a_begin;
    int n_Db = b_end - b_begin;

    log(LogLevel::Debug) << "n_Da: " << n_Da << ", n_Db: " << n_Db << "\n";

    for (int t = 0; t < graph.size(); ++t) {
        log(LogLevel::Debug) << "Checking graph at term: " << t << "\n";
        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        log(LogLevel::Debug) << "Vertex " << vertex << " found in graph at term " << t << "\n";

        int da = std::count_if(vertices.begin() + a_begin, vertices.begin() + a_end, [&](int v) {
            return graph[t].find(v) != graph[t].end();
        });
        int db = std::count_if(vertices.begin() + b_begin, vertices.begin() + b_end, [&](int v) {
            return graph[t].find(v) != graph[t].end();
        });

        log(LogLevel::Debug) << "da: " << da << ", db: " << db << "\n";

        double addGainA = (da * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (db * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = ((da - 1) * std::log2((n_Da / (double)da)));
        double removeGainB = ((db + 1) * std::log2((n_Db / (double)(db + 2))));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        log(LogLevel::Debug) << "Current gain: " << gain << "\n";
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

    log(LogLevel::Debug) << "Computing move gain for vertex: " << vertex << "\n";

    int n_Da = a_end - a_begin;
    int n_Db = b_end - b_begin;

    log(LogLevel::Debug) << "n_Da: " << n_Da << ", n_Db: " << n_Db << "\n";

    for (int t = 0; t < graph.size(); ++t) {
        log(LogLevel::Debug) << "Checking graph at term: " << t << "\n";

        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        log(LogLevel::Debug) << "Vertex " << vertex << " found in graph at term " << t << "\n";

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
        log(LogLevel::Debug) << "da: " << da << ", db: " << db << "\n";

        double addGainA = (w_a * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (w_b * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = (w_a - graph[t].at(vertex)) * std::log2(n_Da / (double)(da));
        double removeGainB = (w_b + graph[t].at(vertex)) * std::log2(n_Db / (double)(db + 2));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        log(LogLevel::Debug) << "Current gain: " << gain << "\n";
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

    log(LogLevel::Debug) << "Computing move gain for vertex: " << vertex << "\n";

    for (int t = 0; t < graph.size(); ++t) {
        log(LogLevel::Debug) << "Checking graph at term: " << t << "\n";

        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        log(LogLevel::Debug) << "Vertex " << vertex << " found in graph at term " << t << "\n";
        log(LogLevel::Debug) << "W_a: " << W_a << ", W_b: " << W_b << "\n";

        double w_a = std::accumulate(vertices.begin() + a_begin, vertices.begin() + a_end, 0, [&](double acc, int v) {
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc;
        });
        double w_b = std::accumulate(vertices.begin() + b_begin, vertices.begin() + b_end, 0, [&](double acc, int v) {
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc;
        });

        log(LogLevel::Debug) << "w_a: " << w_a << ", w_b: " << w_b << "\n";

        double addGainA = (w_a * std::log2(W_a / (double)(w_a + 1)));
        double addGainB = isClose(W_a, 0) ? 0 : (w_b * std::log2(W_b / (double)(w_b + 1)));
        double removeGainA = (w_a - graph[t].at(vertex)) * std::log2(W_a / (double)(w_a));
        double removeGainB = isClose(W_b, 0) ? 0 : (w_b + graph[t].at(vertex)) * std::log2(W_b / (double)(w_b + graph[t].at(vertex)));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        log(LogLevel::Debug) << "Current gain: " << gain << "\n";
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

    log(LogLevel::Info) << "Initialize computing move gains for vertices in D_a and D_b\n";
    int iterationsWithSwap = 0;

    // Perform local optimization with a fixed number of iterations
    for (int it = 0; it < maxIterations; ++it) {
        std::vector<Gain> gainsA, gainsB;

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
            log(LogLevel::Info) << "No swaps needed, exiting early.\n";
            break; // No swaps needed, we are done
        } else {
            iterationsWithSwap++;
        }
    }

    log(LogLevel::Info) << "begin: " << begin << " end: " << end << " mid: " << mid << " iterations with swap: " << iterationsWithSwap << "\n";
    
    // Recurse on the two halves
    recursiveBisection(graph, vertices, begin, mid, d + 1, maxDepth, maxIterations, logger, computeGainFunc);
    recursiveBisection(graph, vertices, mid, end, d + 1, maxDepth, maxIterations, logger, computeGainFunc);

    return;
}

template<typename Func>
std::vector<int> recursiveBisectionPrevious(convertgraph::bipartiteGraph& graph, 
                                            std::vector<int>& vertices,
                                            int d,
                                            int maxDepth,
                                            int maxIterations,
                                            OrderingLogger& logger,
                                            Func computeGainFunc) {

    // std::cout << "Recursive bisection at depth: " << d << ", vertices size: " << vertices.size() << std::endl;                                    

    if (vertices.size() <= 1 || d >= maxDepth) {
        return vertices; // Base case: no further bisection possible
    }

    int mid = vertices.size() / 2;

    std::vector<int> D_a, D_b;
    D_a.assign(vertices.begin(), vertices.begin() + mid);
    D_b.assign(vertices.begin() + mid, vertices.end());

    log(LogLevel::Info) << "Initialize computing move gains for vertices in D_a and D_b\n";
    int iterationsWithSwap = 0;

    for (int it = 0; it < maxIterations; ++it) {
        std::vector<Gain> gainsA, gainsB;

        for (int i = 0; i < D_a.size(); ++i) {
            int v = D_a[i];
            gainsA.push_back({computeGainFunc(graph, v, D_a, D_b), i});
        }

        for (int i = 0; i < D_b.size(); ++i) {
            int v = D_b[i];
            gainsB.push_back({computeGainFunc(graph, v, D_b, D_a), i});
        }

        std::sort(gainsA.begin(), gainsA.end(), compareCostGainDecreasing);
        std::sort(gainsB.begin(), gainsB.end(), compareCostGainDecreasing);

        bool swapNeeded = false;
        for (int i = 0; i < mid; ++i) {
            if (gainsA[i].gain + gainsB[i].gain <= 0) { continue; }
            std::swap(D_a[gainsA[i].vertexIdx], D_b[gainsB[i].vertexIdx]);
            swapNeeded = true;
        }

        if (!swapNeeded) {
            log(LogLevel::Info) << "No swaps needed, exiting early.\n";
            break; // No swaps needed, we are done
        } else {
            iterationsWithSwap++;
        }

        if (it < 5) {
            log(LogLevel::Debug) << "D_a size: " << D_a.size() << ", D_b size: " << D_b.size() << "\n";
            log(LogLevel::Debug) << "Iteration " << it << ":\n";
            log(LogLevel::Debug) << "Gains A: ";
            for (const auto& gain : gainsA) {
                log(LogLevel::Debug) << gain.gain << " ";
            }
            log(LogLevel::Debug) << "\n";
            log(LogLevel::Debug) << "Gains B: ";
            for (const auto& gain : gainsB) {
                log(LogLevel::Debug) << gain.gain << " ";
            }
            log(LogLevel::Debug) << "\n";
        }
    }

    log(LogLevel::Info) << "iterations with swap: " << iterationsWithSwap << "\n";
    
    std::vector<int> newVerticesA = recursiveBisectionPrevious(graph, D_a, d + 1, maxDepth, maxIterations, logger, computeGainFunc);
    std::vector<int> newVerticesB = recursiveBisectionPrevious(graph, D_b, d + 1, maxDepth, maxIterations, logger, computeGainFunc);
    vertices.clear();
    vertices.insert(vertices.end(), newVerticesA.begin(), newVerticesA.end());
    vertices.insert(vertices.end(), newVerticesB.begin(), newVerticesB.end());

    return vertices;
}

double computeBalancedBinaryTreeCostAfterReordering(std::vector<int>& vertices, 
                                                    convertgraph::bipartiteGraph& graph, 
                                                    const std::vector<std::vector<double>>& demandMatrix,
                                                    int maxDepth,
                                                    int maxIterations,
                                                    OrderingLogger& logger) {
    int nVertices = vertices.size();
                                                        
    //recursiveBisection(graph, vertices, 0 /*begin*/, nVertices/*end*/, 0 /*current level*/, maxDepth, maxIterations, logger, basic::computeMoveGain);

    std::vector<int> reorder = recursiveBisectionPrevious(graph, vertices, 0, maxDepth, maxIterations, logger, graphBisection::computeMoveGainPrevious);                                                    

    std::vector<std::vector<int>> tree(nVertices, std::vector<int>());
    buildBalancedBinaryTree(vertices, tree, {0, nVertices}, -1);

    double totalCost = treeCost(tree, demandMatrix);
    logger.logTotalCost(totalCost);

    return totalCost;
}


} // namespace graphBisection
