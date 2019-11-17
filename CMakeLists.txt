cmake_minimum_required(VERSION 3.15)
project(plugin_test_app CUDA CXX C)

find_package(cpprestsdk REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED program_options)

include("${CMAKE_SOURCE_DIR}/OneAgent-SDK-for-C/onesdk-config.cmake")

#Find all source files
file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/*[.cpp|.cu]")

include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_executable(plugin_test_app ${SRCS})
set_target_properties(plugin_test_app PROPERTIES CXX_STANDARD 17)
target_link_libraries(plugin_test_app PRIVATE cpprestsdk::cpprest ${OpenCV_LIBS} ${Boost_LIBRARIES} onesdk_static)