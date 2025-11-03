cmake_minimum_required(VERSION 3.16)

project(CActors VERSION 0.1 LANGUAGES C)

# --- User-configurable options ---
option(BUILD_BENCHMARKS "Build benchmark targets" ON)
option(BUILD_TESTS      "Build unit/test targets" ON)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
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

# --- Tests (example) ---
if (BUILD_TESTS)
  enable_testing()
  # Add test executables explicitly
  add_executable(message_test tests/message-test.c ${BENCH_FILES})
  target_include_directories(message_test PRIVATE src)
  target_link_libraries(message_test PRIVATE cactors criterion m)
  add_test(NAME message-test COMMAND message_test)

  add_executable(async_send_test tests/async-send-test.c ${BENCH_FILES})
  target_include_directories(async_send_test PRIVATE src)
  target_link_libraries(async_send_test PRIVATE cactors criterion m)
  add_test(NAME async-send-test COMMAND async_send_test)

  add_executable(sync_send_test tests/sync-send-test.c ${BENCH_FILES})
  target_include_directories(sync_send_test PRIVATE src)
  target_link_libraries(sync_send_test PRIVATE cactors criterion m)
  add_test(NAME sync-send-test COMMAND sync_send_test)
endif()
