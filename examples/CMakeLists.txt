# Copyright (c) 2020-2026 Леонид Юрьев aka Leonid Yuriev <leo@yuriev.ru> ###############################################
# SPDX-License-Identifier: Apache-2.0
#
project(mdbx_examples)

# Textbook examples are teaching code that intentionally uses portable CRT
# functions (getenv, strtok, ...). MSVC /W4 /WX promotes every C4996
# deprecation to a hard error; suppress the class for this subproject only
# (the library and tests keep their strict per-call pragma policy).
if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  # uint64_t -> size_t narrowing in teaching snippets is intentional (values
  # are small); suppress C4244 for the examples only.
  add_compile_options(/wd4244)
endif()

if(NOT DEFINED MDBX_LIBRARY)
  set(MDBX_LIBRARY mdbx)
endif()

if(MDBX_BUILD_CXX
   AND CMAKE_CXX_COMPILER_LOADED
   AND NOT MDBX_CXX_STANDARD LESS 17)
  add_executable(mdbx_modern_example example-mdbx.c++)
  set_target_properties(mdbx_modern_example PROPERTIES CXX_STANDARD ${MDBX_CXX_STANDARD} CXX_STANDARD_REQUIRED ON)
  target_link_libraries(mdbx_modern_example ${MDBX_LIBRARY})
  if(DEFINED INTERPROCEDURAL_OPTIMIZATION)
    set_target_properties(mdbx_modern_example PROPERTIES INTERPROCEDURAL_OPTIMIZATION
                                                         $<BOOL:${INTERPROCEDURAL_OPTIMIZATION}>)
  endif()
else()
  message(NOTICE
          "The C++ example will be skipped, since C++17 standard is unavailable or C++ API of libmdbx was disabled.")
  set(MDBX_BUILD_CXX FALSE)
endif()

add_executable(mdbx_legacy_example example-mdbx.c)
target_link_libraries(mdbx_legacy_example ${MDBX_LIBRARY})

