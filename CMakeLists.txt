cmake_minimum_required(VERSION 3.7)
project(plugin_test_app CUDA CXX)

find_package(cpprestsdk REQUIRED)

#Find all source files
file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/*[.cpp|.cu]")

add_executable(plugin_test_app ${SRCS})
set_property(TARGET plugin_test_app PROPERTY CXX_STANDARD 17)
target_link_libraries(plugin_test_app PRIVATE cpprestsdk::cpprest)