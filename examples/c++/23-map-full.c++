// EN: MAP_FULL reproduction and resolution via geometry growth
// EN: Textbook section: Volume III, chapter 17, "Growth, shrinking and defragmentation", §17.3 "MAP_FULL".
// EN: What it demonstrates: a small geometry + large volume → MDBX_MAP_FULL;
// EN:   resolution by increasing the upper bound (set_geometry) and retrying.
// EN:   On 32-bit Windows in-place remapping may be unavailable
// EN:   (MDBX_UNABLE_EXTEND_MAPSIZE); the example then reopens the DB with an increased
// EN:   geometry — the result is the same, but the new mapping is created directly with the needed upper.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 23-map-full.c++ -lmdbx
// EN: Run: ./23-map-full
// EN: Expected output:
// EN:   MDBX_MAP_FULL after <N> inserts
// EN:   grew geometry, continued: total <M> inserts
// EN:   ok: MAP_FULL reproduced and resolved
// EN: On failure — message to stderr and exit 1.
// RU: Воспроизведение MAP_FULL и решение через рост геометрии
// RU: Раздел учебника: Том III, глава 17, «Рост, сжатие и дефрагментация», §17.3 «MAP_FULL».
// RU: Что демонстрирует: малая геометрия + большой объём → MDBX_MAP_FULL;
// RU:   разрешение через увеличение верхней границы (set_geometry) и повтор.
// RU:   На 32-битной Windows переотображение на месте может быть недоступно
// RU:   (MDBX_UNABLE_EXTEND_MAPSIZE); пример тогда переоткрывает БД с увеличенной
// RU:   геометрией — результат тот же, но новое отображение создаётся сразу с нужным upper.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 23-map-full.c++ -lmdbx
// RU: Запуск: ./23-map-full
// RU: Ожидаемый вывод:
// RU:   MDBX_MAP_FULL after <N> inserts
// RU:   grew geometry, continued: total <M> inserts
// RU:   ok: MAP_FULL reproduced and resolved
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-23-map-full.mdbx";
    mdbx::env::remove(path);

    // EN: A rigid geometry: the upper bound == the current size, only 512 KB.
    // RU: Жёсткая геометрия: верхняя граница == текущий размер, всего 512 КБ.
    mdbx::env::geometry geo;
    geo.size_lower = 512 * 1024;
    geo.size_now = 512 * 1024;
    geo.size_upper = 512 * 1024;

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    size_t inserted = 0;
    try {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (;;) {
        const auto key = "k" + std::to_string(inserted);
        const std::string value(1024, 'x');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
        ++inserted;
      }
    } catch (const mdbx::db_full &) {
      std::cout << "MDBX_MAP_FULL after " << inserted << " inserts\n";
    }

    // EN: We increase the upper bound — and continue from the same point.
    // EN: On most platforms mdbx remaps the DB in place; but on 32-bit Windows the
    // EN: required address range after the current mapping may be occupied
    // EN: (MDBX_UNABLE_EXTEND_MAPSIZE). For this case a portable fallback path is
    // EN: provided: reopen the same DB with an increased geometry (the new mapping
    // EN: is created with the needed upper).
    // RU: Увеличиваем верхнюю границу — и продолжаем с того же места.
    // RU: На большинстве платформ mdbx переотображает БД на месте; но на 32-битной
    // RU: Windows нужный диапазон адресов после текущего отображения может
    // RU: оказаться занятым (MDBX_UNABLE_EXTEND_MAPSIZE). Для этого случая
    // RU: предусмотрен переносимый запасной путь: переоткрыть ту же БД с
    // RU: увеличенной геометрией (новое отображение создаётся с нужным upper).
    mdbx::env::geometry bigger;
    bigger.size_upper = 8 * 1024 * 1024;
    try {
      env.set_geometry(bigger);
    } catch (const mdbx::db_unable_extend &) {
      mdbx::env::geometry reopened = geo;
      reopened.size_upper = bigger.size_upper;
      env.close();
      env = mdbx::env_managed(path, mdbx::env_managed::create_parameters().set_geometry(reopened),
                              mdbx::env::operate_parameters());
    }

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 2000; ++i) {
        const auto key = "k" + std::to_string(inserted + i);
        const std::string value(1024, 'y');
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }
    std::cout << "grew geometry, continued: total " << (inserted + 2000) << " inserts\n";

    std::cout << "ok: MAP_FULL reproduced and resolved\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}