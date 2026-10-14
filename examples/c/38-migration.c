// EN: Migration from LMDB: mdb_* → mdbx_* mapping
// EN: Textbook section: Volume V, chapter 31, "Migration from LMDB", §31.4 "Correspondence table".
// RU: Миграция с LMDB: отображение mdb_* → mdbx_*
// RU: Раздел учебника: Том V, глава 31, «Миграция с LMDB», §31.4 «Таблица соответствия».
//
// EN: Map of macros/names when porting code from LMDB (full table — in §31.4):
// RU: Карта макросов/имён при переносе кода с LMDB (полная таблица — в §31.4):
// EN: Note: mdb_env_set_mapsize → mdbx_env_set_geometry is not a plain rename, but a different API.
// RU: Примечание: mdb_env_set_mapsize → mdbx_env_set_geometry — не просто переименование, а другой API.
//   mdb_env_create      → mdbx_env_create
//   mdb_env_open        → mdbx_env_open
//   mdb_env_close       → mdbx_env_close
//   mdb_env_set_mapsize → mdbx_env_set_geometry
//   mdb_txn_begin       → mdbx_txn_begin
//   mdb_txn_commit      → mdbx_txn_commit
//   mdb_txn_abort       → mdbx_txn_abort
//   mdb_dbi_open        → mdbx_dbi_open
//   mdb_get/put/del     → mdbx_get/mdbx_put/mdbx_del
//   mdb_cursor_open/get → mdbx_cursor_open/mdbx_cursor_get
//   MDB_NOTLS           → MDBX_NOSTICKYTHREADS
// EN: Note: instead of MDB_NOSYNC use MDBX_SAFE_NOSYNC or MDBX_UTTERLY_NOSYNC.
// RU: Примечание: вместо MDB_NOSYNC используйте MDBX_SAFE_NOSYNC или MDBX_UTTERLY_NOSYNC.
//   MDB_NOSYNC          → MDBX_SAFE_NOSYNC / MDBX_UTTERLY_NOSYNC
//   MDB_APPEND          → MDBX_APPEND
//   MDB_INTEGERKEY      → MDBX_INTEGERKEY
//   mdb_strerror        → mdbx_strerror
//
// EN: What it demonstrates: correctly opening the environment without the anti-pattern
// EN:   of MDBX_CREATE in the env flags (creation is controlled by the mode argument), as
// EN:   is required after migration.
// EN: Build with sanitizers:
// RU: Что демонстрирует: корректное открытие окружения без антипаттерна
// RU:   MDBX_CREATE в env-флагах (создание управляется аргументом mode), как это
// RU:   требуется после миграции.
// RU: Сборка с санитайзерами:
//   gcc -std=c11 -fsanitize=address,undefined -I<libmdbx-include> -I../common
//       38-migration.c ../common/common.c -lmdbx -o 38-migration
// EN: Run: ./38-migration
// EN: Expected output:
// RU: Запуск: ./38-migration
// RU: Ожидаемый вывод:
//   ok: value = migrated
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-38-migration-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  // EN: No MDBX_CREATE in the env flags: creation is set by the mode argument.
  // RU: Никакого MDBX_CREATE в env-флагах: создание задаётся аргументом mode.
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
  data.iov_base = (void *)"migrated";
  data.iov_len = strlen("migrated");
  rc = mdbx_put(txn, dbi, &key, &data, 0);
  check_rc(rc, "mdbx_put");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = mdbx_get(txn, dbi, &key, &data);
  check_rc(rc, "mdbx_get");
  printf("ok: value = %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  return EXIT_SUCCESS;
}