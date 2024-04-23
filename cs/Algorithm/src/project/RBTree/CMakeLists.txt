
cmake_minimum_required(VERSION 3.15)

project (rbtree)

set (CMAKE_CXX_STANDARD 11)

set (MY_OUTPUT_DIR .)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${MY_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${MY_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${MY_OUTPUT_DIR})

include_directories(.)

aux_source_directory(. SRC)
aux_source_directory(test SRC_TEST)

link_libraries(${G_GOOGLETEST})
link_libraries(pthread)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

add_definitions(-DENABLE_ASSERT)

add_executable(${PROJECT_NAME} ${SRC} ${SRC_TEST})

# 设置安装路径
# cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/output