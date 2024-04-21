set(DESIGN_PATTERNS dp)

set(CREATIONAL ${DESIGN_PATTERNS}_creational)
set(CREATIONAL_PATH Creational)

set(ABSTRACT_FACTORY ${CREATIONAL}_abstract_factory)
set(BUILDER ${CREATIONAL}_builder)

aux_source_directory(${CREATIONAL_PATH}/AbstractFactory ABSTRACT_FACTORY_SRC)
aux_source_directory(${CREATIONAL_PATH}/Builder BUILDER_SRC)

link_libraries(${G_XTOOLS})
include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

add_executable(${ABSTRACT_FACTORY} 
                ${ABSTRACT_FACTORY_SRC})

add_executable(${BUILDER} 
                ${BUILDER_SRC})