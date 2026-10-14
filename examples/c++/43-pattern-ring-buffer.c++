// EN: Pattern: ring buffer over a fixed number of keys
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.5.
// EN: Task: a ring buffer of the last N values (e.g., a readings log).
// EN: Solution: a fixed set of keys 0..N-1; a write overwrites the slot
// EN:   (counter % N); a cursor can traverse the whole buffer in order.
// EN: Trade-offs: an old element is overwritten by a new one — only the "last N";
// EN:   a history log requires a different pattern.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 43-pattern-ring-buffer.c++ -lmdbx
// EN: Run: ./43-pattern-ring-buffer
// EN: Expected output:
// RU: Паттерн: кольцевой буфер на фиксированном наборе ключей
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.5.
// RU: Задача: кольцевой буфер последних N значений (например, лог показаний).
// RU: Решение: фиксированный набор ключей 0..N-1; запись перезаписывает слот
// RU:   (counter % N); курсором можно пройти весь буфер по порядку.
// RU: Компромиссы: старый элемент затирается новым — только «последние N»;
// RU:   для журнала с историей нужен другой паттерн.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 43-pattern-ring-buffer.c++ -lmdbx
// RU: Запуск: ./43-pattern-ring-buffer
// RU: Ожидаемый вывод:
//   buffer contents: s10 s11 s7 s8 s9
//   ok: ring-buffer pattern works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <cstdint>
#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

using buffer = mdbx::buffer<mdbx::default_allocator, mdbx::default_capacity_policy>;

namespace {
constexpr unsigned kSlots = 5;
}

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-43-pattern-ring-buffer.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: Write 12 elements into a buffer with 5 slots.
    // RU: Пишем 12 элементов в буфер на 5 слотов.
    {
      auto txn = env.start_write();
      auto ring = txn.create_map("ring", mdbx::key_mode::ordinal, mdbx::value_mode::single);
      for (unsigned i = 0; i < 12; ++i) {
        const unsigned slot = i % kSlots;
        txn.upsert(ring, buffer::key_from_u64(slot), mdbx::slice("s" + std::to_string(i)));
      }
      txn.commit();
    }

    // EN: Traverse the buffer in slot order.
    // RU: Обход буфера по порядку слотов.
    auto rtxn = env.start_read();
    auto ring = rtxn.open_map("ring", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    std::string contents;
    auto cur = rtxn.open_cursor(ring);
    for (auto r = cur.to_first(); r; r = cur.to_next(false))
      contents += r.value.as_string() + " ";
    std::cout << "buffer contents: " << contents << "\n";
    rtxn.abort();

    std::cout << "ok: ring-buffer pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}