cmake_minimum_required(VERSION 3.0)

project(main)

message("compiler is " ${CMAKE_CXX_COMPILER_ID})

if(NOT CMAKE_BUILD_TYPE)
  message(WARNING "CMAKE_BUILD_TYPE not set, default to release")
  set(CMAKE_BUILD_TYPE "Release")
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
else()
add_compile_options(-Wall -Wextra)
endif()

# main
set(G_GOOGLETEST_MAIN_SRCFILE ${CMAKE_SOURCE_DIR}/test.cpp)

# resource
set(G_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/resources)

# googletest
set(G_GOOGLETEST gtest)
if(${msvc})
set(G_GOOGLETEST_PATH D:/opensrc/googletest/install)
elseif(${gnu})
set(G_GOOGLETEST_PATH /home/test/opensrc/googletest/build/output)
elseif(${clang})
set(G_GOOGLETEST_PATH ~/opensrc/googletest/build/output)
endif()
set(G_GOOGLETEST_LIB_PATH ${G_GOOGLETEST_PATH}/lib)
set(G_GOOGLETEST_INC_PATH ${G_GOOGLETEST_PATH}/include)

# ffmpeg
if(${msvc})
set(G_FFMPEG_PATH D:/opensrc/ffmpeg/ffmpeg-full_build-shared)
elseif(${gnu})
set(G_FFMPEG_PATH /home/test/opensrc/ffmpeg/build/output)
elseif(${clang})
set(G_FFMPEG_PATH ~/opensrc/ffmpeg/build/output)
endif()
set(G_FFMPEG_LIB_PATH ${G_FFMPEG_PATH}/lib)
set(G_FFMPEG_INC_PATH ${G_FFMPEG_PATH}/include)

# SDL2
if(${msvc})
set(G_SDL2_PATH D:/opensrc/SDL/install)
elseif(${clang})
set(G_SDL2_PATH ~/opensrc/SDL/build/output/)
elseif(${gnu})
set(G_SDL2_PATH ~/opensrc/SDL/build/output/)
endif()
set(G_SDL2_LIB_PATH ${G_SDL2_PATH}/lib)
set(G_SDL2_INC_PATH ${G_SDL2_PATH}/include)
set(G_SDL2 SDL2 SDL2main)

# fdk-aac
set(G_FDKAAC_PATH /home/test/opensrc/fdkaac/build/output)
set(G_FDKAAC_LIB_PATH ${G_FDKAAC_PATH}/lib)
set(G_FDKAAC_INC_PATH ${G_FDKAAC_PATH}/include/fdk-aac)

# freetype
if(${msvc})
set(G_FREETYPE_PATH D:/opensrc/freetype/build/out)
elseif(${gnu})
set(G_FREETYPE_PATH ~/opensrc/freetype/build/output)
elseif(${clang})
set(G_FREETYPE_PATH ~/opensrc/freetype/build/output)
endif()
set(G_FREETYPE_LIB_PATH ${G_FREETYPE_PATH}/lib)
set(G_FREETYPE_INC_PATH ${G_FREETYPE_PATH}/include/freetype2)

# opengl depends
## glfw
if(${msvc})
set(G_GLFW_PATH D:/opensrc/glfw/glfw-3.3.9/out/install/x64-Debug)
set(G_GLFW_LIB_PATH ${G_GLFW_PATH}/lib)
set(G_GLFW_INC_PATH ${G_GLFW_PATH}/include)
endif()

# libevent
set(G_LIBEVENT event)
if(${msvc})
elseif(${gnu})
set(G_LIBEVENT_PATH ~/opensrc/libevent/build/output)
elseif(${clang})
endif()
set(G_LIBEVENT_LIB_PATH ${G_LIBEVENT_PATH}/lib)
set(G_LIBEVENT_INC_PATH ${G_LIBEVENT_PATH}/include)

# cjson
set(G_CJSON_LIBS cjson cjson_utils)
set(G_CJSON_PATH ~/opensrc/cjson/build/output)
set(G_CJSON_LIB_PATH ${G_CJSON_PATH}/lib)
set(G_CJSON_INC_PATH ${G_CJSON_PATH}/include)

# curl
set(G_CURL_LIBNAME curl)
set(G_CURL_PATH ~/opensrc/curl/build/output)
set(G_CURL_LIB_PATH ${G_CURL_PATH}/lib)
set(G_CURL_INC_PATH ${G_CURL_PATH}/include)

# test_comm just for test use
set(G_TEST_COMM test_comm)
set(G_TSET_COMM_HEADER_PATH ${CMAKE_SOURCE_DIR}/test_comm)
set(G_TEST_COMM_LIB_PATH ${G_TSET_COMM_LIB_PATH}/lib)

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

# image
set(G_IMAGE ximage)
set(G_IMAGE_BMP_HEADER_PATH ${CMAKE_SOURCE_DIR}/image/bmp)
set(G_IMAGE_IMAGEVIEW_HEADER_PATH ${CMAKE_SOURCE_DIR}/image/imageview)
set(G_IMAGE_FREETYPEWRAPPER_HEADER_PATH ${CMAKE_SOURCE_DIR}/image/freetypewrapper)

# textenc
set(G_TEXTENC xtextenc)
set(G_TEXTENC_UTF8_HEADER_PATH ${CMAKE_SOURCE_DIR}/textenc/utf8_enc)

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

# add googletest include path

link_directories(${G_GOOGLETEST_LIB_PATH})
include_directories(${G_GOOGLETEST_INC_PATH})
include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

if(${gnu})
link_libraries(pthread)
endif()

# ctest

include(CTest)

# debug
# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

option(USE_ASAN "use asan(ASAN)" OFF)
option(USE_BIG_ENDIAN "big endian" OFF)
option(WITH_GTEST "has googletest" OFF)
option(WITH_FREETYPE "has freetype(library)" OFF)
option(WITH_FFMPEG  "has ffmpeg(library)" OFF)
option(WITH_OPENGL "has OpenGL environment" OFF)
option(WITH_LIBEVENT "has libevent" OFF)
option(WITH_SDL "has SDL" OFF)
option(WITH_CJSON "has cJSON" OFF)
option(WITH_CURL "has curl" OFF)
option(EXPORT_COMPILE_OPTION "export compile option(compile_commands.json)" ON)

if (USE_ASAN)
  set(
    CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer"
  )
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer"
  )
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

add_subdirectory(demos/cjson)
add_subdirectory(demos/curl)
add_subdirectory(demos/ffmpeg)
add_subdirectory(demos/mjson)
add_subdirectory(demos/mkdir)
add_subdirectory(demos/network)
add_subdirectory(demos/h264_sei)

if(WITH_SDL)
add_subdirectory(demos/sdl)
endif()

add_subdirectory(demos/tinyframe)

add_subdirectory(image)

if(${gnu})
add_subdirectory(md5dir)
add_subdirectory(media-server)
endif()

add_subdirectory(stl)
add_subdirectory(test_comm)
add_subdirectory(textenc)
if(WITH_FFMPEG)
add_subdirectory(xmedia/xplay)
if(WITH_SDL)
add_subdirectory(xmedia/xdisplay)
endif()
endif()
add_subdirectory(xmedia/pcap2ts)
add_subdirectory(xtools)