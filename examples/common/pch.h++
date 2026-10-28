// EN: Precompiled header shared by every C++ textbook example.
// EN: It gathers the headers every example includes anyway (the common example
// EN: helpers, the libmdbx C++ API and the common standard headers), so they
// EN: are parsed once into a precompiled header (see cmake/pch.cmake and the
// EN: mdbx_examples_pch anchor in examples/CMakeLists.txt) and reused by all
// EN: ~55 example binaries via target_precompile_headers(REUSE_FROM ...).
// EN:
// EN: Keep this header free of any project-local define that is not set for
// EN: every consumer: REUSE_FROM requires the compile flags of all reusing
// EN: targets to match the anchor target exactly.
// RU: Прекомпилируемый заголовок, общий для всех примеров учебника на C++.
// RU: Собирает заголовки, которые включает каждый пример (общие хелперы
// RU: примеров, C++ API libmdbx и общую часть стандартной библиотеки), чтобы
// RU: они разбирались один раз (см. cmake/pch.cmake и якорь mdbx_examples_pch
// RU: в examples/CMakeLists.txt) и переиспользовались всеми ~55 бинарниками
// RU: примеров через target_precompile_headers(REUSE_FROM ...).
// RU: Не добавляйте сюда локальных определений, которые задаются не для всех
// RU: потребителей: REUSE_FROM требует точного совпадния флагов компиляции
// RU: якоря и всех переиспользующих целей.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

/* The common example helpers include <mdbx.h++>, <string> and <string_view>.
   A few examples put their own <iostream>/<string>/... block before including
   common.h++, so the standard headers are listed here as well to cover either
   include order. */
#include "common.h++"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
