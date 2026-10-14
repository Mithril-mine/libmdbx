// EN: Tree height and page types via env_stat (C)
// EN: Textbook section: Volume III, chapter 13, "Storage architecture", §13.4 "Tree estimation".
// EN: What it demonstrates: mdbx_env_stat_ex(): leaf/branch/overflow pages, the
// EN:   B-tree height; writing a value larger than the page size creates overflow pages.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 18-tree-height.c ../common/common.c -lmdbx
// EN: Run: ./18-tree-height
// EN: Expected output (numbers depend on the data):
// RU: Высота дерева и типы страниц через env_stat (C)
// RU: Раздел учебника: Том III, глава 13, «Архитектура хранения», §13.4 «Оценка дерева».
// RU: Что демонстрирует: mdbx_env_stat_ex(): страницы leaf/branch/overflow, высоту
// RU:   B-дерева; запись значения больше размера страницы создаёт overflow-страницы.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 18-tree-height.c ../common/common.c -lmdbx
// RU: Запуск: ./18-tree-height
// RU: Ожидаемый вывод (числа зависят от данных):
//   stat: depth=<N> leaf=<N> branch=<N> overflow=<N> entries=5001
//   ok: tree height and page types observed; overflow value detected
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-18-tree-height-c.mdbx", tmpdir());
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
  for (int i = 0; i < 5000; ++i) {
    char kbuf[16];
    snprintf(kbuf, sizeof(kbuf), "k%d", i);
    key.iov_base = kbuf;
    key.iov_len = strlen(kbuf);
    data.iov_base = (void *)"v";
    data.iov_len = 1;
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    check_rc(rc, "mdbx_put");
  }
  // EN: A value larger than the page size (4096) goes to overflow pages.
  // RU: Значение больше размера страницы (4096) уходит на overflow-страницы.
  char *big = malloc(64 * 1024);
  if (!big)
    die("malloc");
  memset(big, 'x', 64 * 1024);
  key.iov_base = (void *)"big";
  key.iov_len = strlen("big");
  data.iov_base = big;
  data.iov_len = 64 * 1024;
  rc = mdbx_put(txn, dbi, &key, &data, 0);
  check_rc(rc, "mdbx_put(big)");
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  MDBX_stat stat;
  memset(&stat, 0, sizeof(stat));
  rc = mdbx_env_stat_ex(env, NULL, &stat, sizeof(stat));
  check_rc(rc, "mdbx_env_stat_ex");
  printf("stat: depth=%u leaf=%llu branch=%llu overflow=%llu entries=%llu\n", stat.ms_depth,
         (unsigned long long)stat.ms_leaf_pages, (unsigned long long)stat.ms_branch_pages,
         (unsigned long long)stat.ms_overflow_pages, (unsigned long long)stat.ms_entries);

  if (stat.ms_overflow_pages == 0)
    die("FAIL: expected overflow pages for a large value");
  if (stat.ms_entries < 5001)
    die("FAIL: unexpected entry count");

  free(big);
  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: tree height and page types observed; overflow value detected\n");
  return EXIT_SUCCESS;
}