// EN: Scenario configurations: template for choosing options
// EN: Textbook section: Volume IV, chapter 22, "Choosing a configuration for a scenario".
// EN: What it demonstrates: six typical environment configurations (write-heavy,
// EN:   read-heavy, mobile, analytics, multi-process reading, low latency) as functions
// EN:   returning operate_parameters; applying one of them and setting additional
// EN:   runtime options on the opened environment.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 29-scenario-configs.c++ -lmdbx
// EN: Run: ./29-scenario-configs
// EN: Expected output:
// EN:   write-heavy config applied; inserted 1000 keys
// EN:   ok: scenario config template applied
// EN: On failure — message to stderr and exit 1.
// RU: Конфигурации под сценарии: шаблон выбора опций
// RU: Раздел учебника: Том IV, глава 22, «Выбор конфигурации под сценарий».
// RU: Что демонстрирует: шесть типовых конфигураций окружения (запись-heavy,
// RU:   чтение-heavy, мобильное, аналитика, многопроцессное чтение, низкая
// RU:   задержка) как функции, возвращающие operate_parameters; применение одной
// RU:   из них и установка дополнительных runtime-опций на открытом окружении.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 29-scenario-configs.c++ -lmdbx
// RU: Запуск: ./29-scenario-configs
// RU: Ожидаемый вывод:
// RU:   write-heavy config applied; inserted 1000 keys
// RU:   ok: scenario config template applied
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

namespace {

using params = mdbx::env::operate_parameters;
using opt = mdbx::env::extra_runtime_option;

// EN: Write-heavy: lazy synchronization, a sufficient number of named tables.
// RU: Запись-heavy: ленивая синхронизация, достаточно именованных таблиц.
params write_heavy() { return params().lazy_weak_tail().set_max_maps(64); }

// EN: Read-heavy: maximum durability, many reader slots.
// RU: Чтение-heavy: максимальная долговечность, много читательских слотов.
[[maybe_unused]] params read_heavy() { return params().robust_synchronous().set_max_readers(1024).set_max_maps(64); }

// EN: Mobile device: small file size, gentle treatment of flash memory.
// RU: Мобильное устройство: малый размер файла, бережное отношение к флеш-памяти.
[[maybe_unused]] params mobile() { return params().lazy_weak_tail().set_max_maps(64); }

// EN: Analytics: batch writes, no nitpicking over balancing.
// RU: Аналитика: пакетная запись, не мелочиться с балансировкой.
[[maybe_unused]] params analytics() { return params().lazy_weak_tail().set_max_maps(64); }

// EN: Multi-process reading: many reader slots, stable geometry.
// RU: Многопроцессное чтение: много читательских слотов, стабильная геометрия.
[[maybe_unused]] params multi_process_read() { return params().set_max_readers(4096).set_max_maps(64); }

// EN: Low latency: NOMETASYNC (data is on disk, metadata may lag behind).
// RU: Низкая задержка: NOMETASYNC (данные на диске, метаданные могут отставать).
[[maybe_unused]] params low_latency() { return params().half_synchronous_weak_last().set_max_maps(64); }

} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-29-scenario-configs.mdbx";
    mdbx::env::remove(path);

    // EN: "Application template": take the configuration for the scenario and work.
    // RU: «Шаблон применения»: берём конфигурацию под сценарий и работаем.
    auto env = example::env_open(path, write_heavy());
    // EN: Additional runtime options are set on the opened environment:
    // RU: Дополнительные runtime-опции выставляются на открытом окружении:
    env.set_extra_option(opt::prefault_write_enable, 1);
    // EN: ~33% of the page
    // RU: ~33% страницы
    env.set_extra_option(opt::merge_threshold_dot16, 65536 / 3);
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 1000; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }
    std::cout << "write-heavy config applied; inserted 1000 keys\n";

    std::cout << "ok: scenario config template applied\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}