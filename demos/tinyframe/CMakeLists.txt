set(DEMO_NAME demo_tinyframe)

aux_source_directory(libsrc src_lib)
aux_source_directory(tinyframedemo src_demo)

include_directories(libsrc tinyframedemo)

add_executable(${DEMO_NAME} ${src_lib} ${src_demo})