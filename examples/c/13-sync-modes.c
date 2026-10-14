// EN: Durability modes: sync flags, sync_period/sync_bytes (C)
// EN: Textbook section: Volume II, chapter 10, "Durability modes",
// EN:   sections "Synchronization flags", "syncbytes/syncperiod".
// EN: What it demonstrates: toggling the sync flag on the fly via mdbx_env_set_flags();
// EN:   explicit flush to disk via mdbx_env_sync_ex(); setting sync_bytes/sync_period.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 13-sync-modes.c ../common/common.c -lmdbx
// EN: Run: ./13-sync-modes
// EN: Expected output:
// RU: Режимы долговечности: sync-флаги, sync_period/sync_bytes (C)
// RU: Раздел учебника: Том II, глава 10, «Режимы долговечности»,
// RU:   разделы «Флаги синхронизации», «syncbytes/syncperiod».
// RU: Что демонстрирует: смену sync-флага на лету через mdbx_env_set_flags();
// RU:   явный сброс на диск через mdbx_env_sync_ex(); установку sync_bytes/sync_period.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 13-sync-modes.c ../common/common.c -lmdbx
// RU: Запуск: ./13-sync-modes
// RU: Ожидаемый вывод:
//   durable put: ok
//   safe_nosync put: ok
//   durable put again: ok
//   ok: sync flags toggled at runtime; sync_ex works
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

static int put_key_value(MDBX_txn *txn, MDBX_dbi dbi, const char *key_s, const char *value_s) {
  MDBX_val key, data;
  key.iov_base = (void *)key_s;
  key.iov_len = strlen(key_s);
  data.iov_base = (void *)value_s;
  data.iov_len = strlen(value_s);
  return mdbx_put(txn, dbi, &key, &data, 0);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-13-sync-modes-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = NULL;
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  int rc;

  // EN: Opening in full-durability mode (the default).
  // RU: Открытие в режиме полной долговечности (по умолчанию).
  rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");
  mdbx_env_set_maxdbs(env, 32);
  rc = mdbx_env_open(env, path, 0 /* MDBX_SYNC_DURABLE */, 0640);
  check_rc(rc, "mdbx_env_open");

  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_key_value(txn, dbi, "key", "v1");
  check_rc(rc, "mdbx_put");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");
  printf("durable put: ok\n");

  // EN: Switching to SAFE_NOSYNC on the fly.
  // RU: Переключение в SAFE_NOSYNC на лету.
  rc = mdbx_env_set_flags(env, MDBX_SAFE_NOSYNC, true);
  check_rc(rc, "mdbx_env_set_flags(SAFE_NOSYNC, on)");
  mdbx_env_set_option(env, MDBX_opt_sync_bytes, 64 * 1024);
  mdbx_env_set_option(env, MDBX_opt_sync_period, (uint64_t)(1 * 65536));
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_key_value(txn, dbi, "key", "v2");
  check_rc(rc, "mdbx_put");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");
  printf("safe_nosync put: ok\n");

  // EN: Explicit flush to disk and return to full durability.
  // RU: Явный сброс на диск и возврат к полной долговечности.
  rc = mdbx_env_sync_ex(env, true, false);
  check_rc(rc, "mdbx_env_sync_ex");
  rc = mdbx_env_set_flags(env, MDBX_SAFE_NOSYNC, false);
  check_rc(rc, "mdbx_env_set_flags(SAFE_NOSYNC, off)");
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_key_value(txn, dbi, "key", "v3");
  check_rc(rc, "mdbx_put");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");
  printf("durable put again: ok\n");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: sync flags toggled at runtime; sync_ex works\n");
  return EXIT_SUCCESS;
}