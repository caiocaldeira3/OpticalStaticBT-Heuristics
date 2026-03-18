#include <gtest/gtest.h>
#include <numeric>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "core/bisectionRunRecord.hh"

namespace fs = std::filesystem;

// ── helpers ──────────────────────────────────────────────────────────────────

static RunConfig makeConfig(int maxIter = 5, int maxDepth = 3,
                            const std::string& outDir = "") {
    RunConfig cfg;
    cfg.algorithm       = "test-algo";
    cfg.datasetName     = "test-dataset";
    cfg.maxIterations   = maxIter;
    cfg.maxDepth        = maxDepth;
    cfg.outputDirectory = outDir;
    return cfg;
}

// ── Construction & config accessor ───────────────────────────────────────────

TEST(BisectionRunRecordTest, ConfigIsPreservedAfterConstruction) {
    RunConfig cfg = makeConfig(10, 4);
    BisectionRunRecord record(cfg);

    EXPECT_EQ(record.config().algorithm,     "test-algo");
    EXPECT_EQ(record.config().datasetName,   "test-dataset");
    EXPECT_EQ(record.config().maxIterations, 10);
    EXPECT_EQ(record.config().maxDepth,      4);
}

// ── averageCostGain ───────────────────────────────────────────────────────────

TEST(BisectionRunRecordTest, AverageCostGain_EmptyReturnsZero) {
    BisectionRunRecord record(makeConfig());
    EXPECT_DOUBLE_EQ(record.averageCostGain(), 0.0);
}

TEST(BisectionRunRecordTest, AverageCostGain_SingleSample) {
    BisectionRunRecord record(makeConfig());
    record.recordCostGain(4.0);
    EXPECT_DOUBLE_EQ(record.averageCostGain(), 4.0);
}

TEST(BisectionRunRecordTest, AverageCostGain_MultipleSamples) {
    BisectionRunRecord record(makeConfig());
    record.recordCostGain(1.0);
    record.recordCostGain(3.0);
    record.recordCostGain(5.0);
    EXPECT_DOUBLE_EQ(record.averageCostGain(), 3.0);
}

TEST(BisectionRunRecordTest, AverageCostGain_NegativeValues) {
    BisectionRunRecord record(makeConfig());
    record.recordCostGain(-2.0);
    record.recordCostGain(2.0);
    EXPECT_DOUBLE_EQ(record.averageCostGain(), 0.0);
}

// ── averageSwappedPairs ───────────────────────────────────────────────────────

TEST(BisectionRunRecordTest, AverageSwappedPairs_EmptyReturnsZero) {
    BisectionRunRecord record(makeConfig());
    EXPECT_DOUBLE_EQ(record.averageSwappedPairs(), 0.0);
}

TEST(BisectionRunRecordTest, AverageSwappedPairs_SingleSample) {
    BisectionRunRecord record(makeConfig());
    record.recordSwappedPairs(6);
    EXPECT_DOUBLE_EQ(record.averageSwappedPairs(), 6.0);
}

TEST(BisectionRunRecordTest, AverageSwappedPairs_MultipleSamples) {
    BisectionRunRecord record(makeConfig());
    record.recordSwappedPairs(2);
    record.recordSwappedPairs(4);
    record.recordSwappedPairs(6);
    EXPECT_DOUBLE_EQ(record.averageSwappedPairs(), 4.0);
}

// ── averageIterationCount ─────────────────────────────────────────────────────

TEST(BisectionRunRecordTest, AverageIterationCount_EmptyReturnsZero) {
    BisectionRunRecord record(makeConfig());
    EXPECT_DOUBLE_EQ(record.averageIterationCount(), 0.0);
}

TEST(BisectionRunRecordTest, AverageIterationCount_SingleSample) {
    BisectionRunRecord record(makeConfig());
    record.recordIterationCount(3);
    EXPECT_DOUBLE_EQ(record.averageIterationCount(), 3.0);
}

TEST(BisectionRunRecordTest, AverageIterationCount_MultipleSamples) {
    BisectionRunRecord record(makeConfig());
    record.recordIterationCount(1);
    record.recordIterationCount(3);
    record.recordIterationCount(5);
    EXPECT_DOUBLE_EQ(record.averageIterationCount(), 3.0);
}

// ── maxIterationHitCount ──────────────────────────────────────────────────────

TEST(BisectionRunRecordTest, MaxIterationHitCount_NoSamplesReturnsZero) {
    BisectionRunRecord record(makeConfig(5));
    EXPECT_EQ(record.maxIterationHitCount(), 0);
}

TEST(BisectionRunRecordTest, MaxIterationHitCount_NoneHitMax) {
    BisectionRunRecord record(makeConfig(5));
    record.recordIterationCount(1);
    record.recordIterationCount(3);
    EXPECT_EQ(record.maxIterationHitCount(), 0);
}

TEST(BisectionRunRecordTest, MaxIterationHitCount_SomeHitMax) {
    BisectionRunRecord record(makeConfig(5));
    record.recordIterationCount(5);
    record.recordIterationCount(3);
    record.recordIterationCount(5);
    EXPECT_EQ(record.maxIterationHitCount(), 2);
}

TEST(BisectionRunRecordTest, MaxIterationHitCount_AllHitMax) {
    BisectionRunRecord record(makeConfig(5));
    record.recordIterationCount(5);
    record.recordIterationCount(5);
    EXPECT_EQ(record.maxIterationHitCount(), 2);
}

// ── reset ─────────────────────────────────────────────────────────────────────

