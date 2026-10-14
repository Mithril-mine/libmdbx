// EN: Inverted index: word -> list of doc IDs (DUPSORT)
// EN: Textbook section: Volume II, chapter 7, "Multi-values and DUPSORT", section "Inverted indexes".
// EN: What it demonstrates: building an inverted index "word → list of document
// EN:   IDs" over a DUPSORT table and a query by word.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 08-inverted-index.c++ -lmdbx
// EN: Run: ./08-inverted-index
// EN: Expected output:
// EN:   word[fox] -> doc0 doc1
// EN:   word[brown] -> doc0 doc2
// EN:   word[lazy] -> doc2
// EN:   ok: inverted index built and queried
// EN: On failure — message to stderr and exit 1.
// RU: Inverted index: word -> list of doc IDs (DUPSORT)
// RU: Раздел учебника: Том II, глава 7, «Мультизначения и DUPSORT», раздел «Инвертированные индексы».
// RU: Что демонстрирует: построение инвертированного индекса «слово → список ID
// RU:   документов» поверх DUPSORT-таблицы и запрос по слову.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 08-inverted-index.c++ -lmdbx
// RU: Запуск: ./08-inverted-index
// RU: Ожидаемый вывод:
// RU:   word[fox] -> doc0 doc1
// RU:   word[brown] -> doc0 doc2
// RU:   word[lazy] -> doc2
// RU:   ok: inverted index built and queried
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <sstream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-08-inverted-index.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto docs = txn.create_map("docs", mdbx::key_mode::usual, mdbx::value_mode::single);
      // EN: The index: key is a word, values are document IDs (multi-values).
      // RU: Индекс: ключ — слово, значения — ID документов (мультизначения).
      auto words = txn.create_map("words", mdbx::key_mode::usual, mdbx::value_mode::multi);

      static const struct {
        const char *id;
        const char *text;
      } entries[] = {{"doc0", "the quick brown fox"}, {"doc1", "quick red fox"}, {"doc2", "lazy brown dog"}};

      for (const auto &e : entries) {
        txn.insert(docs, mdbx::slice(e.id), mdbx::slice(e.text));
        std::istringstream stream(e.text);
        std::string word;
        while (stream >> word)
          // EN: UPSERT adds the ID to the word
          // RU: UPSERT добавляет ID к слову
          txn.upsert(words, mdbx::slice(word), mdbx::slice(e.id));
      }
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto words = rtxn.open_map("words", mdbx::key_mode::usual, mdbx::value_mode::multi);
    auto cur = rtxn.open_cursor(words);

    // EN: Query: print the list of documents for each word.
    // RU: Запрос: для каждого слова вывести список документов.
    for (const char *w : {"fox", "brown", "lazy"}) {
      std::string ids;
      for (auto r = cur.to_key_exact(mdbx::slice(w), false); r; r = cur.to_current_next_multi(false))
        ids += r.value.as_string() + " ";
      std::cout << "word[" << w << "] -> " << ids << "\n";
    }

    rtxn.abort();

    std::cout << "ok: inverted index built and queried\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}