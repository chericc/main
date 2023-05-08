cmake_minimum_required(VERSION 3.0)

project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# main
set(G_GOOGLETEST_MAIN_SRCFILE ${CMAKE_SOURCE_DIR}/main.cpp)

# resource
set(G_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/resources)

# googletest
set(G_GOOGLETEST_PATH /home/test/opensrc/googletest/build/output)
set(G_GOOGLETEST_LIB_PATH ${G_GOOGLETEST_PATH}/lib)
set(G_GOOGLETEST_INC_PATH ${G_GOOGLETEST_PATH}/include)

# ffmpeg
set(G_FFMPEG_PATH /home/test/opensrc/ffmpeg/build/output)
set(G_FFMPEG_LIB_PATH ${G_FFMPEG_PATH}/lib)
set(G_FFMPEG_INC_PATH ${G_FFMPEG_PATH}/include)

# xtools
set(G_XTOOLS xtools)
set(G_XTOOLS_XLOG_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xlog)

# wav
set(G_XWAV_DEMUXER xwav_demuxer)
set(G_XWAV_DEMUXER_HEADER_PATH ${CMAKE_SOURCE_DIR}/audio/wav/demuxer)

# bmp
set(G_BMP bmp)
set(G_BMP_HEADER_PATH ${CMAKE_SOURCE_DIR}/image/bmp)

# platform

string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "MSVC" msvc)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU" gnu)

# add googletest include path

link_directories(${G_GOOGLETEST_LIB_PATH})
include_directories(${G_GOOGLETEST_INC_PATH})

# ctest

include(CTest)

# targets
add_subdirectory(xtools)
#add_subdirectory(xmedia/xplay)
add_subdirectory(audio/wav/demuxer)
#add_subdirectory(image/bmp)