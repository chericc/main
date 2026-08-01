cmake_minimum_required(VERSION 3.6)
project(alg)

cmake_minimum_required(VERSION 3.10)
project(AlgorithmDemo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ALG demo_alg)

aux_source_directory(main src_demo_alg_main)
aux_source_directory(heap src_heap)
aux_source_directory(quicksort src_quicksort)
aux_source_directory(nth src_nth)
aux_source_directory(tree src_tree)
aux_source_directory(greedy_algorithm src_greedy)

add_subdirectory(dynamic_programing)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    main
    heap
    quicksort
    nth
    tree
    greedy_algorithm
)

add_executable(${ALG} 
    ${src_demo_alg_main} 
    ${src_heap}
    ${src_quicksort}
    ${src_nth}
    ${src_tree}
    ${src_greedy}
    $<TARGET_OBJECTS:alg_dp>
)

target_link_libraries(${ALG} alg_dp)