// EN: CRUD: put/get/del with write flags
// EN: Textbook section: Volume I, chapter 4, "Basic CRUD operations",
// EN:   sections "put/get/del/replace", "Write flags".
// EN: What it demonstrates: MDBX_NOOVERWRITE and MDBX_KEYEXIST as an expected code;
// EN:   MDBX_UPSERT; MDBX_CURRENT; deletion; MDBX_NOTFOUND as an expected code.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 03-crud.c ../common/common.c -lmdbx
// EN: Run: ./03-crud
// EN: Expected output:
// RU: CRUD: put/get/del с флагами записи
// RU: Раздел учебника: Том I, глава 4, «Базовые операции CRUD»,
// RU:   разделы «put/get/del/replace», «Флаги записи».
// RU: Что демонстрирует: MDBX_NOOVERWRITE и MDBX_KEYEXIST как ожидаемый код;
// RU:   MDBX_UPSERT; MDBX_CURRENT; удаление; MDBX_NOTFOUND как ожидаемый код.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 03-crud.c ../common/common.c -lmdbx
// RU: Запуск: ./03-crud
// RU: Ожидаемый вывод:
//   insert duplicate -> MDBX_KEYEXIST as expected
//   after upsert: value2
//   update existing: value3
//   update absent -> MDBX_NOTFOUND as expected
//   erase(key) -> removed=true
//   get(after erase) -> MDBX_NOTFOUND as expected
//   ok: NOOVERWRITE/UPSERT/CURRENT; KEYEXIST/NOTFOUND handled explicitly
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-03-crud-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key = {0}, data = {0};
  int rc;

  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");

  key.iov_base = (void *)"key";
  key.iov_len = strlen("key");

  // EN: NOOVERWRITE: inserting a unique key.
  // RU: NOOVERWRITE: вставка уникального ключа.
  data.iov_base = (void *)"value1";
  data.iov_len = strlen("value1");
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
  check_rc(rc, "mdbx_put(NOOVERWRITE)");

  // EN: Re-inserting the same key -> MDBX_KEYEXIST.
  // RU: Повторная вставка того же ключа -> MDBX_KEYEXIST.
  data.iov_base = (void *)"value2";
  data.iov_len = strlen("value2");
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
  if (rc == MDBX_SUCCESS)
    die("FAIL: duplicate insert did not return MDBX_KEYEXIST");
  if (rc != MDBX_KEYEXIST)
    die("mdbx_put(NOOVERWRITE): unexpected code %d (%s)", rc, mdbx_strerror(rc));
  printf("insert duplicate -> MDBX_KEYEXIST as expected\n");

  // EN: UPSERT: insert or overwrite.
  // RU: UPSERT: вставить или перезаписать.
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_UPSERT);
  check_rc(rc, "mdbx_put(UPSERT)");
  rc = mdbx_get(txn, dbi, &key, &data);
  check_rc(rc, "mdbx_get");
  printf("after upsert: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);

  // EN: CURRENT (update): update only an existing key.
  // RU: CURRENT (update): обновить только существующий ключ.
  data.iov_base = (void *)"value3";
  data.iov_len = strlen("value3");
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_CURRENT);
  check_rc(rc, "mdbx_put(CURRENT)");
  rc = mdbx_get(txn, dbi, &key, &data);
  check_rc(rc, "mdbx_get");
  printf("update existing: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);

  // EN: CURRENT on an absent key -> MDBX_NOTFOUND.
  // RU: CURRENT на отсутствующем ключе -> MDBX_NOTFOUND.
  MDBX_val absent = {0};
  absent.iov_base = (void *)"absent";
  absent.iov_len = strlen("absent");
  rc = mdbx_put(txn, dbi, &absent, &data, MDBX_CURRENT);
  if (rc == MDBX_SUCCESS)
    die("FAIL: update of absent key did not fail");
  if (rc != MDBX_NOTFOUND)
    die("mdbx_put(CURRENT): unexpected code %d (%s)", rc, mdbx_strerror(rc));
  printf("update absent -> MDBX_NOTFOUND as expected\n");

  // EN: Deletion by key.
  // RU: Удаление по ключу.
  rc = mdbx_del(txn, dbi, &key, NULL);
  if (rc == MDBX_SUCCESS) {
    printf("erase(key) -> removed=true\n");
  } else if (rc == MDBX_NOTFOUND) {
    printf("erase(key) -> removed=false\n");
  } else {
    die("mdbx_del: %s", mdbx_strerror(rc));
  }

  // EN: get of a deleted key -> MDBX_NOTFOUND.
  // RU: get удалённого ключа -> MDBX_NOTFOUND.
  rc = mdbx_get(txn, dbi, &key, &data);
  if (rc == MDBX_SUCCESS)
    die("FAIL: get after erase did not fail");
  if (rc != MDBX_NOTFOUND)
    die("mdbx_get: unexpected code %d (%s)", rc, mdbx_strerror(rc));
  printf("get(after erase) -> MDBX_NOTFOUND as expected\n");

  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: NOOVERWRITE/UPSERT/CURRENT; KEYEXIST/NOTFOUND handled explicitly\n");
  return EXIT_SUCCESS;
}