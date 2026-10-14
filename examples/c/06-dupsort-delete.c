// EN: Safe-delete in DUPSORT: two-cursor pattern
// EN: Textbook section: Volume II, chapter 6, "Cursors", section "Safe-delete in DUPSORT" (§6.7).
// EN: What it demonstrates: the pattern of safely deleting values in a DUPSORT table
// EN:   with two cursors — one iterates (MDBX_NEXT_DUP), the second deletes the current pair.
// EN:   After mdbx_cursor_del() the cursor position is invalid, so iteration
// EN:   and deletion are separated onto different cursors.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 06-dupsort-delete.c ../common/common.c -lmdbx
// EN: Run: ./06-dupsort-delete
// EN: Expected output:
// RU: Безопасное удаление в DUPSORT: паттерн «два курсора»
// RU: Раздел учебника: Том II, глава 6, «Курсоры», раздел «Safe-delete в DUPSORT» (§6.7).
// RU: Что демонстрирует: паттерн безопасного удаления значений в DUPSORT-таблице
// RU:   двумя курсорами — один итерирует (MDBX_NEXT_DUP), второй удаляет текущую пару.
// RU:   После mdbx_cursor_del() позиция курсора недействительна, поэтому итерация
// RU:   и удаление разнесены на разные курсоры.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 06-dupsort-delete.c ../common/common.c -lmdbx
// RU: Запуск: ./06-dupsort-delete
// RU: Ожидаемый вывод:
//   values before delete (target): v1 v2 v3 v4
//   deleted 3 values of "target"
//   values after delete (target): v1
//   ok: safe-delete with two cursors
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

static void dump_key(MDBX_txn *txn, MDBX_dbi dbi, const char *label, const char *key_s) {
  MDBX_cursor *cur = NULL;
  MDBX_val key, data = {0};
  int rc = mdbx_cursor_open(txn, dbi, &cur);
  check_rc(rc, "mdbx_cursor_open");
  set_str(&key, key_s);
  printf("%s", label);
  rc = mdbx_cursor_get(cur, &key, &data, MDBX_SET);
  while (rc == MDBX_SUCCESS) {
    printf(" %.*s", (int)data.iov_len, (const char *)data.iov_base);
    rc = mdbx_cursor_get(cur, &key, &data, MDBX_NEXT_DUP);
  }
  if (rc != MDBX_NOTFOUND)
    die("mdbx_cursor_get(NEXT_DUP): %s", mdbx_strerror(rc));
  printf("\n");
  mdbx_cursor_close(cur);
}

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-06-dupsort-delete-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key, data = {0};
  int rc;

  static const char *keys[] = {"alpha", "target", "omega"};
  static const char *values[] = {"v1", "v2", "v3", "v4"};

  // EN: Filling the DUPSORT table.
  // RU: Заполнение DUPSORT-таблицы.
  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, "multi", MDBX_CREATE | MDBX_DUPSORT, &dbi);
  check_rc(rc, "mdbx_dbi_open(multi)");
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); ++j) {
      set_str(&key, keys[i]);
      set_str(&data, values[j]);
      rc = mdbx_put(txn, dbi, &key, &data, 0);
      check_rc(rc, "mdbx_put");
    }
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, "multi", MDBX_DUPSORT, &dbi);
  check_rc(rc, "mdbx_dbi_open(multi)");

  dump_key(txn, dbi, "values before delete (target):", "target");

  // EN: The "two cursors" pattern: `it` iterates over the key's duplicates,
  // EN: `del` deletes the current pair (the position of `it` stays valid meanwhile).
  // RU: Паттерн «два курсора»: `it` итерирует по дубликатам ключа,
  // RU: `del` удаляет текущую пару (позиция `it` при этом остаётся валидной).
  MDBX_cursor *it = NULL, *del = NULL;
  rc = mdbx_cursor_open(txn, dbi, &it);
  check_rc(rc, "mdbx_cursor_open(it)");
  rc = mdbx_cursor_open(txn, dbi, &del);
  check_rc(rc, "mdbx_cursor_open(del)");

  set_str(&key, "target");
  size_t deleted = 0;
  // EN: the first dup of the key — keep it
  // RU: первый dup ключа — сохраняем
  rc = mdbx_cursor_get(it, &key, &data, MDBX_SET);
  // EN: delete starting from the second one
  // RU: удаляем со второго
  if (rc == MDBX_SUCCESS)
    rc = mdbx_cursor_get(it, &key, &data, MDBX_NEXT_DUP);
  while (rc == MDBX_SUCCESS) {
    MDBX_val dk = key, dv = data;
    // EN: the second cursor on the same position
    // RU: второй курсор на ту же позицию
    rc = mdbx_cursor_get(del, &dk, &dv, MDBX_GET_BOTH);
    if (rc != MDBX_SUCCESS)
      break;
    // EN: delete only the current value
    // RU: удалить только текущее значение
    rc = mdbx_cursor_del(del, MDBX_CURRENT);
    if (rc != MDBX_SUCCESS)
      break;
    ++deleted;
    // EN: the next pair of the key
    // RU: следующая пара ключа
    rc = mdbx_cursor_get(it, &key, &data, MDBX_NEXT_DUP);
  }
  if (rc != MDBX_NOTFOUND)
    die("mdbx_cursor_get(NEXT_DUP)/del: %s", mdbx_strerror(rc));
  mdbx_cursor_close(it);
  mdbx_cursor_close(del);
  printf("deleted %zu values of \"target\"\n", deleted);

  dump_key(txn, dbi, "values after delete (target):", "target");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: safe-delete with two cursors\n");
  return EXIT_SUCCESS;
}