# Copyright (c) 2015-2026 Леонид Юрьев aka Leonid Yuriev <leo@yuriev.ru> ###############################################
# SPDX-License-Identifier: Apache-2.0
#
# Shared precompiled-header infrastructure.
#
# Precompiling the headers every translation unit needs anyway (GoogleTest, the
# libmdbx APIs, the common standard headers) saves minutes on a clean build:
# each gtest binary and each C++ example is a single TU, so without a PCH the
# same headers are re-parsed once per binary. A single anchor library owns the
# PCH and every consumer reuses it via target_precompile_headers(REUSE_FROM ...).
#
# target_precompile_headers()/REUSE_FROM need CMake >= 3.16 while the project
# itself still builds on 3.12, so the PCH is an opt-in acceleration gated on
# the CMake version rather than a raised project-wide minimum. The gate is
# expressed by which definition of the <name>_apply macro is in effect (the
# real one is defined here alongside the anchor, a no-op fallback after this
# file), so call sites never carry a version branch.
#
# REUSE_FROM requires the compile flags of the anchor and of every reusing
# target to match: the same C++ standard, the same include dirs and the same
# PIC/PIE mode. The anchor mirrors the consumers' flags (passed as arguments),
# and a consumer whose flags differ (e.g. an MDBX_PROBES one) simply does not
# reuse the PCH — GCC then falls back to a normal compile without an error, so
# this is safe.

# mdbx_add_pch_anchor(<name> <header> <anchor_source>
#                     [CXX_STANDARD <n>]
#                     [INCLUDE <dir>...]
#                     [LINK <lib>...]
#                     [LINK_PRIVATE <lib>...])
#
# Creates a static anchor library <name> that precompiles <header>, and defines
# the <name>_apply(<target>) macro that makes a target reuse that PCH. When the
# PCH is disabled (MDBX_ENABLE_PCH=OFF), CMake < 3.16, or the C++ compiler is
# not loaded, the macro is a no-op and no anchor library is created.
#
# The standard, include dirs and (PRIVATE) link libraries are applied to the
# anchor so the precompiled header is produced with the same flags the
# consumers compile with. The anchor is linked PRIVATE so the dependency does
# not propagate to consumers (they link the library themselves).
#
# This is a macro (not a function) on purpose: it must create the anchor
# library and the <name>_apply macro in the *calling* scope, so both become
# visible to the targets declared after the call.
macro(mdbx_add_pch_anchor name header anchor_source)
  cmake_parse_arguments(PCH "" "CXX_STANDARD" "INCLUDE;LINK;LINK_PRIVATE" ${ARGN})

  if(MDBX_ENABLE_PCH AND NOT CMAKE_VERSION VERSION_LESS 3.16 AND CMAKE_CXX_COMPILER_LOADED)
    add_library(${name} STATIC "${anchor_source}")
    if(PCH_INCLUDE)
      target_include_directories(${name} PUBLIC ${PCH_INCLUDE})
    endif()
    if(PCH_LINK)
      target_link_libraries(${name} PUBLIC ${PCH_LINK})
    endif()
    if(PCH_LINK_PRIVATE)
      # The anchor is only a PCH carrier, its consumers already link the
      # library themselves, so the PRIVATE dependency must not propagate.
      target_link_libraries(${name} PRIVATE ${PCH_LINK_PRIVATE})
    endif()
    # The consumers are executables: with CMAKE_POSITION_INDEPENDENT_CODE on
    # Linux/GCC they are compiled -fPIE, while a static library defaults to
    # -fPIC. GCC/Clang refuse to reuse a PCH created with different PIC/PIE
    # settings (a fatal -Winvalid-pch under -Werror), so build the anchor's
    # PCH with the very PIE option the consumers get. Use
    # CMAKE_CXX_COMPILE_OPTIONS_PIE rather than a hardcoded -fPIE: it is
    # "-fPIE" on Linux/GCC but empty on Windows (MinGW leaves
    # COMPILE_OPTIONS_PIE empty and MSVC has no such flag), which is exactly
    # where the executables are built without -fPIE.
    set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    if(CMAKE_CXX_COMPILE_OPTIONS_PIE AND CMAKE_POSITION_INDEPENDENT_CODE)
      target_compile_options(${name} PRIVATE ${CMAKE_CXX_COMPILE_OPTIONS_PIE})
    endif()
    if(PCH_CXX_STANDARD)
      set_target_properties(${name} PROPERTIES CXX_STANDARD ${PCH_CXX_STANDARD} CXX_STANDARD_REQUIRED ON)
    endif()
    target_precompile_headers(${name} PRIVATE "${header}")
    # Real definition of the apply helper, in effect only when the anchor was
    # built; call sites just invoke <name>_apply() with no branch.
    macro(${name}_apply target)
      target_link_libraries(${target} ${name})
      target_precompile_headers(${target} REUSE_FROM ${name})
    endmacro()
  else()
    macro(${name}_apply target)
    endmacro()
  endif()
endmacro()
