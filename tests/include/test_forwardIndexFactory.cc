#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "util/forwardIndexFactory.hh"

// ── createLogGapForwardIndex ────────────────────────────────────────────────

TEST(LogGapForwardIndexTest, AllZeros_ProducesEmptyTermLists) {
    std::vector<std::vector<double>> dm = {
        {0.0, 0.0},
        {0.0, 0.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 2);
    EXPECT_TRUE(idx.terms(0).empty());
    EXPECT_TRUE(idx.terms(1).empty());
}

TEST(LogGapForwardIndexTest, SingleVertex_NoSelfLoop) {
    std::vector<std::vector<double>> dm = {{5.0}};
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 1);
    EXPECT_TRUE(idx.terms(0).empty());
}

TEST(LogGapForwardIndexTest, SymmetricMatrix_TermsAreNeighborIds) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 2.0},
        {0.0, 2.0, 0.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 3);

    auto t0 = idx.terms(0);
    ASSERT_EQ(t0.size(), 1);
    EXPECT_EQ(t0[0], 1);

    auto t1 = idx.terms(1);
    ASSERT_EQ(t1.size(), 2);
    EXPECT_EQ(t1[0], 0);
    EXPECT_EQ(t1[1], 2);

    auto t2 = idx.terms(2);
    ASSERT_EQ(t2.size(), 1);
    EXPECT_EQ(t2[0], 1);
}

TEST(LogGapForwardIndexTest, AsymmetricMatrix_BothDirectionsProduceTerms) {
    std::vector<std::vector<double>> dm = {
        {0.0, 3.0},
        {0.0, 0.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 2);

    auto t0 = idx.terms(0);
    ASSERT_EQ(t0.size(), 1);
    EXPECT_EQ(t0[0], 1);

    // vertex 1 has no outgoing non-zero edges
    EXPECT_TRUE(idx.terms(1).empty());
}

TEST(LogGapForwardIndexTest, FullyConnected_AllNeighborsPresent) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1.0, 2.0},
        {3.0, 0.0, 4.0},
        {5.0, 6.0, 0.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 3);

    for (uint32_t d = 0; d < 3; ++d) {
        auto terms = idx.terms(d);
        EXPECT_EQ(terms.size(), 2);
    }
}

TEST(LogGapForwardIndexTest, DiagonalIgnored) {
    // Only the diagonal has non-zero values
    std::vector<std::vector<double>> dm = {
        {9.0, 0.0},
        {0.0, 7.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_TRUE(idx.terms(0).empty());
    EXPECT_TRUE(idx.terms(1).empty());
}

TEST(LogGapForwardIndexTest, NearZeroTreatedAsZero) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1e-11},
        {1e-11, 0.0}
    };
    auto idx = pisa::createLogGapForwardIndex(dm);

    EXPECT_TRUE(idx.terms(0).empty());
    EXPECT_TRUE(idx.terms(1).empty());
}

// ── createMlogaForwardIndex ─────────────────────────────────────────────────

TEST(MlogaForwardIndexTest, AllZeros_NoEdges) {
    std::vector<std::vector<double>> dm = {
        {0.0, 0.0},
        {0.0, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 0);
    EXPECT_TRUE(idx.terms(0).empty());
    EXPECT_TRUE(idx.terms(1).empty());
}

TEST(MlogaForwardIndexTest, SingleVertex_SelfLoopIgnored) {
    std::vector<std::vector<double>> dm = {{0.0}};
    auto idx = pisa::createMlogaForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 0);
    EXPECT_TRUE(idx.terms(0).empty());
}

TEST(MlogaForwardIndexTest, SingleVertex_SelfLoopCounted) {
    // Non-zero diagonal: self-loop counts as an edge
    std::vector<std::vector<double>> dm = {{5.0}};
    auto idx = pisa::createMlogaForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 1);
    auto t0 = idx.terms(0);
    ASSERT_EQ(t0.size(), 1);
    EXPECT_EQ(t0[0], 0);
}

TEST(MlogaForwardIndexTest, SymmetricPair_SharedEdgeId) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1.0},
        {1.0, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    // One unique edge between vertex 0 and 1
    EXPECT_EQ(idx.termCount(), 1);

    auto t0 = idx.terms(0);
    auto t1 = idx.terms(1);
    ASSERT_EQ(t0.size(), 1);
    ASSERT_EQ(t1.size(), 1);
    EXPECT_EQ(t0[0], t1[0]); // same edge ID
}

TEST(MlogaForwardIndexTest, Triangle_ThreeEdges) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1.0, 1.0},
        {1.0, 0.0, 1.0},
        {1.0, 1.0, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    // Edges: (0,1), (0,2), (1,2) → 3 edge IDs
    EXPECT_EQ(idx.termCount(), 3);

    auto t0 = idx.terms(0);
    auto t1 = idx.terms(1);
    auto t2 = idx.terms(2);

    EXPECT_EQ(t0.size(), 2); // edges (0,1) and (0,2)
    EXPECT_EQ(t1.size(), 2); // edges (0,1) and (1,2)
    EXPECT_EQ(t2.size(), 2); // edges (0,2) and (1,2)

    // Vertex 0 and 1 share an edge
    std::vector<uint32_t> common01;
    std::set_intersection(t0.begin(), t0.end(), t1.begin(), t1.end(), std::back_inserter(common01));
    EXPECT_EQ(common01.size(), 1);
}

TEST(MlogaForwardIndexTest, AsymmetricWeights_CombinedForEdge) {
    // dm[0][1]=3, dm[1][0]=2 → weight for edge(0,1) = 5
    std::vector<std::vector<double>> dm = {
        {0.0, 3.0},
        {2.0, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 1);
    EXPECT_EQ(idx.terms(0).size(), 1);
    EXPECT_EQ(idx.terms(1).size(), 1);
}

TEST(MlogaForwardIndexTest, NearZeroEdge_Excluded) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1e-11},
        {1e-11, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    EXPECT_EQ(idx.termCount(), 0);
    EXPECT_TRUE(idx.terms(0).empty());
    EXPECT_TRUE(idx.terms(1).empty());
}

TEST(MlogaForwardIndexTest, SparseMatrix_CorrectEdgeCount) {
    std::vector<std::vector<double>> dm = {
        {0.0, 1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 2.0},
        {0.0, 0.0, 2.0, 0.0}
    };
    auto idx = pisa::createMlogaForwardIndex(dm);

    // Two edges: (0,1) and (2,3)
    EXPECT_EQ(idx.termCount(), 2);

    EXPECT_EQ(idx.terms(0).size(), 1);
    EXPECT_EQ(idx.terms(1).size(), 1);
    EXPECT_EQ(idx.terms(2).size(), 1);
    EXPECT_EQ(idx.terms(3).size(), 1);

    // Edges in different components have different IDs
    EXPECT_NE(idx.terms(0)[0], idx.terms(2)[0]);
}
