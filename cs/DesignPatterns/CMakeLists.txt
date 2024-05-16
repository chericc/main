set(DESIGN_PATTERNS dp)

set(CREATIONAL ${DESIGN_PATTERNS}_creational)
set(CREATIONAL_PATH Creational)
set(STRUCTURAL ${DESIGN_PATTERNS}_structural)
set(STRUCTURAL_PATH Structural)

# creational
set(ABSTRACT_FACTORY ${CREATIONAL}_abstract_factory)
set(BUILDER ${CREATIONAL}_builder)
set(FACTORY_METHOD ${CREATIONAL}_factory_method)
set(PROTOTYPE ${CREATIONAL}_prototype)
set(SINGLETON ${CREATIONAL}_singleton)

# structural
set(ADAPTER ${STRUCTURAL}_adapter)

aux_source_directory(${CREATIONAL_PATH}/AbstractFactory ABSTRACT_FACTORY_SRC)
aux_source_directory(${CREATIONAL_PATH}/Builder BUILDER_SRC)
aux_source_directory(${CREATIONAL_PATH}/FactoryMethod FACTORY_METHOD_SRC)
aux_source_directory(${CREATIONAL_PATH}/Prototype PROTOTYPE_SRC)
aux_source_directory(${CREATIONAL_PATH}/Singleton SINGLETON_SRC)

aux_source_directory(${STRUCTURAL_PATH}/Adapter ADAPTER_SRC)

link_libraries(${G_XTOOLS})
include_directories(${G_XTOOLS_XLOG_HEADER_PATH})

# creational/abstract_factory
add_executable(${ABSTRACT_FACTORY} 
                ${ABSTRACT_FACTORY_SRC})

# creational/builder
add_executable(${BUILDER} 
                ${BUILDER_SRC})

# creational/factory_method
add_executable(${FACTORY_METHOD}
                ${FACTORY_METHOD_SRC})

# creational/prototype
add_executable(${PROTOTYPE}
                ${PROTOTYPE_SRC})

# creational/singleton
add_executable(${SINGLETON}
                ${SINGLETON_SRC})

# structural/adapter
add_executable(${ADAPTER}
                ${ADAPTER_SRC})