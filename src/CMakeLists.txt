aux_source_directory(. SRC_LIST)
# set(SRC_LIST 
#     Channel.cc EventLoop.cc Logging.cc LogStream.cc Timestamp.cc)
add_library(muduozdh ${SRC_LIST})
target_link_libraries(muduozdh pthread)