#include <gtest/gtest.h>
#include "convertgraph.hh"

using namespace convertgraph;

TEST(ConvertGraphTest, MLogA_SimpleMatrix) {
    std::vector<std::vector<double>> demandMatrix = {
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 2.0},
        {0.0, 2.0, 0.0}
    };
    auto result = convertGraphToBipartiteGraphMLogA(demandMatrix);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], (std::vector<int>{0,1}));
    EXPECT_EQ(result[1], (std::vector<int>{1,2}));
}

TEST(ConvertGraphTest, MLogA_AllZeros) {
    std::vector<std::vector<double>> zeroMatrix(2, std::vector<double>(2, 0.0));
    auto result = convertGraphToBipartiteGraphMLogA(zeroMatrix);
    EXPECT_TRUE(result.empty());
}

TEST(ConvertGraphTest, MLogA_AsymmetricMatrix) {
    std::vector<std::vector<double>> asymMatrix = {
        {0.0, 3.0},
        {0.0, 0.0}
    };
    auto result = convertGraphToBipartiteGraphMLogA(asymMatrix);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], (std::vector<int>{0,1}));
}

TEST(ConvertGraphTest, MLogA_SingleVertex) {
    std::vector<std::vector<double>> single = {{0.0}};
    auto result = convertGraphToBipartiteGraphMLogA(single);
    EXPECT_TRUE(result.empty());
}

TEST(ConvertGraphTest, MLogA_LargeSparseMatrix) {
    std::vector<std::vector<double>> mat(5, std::vector<double>(5, 0.0));
    mat[0][4] = 1.0;
    mat[2][3] = 2.0;
    auto result = convertGraphToBipartiteGraphMLogA(mat);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], (std::vector<int>{0,4}));
    EXPECT_EQ(result[1], (std::vector<int>{2,3}));
}

TEST(ConvertGraphTest, MLogGapA_SimpleMatrix) {
    std::vector<std::vector<double>> demandMatrix = {
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 2.0},
        {0.0, 2.0, 0.0}
    };
    auto result = convertGraphToBipartiteGraphMLogGapA(demandMatrix);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], (std::vector<int>{1}));
    EXPECT_EQ(result[1], (std::vector<int>{0,2}));
    EXPECT_EQ(result[2], (std::vector<int>{1}));
}

TEST(ConvertGraphTest, MLogGapA_AllZeros) {
    std::vector<std::vector<double>> zeroMatrix(2, std::vector<double>(2, 0.0));
    auto result = convertGraphToBipartiteGraphMLogGapA(zeroMatrix);
    ASSERT_EQ(result.size(), 2);
    EXPECT_TRUE(result[0].empty());
    EXPECT_TRUE(result[1].empty());
}

TEST(ConvertGraphTest, MLogGapA_AsymmetricMatrix) {
    std::vector<std::vector<double>> asymMatrix = {
        {0.0, 3.0},
        {0.0, 0.0}
    };
    auto result = convertGraphToBipartiteGraphMLogGapA(asymMatrix);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], (std::vector<int>{1}));
    EXPECT_TRUE(result[1].empty());
}

TEST(ConvertGraphTest, MLogGapA_SingleVertex) {
    std::vector<std::vector<double>> single = {{0.0}};
    auto result = convertGraphToBipartiteGraphMLogGapA(single);
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(result[0].empty());
}

TEST(ConvertGraphTest, MLogGapA_FullMatrix) {
    std::vector<std::vector<double>> mat = {
        {0.0, 1.0, 2.0},
        {3.0, 0.0, 4.0},
        {5.0, 6.0, 0.0}
    };
    auto result = convertGraphToBipartiteGraphMLogGapA(mat);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], (std::vector<int>{1,2}));
    EXPECT_EQ(result[1], (std::vector<int>{0,2}));
    EXPECT_EQ(result[2], (std::vector<int>{0,1}));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