TEST(BisectionRunRecordTest, Reset_ClearsSamples) {
    BisectionRunRecord record(makeConfig(5));
    record.recordCostGain(10.0);
    record.recordSwappedPairs(4);
    record.recordIterationCount(5);
    record.recordTotalCost(99.0);
    record.recordMLogACost(88.0);

    record.reset();

    EXPECT_DOUBLE_EQ(record.averageCostGain(),     0.0);
    EXPECT_DOUBLE_EQ(record.averageSwappedPairs(),  0.0);
    EXPECT_DOUBLE_EQ(record.averageIterationCount(), 0.0);
    EXPECT_EQ(record.maxIterationHitCount(),         0);
}

TEST(BisectionRunRecordTest, Reset_PreservesConfig) {
    RunConfig cfg = makeConfig(7, 2);
    BisectionRunRecord record(cfg);
    record.recordCostGain(1.0);

    record.reset();

    EXPECT_EQ(record.config().maxIterations, 7);
    EXPECT_EQ(record.config().maxDepth,      2);
    EXPECT_EQ(record.config().algorithm,     "test-algo");
}

TEST(BisectionRunRecordTest, Reset_AllowsNewSamplesAfterReset) {
    BisectionRunRecord record(makeConfig());
    record.recordCostGain(50.0);
    record.reset();

    record.recordCostGain(2.0);
    record.recordCostGain(4.0);
    EXPECT_DOUBLE_EQ(record.averageCostGain(), 3.0);
}

// ── appendToCsv ───────────────────────────────────────────────────────────────

class BisectionRunRecordFileTest : public ::testing::Test {
protected:
    fs::path tmpDir_;

    void SetUp() override {
        tmpDir_ = fs::temp_directory_path() / "brr_test_XXXXXX";
        // Use a unique path by appending the test PID
        tmpDir_ += std::to_string(::getpid());
        fs::create_directories(tmpDir_);
    }

    void TearDown() override {
        fs::remove_all(tmpDir_);
    }

    std::string readFile(const fs::path& path) {
        std::ifstream f(path);
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }
};

TEST_F(BisectionRunRecordFileTest, AppendToCsv_WritesCorrectRow) {
    RunConfig cfg = makeConfig(10, 3, tmpDir_.string());
    BisectionRunRecord record(cfg);
    record.recordTotalCost(42.5);
    record.recordMLogACost(7.25);

    record.appendToCsv();

    std::string content = readFile(tmpDir_ / "result.csv");
    EXPECT_NE(content.find("test-dataset"), std::string::npos);
    EXPECT_NE(content.find("test-algo"),    std::string::npos);
    EXPECT_NE(content.find("10"),           std::string::npos);
    EXPECT_NE(content.find("42.5"),         std::string::npos);
    EXPECT_NE(content.find("7.25"),         std::string::npos);
}

TEST_F(BisectionRunRecordFileTest, AppendToCsv_AppendsMultipleRows) {
    RunConfig cfg = makeConfig(10, 3, tmpDir_.string());

    BisectionRunRecord r1(cfg);
    r1.recordTotalCost(1.0);
    r1.recordMLogACost(2.0);
    r1.appendToCsv();

    BisectionRunRecord r2(cfg);
    r2.recordTotalCost(3.0);
    r2.recordMLogACost(4.0);
    r2.appendToCsv();

    std::string content = readFile(tmpDir_ / "result.csv");
    // Both rows present — count newlines (each row ends with \n)
    int newlines = static_cast<int>(std::count(content.begin(), content.end(), '\n'));
    EXPECT_EQ(newlines, 2);
}

// ── appendMetrics ─────────────────────────────────────────────────────────────

TEST_F(BisectionRunRecordFileTest, AppendMetrics_WritesHeaderOnFirstCall) {
    RunConfig cfg = makeConfig(5, 2, tmpDir_.string());
    BisectionRunRecord record(cfg);
    record.recordIterationCount(3);
    record.recordSwappedPairs(2);
    record.recordCostGain(1.0);

    record.appendMetrics();

    std::string content = readFile(tmpDir_ / "metrics.out");
    EXPECT_NE(content.find("max-it-occ"),      std::string::npos);
    EXPECT_NE(content.find("avg-iterations"),  std::string::npos);
    EXPECT_NE(content.find("avg-cost-gain"),   std::string::npos);
}

TEST_F(BisectionRunRecordFileTest, AppendMetrics_NoHeaderOnSubsequentCalls) {
    RunConfig cfg = makeConfig(5, 2, tmpDir_.string());

    BisectionRunRecord r1(cfg);
    r1.recordIterationCount(5);
    r1.appendMetrics();

    BisectionRunRecord r2(cfg);
    r2.recordIterationCount(3);
    r2.appendMetrics();

    std::string content = readFile(tmpDir_ / "metrics.out");
    // Header line should appear exactly once
    size_t first  = content.find("max-it-occ");
    size_t second = content.find("max-it-occ", first + 1);
    EXPECT_NE(first,  std::string::npos);
    EXPECT_EQ(second, std::string::npos);
}

TEST_F(BisectionRunRecordFileTest, AppendMetrics_CorrectMaxIterHitCount) {
    RunConfig cfg = makeConfig(5, 2, tmpDir_.string());
    BisectionRunRecord record(cfg);
    record.recordIterationCount(5);
    record.recordIterationCount(5);
    record.recordIterationCount(3);
    record.recordSwappedPairs(1);
    record.recordCostGain(0.5);

    record.appendMetrics();

    std::string content = readFile(tmpDir_ / "metrics.out");
    // Second line should start with "2" (two max-iteration hits)
    size_t headerEnd = content.find('\n');
    ASSERT_NE(headerEnd, std::string::npos);
    std::string dataLine = content.substr(headerEnd + 1);
    EXPECT_EQ(dataLine[0], '2');
}
