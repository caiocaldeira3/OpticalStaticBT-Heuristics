#pragma once


#include <set>
#include <memory>
#include <vector>
#include <algorithm>
#include <numeric> 

#include <core/util.hh>
#include <convertgraph.hh>


namespace basic {

double computeMoveGain(convertgraph::bipartiteGraph& graph, 
                        int vertex, 
                        std::vector<int>& D_a, 
                        std::vector<int>& D_b) {
    double gain = 0.0;

    //std::cout << "Computing move gain for vertex: " << vertex << std::endl;

    for (int t = 0; t < graph.size(); ++t) {
        //std::cout << "Checking graph at term: " << t << std::endl;
        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        //std::cout << "Vertex " << vertex << " found in graph at term " << t << std::endl;

        int n_Da = D_a.size();
        int n_Db = D_b.size();

        int da = std::count_if(D_a.begin(), D_a.end(), [&](int v) { return graph[t].find(v) != graph[t].end(); });
        int db = std::count_if(D_b.begin(), D_b.end(), [&](int v) { return graph[t].find(v) != graph[t].end(); });

        //std::cout << "da: " << da << ", db: " << db << std::endl;

        double addGainA = (da * std::log2(n_Da / (double)(da + 1)));
        double addGainB = (db * std::log2(n_Db / (double)(db + 1)));
        double removeGainA = ((da - 1) * std::log2((n_Da / (double)da)));
        double removeGainB = ((db + 1) * std::log2((n_Db / (double)(db + 2))));

        gain = gain + (addGainA + addGainB);
        gain = gain - (removeGainA + removeGainB);

        //std::cout << "Current gain: " << gain << std::endl;
    }

    return gain;
}

double computeMoveGainW(convertgraph::bipartiteGraph& graph, 
                        int vertex, 
                        std::vector<int>& D_a, 
                        std::vector<int>& D_b) {
    double gain = 0.0;

    // std::cout << "Computing move gain for vertex: " << vertex << std::endl;

    for (int t = 0; t < graph.size(); ++t) {
        //std::cout << "Checking graph at term: " << t << std::endl;
        if (graph[t].find(vertex) == graph[t].end()) {
            continue;
        }
        // std::cout << "Vertex " << vertex << " found in graph at term " << t << std::endl;

        int n_Da = D_a.size();
        int n_Db = D_b.size();

        double da = std::accumulate(D_a.begin(), D_a.end(), 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });
        double db = std::accumulate(D_b.begin(), D_b.end(), 0, [&](double acc, int v) { 
            return graph[t].find(v) != graph[t].end() ? acc + graph[t].at(v) : acc; 
        });
        // std::cout << "da: " << da << ", db: " << db << std::endl;

        gain = gain + ((da * std::log2(n_Da / (double)(da + 1))) + (db * std::log2(n_Db / (double)(db + 1))));
        gain = gain - (((da - graph[t].at(vertex)) * std::log2((n_Da / (double)da))) 
                    + ((db + graph[t].at(vertex)) * std::log2((n_Db / (double)(db + graph[t].at(vertex) + 1)))));

        // std::cout << "Current gain: " << gain << std::endl;
    }

    return gain;
}

std::vector<int> recursiveBisection(convertgraph::bipartiteGraph& graph, 
                                    std::vector<int>& vertices,
                                    int d,
                                    int maxDepth,
                                    int maxIterations) {

    // std::cout << "Recursive bisection at depth: " << d << ", vertices size: " << vertices.size() << std::endl;                                    

    if (vertices.size() <= 1 || d > maxDepth) {
        return vertices; // Base case: no further bisection possible
    }

    int mid = vertices.size() / 2;

    std::vector<int> D_a, D_b;
    D_a.assign(vertices.begin(), vertices.begin() + mid);
    D_b.assign(vertices.begin() + mid, vertices.end());

    std::vector<double> gainsA(D_a.size(), 0.0);
    std::vector<double> gainsB(D_b.size(), 0.0);

    //std::cout << "Initialize computing move gains for vertices in D_a and D_b" << std::endl;

    for (int it = 0; it < maxIterations; ++it) {
        for (int i = 0; i < mid; ++i) {
            int v = D_a[i];
            int u = D_b[i];
            gainsA[i] = basic::computeMoveGain(graph, v, D_a, D_b);
            gainsB[i] = basic::computeMoveGain(graph, v, D_b, D_a);
        }

        std::sort(gainsA.begin(), gainsA.end());
        std::sort(gainsB.begin(), gainsB.end());

        bool swapNeeded = false;
        for (int i = 0; i < mid; ++i) {
            if (gainsA[i] + gainsB[i] <= 0) { continue; }
            std::swap(D_a[i], D_b[i]);
            swapNeeded = true;
        }

        if (!swapNeeded) {
            //std::cout << "No swaps needed, exiting early." << std::endl;
            break; // No swaps needed, we are done
        }
    }
    
    std::vector<int> newVerticesA = recursiveBisection(graph, D_a, d + 1, maxDepth, maxIterations);
    std::vector<int> newVerticesB = recursiveBisection(graph, D_b, d + 1, maxDepth, maxIterations);
    vertices.clear();
    vertices.insert(vertices.end(), newVerticesA.begin(), newVerticesA.end());
    vertices.insert(vertices.end(), newVerticesB.begin(), newVerticesB.end());

    return vertices;
}

} // namespace basic