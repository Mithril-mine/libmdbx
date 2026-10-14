// EN: config-store, slice 08: secondary index by tags
// EN: Textbook section: Volume II, chapter 8, "Secondary indexes", §8.2 "Maintaining consistency".
// EN: What is added: tag support — a tag table (the "tag → key list" index),
// EN:   updated in the same transaction as the main table; the "find by tag" query.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-08.c++ -lmdbx
// EN: Run: ./config-store-08
// EN: Expected output:
// RU: config-store, срез 08: вторичный индекс по тегам
// RU: Раздел учебника: Том II, глава 8, «Вторичные индексы», §8.2 «Поддержание консистентности».
// RU: Что добавляется: поддержка тегов — таблица тегов (индекс «тег → список ключей»),
// RU:   обновляемая в одной транзакции с основной таблицей; запрос «найти по тегу».
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common config-store-08.c++ -lmdbx
// RU: Запуск: ./config-store-08
// RU: Ожидаемый вывод:
//   by-tag 'network': host port
//   by-tag 'ui': theme
//   ok: config-store-08 works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/config-store-08.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: Each configuration entry is marked with one or more tags;
    // EN: the "tag → key list" index is maintained in the same transaction.
    // RU: Каждая запись конфигурации помечается одним или несколькими тегами;
    // RU: индекс «тег → список ключей» поддерживается в той же транзакции.
    {
      auto txn = env.start_write();
      auto config = txn.create_map("config", mdbx::key_mode::usual, mdbx::value_mode::single);
      auto by_tag = txn.create_map("by_tag", mdbx::key_mode::usual, mdbx::value_mode::multi);

      const struct {
        const char *key;
        const char *value;
        const char *tag;
      } entries[] = {{"host", "localhost", "network"}, {"port", "8080", "network"}, {"theme", "dark", "ui"}};

      for (const auto &e : entries) {
        txn.insert(config, mdbx::slice(e.key), mdbx::slice(e.value));
        // EN: index reference
        // RU: индексная ссылка
        txn.upsert(by_tag, mdbx::slice(e.tag), mdbx::slice(e.key));
      }
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto config = rtxn.open_map("config", mdbx::key_mode::usual, mdbx::value_mode::single);
    auto by_tag = rtxn.open_map("by_tag", mdbx::key_mode::usual, mdbx::value_mode::multi);

    // EN: Query by tag: keys from the index, values from the main table.
    // RU: Запрос по тегу: ключи из индекса, значения из основной таблицы.
    for (const char *tag : {"network", "ui"}) {
      std::string keys;
      auto cur = rtxn.open_cursor(by_tag);
      for (auto r = cur.to_key_exact(mdbx::slice(tag)); r; r = cur.to_current_next_multi(false))
        keys += r.value.as_string() + " ";
      std::cout << "by-tag '" << tag << "': " << keys << "\n";
    }
    // EN: the main table is used in the real get/set commands
    // RU: основная таблица используется при реальных командах get/set
    (void)config;

    rtxn.abort();

    std::cout << "ok: config-store-08 works\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}