cmake_minimum_required(VERSION 3.24)
project(cb)

enable_testing()

include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

add_executable(cb_test circular_buffer_tests.cpp)

target_link_libraries(cb_test PRIVATE GTest::gtest GTest::gtest_main)
