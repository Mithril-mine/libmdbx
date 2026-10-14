// EN: Multi-values and DUPSORT: DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// EN: Textbook section: Volume II, chapter 7, "Multi-values and DUPSORT",
// EN:   sections "DUPFIXED/INTEGERDUP", "GET_BOTH", "Inverted indexes".
// EN: What it demonstrates: a table with multiple values (MDBX_DUPSORT|
// EN:   MDBX_DUPFIXED); inserting fixed-length values (uint32_t);
// EN:   reading all values of a key; exact pair lookup (MDBX_GET_BOTH) and range
// EN:   lookup by value range (MDBX_GET_BOTH_RANGE).
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 07-dupsort.c ../common/common.c -lmdbx
// EN: Run: ./07-dupsort
// EN: Expected output:
// RU: Мультизначения и DUPSORT: DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// RU: Раздел учебника: Том II, глава 7, «Мультизначения и DUPSORT»,
// RU:   разделы «DUPFIXED/INTEGERDUP», «GET_BOTH», «Инвертированные индексы».
// RU: Что демонстрирует: таблицу со множественными значениями (MDBX_DUPSORT|
// RU:   MDBX_DUPFIXED); вставку фиксированных по длине значений (uint32_t);
// RU:   чтение всех значений ключа; точный поиск пары (MDBX_GET_BOTH) и поиск
// RU:   по диапазону значений (MDBX_GET_BOTH_RANGE).
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 07-dupsort.c ../common/common.c -lmdbx
// RU: Запуск: ./07-dupsort
// RU: Ожидаемый вывод:
//   values(sensor): 1 3 5 7 9
//   get_both(9) -> 9
//   get_both_range(>=6) -> 7
//   ok: DUPSORT|DUPFIXED, GET_BOTH/GET_BOTH_RANGE
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

static void set_str(MDBX_val *val, const char *s) {
  val->iov_base = (void *)s;
  val->iov_len = strlen(s);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-07-dupsort-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key = {0}, data = {0};
  int rc;

  // EN: Filling the DUPSORT|DUPFIXED table with uint32_t values.
  // RU: Заполнение DUPSORT|DUPFIXED-таблицы значениями uint32_t.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, "multi", MDBX_CREATE | MDBX_DUPSORT | MDBX_DUPFIXED, &dbi);
  check_rc(rc, "mdbx_dbi_open(multi)");
  set_str(&key, "sensor");
  const uint32_t values[] = {5, 1, 9, 3, 7};
  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
    data.iov_base = (void *)&values[i];
    data.iov_len = sizeof(values[i]);
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    check_rc(rc, "mdbx_put");
  }
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, "multi", MDBX_DUPSORT | MDBX_DUPFIXED, &dbi);
  check_rc(rc, "mdbx_dbi_open(multi)");
  MDBX_cursor *cur = NULL;
  rc = mdbx_cursor_open(txn, dbi, &cur);
  check_rc(rc, "mdbx_cursor_open");

  // EN: Reading all values of the key (they are stored sorted).
  // RU: Чтение всех значений ключа (хранятся отсортированными).
  set_str(&key, "sensor");
  printf("values(sensor):");
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_SET);
  while (rc == MDBX_SUCCESS) {
    printf(" %u", *(const uint32_t *)data.iov_base);
    rc = mdbx_cursor_get(cur, &key, &data, MDBX_NEXT_DUP);
  }
  if (rc != MDBX_NOTFOUND)
    die("mdbx_cursor_get(NEXT_DUP): %s", mdbx_strerror(rc));
  printf("\n");

  // EN: GET_BOTH: exact lookup of a "key + value" pair.
  // RU: GET_BOTH: точный поиск пары «ключ + значение».
  const uint32_t needle = 9;
  data.iov_base = (void *)&needle;
  data.iov_len = sizeof(needle);
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH);
  if (rc == MDBX_SUCCESS)
    printf("get_both(9) -> %u\n", *(const uint32_t *)data.iov_base);
  else
    die("mdbx_cursor_get(GET_BOTH): %s", mdbx_strerror(rc));

  // EN: GET_BOTH_RANGE: the first value not less than the given one.
  // RU: GET_BOTH_RANGE: первое значение, не меньшее заданного.
  const uint32_t low = 6;
  data.iov_base = (void *)&low;
  data.iov_len = sizeof(low);
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH_RANGE);
  if (rc == MDBX_SUCCESS)
    printf("get_both_range(>=6) -> %u\n", *(const uint32_t *)data.iov_base);
  else
    die("mdbx_cursor_get(GET_BOTH_RANGE): %s", mdbx_strerror(rc));

  mdbx_cursor_close(cur);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: DUPSORT|DUPFIXED, GET_BOTH/GET_BOTH_RANGE\n");
  return EXIT_SUCCESS;
}