cmake_minimum_required(VERSION 3.16)

project(CActors VERSION 0.1 LANGUAGES C)

# --- User-configurable options ---
option(BUILD_BENCHMARKS "Build benchmark targets" ON)
option(BUILD_TESTS      "Build unit/test targets" ON)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(BUILD_WITH_ADDRESSSANITIZER "Build with memory checking" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

if(BUILD_WITH_ADDRESSSANITIZER)
  add_compile_options(-fsanitize=address -g -O1 -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

AUX_SOURCE_DIRECTORY(src SOURCE_FILES)
AUX_SOURCE_DIRECTORY(src/benchmarks BENCH_FILES)
AUX_SOURCE_DIRECTORY(src/c-actors C_ACTORS_FILES)

add_library(cactors STATIC ${C_ACTORS_FILES})

target_compile_options(cactors PRIVATE
  -Wall -Wextra -Wpedantic
  $<$<BOOL:${WARNINGS_AS_ERRORS}>:-Werror>
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:Release>:-O3>
)

add_executable(main ${SOURCE_FILES} ${BENCH_FILES})

target_compile_options(main PRIVATE
  -Wall -Wextra -Wpedantic
  $<$<BOOL:${WARNINGS_AS_ERRORS}>:-Werror>
  $<$<CONFIG:Debug>:-g>
  $<$<CONFIG:Release>:-O3>
)

target_include_directories(main PRIVATE src)
target_link_libraries(main PRIVATE cactors m)


if (BUILD_TESTS)
  enable_testing()

  function(add_cactors_test_by_name name)
    add_executable(${name} "${CMAKE_SOURCE_DIR}/tests/${name}.c" ${BENCH_FILES})
    target_include_directories(${name} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${name} PRIVATE cactors criterion m)
    add_test(NAME ${name} COMMAND ${name})
  endfunction()

  foreach(t
      "message-test"
      "async-send-test"
      "sync-send-test"
    )
    add_cactors_test_by_name(${t})
  endforeach()
endif()
