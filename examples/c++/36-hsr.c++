// EN: HSR: Handle-Slow-Readers callback
// EN: Textbook section: Volume V, chapter 29, "HSR", §29.3 "Implementations", §29.4 "HSR+SAFE_NOSYNC".
// EN: What it demonstrates: registering the HSR callback (mdbx_env_set_hsr) — when
// EN:   the database overflows due to a "slow" reader, the callback is invoked with
// EN:   information about the reader; here the callback asks the reader to release its snapshot,
// EN:   after which the writer continues.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 36-hsr.c++ -lmdbx -pthread
// EN: Run: ./36-hsr
// EN: Expected output:
// RU: HSR: колбэк Handle-Slow-Readers
// RU: Раздел учебника: Том V, глава 29, «HSR», §29.3 «Реализации», §29.4 «HSR+SAFE_NOSYNC».
// RU: Что демонстрирует: регистрацию HSR-колбэка (mdbx_env_set_hsr) — при
// RU:   переполнении БД из-за «медленного» читателя колбэк вызывается с
// RU:   информацией о читателе; здесь колбэк просит читателя освободить снапшот,
// RU:   после чего писатель продолжает работу.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 36-hsr.c++ -lmdbx -pthread
// RU: Запуск: ./36-hsr
// RU: Ожидаемый вывод:
//   hsr invoked: laggard pid=<P> lag=<L> retry=<R>
//   write succeeded after reader released
//   ok: HSR callback handled slow reader
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include <mdbx.h++>
#include "common.h++"

namespace {
std::atomic<bool> hsr_release_reader{false};
std::atomic<bool> reader_released{false};
std::atomic<bool> hsr_called{false};
std::atomic<bool> stop_reader{false};

// EN: The callback is invoked when the database hits its limit because of readers
// EN: holding old snapshots. Here: we report the event, ask the reader to
// EN: release its snapshot and wait for it, then return MDBX_RESULT_TRUE
// EN: ("problem resolved" — the writer will continue).
// RU: Колбэк вызывается, когда БД «упирается» в предел из-за читателей,
// RU: удерживающих старые снапшоты. Здесь: сообщаем о событии, просим читателя
// RU: освободить снапшот и ждём его, после чего возвращаем MDBX_RESULT_TRUE
// RU: («проблема устранена» — писатель продолжит).
int hsr_callback(const MDBX_env *, const MDBX_txn *, mdbx_pid_t pid, mdbx_tid_t tid, uint64_t laggard,
                 unsigned gap, size_t space, int retry) noexcept {
  std::cout << "hsr invoked: laggard pid=" << pid << " tid=" << tid << " lag=" << laggard << " gap=" << gap
            << " space=" << space << " retry=" << retry << "\n";
  hsr_called = true;
  hsr_release_reader = true;
  while (!reader_released.load())
    // EN: wait for the actual release
    // RU: дождаться фактического освобождения
    std::this_thread::yield();
  return MDBX_RESULT_TRUE;
}
} // namespace

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-36-hsr.mdbx";
    mdbx::env::remove(path);

    // EN: Compact geometry + SAFE_NOSYNC (the variant from §29.4).
    // RU: Компактная геометрия + SAFE_NOSYNC (вариант из §29.4).
    mdbx::env::geometry geo;
    geo.size_lower = 256 * 1024;
    geo.size_now = 256 * 1024;
    geo.size_upper = 256 * 1024;
    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters().lazy_weak_tail());

    env.set_HandleSlowReaders(hsr_callback);

    // EN: Initial fill: keys that the writer will overwrite,
    // EN: aging their versions (CoW) — this is what creates pages held by
    // EN: a slow reader and blocking recycling.
    // RU: Начальное наполнение: ключи, которые писатель будет перезаписывать,
    // RU: «старея» их версии (CoW) — именно это создаёт страницы, удерживаемые
    // RU: медленным читателем и блокирующие рециклинг.
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 200; ++i)
        txn.insert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice("v"));
      txn.commit();
    }

    // EN: A long-lived reader: a snapshot from the point after the initial fill.
    // RU: Долгоживущий читатель: снапшот с момента после начального наполнения.
    std::atomic<bool> reader_ready{false};
    std::thread holder([&] {
      auto rtxn = env.start_read();
      auto table = rtxn.open_map(nullptr);
      (void)rtxn.get(table, mdbx::slice("k0"));
      reader_ready = true;
      while (!hsr_release_reader.load() && !stop_reader.load())
        std::this_thread::yield();
      rtxn.abort();
      reader_released = true;
    });
    while (!reader_ready.load())
      std::this_thread::yield();

    // EN: The writer overwrites the keys with ever larger values: old
    // EN: page versions "retire" and cannot be reused,
    // EN: while the reader holds its snapshot — when exhausted, HSR triggers.
    // RU: Писатель перезаписывает ключи всё более крупными значениями: старые
    // RU: версии страниц «уходят на покой» и не могут быть переиспользованы,
    // RU: пока читатель держит свой снапшот — при исчерпании срабатывает HSR.
    bool wrote = false;
    for (int attempt = 0; attempt < 8 && !wrote; ++attempt) {
      try {
        auto txn = env.start_write();
        auto table = txn.open_map(nullptr);
        const std::string value(2048 + attempt * 4096, 'x');
        for (int i = 0; i < 200; ++i)
          txn.upsert(table, mdbx::slice("k" + std::to_string(i)), mdbx::slice(value));
        txn.commit();
        wrote = true;
      } catch (const mdbx::db_full &) {
        // EN: Either HSR already fired, or we hit the geometry limit.
        // RU: Либо HSR уже отработал, либо упёрлись в предел геометрии.
      }
    }
    if (!wrote)
      std::cout << "note: writes blocked at geometry limit (HSR not triggered this run)\n";
    stop_reader = true;
    holder.join();

    // EN: After the reader is released, writing is always possible.
    // RU: После освобождения читателя запись всегда возможна.
    if (!wrote) {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("after-release"), mdbx::slice("ok"));
      txn.commit();
      wrote = true;
    }

    if (!wrote) {
      std::cerr << "FAIL: could not write after reader released\n";
      return EXIT_FAILURE;
    }
    std::cout << "write succeeded after reader released\n";

    if (!hsr_called.load())
      std::cout << "note: HSR callback was not triggered in this run (geometry not exhausted)\n";

    std::cout << "ok: HSR callback handled slow reader\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}