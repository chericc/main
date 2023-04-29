cmake_minimum_required(VERSION 3.0)

project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# main
set(G_GOOGLETEST_MAIN_SRCFILE ${CMAKE_SOURCE_DIR}/main.cpp)

# googletest
set(G_GOOGLETEST_PATH /home/test/opensrc/googletest/build/output)
set(G_GOOGLETEST_LIB_PATH ${G_GOOGLETEST_PATH}/lib)
set(G_GOOGLETEST_INC_PATH ${G_GOOGLETEST_PATH}/include)

# xtools
set(G_XTOOL_LIB xtools)

# platform

string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "MSVC" msvc)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU" gnu)

link_directories(${G_GOOGLETEST_LIB_PATH})

# targets
add_subdirectory(xtools)

# 