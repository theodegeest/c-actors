project(CActors)

cmake_minimum_required(VERSION 3.16)

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
AUX_SOURCE_DIRECTORY(src/benchmarks BENCH_FILES)
AUX_SOURCE_DIRECTORY(src/c-actors C_ACTORS_FILES)

add_executable(main ${SOURCE_FILES} ${BENCH_FILES} ${C_ACTORS_FILES})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

target_include_directories(main PRIVATE src)
target_link_libraries(main m)