if(CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
  message(NOTICE "No emulator to run cross-compiled tests")
  set(MDBX_EXAMPLES_RUN_TESTS FALSE)
  add_test(NAME fake_since_no_crosscompiling_emulator COMMAND ${CMAKE_COMMAND} -E echo
                                                              "No emulator to run cross-compiled tests")
else()
  set(MDBX_EXAMPLES_RUN_TESTS TRUE)
  add_test(NAME c_api COMMAND mdbx_legacy_example)
  if(MDBX_BUILD_CXX)
    add_test(NAME c++_api COMMAND mdbx_modern_example)
  endif()
endif()

#-------------------------------------------------------------------------------
# Textbook examples (docs/textbook): scenarios S01..S47 (C++17 primary, C11
# secondary) plus the config-store end-to-end project. Each executable exits 0
# only on a fully successful run.
#
# The dist/ amalgamated package ships only examples/{CMakeLists.txt,
# example-mdbx.c, example-mdbx.c++, pcrf, README.md} (see GNUmakefile), so the
# textbook sources are absent there and must be skipped (MDBX_AMALGAMATED_SOURCE).
if(NOT MDBX_AMALGAMATED_SOURCE)

set(MDBX_TEXTBOOK_COMMON_C common/common.c)

# Adds a C11 textbook example: target, source, links the shared C helpers.
function(mdbx_textbook_add_c target source)
  add_executable(${target} ${source} ${MDBX_TEXTBOOK_COMMON_C})
  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/common")
  set_target_properties(${target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
  target_link_libraries(${target} ${MDBX_LIBRARY})
  if(DEFINED INTERPROCEDURAL_OPTIMIZATION)
    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION
                                                 $<BOOL:${INTERPROCEDURAL_OPTIMIZATION}>)
  endif()
  if(MDBX_EXAMPLES_RUN_TESTS)
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

# Adds a C++17 textbook example: target, source.
function(mdbx_textbook_add_cxx target source)
  add_executable(${target} ${source})
  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/common")
  set_target_properties(${target} PROPERTIES CXX_STANDARD ${MDBX_CXX_STANDARD}
                                                        CXX_STANDARD_REQUIRED ON)
  target_link_libraries(${target} ${MDBX_LIBRARY})
  if(DEFINED INTERPROCEDURAL_OPTIMIZATION)
    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION
                                                 $<BOOL:${INTERPROCEDURAL_OPTIMIZATION}>)
  endif()
  if(MDBX_EXAMPLES_RUN_TESTS)
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

set(MDBX_TEXTBOOK_EXAMPLES_CXX
    01-hello.c++
    02-data-model.c++
    03-crud.c++
    04-transactions.c++
    05-cursors.c++
    06-dupsort-delete.c++
    07-dupsort.c++
    08-inverted-index.c++
    09-secondary-index.c++
    10-composite-key.c++
    11-geometry-options.c++
    12-env-stat.c++
    13-sync-modes.c++
    14-multithreading.c++
    15-fork-resurrect.c++
    16-parking.c++
    17-error-handling.c++
    18-tree-height.c++
    19-readers-lag.c++
    20-commit-latency.c++
    21-gc-observe.c++
    22-gc-limits.c++
    23-map-full.c++
    24-defrag.c++
    25-nested-txn.c++
    26-two-processes.c++
    27-recovery.c++
    28-waf-batching.c++
    29-scenario-configs.c++
    30-append-prefault.c++
    31-get-cached.c++
    32-bulk-ops.c++
    33-profiler.c++
    34-rules-counter.c++
    35-platform-notes.c++
    36-hsr.c++
    37-diagnostics.c++
    39-pattern-sequence-id.c++
    40-pattern-secondary-index.c++
    41-pattern-composite-key.c++
    42-pattern-queue.c++
    43-pattern-ring-buffer.c++
    44-pattern-full-scan.c++
    45-pattern-replication.c++
    46-pattern-read-your-writes.c++
    47-cpp-api.c++)
set(MDBX_TEXTBOOK_EXAMPLES_C
    01-hello.c
    02-data-model.c
    03-crud.c
    04-transactions.c
    05-cursors.c
    06-dupsort-delete.c
    07-dupsort.c
    08-inverted-index.c
    11-geometry-options.c
    12-env-stat.c
    13-sync-modes.c
    17-error-handling.c
    18-tree-height.c
    38-migration.c)

if(MDBX_BUILD_CXX)
  foreach(source IN LISTS MDBX_TEXTBOOK_EXAMPLES_CXX)
    get_filename_component(name "${source}" NAME_WE)
    mdbx_textbook_add_cxx("mdbx_ex_${name}" "c++/${source}")
  endforeach()

  # Сквозной проект config-store: срезы 02–12 (C++17).
  set(MDBX_TEXTBOOK_CONFIG_STORE
      config-store-02.c++
      config-store-04.c++
      config-store-05.c++
      config-store-06.c++
      config-store-08.c++
      config-store-09.c++
      config-store-10.c++
      config-store-11.c++
      config-store-12.c++)

  foreach(source IN LISTS MDBX_TEXTBOOK_CONFIG_STORE)
    get_filename_component(name "${source}" NAME_WE)
    string(REPLACE "config-store-" "" short "${name}")
    mdbx_textbook_add_cxx("mdbx_config_store_${short}" "config-store/${source}")
  endforeach()
endif()

foreach(source IN LISTS MDBX_TEXTBOOK_EXAMPLES_C)
  get_filename_component(name "${source}" NAME_WE)
  mdbx_textbook_add_c("mdbx_ex_${name}_c" "c/${source}")
endforeach()

endif() # NOT MDBX_AMALGAMATED_SOURCE (textbook examples not shipped in dist)

if(UNIX)
  add_executable(pcrf_simulator c/pcrf/pcrf_simulator.c)
  target_link_libraries(pcrf_simulator ${MDBX_LIBRARY})
endif()
