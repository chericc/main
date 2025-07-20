cmake_minimum_required(VERSION 3.0)

project(main)

message("compiler is " ${CMAKE_CXX_COMPILER_ID})

if(NOT CMAKE_BUILD_TYPE)
  message(WARNING "CMAKE_BUILD_TYPE not set, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "CMAKE_BUILD_TYPE not set, default to Release" FORCE)
endif()

# platform
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "MSVC" msvc)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU" gnu)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "AppleClang" clang)
set(G_PLATFORM_MSVC "X_PLATFORM_MSVC")
set(G_PLATFORM_GNU "X_PLATFORM_GNU")
set(G_PLATFORM_CLANG "X_PLATFORM_CLANG")

# dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# cxx standard
set(CMAKE_CXX_STANDARD 14) # gtest needs c++ version 14+
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# warning
if(${msvc})
#add_compile_options(/W4)
elseif(${gnu})
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
elseif(${clang})
add_compile_options(-Wall -Wno-unused-parameter)
else()
add_compile_options(-Wall -Wextra)
endif()

# googletest
find_library(LIB_GTEST gtest)
find_path(HEADER_GTEST gtest/gtest.h)

# ffmpeg
find_library(LIB_FFMPEG_AVFORMAT avformat)
find_library(LIB_FFMPEG_AVCODEC avcodec)
find_library(LIB_FFMPEG_AVUTIL avutil)
find_library(LIB_FFMPEG_SWRESAMPLE swresample)
find_library(LIB_FFMPEG_SWSCALE swscale)
find_path(HEADER_FFMPEG libavformat)

# SDL2
find_library(LIB_SDL2 SDL2-2.0)

# fdk-aac
find_library(LIB_FDKAAC fdk-aac)

# freetype
find_library(LIB_FREETYPE freetype)
find_path(HEADER_FREETYPE freetype PATH_SUFFIXES freetype2) # freetype2/freetype

# libevent
find_library(LIB_EVENT event)

# cjson
find_library(LIB_CJSON cjson cjson_utils)

# curl
find_library(LIB_CURL curl)

# libwebsocket
find_library(LIB_WEBSOCKETS websockets)
find_path(HEADER_LIBWEBSOCKETS libwebsockets.h)

# live555
find_path(PATH_LIB_LIVE555 liveMedia)

# lvgl
find_library(LIB_LVGL liblvgl.a)
find_library(LIB_LVGL_THORVG liblvgl_thorvg.a)
find_path(HEADER_LVGL lvgl/lvgl.h)

# nanomsg
find_library(LIB_NANOMSG libnanomsg.a)
find_path(HEADER_NANOMSG nanomsg PATH_SUFFIXES include)

# c-ares
find_library(LIB_C_ARES libcares.a)
find_path(HEADER_C_ARES ares.h)

# sqlite
find_library(LIB_SQLITE3 sqlite3)
find_path(HEADER_SQLITE3 sqlite3.h)

# tinyframe
find_path(HEADER_TINYFRAME 
    NAMES TinyFrame.h
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/demos/tinyframe/libsrc
    NO_DEFAULT_PATH)

# test_comm just for test use

# main
set(G_GOOGLETEST_MAIN_SRCFILE ${CMAKE_SOURCE_DIR}/test.cpp)

# resource
set(G_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/resources)

set(G_TEST_COMM test_comm)
set(G_TSET_COMM_HEADER_PATH ${CMAKE_SOURCE_DIR}/test_comm)

# xtools
set(G_XTOOLS xtools)
set(G_XTOOLS_XLOG_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xlog)
set(G_XTOOLS_XIO_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xio)
set(G_XTOOLS_XTHREAD_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xthread)
set(G_XTOOLS_XUTILITY_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xutility)
set(G_XTOOLS_XCROSSPLATFORM_HEADER_PATH ${CMAKE_SOURCE_DIR}/xtools/xcrossplatform)

# wav
set(G_XWAV_DEMUXER xwav_demuxer)
set(G_XWAV_DEMUXER_HEADER_PATH ${CMAKE_SOURCE_DIR}/audio/wav/demuxer)

# fdkaac_wrapper
set(G_FDKAAC_WRAPPER fdkaac_wrapper)
set(G_FDKAAC_WRAPPER_HEADER_PATH ${CMAKE_SOURCE_DIR}/audio/aac/fdkaac_wrapper)

# xplay
set(G_XPLAY xplay)
set(G_XPLAY_HEADER_PATH ${CMAKE_SOURCE_DIR}/xmedia/xplay)

# md5dir
set(G_MD5DIR xmd5dir)

# stl

# net
set(G_NET net)

# pcap2ts
set(G_PCAP2TS pcap2ts)
set(G_PCAP2TS_HEADER_PATH ${CMAKE_SOURCE_DIR}/xmedia/pcap2ts)

if(${gnu})
link_libraries(pthread)
endif()

# ctest

include(CTest)

option(USE_ASAN "use asan(ASAN)" OFF)
option(USE_BIG_ENDIAN "big endian" OFF)
option(EXPORT_COMPILE_OPTION "export compile option(compile_commands.json)" ON)

if (USE_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

if (EXPORT_COMPILE_OPTION)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# endian
if(USE_BIG_ENDIAN)
add_definitions(-DX_BIGENDIAN)
endif()

# platform
set(G_PLATFORM_MSVC "X_PLATFORM_MSVC")
set(G_PLATFORM_GNU "X_PLATFORM_GNU")
set(G_PLATFORM_CLANG "X_PLATFORM_CLANG")
if(${msvc})
add_definitions(-D${G_PLATFORM_MSVC})
elseif(${gnu})
add_definitions(-D${G_PLATFORM_GNU})
elseif(${clang})
add_definitions(-D${G_PLATFORM_CLANG})
endif()

# targets
add_subdirectory(audio/wav/demuxer)
# add_subdirectory(audio/aac/fdkaac_wrapper)

add_subdirectory(cs/DesignPatterns)
add_subdirectory(cs/Algorithm)

add_subdirectory(demos)

if(${gnu})
add_subdirectory(md5dir)
endif()

add_subdirectory(stl)
add_subdirectory(test_comm)

if(LIB_FFMPEG_AVFORMAT
  AND LIB_FFMPEG_AVCODEC
  AND LIB_FFMPEG_AVUTIL
  AND LIB_FFMPEG_SWRESAMPLE
  AND LIB_FFMPEG_SWSCALE)
add_subdirectory(xmedia/xplay)
if(LIB_SDL2)
add_subdirectory(xmedia/xdisplay)
endif() # sdl
endif() # ffmpeg

add_subdirectory(xmedia/pcap2ts)
add_subdirectory(xtools)