// EN: Inverted index: word -> list of doc IDs (DUPSORT)
// EN: Textbook section: Volume II, chapter 7, "Multi-values and DUPSORT", section "Inverted indexes".
// EN: What it demonstrates: building an inverted index "word → list of document
// EN:   IDs" on top of a DUPSORT table and a query by word.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 08-inverted-index.c ../common/common.c -lmdbx
// EN: Run: ./08-inverted-index
// EN: Expected output:
// RU: Инвертированный индекс: слово → список ID документов (DUPSORT)
// RU: Раздел учебника: Том II, глава 7, «Мультизначения и DUPSORT», раздел «Инвертированные индексы».
// RU: Что демонстрирует: построение инвертированного индекса «слово → список ID
// RU:   документов» поверх DUPSORT-таблицы и запрос по слову.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 08-inverted-index.c ../common/common.c -lmdbx
// RU: Запуск: ./08-inverted-index
// RU: Ожидаемый вывод:
//   word[fox] -> doc0 doc1
//   word[brown] -> doc0 doc2
//   word[lazy] -> doc2
//   ok: inverted index built and queried
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

static void set_str(MDBX_val *val, const char *s) {
  val->iov_base = (void *)s;
  val->iov_len = strlen(s);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-08-inverted-index-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi docs, words;
  MDBX_val key = {0}, data = {0};
  int rc;

  static const char *entry_ids[] = {"doc0", "doc1", "doc2"};
  static const char *entry_texts[] = {"the quick brown fox", "quick red fox", "lazy brown dog"};

  // EN: Filling the tables: documents + inverted index.
  // RU: Заполнение таблиц: документы + инвертированный индекс.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, "docs", MDBX_CREATE, &docs);
  check_rc(rc, "mdbx_dbi_open(docs)");
  rc = mdbx_dbi_open(txn, "words", MDBX_CREATE | MDBX_DUPSORT, &words);
  check_rc(rc, "mdbx_dbi_open(words)");

  for (size_t i = 0; i < sizeof(entry_ids) / sizeof(entry_ids[0]); ++i) {
    set_str(&key, entry_ids[i]);
    set_str(&data, entry_texts[i]);
    rc = mdbx_put(txn, docs, &key, &data, 0);
    check_rc(rc, "mdbx_put(docs)");

    // EN: Split the text into words and attach each word to the document ID.
    // EN: (A portable tokenizer instead of strtok: it is declared deprecated
    // EN: in the Windows CRT, and strtok_r/strtok_s differ between platforms.)
    // RU: Разбить текст на слова и привязать каждое к ID документа.
    // RU: (Портативный токенизатор вместо strtok: тот объявлен deprecated
    // RU: в CRT Windows, а strtok_r/strtok_s различаются между платформами.)
    char text[128];
    snprintf(text, sizeof(text), "%s", entry_texts[i]);
    for (char *word = text;;) {
      while (*word == ' ')
        ++word;
      if (!*word)
        break;
      char *end = word;
      while (*end && *end != ' ')
        ++end;
      const char saved = *end;
      *end = '\0';
      set_str(&key, word);
      *end = saved;
      set_str(&data, entry_ids[i]);
      rc = mdbx_put(txn, words, &key, &data, MDBX_NODUPDATA);
      if (rc != MDBX_SUCCESS && rc != MDBX_KEYEXIST)
        die("mdbx_put(words): %s", mdbx_strerror(rc));
      word = end;
    }
  }
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  // EN: Query: for each word print the list of documents.
  // RU: Запрос: для каждого слова вывести список документов.
  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, "words", MDBX_DUPSORT, &words);
  check_rc(rc, "mdbx_dbi_open(words)");
  MDBX_cursor *cur = NULL;
  rc = mdbx_cursor_open(txn, words, &cur);
  check_rc(rc, "mdbx_cursor_open");

  static const char *queries[] = {"fox", "brown", "lazy"};
  for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i) {
    set_str(&key, queries[i]);
    printf("word[%s] ->", queries[i]);
    rc = mdbx_cursor_get(cur, &key, &data, MDBX_SET);
    while (rc == MDBX_SUCCESS) {
      printf(" %.*s", (int)data.iov_len, (const char *)data.iov_base);
      rc = mdbx_cursor_get(cur, &key, &data, MDBX_NEXT_DUP);
    }
    if (rc != MDBX_NOTFOUND)
      die("mdbx_cursor_get(NEXT_DUP): %s", mdbx_strerror(rc));
    printf("\n");
  }

  mdbx_cursor_close(cur);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: inverted index built and queried\n");
  return EXIT_SUCCESS;
}