project(CActors)

cmake_minimum_required(VERSION 3.16)

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)

add_executable(main ${SOURCE_FILES})

set(CMAKE_BUILD_TYPE Debug)

target_include_directories(main PRIVATE src)
target_link_libraries(main m)
