mkdir -p build
cd build

OPENSRC_LIB_PATH=~/opensrc
FFMPEG_PATH=$OPENSRC_LIB_PATH/ffmpeg/build/output
LIVE555_PATH=$OPENSRC_LIB_PATH/live555/live
NANOMSG_PATH=$OPENSRC_LIB_PATH/nanomsg/build/output
CARES_PATH=$OPENSRC_LIB_PATH/cares/build/output
GTEST_PATH=$OPENSRC_LIB_PATH/googletest/build/output

CUSTOM_INCS=$CUSTOM_INCS:$FFMPEG_PATH/include
CUSTOM_INCS=$CUSTOM_INCS:$LIVE555_PATH/include
CUSTOM_INCS=$CUSTOM_INCS:$NANOMSG_PATH/include
CUSTOM_INCS=$CUSTOM_INCS:$CARES_PATH/include
CUSTOM_INCS=$CUSTOM_INCS:$GTEST_PATH/include

CUSTOM_LIBS=$CUSTOM_LIBS:$FFMPEG_PATH/lib
CUSTOM_LIBS=$CUSTOM_LIBS:$LIVE555_PATH/lib
CUSTOM_LIBS=$CUSTOM_LIBS:$NANOMSG_PATH/lib
CUSTOM_LIBS=$CUSTOM_LIBS:$CARES_PATH/lib
CUSTOM_LIBS=$CUSTOM_LIBS:$GTEST_PATH/lib

CUSTOM_PATHS=$CUSTOM_PATHS:$LIVE555_PATH

export CMAKE_INCLUDE_PATH=$CUSTOM_INCS
export CMAKE_LIBRARY_PATH=$CUSTOM_LIBS
export CMAKE_PREFIX_PATH=$CUSTOM_PATHS
cmake .. -DCMAKE_BUILD_TYPE=Debug

make $@