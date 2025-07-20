
cmake_minimum_required(VERSION 3.15)

project (gtestdemo)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

set(SRC_DIR_T1 ${CMAKE_SOURCE_DIR}/t1_functions)
set(SRC_DIR_T2 ${CMAKE_SOURCE_DIR}/t2_class)
set(SRC_DIR_T3 ${CMAKE_SOURCE_DIR}/t3_fixture)
set(SRC_DIR_T11 ${CMAKE_SOURCE_DIR}/t11_mock)

aux_source_directory(${SRC_DIR_T1} SRC_T1)
aux_source_directory(${SRC_DIR_T2} SRC_T2)
aux_source_directory(${SRC_DIR_T3} SRC_T3)
aux_source_directory(${SRC_DIR_T11} SRC_T11)

link_libraries(${LIB_GTEST})
link_libraries(gmock)
link_libraries(pthread)

add_executable(t1 ${SRC_T1})
add_executable(t2 ${SRC_T2})
add_executable(t3 ${SRC_T3})
add_executable(t11 ${SRC_T11})