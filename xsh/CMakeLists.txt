
cmake_minimum_required(VERSION 3.15)

project (xsh)

set (CMAKE_CXX_STANDARD 11)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)

include_directories(src)

aux_source_directory(src src_src)

link_libraries(pthread)

# SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Release")

add_compile_options(-Wall)

add_executable(${PROJECT_NAME} ${src_src})

