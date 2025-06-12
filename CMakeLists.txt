cmake_minimum_required(VERSION 3.10)
project(OpticalStaticBT-Heuristics CXX)

set(CMAKE_CXX_STANDARD 17)
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${INC_DIR})

# === Main executables ===
add_executable(main ${SRCDIR}/main.cc)
add_executable(pop ${SRCDIR}/popGen.cc)
add_executable(work ${SRCDIR}/workload.cc)
add_executable(test ${SRCDIR}/test_graphbissectionLog.cc)

set_target_properties(main pop work test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
