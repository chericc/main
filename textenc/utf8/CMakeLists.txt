
cmake_minimum_required(VERSION 3.0)

project (utf)

set (CMAKE_CXX_STANDARD 14)

aux_source_directory(. SRC)
aux_source_directory(test SRC_TEST)

link_libraries(gtest pthread)

include_directories(.)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

add_executable(${PROJECT_NAME} ${SRC} ${SRC_TEST})

# 设置安装路径
#cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/output

# 启用警告
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

