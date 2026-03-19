#include <gtest/gtest.h>
#include <vector>
#include <cstdint>

#include "util/forwardIndex.hh"

// ── Construction ─────────────────────────────────────────────────────────────

TEST(ForwardIndexTest, DefaultConstruction_TermCountIsZero) {
    pisa::forwardIndex idx;
    EXPECT_EQ(idx.termCount(), 0);
}

TEST(ForwardIndexTest, Construction_PreservesTermCount) {
    std::vector<std::vector<uint32_t>> docTerms = {{0, 1}, {2, 3}};
    pisa::forwardIndex idx(docTerms, 10);
    EXPECT_EQ(idx.termCount(), 10);
}

TEST(ForwardIndexTest, Construction_EmptyDocTerms) {
    std::vector<std::vector<uint32_t>> docTerms = {};
    pisa::forwardIndex idx(docTerms, 5);
    EXPECT_EQ(idx.termCount(), 5);
}

// ── terms() ──────────────────────────────────────────────────────────────────

TEST(ForwardIndexTest, Terms_SingleDocument) {
    std::vector<std::vector<uint32_t>> docTerms = {{3, 7, 11}};
    pisa::forwardIndex idx(docTerms, 20);

    auto result = idx.terms(0);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 3);
    EXPECT_EQ(result[1], 7);
    EXPECT_EQ(result[2], 11);
}

TEST(ForwardIndexTest, Terms_MultipleDocuments) {
    std::vector<std::vector<uint32_t>> docTerms = {
        {0, 1, 2},
        {10, 20},
        {100}
    };
    pisa::forwardIndex idx(docTerms, 200);

    auto t0 = idx.terms(0);
    ASSERT_EQ(t0.size(), 3);
    EXPECT_EQ(t0[0], 0);
    EXPECT_EQ(t0[1], 1);
    EXPECT_EQ(t0[2], 2);

    auto t1 = idx.terms(1);
    ASSERT_EQ(t1.size(), 2);
    EXPECT_EQ(t1[0], 10);
    EXPECT_EQ(t1[1], 20);

    auto t2 = idx.terms(2);
    ASSERT_EQ(t2.size(), 1);
    EXPECT_EQ(t2[0], 100);
}

TEST(ForwardIndexTest, Terms_EmptyDocument) {
    std::vector<std::vector<uint32_t>> docTerms = {
        {1, 2},
        {},
        {5}
    };
    pisa::forwardIndex idx(docTerms, 10);

    auto t1 = idx.terms(1);
    EXPECT_TRUE(t1.empty());
}

TEST(ForwardIndexTest, Terms_AllDocumentsEmpty) {
    std::vector<std::vector<uint32_t>> docTerms = {{}, {}, {}};
    pisa::forwardIndex idx(docTerms, 5);

    for (uint32_t d = 0; d < 3; ++d) {
        EXPECT_TRUE(idx.terms(d).empty());
    }
}

TEST(ForwardIndexTest, Terms_ReturnsIndependentCopy) {
    std::vector<std::vector<uint32_t>> docTerms = {{4, 5, 6}};
    pisa::forwardIndex idx(docTerms, 10);

    auto copy1 = idx.terms(0);
    auto copy2 = idx.terms(0);
    copy1[0] = 999;

    EXPECT_EQ(copy2[0], 4);  // copy2 unaffected
}

TEST(ForwardIndexTest, Terms_LargeDocument) {
    std::vector<uint32_t> bigDoc(1000);
    for (uint32_t i = 0; i < 1000; ++i) bigDoc[i] = i;

    std::vector<std::vector<uint32_t>> docTerms = {bigDoc};
    pisa::forwardIndex idx(docTerms, 1000);

    auto result = idx.terms(0);
    ASSERT_EQ(result.size(), 1000);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[999], 999);
}
