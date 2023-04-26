cmake_minimum_required(VERSION 3.0)

project (xplay)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

#set(FFMPEG_DIR ~/opensrc/ffmpeg/build/output/)
set(FFMPEG_DIR C:\\Users\\test\\depends\\ffmpeg)
set(XTOOLS_DIR C:\\Users\\test\\depends\\xtools)

include_directories(${FFMPEG_DIR}/include ${XTOOLS_DIR}/include)
link_directories(${FFMPEG_DIR}/lib ${XTOOLS_DIR}/lib)

link_libraries(avformat swscale avformat avcodec avutil swresample)
link_libraries(xtools)
#link_libraries(pthread)

aux_source_directory(xplay src_xplay)
add_executable(${PROJECT_NAME} ${src_xplay})