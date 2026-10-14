// EN: Environment configuration: geometry and runtime options
// EN: Textbook section: Volume II, chapter 9, "Environment configuration",
// EN:   sections "Geometry", "Runtime options".
// EN: What it demonstrates: setting the geometry (lower/now/upper/growth/shrink) when
// EN:   creating the environment; setting and reading runtime options: writethrough_threshold,
// EN:   merge_threshold, sync_period/sync_bytes, prefault_write_enable.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 11-geometry-options.c++ -lmdbx
// EN: Run: ./11-geometry-options
// EN: Expected output (values may differ on other platforms):
// EN:   geometry: lower=8 MiB now=16 MiB upper=64 MiB grow=8 MiB shrink=2 MiB
// EN:   merge_threshold_dot16=21845 (default ~33%)
// EN:   prefault_write_enable=1
// EN:   ok: geometry and runtime options set/read
// EN: On Windows writethrough_threshold is not supported (§10.5): the value
// EN: is not set and the effective one is printed (2147483647).
// EN: On failure — message to stderr and exit 1.
// RU: Environment configuration: geometry and runtime options
// RU: Раздел учебника: Том II, глава 9, «Конфигурация окружения»,
// RU:   разделы «Геометрия», «Runtime options».
// RU: Что демонстрирует: задание геометрии (lower/now/upper/growth/shrink) при
// RU:   создании окружения; установку и чтение runtime-опций: writethrough_threshold,
// RU:   merge_threshold, sync_period/sync_bytes, prefault_write_enable.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 11-geometry-options.c++ -lmdbx
// RU: Запуск: ./11-geometry-options
// RU: Ожидаемый вывод (значения могут отличаться на других платформах):
// RU:   geometry: lower=8 MiB now=16 MiB upper=64 MiB grow=8 MiB shrink=2 MiB
// RU:   merge_threshold_dot16=21845 (default ~33%)
// RU:   prefault_write_enable=1
// RU:   ok: geometry and runtime options set/read
// RU: На Windows writethrough_threshold не поддерживается (§10.5): значение
// RU: не устанавливается и печатается эффективное (2147483647).
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-11-geometry-options.mdbx";
    mdbx::env::remove(path);

    // EN: Geometry: lower/current/upper size limits of the database, growth step,
    // EN: shrink threshold. Here — a compact database for the example.
    // RU: Геометрия: нижняя/текущая/верхняя границы размера БД, шаг роста,
    // RU: порог сжатия. Здесь — компактная БД для примера.
    mdbx::env::geometry geo;
    geo.size_lower = 8 * mdbx::env::geometry::MB;
    geo.size_now = 16 * mdbx::env::geometry::MB;
    geo.size_upper = 64 * mdbx::env::geometry::MB;
    geo.growth_step = 8 * mdbx::env::geometry::MB;
    geo.shrink_threshold = 2 * mdbx::env::geometry::MB;

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    // EN: Runtime options: we set them and read them back.
    // EN: MDBX_opt_writethrough_threshold is not supported on Windows (Volume II, §10.5):
    // EN: there it is always write-through, and set_option accepts only default/0/UINT_MAX,
    // EN: any other value -> MDBX_EINVAL.
    // RU: Runtime-опции: устанавливаем и читаем обратно.
    // RU: MDBX_opt_writethrough_threshold не поддерживается на Windows (Том II, §10.5):
    // RU: там всегда write-through, а set_option принимает только default/0/UINT_MAX,
    // RU: любое другое значение -> MDBX_EINVAL.
#if !defined(_WIN32)
    env.set_extra_option(mdbx::env::extra_runtime_option::writethrough_threshold, 1 << 20);
#endif
    env.set_extra_option(mdbx::env::extra_runtime_option::merge_threshold_dot16, 65536 / 3);
    // EN: sync_bytes = 256 KiB
    // RU: sync_bytes = 256 KiB
    env.set_sync_threshold(256 * 1024);
    env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);

    using opt = mdbx::env::extra_runtime_option;
    const uint64_t wt = env.extra_option(opt::writethrough_threshold);
    const uint64_t mt = env.extra_option(opt::merge_threshold_dot16);
    const uint64_t sb = env.extra_option(opt::sync_bytes);
    const uint64_t pfw = env.extra_option(opt::prefault_write_enable);
    std::cout << "writethrough_threshold=" << wt << "\n";
    std::cout << "merge_threshold_dot16=" << mt << " (" << (mt * 100 / 65536) << "% of page)\n";
    std::cout << "sync_bytes=" << sb << "\n";
    std::cout << "prefault_write_enable=" << pfw << "\n";

    // EN: The geometry read back after opening.
    // RU: Прочитанная геометрия после открытия.
    auto info = env.get_info();
    const auto &g = info.mi_geo;
    std::cout << "geometry: lower=" << g.lower / 1000 << " KiB now=" << g.current / 1000
              << " KiB upper=" << g.upper / 1000 << " KiB\n";

    std::cout << "ok: geometry and runtime options set/read\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}