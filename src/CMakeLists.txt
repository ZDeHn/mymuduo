aux_source_directory(. SRC_LIST)

add_library(muduozdh ${SRC_LIST})
target_link_libraries(muduozdh pthread)