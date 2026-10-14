// EN: Data model: MDBX_val, integer keys, empty key/value, named tables
// EN: Textbook section: Volume I, chapter 3, "Basic data model",
// EN:   sections "MDBX_val", "Main table and dbi", "Integer keys", "Zero keys/values".
// EN: What it demonstrates: MDBX_val (iov_base + iov_len); the main and named tables;
// EN:   integer keys (MDBX_INTEGERKEY); an empty key and an empty value.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 02-data-model.c ../common/common.c -lmdbx
// EN: Run: ./02-data-model
// EN: Expected output:
// RU: Модель данных: MDBX_val, integer-ключи, пустой ключ/значение, именованные таблицы
// RU: Раздел учебника: Том I, глава 3, «Базовая модель данных»,
// RU:   разделы «MDBX_val», «Главная таблица и dbi», «Integer-ключи», «Нулевые ключи/значения».
// RU: Что демонстрирует: MDBX_val (iov_base + iov_len); главную и именованные таблицы;
// RU:   integer-ключи (MDBX_INTEGERKEY); пустой ключ и пустое значение.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 02-data-model.c ../common/common.c -lmdbx
// RU: Запуск: ./02-data-model
// RU: Ожидаемый вывод:
//   main[key] = value
//   ordinal[1000] = thousand
//   ok: integer keys, named table, empty key/value
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-02-data-model-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi main_db, named_db, ord_db;
  MDBX_val key = {0}, data = {0};
  int rc;

  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");

  // EN: Main table: NULL name, ordinary keys.
  // RU: Главная таблица: имя NULL, обычные ключи.
  rc = mdbx_dbi_open(txn, NULL, 0, &main_db);
  check_rc(rc, "mdbx_dbi_open(main)");
  key.iov_base = (void *)"key";
  key.iov_len = strlen("key");
  data.iov_base = (void *)"value";
  data.iov_len = strlen("value");
  rc = mdbx_put(txn, main_db, &key, &data, 0);
  check_rc(rc, "mdbx_put(main)");

  // EN: Named table: a separate key space within the same database.
  // RU: Именованная таблица: отдельное пространство ключей внутри той же БД.
  rc = mdbx_dbi_open(txn, "named", MDBX_CREATE, &named_db);
  check_rc(rc, "mdbx_dbi_open(named)");
  key.iov_base = (void *)"config";
  key.iov_len = strlen("config");
  data.iov_base = (void *)"42";
  data.iov_len = strlen("42");
  rc = mdbx_put(txn, named_db, &key, &data, 0);
  check_rc(rc, "mdbx_put(named)");

  // EN: Integer keys: MDBX_INTEGERKEY, keys are uint64_t in native byte order.
  // RU: Integer-ключи: MDBX_INTEGERKEY, ключи — uint64_t в нативном порядке байт.
  rc = mdbx_dbi_open(txn, "ordinal", MDBX_CREATE | MDBX_INTEGERKEY, &ord_db);
  check_rc(rc, "mdbx_dbi_open(ordinal)");
  uint64_t k = 1000;
  key.iov_base = &k;
  key.iov_len = sizeof(k);
  data.iov_base = (void *)"thousand";
  data.iov_len = strlen("thousand");
  rc = mdbx_put(txn, ord_db, &key, &data, 0);
  check_rc(rc, "mdbx_put(ordinal)");

  // EN: An empty key (zero length) and an empty value.
  // RU: Пустой ключ (нулевой длины) и пустое значение.
  key.iov_base = NULL;
  key.iov_len = 0;
  data.iov_base = (void *)"empty-key-value";
  data.iov_len = strlen("empty-key-value");
  rc = mdbx_put(txn, main_db, &key, &data, 0);
  check_rc(rc, "mdbx_put(empty key)");
  key.iov_base = (void *)"empty";
  key.iov_len = strlen("empty");
  data.iov_base = NULL;
  data.iov_len = 0;
  rc = mdbx_put(txn, named_db, &key, &data, 0);
  check_rc(rc, "mdbx_put(empty value)");

  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  // EN: Reading in a read-only transaction.
  // RU: Чтение в read-only транзакции.
  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(read)");
  rc = mdbx_dbi_open(txn, NULL, 0, &main_db);
  check_rc(rc, "mdbx_dbi_open(main)");
  key.iov_base = (void *)"key";
  key.iov_len = strlen("key");
  rc = mdbx_get(txn, main_db, &key, &data);
  check_rc(rc, "mdbx_get");
  printf("main[key] = %.*s\n", (int)data.iov_len, (const char *)data.iov_base);

  rc = mdbx_dbi_open(txn, "ordinal", MDBX_INTEGERKEY, &ord_db);
  check_rc(rc, "mdbx_dbi_open(ordinal)");
  key.iov_base = &k;
  key.iov_len = sizeof(k);
  rc = mdbx_get(txn, ord_db, &key, &data);
  check_rc(rc, "mdbx_get(ordinal)");
  printf("ordinal[1000] = %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: integer keys, named table, empty key/value\n");
  return EXIT_SUCCESS;
}