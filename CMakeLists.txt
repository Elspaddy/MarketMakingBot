cmake_minimum_required (VERSION 3.20.0)

project (MarketMakingBot CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_options("$<$<CONFIG:RELEASE>:-W;-Wall;-O3;-pedantic>")
add_compile_options("$<$<CONFIG:DEBUG>:-W;-Wall;-O0;-g;-pedantic>")

add_executable(MarketMakingBot MarketMakingBot.cpp)

################################
# GFlags
################################
find_package(gflags REQUIRED)
include_directories(${GFLAGS_INCLUDE_DIRS})
target_link_libraries(MarketMakingBot gflags)

################################
# GTest
################################
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

################################
# Unit Tests
################################
# Add test cpp file
add_executable(runUnitTests testgtest.cpp)
# Link test runable against gtest & gtest_main
target_link_libraries(runUnitTests gtest gtest_main pthread)
add_test(runUnitTests runUnitTests)
