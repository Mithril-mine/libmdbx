// EN: Cursors: FIRST/NEXT, SET_RANGE, LAST/PREV
// EN: Textbook section: Volume II, chapter 6, "Cursors",
// EN:   sections "Positioning", "Batch operations".
// EN: What it demonstrates: forward iteration (MDBX_FIRST/MDBX_NEXT) and backward
// EN:   (MDBX_LAST/MDBX_PREV); the "nearest greater key" search (MDBX_SET_RANGE).
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 05-cursors.c ../common/common.c -lmdbx
// EN: Run: ./05-cursors
// EN: Expected output:
// RU: Курсоры: FIRST/NEXT, SET_RANGE, LAST/PREV
// RU: Раздел учебника: Том II, глава 6, «Курсоры»,
// RU:   разделы «Позиционирование», «Пакетные операции».
// RU: Что демонстрирует: итерацию вперёд (MDBX_FIRST/MDBX_NEXT) и назад
// RU:   (MDBX_LAST/MDBX_PREV); поиск «ближайшего большего ключа» (MDBX_SET_RANGE).
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 05-cursors.c ../common/common.c -lmdbx
// RU: Запуск: ./05-cursors
// RU: Ожидаемый вывод:
//   forward (10): k0 k1 k2 k3 k4 k5 k6 k7 k8 k9
//   set_range(k5) -> k5
//   set_range(k5x) -> k6
//   backward (10): k9 k8 k7 k6 k5 k4 k3 k2 k1 k0
//   ok: FIRST/NEXT, SET_RANGE, LAST/PREV iterated
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

static void set_key(char *buf, size_t bufsize, const char *prefix, int i) {
  snprintf(buf, bufsize, "%s%d", prefix, i);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-05-cursors-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key = {0}, data = {0};
  int rc;

  // EN: Filling the table with keys k0..k9.
  // RU: Заполнение таблицы ключами k0..k9.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  for (int i = 0; i < 10; ++i) {
    char kbuf[16], vbuf[16];
    set_key(kbuf, sizeof(kbuf), "k", i);
    set_key(vbuf, sizeof(vbuf), "v", i);
    set_str(&key, kbuf);
    set_str(&data, vbuf);
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    check_rc(rc, "mdbx_put");
  }
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
  check_rc(rc, "mdbx_txn_begin(readonly)");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  MDBX_cursor *cur = NULL;
  rc = mdbx_cursor_open(txn, dbi, &cur);
  check_rc(rc, "mdbx_cursor_open");

  // EN: Forward iteration: FIRST then NEXT until the end (the end is MDBX_NOTFOUND).
  // RU: Прямая итерация: FIRST затем NEXT до конца (конец — MDBX_NOTFOUND).
  size_t forward_count = 0;
  printf("forward:");
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_FIRST);
  while (rc == MDBX_SUCCESS) {
    printf(" %.*s", (int)key.iov_len, (const char *)key.iov_base);
    ++forward_count;
    rc = mdbx_cursor_get(cur, &key, &data, MDBX_NEXT);
  }
  if (rc != MDBX_NOTFOUND)
    die("mdbx_cursor_get(NEXT): %s", mdbx_strerror(rc));
  printf(" (%zu)\n", forward_count);

  // EN: SET_RANGE: the first key not less than the given one.
  // RU: SET_RANGE: первый ключ, не меньший заданного.
  set_str(&key, "k5");
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_SET_RANGE);
  if (rc == MDBX_SUCCESS)
    printf("set_range(k5) -> %.*s\n", (int)key.iov_len, (const char *)key.iov_base);
  else
    die("mdbx_cursor_get(SET_RANGE k5): %s", mdbx_strerror(rc));

  set_str(&key, "k5x");
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_SET_RANGE);
  if (rc == MDBX_SUCCESS)
    printf("set_range(k5x) -> %.*s\n", (int)key.iov_len, (const char *)key.iov_base);
  else
    die("mdbx_cursor_get(SET_RANGE k5x): %s", mdbx_strerror(rc));

  // EN: Backward iteration: LAST then PREV until the beginning.
  // RU: Обратная итерация: LAST затем PREV до начала.
  size_t backward_count = 0;
  printf("backward:");
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_LAST);
  while (rc == MDBX_SUCCESS) {
    printf(" %.*s", (int)key.iov_len, (const char *)key.iov_base);
    ++backward_count;
    rc = mdbx_cursor_get(cur, &key, &data, MDBX_PREV);
  }
  if (rc != MDBX_NOTFOUND)
    die("mdbx_cursor_get(PREV): %s", mdbx_strerror(rc));
  printf(" (%zu)\n", backward_count);

  mdbx_cursor_close(cur);
  rc = mdbx_txn_abort(txn);
  check_rc(rc, "mdbx_txn_abort");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: FIRST/NEXT, SET_RANGE, LAST/PREV iterated\n");
  return EXIT_SUCCESS;
}