// EN: Anchor translation unit that owns the shared precompiled header of the
// EN: C++ textbook examples (examples/common/pch.h++). Each example binary
// EN: reuses this PCH via target_precompile_headers(... REUSE_FROM
// EN: mdbx_examples_pch), so the heavy libmdbx C++ API + standard-library
// EN: headers are parsed once instead of once per example. The TU itself is
// EN: intentionally empty; it exists only to give the STATIC anchor library
// EN: something to compile the PCH for.
// RU: Якорный модуль, владеющий общим прекомпилируемым заголовком C++-
// RU: примеров учебника (examples/common/pch.h++). Каждый бинарник примера
// RU: переиспользует этот PCH через target_precompile_headers(... REUSE_FROM
// RU: mdbx_examples_pch), так что тяжёлые заголовки C++ API libmdbx и
// RU: стандартной библиотеки разбираются один раз, а не для каждого примера.
// RU: Сам модуль намеренно пустой: он нужен лишь для того, чтобы STATIC-
// RU: библиотеке-якорю было что компилировать для PCH.
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h++"
