cmake_minimum_required(VERSION 2.6)
project(MCME C)
set(VERSION "0.1")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -O3")
set (MCME_SOURCES
  src/test.c
  src/slist.c
  src/nbt.c)
set (MCME_DEPENDENCIES
  zlib)
set (MCME_INCLUDES
  lib/zlib-1.2.8)
add_subdirectory(lib)
include_directories(${MCME_INCLUDES})
add_executable(mcme ${MCME_SOURCES})
target_link_libraries(mcme ${MCME_DEPENDENCIES})