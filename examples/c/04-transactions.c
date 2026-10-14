// EN: Transactions: begin/commit/abort, read-only transactions
// EN: Textbook section: Volume I, chapter 5, "Transactions — first steps",
// EN:   sections "begin/commit/abort", "read-only".
// EN: What it demonstrates: commit persists changes; abort rolls them back;
// EN:   read-only transactions see only committed data.
// EN:   Note: observing a stable snapshot (MVCC) on top of uncommitted
// EN:   and subsequent commits requires a separate thread (overlapping read+write
// EN:   transactions of one thread is forbidden: MDBX_TXN_OVERLAPPING) — see the C++ version.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 04-transactions.c ../common/common.c -lmdbx
// EN: Run: ./04-transactions
// EN: Expected output:
// RU: Транзакции: begin/commit/abort, read-only транзакции
// RU: Раздел учебника: Том I, глава 5, «Транзакции — первые шаги»,
// RU:   разделы «begin/commit/abort», «read-only».
// RU: Что демонстрирует: commit фиксирует изменения; abort откатывает их;
// RU:   read-only транзакции видят только закоммиченные данные.
// RU:   Примечание: наблюдение стабильного снапшота (MVCC) поверх незакоммиченных
// RU:   и последующих коммитов требует отдельного потока (пересечение read+write
// RU:   транзакций одного потока запрещено: MDBX_TXN_OVERLAPPING) — см. C++-версию.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 04-transactions.c ../common/common.c -lmdbx
// RU: Запуск: ./04-transactions
// RU: Ожидаемый вывод:
//   reader sees after v1 commit: v1
//   reader sees after abort of v2: v1
//   reader sees after v3 commit: v3
//   ok: commit persists, abort rolls back
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

static int put_str(MDBX_txn *txn, MDBX_dbi dbi, const char *key_s, const char *value_s) {
  MDBX_val key, data;
  set_str(&key, key_s);
  set_str(&data, value_s);
  return mdbx_put(txn, dbi, &key, &data, 0);
}

static int get_str(MDBX_txn *txn, MDBX_dbi dbi, const char *key_s, MDBX_val *data) {
  MDBX_val key;
  set_str(&key, key_s);
  return mdbx_get(txn, dbi, &key, data);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-04-transactions-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val data = {0};
  int rc;

  // EN: Transaction 1: writing v1 and commit.
  // RU: Транзакция 1: запись v1 и commit.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_str(txn, dbi, "key", "v1");
  check_rc(rc, "mdbx_put(v1)");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  // EN: A read-only transaction sees the committed v1.
  // RU: Read-only транзакция видит закоммиченное v1.
  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = get_str(txn, dbi, "key", &data);
  check_rc(rc, "mdbx_get");
  printf("reader sees after v1 commit: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  // EN: Transaction 2: overwriting with v2 and abort — the change is rolled back.
  // RU: Транзакция 2: перезапись v2 и abort — изменение откатывается.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_str(txn, dbi, "key", "v2");
  check_rc(rc, "mdbx_put(v2)");
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = get_str(txn, dbi, "key", &data);
  check_rc(rc, "mdbx_get");
  printf("reader sees after abort of v2: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  // EN: Transaction 3: overwriting with v3 and commit.
  // RU: Транзакция 3: перезапись v3 и commit.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = put_str(txn, dbi, "key", "v3");
  check_rc(rc, "mdbx_put(v3)");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  rc = get_str(txn, dbi, "key", &data);
  check_rc(rc, "mdbx_get");
  printf("reader sees after v3 commit: %.*s\n", (int)data.iov_len, (const char *)data.iov_base);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: commit persists, abort rolls back\n");
  return EXIT_SUCCESS;
}