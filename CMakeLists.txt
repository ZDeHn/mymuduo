cmake_minimum_required(VERSION 3.0)
project(muduozdh)

set(CAMKE_BUILD_TYPE "debug")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/HttpServer)
include_directories(${PROJECT_SOURCE_DIR}/SkiplistServer)
include_directories(${PROJECT_SOURCE_DIR}/testcase)

add_subdirectory(src)
add_subdirectory(HttpServer)
add_subdirectory(SkipListServer)
add_subdirectory(testcase)

link_directories(${PROJECT_SOURCE_DIR}/lib)