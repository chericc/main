set(ALG demo_alg)

aux_source_directory(main src_demo_alg_main)
aux_source_directory(heap src_heap)
aux_source_directory(quicksort src_quicksort)
aux_source_directory(nth src_nth)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH} 
    main
    heap
    quicksort
    nth)

link_directories(${G_XTOOLS})
link_libraries(${G_XTOOLS})

add_executable(${ALG} 
    ${src_demo_alg_main} 
    ${src_heap}
    ${src_quicksort}
    ${src_nth})