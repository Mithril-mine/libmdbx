// EN: Hello, libmdbx — a minimal example
// EN: Textbook section: Volume I, chapter 2, "Installation and first launch", section "Minimal example".
// EN: What it demonstrates: the full cycle open (create) → put → get → del → close;
// EN:   the semantics of mode in mdbx_env_open() (0 — open only an existing DB)
// EN:   and the absence of MDBX_CREATE in the env flags (creation is controlled by the mode argument).
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 01-hello.c ../common/common.c -lmdbx
// EN: Run: ./01-hello
// EN: Expected output:
// RU: Hello, libmdbx — минимальный пример
// RU: Раздел учебника: Том I, глава 2, «Установка и первый запуск», раздел «Минимальный пример».
// RU: Что демонстрирует: полный цикл open (создание) → put → get → del → close;
// RU:   семантику mode у mdbx_env_open() (0 — открыть только существующую БД)
// RU:   и отсутствие MDBX_CREATE в env-флагах (создание управляется аргументом mode).
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 01-hello.c ../common/common.c -lmdbx
// RU: Запуск: ./01-hello
// RU: Ожидаемый вывод:
//   got: hello, libmdbx
//   reopen(mode=0) on existing db: ok
//   reopen(mode=0) on missing db: refused (ENOFILE)
//   ok: value = "hello, libmdbx"; deleted
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-01-hello-c.mdbx", tmpdir());
  // EN: wipe leftovers from the previous run
  // RU: стереть остатки предыдущего запуска
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = NULL;
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key = {0}, data = {0};
  int rc;

  // EN: Creation: mode != 0 allows creating a new database.
  // RU: Создание: mode != 0 разрешает создание новой БД.
  rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");
  mdbx_env_set_maxdbs(env, 32);
  rc = mdbx_env_open(env, path, MDBX_SAFE_NOSYNC, 0640);
  check_rc(rc, "mdbx_env_open");

  // EN: put through the main table.
  // RU: put через главную таблицу.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  // EN: NULL name — the main table
  // RU: имя NULL — главная таблица
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  key.iov_base = (void *)"key";
  key.iov_len = strlen("key");
  data.iov_base = (void *)"hello, libmdbx";
  data.iov_len = strlen("hello, libmdbx");
  rc = mdbx_put(txn, dbi, &key, &data, 0);
  check_rc(rc, "mdbx_put");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  // EN: get in a read-only transaction.
  // RU: get в read-only транзакции.
  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = mdbx_get(txn, dbi, &key, &data);
  check_rc(rc, "mdbx_get");
  printf("got: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  // EN: delete.
  // RU: delete.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = mdbx_del(txn, dbi, &key, NULL);
  check_rc(rc, "mdbx_del");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  mdbx_env_close(env);

  // EN: Reopening an existing DB: mode == 0 does not create a new one.
  // RU: Переоткрытие существующей БД: mode == 0 не создаёт новую.
  rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");
  mdbx_env_set_maxdbs(env, 32);
  rc = mdbx_env_open(env, path, MDBX_SAFE_NOSYNC, 0);
  check_rc(rc, "mdbx_env_open(mode=0)");
  printf("reopen(mode=0) on existing db: ok\n");
  mdbx_env_close(env);

  // EN: Opening a missing DB with mode == 0 must fail with an error.
  // RU: Открытие отсутствующей БД с mode == 0 обязано завершиться ошибкой.
  snprintf(path, sizeof(path), "%s/mdbx-01-hello.absent.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");
  mdbx_env_set_maxdbs(env, 32);
  rc = mdbx_env_open(env, path, MDBX_SAFE_NOSYNC, 0);
  if (rc == MDBX_SUCCESS) {
    mdbx_env_close(env);
    die("FAIL: unexpectedly opened a non-existent database");
  }
  printf("reopen(mode=0) on missing db: refused (%s)\n", mdbx_strerror(rc));
  mdbx_env_close(env);

  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: value = \"hello, libmdbx\"; deleted\n");
  return EXIT_SUCCESS;
}