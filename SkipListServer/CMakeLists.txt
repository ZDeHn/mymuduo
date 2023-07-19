set(SKIPLISTSERVER SkipListServer.cc)
add_executable(SkipListServer ${SKIPLISTSERVER})

set(SKIPLISTCLIENT SkipListClient.cc)
add_executable(SkipListClient ${SKIPLISTCLIENT})

target_link_libraries(SkipListServer muduozdh pthread)
