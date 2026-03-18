cmake_minimum_required(VERSION 3.10)
project(OpticalStaticBT-Heuristics CXX)

set(CMAKE_CXX_STANDARD 17)
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(TSTDIR ${CMAKE_SOURCE_DIR}/tests)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${INC_DIR})

# === GoogleTest ===
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Make GoogleTest available to be linked
FetchContent_MakeAvailable(googletest)

# === Enable Testing ===
# This command is required to use CTest (CMake's test runner)
enable_testing()

# === Main executables ===
add_executable(main ${SRCDIR}/main.cc)
add_executable(pop ${SRCDIR}/popGen.cc)
add_executable(work ${SRCDIR}/workload.cc)
add_executable(run ${SRCDIR}/run.cc)

# === Test executable ===
add_executable(run_tests
	${TSTDIR}/hello_test.cc
	${TSTDIR}/include/test_convertgraph.cc
	${TSTDIR}/include/test_bisectionRunRecord.cc
)

# === Link GoogleTest to Executable ===
target_link_libraries(run_tests GTest::gtest_main)

# === Discover Tests ===
# This tells CMake to automatically find and register your TEST() macros
include(GoogleTest)
gtest_discover_tests(run_tests)

# === Output directory ===
set_target_properties(main pop work run PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
