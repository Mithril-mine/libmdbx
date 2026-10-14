// EN: Pattern: task queue (dequeue via cursor)
// EN: Textbook section: Volume V, chapter 32, "Design patterns", §32.4.
// EN: Task: a task queue table with producer/consumer.
// EN: Solution: the key is a monotonic task number (ordinal), the value is the payload;
// EN:   the consumer takes the first key via a cursor and deletes it.
// EN: Trade-offs: FIFO by number; re-reading a task after a consumer crash
// EN:   requires state flags; concurrent consumers are serialized
// EN:   by the write lock.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 42-pattern-queue.c++ -lmdbx
// EN: Run: ./42-pattern-queue
// EN: Expected output:
// RU: Паттерн: очередь задач (извлечение через курсор)
// RU: Раздел учебника: Том V, глава 32, «Паттерны проектирования», §32.4.
// RU: Задача: таблица-очередь задач с producer/consumer.
// RU: Решение: ключ — монотонный номер задачи (ordinal), значение — payload;
// RU:   consumer забирает первый ключ через курсор и удаляет его.
// RU: Компромиссы: FIFO по номеру; повторное чтение задачи после краха потребителя
// RU:   требует флагов состояния; конкурентные потребители сериализуются
// RU:   write-блокировкой.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 42-pattern-queue.c++ -lmdbx
// RU: Запуск: ./42-pattern-queue
// RU: Ожидаемый вывод:
//   dequeued: 1:a 2:b 3:c
//   queue empty after drain
//   ok: queue pattern works
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

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-42-pattern-queue.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: Producer: tasks with monotonic numbers.
    // RU: Производитель: задачи с монотонными номерами.
    {
      auto txn = env.start_write();
      auto queue = txn.create_map("queue", mdbx::key_mode::ordinal, mdbx::value_mode::single);
      uint64_t seq = 0;
      for (const char *task : {"a", "b", "c"})
        txn.insert(queue, buffer::key_from_u64(++seq), mdbx::slice(task));
      txn.commit();
    }

    // EN: Consumer: takes the first element via a cursor and deletes it.
    // RU: Потребитель: забирает первый элемент через курсор и удаляет.
    auto txn = env.start_write();
    auto queue = txn.open_map("queue", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    std::string dequeued;
    auto cur = txn.open_cursor(queue);
    while (true) {
      auto r = cur.to_first(false);
      if (!r)
        break;
      const auto id = r.key.as_uint64();
      const auto task = r.value.as_string();
      dequeued += std::to_string(id) + ":" + std::string(task) + " ";
      cur.erase(false);
    }
    txn.commit();
    std::cout << "dequeued: " << dequeued << "\n";

    auto rtxn = env.start_read();
    auto rqueue = rtxn.open_map("queue", mdbx::key_mode::ordinal, mdbx::value_mode::single);
    if (rtxn.get_map_stat(rqueue).ms_entries != 0) {
      std::cerr << "FAIL: queue not empty after drain\n";
      return EXIT_FAILURE;
    }
    rtxn.abort();
    std::cout << "queue empty after drain\n";

    std::cout << "ok: queue pattern works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}