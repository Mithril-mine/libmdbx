// EN: Environment statistics: env_stat / env_info (C)
// EN: Textbook section: Volume II, chapter 9, "Environment configuration", section "env_stat".
// EN: What it demonstrates: mdbx_env_stat_ex()/mdbx_env_info_ex(): pages by type
// EN:   (leaf/branch/overflow), the B-tree height, entry count, geometry size,
// EN:   txnid and the "trailing" reader.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 12-env-stat.c ../common/common.c -lmdbx
// EN: Run: ./12-env-stat
// EN: Expected output (values depend on the amount of data):
// RU: Статистика окружения: env_stat / env_info (C)
// RU: Раздел учебника: Том II, глава 9, «Конфигурация окружения», раздел «env_stat».
// RU: Что демонстрирует: mdbx_env_stat_ex()/mdbx_env_info_ex(): страницы по типам
// RU:   (leaf/branch/overflow), высоту B-дерева, число записей, размер геометрии,
// RU:   txnid и «хвостового» читателя.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 12-env-stat.c ../common/common.c -lmdbx
// RU: Запуск: ./12-env-stat
// RU: Ожидаемый вывод (значения зависят от объёма данных):
//   stat: ps=4096 depth=2 leaf=1 branch=1 overflow=0 entries=100
//   info: recent_txnid=<N> latter_reader_txnid=<M>
//   ok: env_stat/env_info read
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-12-env-stat-c.mdbx", tmpdir());
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
  for (int i = 0; i < 100; ++i) {
    char kbuf[16], vbuf[32];
    snprintf(kbuf, sizeof(kbuf), "%d", i);
    snprintf(vbuf, sizeof(vbuf), "value-%d", i);
    key.iov_base = kbuf;
    key.iov_len = strlen(kbuf);
    data.iov_base = vbuf;
    data.iov_len = strlen(vbuf);
    rc = mdbx_put(txn, dbi, &key, &data, 0);
    check_rc(rc, "mdbx_put");
  }
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  MDBX_stat stat;
  memset(&stat, 0, sizeof(stat));
  rc = mdbx_env_stat_ex(env, NULL, &stat, sizeof(stat));
  check_rc(rc, "mdbx_env_stat_ex");
  printf("stat: ps=%u depth=%u leaf=%llu branch=%llu overflow=%llu entries=%llu\n", stat.ms_psize, stat.ms_depth,
         (unsigned long long)stat.ms_leaf_pages, (unsigned long long)stat.ms_branch_pages,
         (unsigned long long)stat.ms_overflow_pages, (unsigned long long)stat.ms_entries);

  MDBX_envinfo info;
  memset(&info, 0, sizeof(info));
  rc = mdbx_env_info_ex(env, NULL, &info, sizeof(info));
  check_rc(rc, "mdbx_env_info_ex");
  printf("info: recent_txnid=%" PRIu64 " latter_reader_txnid=%" PRIu64 " geo.current=%zu\n",
         info.mi_recent_txnid, info.mi_latter_reader_txnid, (size_t)info.mi_geo.current);

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: env_stat/env_info read\n");
  return EXIT_SUCCESS;
}