cmake_minimum_required(VERSION 3.10)
project(OpticalStaticBT-Heuristics CXX)

set(CMAKE_CXX_STANDARD 17)
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(TSTDIR ${CMAKE_SOURCE_DIR}/tests)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

# === Disable oneTBB's internal tests and examples ===
# We force these variables into the cache so oneTBB respects them
set(TBB_TEST OFF CACHE BOOL "" FORCE)
set(TBB_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TBB_STRICT OFF CACHE BOOL "" FORCE) # Prevents failing on compiler warning

include_directories(${INC_DIR})

# ===. Fetch oneTBB ===
include(FetchContent)
FetchContent_Declare(
  onetbb
  GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB.git
  GIT_TAG        v2021.12.0 # Use a stable release tag!
  GIT_SHALLOW    ON         # Only download this specific commit to save time
)
FetchContent_MakeAvailable(onetbb)

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

# === Link oneTBB ===
target_link_libraries(run PRIVATE TBB::tbb)

# === Test executable ===
add_executable(run_tests
	${TSTDIR}/include/test_bisectionRunRecord.cc
	${TSTDIR}/include/test_forwardIndex.cc
	${TSTDIR}/include/test_forwardIndexFactory.cc
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
