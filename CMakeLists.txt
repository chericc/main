cmake_minimum_required(VERSION 3.0)

project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# platform
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "MSVC" msvc)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU" gnu)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "AppleClang" clang)

# main
set(G_GOOGLETEST_MAIN_SRCFILE ${CMAKE_SOURCE_DIR}/main.cpp)

# resource
set(G_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/resources)

# googletest
if(${msvc})
set(G_GOOGLETEST_PATH C:/Users/test/opensrc/googletest/install)
elseif(${gnu})
set(G_GOOGLETEST_PATH /home/test/opensrc/googletest/build/output)
elseif(${clang})
set(G_GOOGLETEST_PATH ~/opensrc/googletest/build/output)
endif()
set(G_GOOGLETEST_LIB_PATH ${G_GOOGLETEST_PATH}/lib)
set(G_GOOGLETEST_INC_PATH ${G_GOOGLETEST_PATH}/include)

# ffmpeg
if(${msvc})
set(G_FFMPEG_PATH C:/Users/test/opensrc/ffmpeg/ffmpeg-6.0-full_build-shared)
elseif(${gnu})
set(G_FFMPEG_PATH /home/test/opensrc/ffmpeg/build/output)
elseif(${clang})
set(G_FFMPEG_PATH ~/opensrc/ffmpeg/build/output)
endif()
set(G_FFMPEG_LIB_PATH ${G_FFMPEG_PATH}/lib)
set(G_FFMPEG_INC_PATH ${G_FFMPEG_PATH}/include)

# SDL2
set(G_SDL2_PATH C:/Users/test/opensrc/SDL/install)
set(G_SDL2_LIB_PATH ${G_SDL2_PATH}/lib)
set(G_SDL2_INC_PATH ${G_SDL2_PATH}/include/SDL2)

# fdk-aac
set(G_FDKAAC_PATH /home/test/opensrc/fdkaac/build/output)
set(G_FDKAAC_LIB_PATH ${G_FDKAAC_PATH}/lib)
set(G_FDKAAC_INC_PATH ${G_FDKAAC_PATH}/include/fdk-aac)

# xtools
set(G_XTOOLS xtools)
set(G_XTOOLS_XLOG_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xlog)
set(G_XTOOLS_XIO_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xio)

# wav
set(G_XWAV_DEMUXER xwav_demuxer)
set(G_XWAV_DEMUXER_HEADER_PATH ${CMAKE_SOURCE_DIR}/audio/wav/demuxer)

# fdkaac_wrapper
set(G_FDKAAC_WRAPPER fdkaac_wrapper)
set(G_FDKAAC_WRAPPER_HEADER_PATH ${CMAKE_SOURCE_DIR}/audio/aac/fdkaac_wrapper)

# bmp
set(G_BMP bmp)
set(G_BMP_HEADER_PATH ${CMAKE_SOURCE_DIR}/image/bmp)

# add googletest include path

link_directories(${G_GOOGLETEST_LIB_PATH})
include_directories(${G_GOOGLETEST_INC_PATH})

# ctest

include(CTest)

# debug
SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

# targets
add_subdirectory(xtools)
add_subdirectory(audio/wav/demuxer)
#add_subdirectory(audio/aac/fdkaac_wrapper)
add_subdirectory(image/bmp)
#add_subdirectory(xmedia/xplay)
#add_subdirectory(xmedia/xdisplay)