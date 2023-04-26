cmake_minimum_required(VERSION 3.0)

project (xtools_proj)

set(CMAKE_CXX_STANDARD 14)

set(XTOOL_LIB xtools)
set(XTOOL_TEST_XLOG test_xlog)
set(XTOOL_TEST_XCONFIG test_xconfig)
set(XTOOL_TEST_XSTRING test_xstring)

set(GTEST_PATH C:\\Users\\test\\depends\\googletest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

message("compiler id is ${CMAKE_CXX_COMPILER_ID}")
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "MSVC" msvc)
string(COMPARE EQUAL ${CMAKE_CXX_COMPILER_ID} "GNU" gnu)

aux_source_directory(xconfig src_xconfig)
aux_source_directory(xlog src_xlog)
aux_source_directory(xstring src_xstring)

aux_source_directory(xconfig/test src_xconfig_test)
aux_source_directory(xlog/test src_xlog_test)
aux_source_directory(xstring/test src_xstring_test)

include_directories(xconfig)
include_directories(xlog)
include_directories(xstring)

include_directories(${GTEST_PATH}/include)

#add_library(${XTOOL_LIB} STATIC ${src_xlog} ${src_xconfig} ${src_xstring})
add_library(${XTOOL_LIB} SHARED ${src_xlog} ${src_xconfig} ${src_xstring})

link_directories(${GTEST_PATH}/lib)

if(${gnu})
set(COMMON_LIB gtest ${XTOOL_LIB} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${XTOOL_LIB})
endif()

add_executable(${XTOOL_TEST_XLOG} ${src_xlog_test})
target_link_libraries(${XTOOL_TEST_XLOG} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XCONFIG} ${src_xconfig_test})
target_link_libraries(${XTOOL_TEST_XCONFIG} ${COMMON_LIB})

add_executable(${XTOOL_TEST_XSTRING} ${src_xstring_test})
target_link_libraries(${XTOOL_TEST_XSTRING} ${COMMON_LIB})

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

include(GNUInstallDirs)
install(TARGETS ${XTOOL_LIB}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES xlog/xlog.hpp xconfig/xconfig.hpp xstring/xstring.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtools)
