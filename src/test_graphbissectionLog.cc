#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

#include <convertgraph.hh>
#include <graphbissectionLog.hh>

void test_convertGraphToBipartiteGraphMLogA() {
    using namespace convertgraph;

    // Test 1: Simple 3x3 demand matrix
    std::vector<std::vector<double>> demandMatrix = {
        {0.0, 1.0, 2.0},
        {1.0, 0.0, 3.0},
        {2.0, 3.0, 0.0}
    };

    bipartiteGraph bGraph = convertGraphToBipartiteGraphMLogA(demandMatrix);

    // There should be 3 edges: (0,1), (0,2), (1,2)
    assert(bGraph.size() == 3);

    // Edge (0,1)
    assert(bGraph[0].size() == 2);
    assert(bGraph[0].at(0) == 1.0);
    assert(bGraph[0].at(1) == 1.0);

    // Edge (0,2)
    assert(bGraph[1].size() == 2);
    assert(bGraph[1].at(0) == 2.0);
    assert(bGraph[1].at(2) == 2.0);

    // Edge (1,2)
    assert(bGraph[2].size() == 2);
    assert(bGraph[2].at(1) == 3.0);
    assert(bGraph[2].at(2) == 3.0);

    std::cout << "test_convertGraphToBipartiteGraphMLogA passed.\n";
}

void test_convertGraphToBipartiteGraphMLogGapA() {
    using namespace convertgraph;

    // Test 2: Simple 3x3 demand matrix
    std::vector<std::vector<double>> demandMatrix = {
        {0.0, 1.0, 2.0},
        {1.0, 0.0, 3.0},
        {2.0, 3.0, 0.0}
    };

    bipartiteGraph bGraph = convertGraphToBipartiteGraphMLogGapA(demandMatrix);

    // There should be 3 nodes, each with their adjacency map
    assert(bGraph.size() == 3);

    // Node 0: edges to 1 and 2
    assert(bGraph[0].size() == 2);
    assert(bGraph[0].at(1) == 1.0);
    assert(bGraph[0].at(2) == 2.0);

    // Node 1: edges to 0 and 2
    assert(bGraph[1].size() == 2);
    assert(bGraph[1].at(0) == 1.0);
    assert(bGraph[1].at(2) == 3.0);

    // Node 2: edges to 0 and 1
    assert(bGraph[2].size() == 2);
    assert(bGraph[2].at(0) == 2.0);
    assert(bGraph[2].at(1) == 3.0);

    std::cout << "test_convertGraphToBipartiteGraphMLogGapA passed.\n";
}

void test_computeMoveGain_basic() {
    // Simple bipartite graph
    convertgraph::bipartiteGraph graph(4);
    graph[0][0] = 1;
    graph[0][1] = 1;
    graph[1][0] = 1;
    graph[1][2] = 1;
    graph[2][1] = 1;
    graph[2][2] = 1;
    graph[3][2] = 1;
    graph[3][3] = 1;

    std::vector<int> D_a = {0, 2};
    std::vector<int> D_b = {1, 3};

    // Test move gain for vertex 0
    double gain = basic::computeMoveGain(graph, 0, D_a, D_b);
    std::cout << "computeMoveGain (vertex 0): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test move gain for vertex 1
    gain = basic::computeMoveGain(graph, 1, D_b, D_a);
    std::cout << "computeMoveGain (vertex 1): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test move gain for vertex 2
    gain = basic::computeMoveGain(graph, 2, D_a, D_b);
    std::cout << "computeMoveGain (vertex 2): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test move gain for vertex 3
    gain = basic::computeMoveGain(graph, 3, D_b, D_a);
    std::cout << "computeMoveGain (vertex 3): " << gain << std::endl;
    assert(std::isfinite(gain));
}

void test_computeMoveGainW_basic() {
    // Weighted bipartite graph: 2 rows, 4 vertices
    convertgraph::bipartiteGraph graph(4);
    graph[0][0] = 1;
    graph[0][1] = 1;
    graph[1][0] = 1;
    graph[1][2] = 1;
    graph[2][1] = 1;
    graph[2][2] = 1;
    graph[3][2] = 1;
    graph[3][3] = 1;

    std::vector<int> D_a = {0, 2};
    std::vector<int> D_b = {1, 3};

    // Test weighted move gain for vertex 0
    double gain = basic::computeMoveGainW(graph, 0, D_a, D_b);
    std::cout << "computeMoveGainW (vertex 0): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test weighted move gain for vertex 1
    gain = basic::computeMoveGainW(graph, 1, D_b, D_a);
    std::cout << "computeMoveGainW (vertex 1): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test weighted move gain for vertex 2
    gain = basic::computeMoveGainW(graph, 2, D_a, D_b);
    std::cout << "computeMoveGainW (vertex 2): " << gain << std::endl;
    assert(std::isfinite(gain));

    // Test weighted move gain for vertex 3
    gain = basic::computeMoveGainW(graph, 3, D_b, D_a);
    std::cout << "computeMoveGainW (vertex 3): " << gain << std::endl;
    assert(std::isfinite(gain));
}

void test_recursiveBisection_basic() {
    // Simple bipartite graph: 1 row, 4 vertices
    convertgraph::bipartiteGraph graph(4);
    graph[0][0] = 1;
    graph[0][1] = 1;
    graph[1][0] = 1;
    graph[1][2] = 1;
    graph[2][1] = 1;
    graph[2][2] = 1;
    graph[3][2] = 1;
    graph[3][3] = 1;

    std::vector<int> vertices = {0, 2, 1, 3};
    int maxDepth = 3;
    int maxIterations = 3;

    std::vector<int> result = basic::recursiveBisection(graph, vertices, 0, maxDepth, maxIterations);
    std::cout << "recursiveBisection result size: " << result.size() << std::endl;
    assert(result.size() == 4);

    // Should contain all original vertices
    for (int v = 0; v < 4; ++v) {
        assert(std::find(result.begin(), result.end(), v) != result.end());
    }

    std::cout << "recursiveBisection result: ";
    for (int v : result) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

int main (int argc, char* argv[]) {
    test_convertGraphToBipartiteGraphMLogA();
    test_convertGraphToBipartiteGraphMLogGapA();
    test_computeMoveGain_basic();
    test_computeMoveGainW_basic();
    test_recursiveBisection_basic();
    std::cout << "All graphbissectionLog tests passed!" << std::endl;
}