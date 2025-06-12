#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

// Helper to run the main executable with arguments and capture exit code
int run_main_with_args(const std::vector<std::string>& args) {
    std::string cmd = "./main";
    for (const auto& arg : args) {
        cmd += " " + arg;
    }
    return std::system(cmd.c_str());
}

// Fixture to set up and tear down test files
class MainTest : public ::testing::Test {
protected:
    std::string weights_dir = "weights";
    std::string output_dir = "output";
    std::string input_name = "testinput";
    std::string weights_file;

    void SetUp() override {
        std::filesystem::create_directory(weights_dir);
        std::filesystem::create_directory(output_dir);
        weights_file = weights_dir + "/" + input_name;
        std::ofstream ofs(weights_file);
        ofs << "3\n0 1 1.5\n1 2 2.5\n2 0 3.5\n";
        ofs.close();
    }

    void TearDown() override {
        std::filesystem::remove_all(weights_dir);
        std::filesystem::remove_all(output_dir);
    }
};

TEST_F(MainTest, FailsOnMissingInput) {
    // Remove weights file to simulate missing input
    std::filesystem::remove(weights_file);
    int ret = run_main_with_args({input_name, "0"});
    // Should fail (non-zero exit code)
    ASSERT_NE(ret, 0);
}

TEST_F(MainTest, SucceedsWithDefaultArgs) {
    int ret = run_main_with_args({input_name, "0"});
    // Should succeed (exit code 0)
    ASSERT_EQ(ret, 0);
    // Output directory for noop should exist
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/" + input_name + "/noop/"));
}

TEST_F(MainTest, RunsWithAlgorithmsArg) {
    int ret = run_main_with_args({input_name, "1", "--algorithms", "basic.mloggap.onehop"});
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/" + input_name + "/basic/"));
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/" + input_name + "/mloggap/"));
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/" + input_name + "/onehop/"));
}

TEST_F(MainTest, HandlesParallelizeFlag) {
    int ret = run_main_with_args({input_name, "0", "-p"});
    ASSERT_EQ(ret, 0);
}

TEST_F(MainTest, HandlesTestNumber) {
    int ret = run_main_with_args({input_name, "0", "--test-number", "42"});
    ASSERT_EQ(ret, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}