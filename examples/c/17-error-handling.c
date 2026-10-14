// EN: Error handling: parsing return codes, retry strategy (C)
// EN: Textbook section: Volume II, chapter 12, "Error handling",
// EN:   sections "Codes", "Retry strategies".
// EN: What it demonstrates: expected codes MDBX_KEYEXIST/MDBX_NOTFOUND are handled
// EN:   explicitly; positive/negative codes and MDBX_RESULT_TRUE/MDBX_RESULT_FALSE;
// EN:   the retry pattern on MDBX_BUSY.
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 17-error-handling.c ../common/common.c -lmdbx
// EN: Run: ./17-error-handling
// EN: Expected output:
// RU: Обработка ошибок: разбор кодов возврата, retry-стратегии (C)
// RU: Раздел учебника: Том II, глава 12, «Обработка ошибок»,
// RU:   разделы «Коды», «Retry-стратегии».
// RU: Что демонстрирует: ожидаемые коды MDBX_KEYEXIST/MDBX_NOTFOUND обрабатываются
// RU:   явно; положительные/отрицательные коды и MDBX_RESULT_TRUE/MDBX_RESULT_FALSE;
// RU:   шаблон retry на MDBX_BUSY.
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 17-error-handling.c ../common/common.c -lmdbx
// RU: Запуск: ./17-error-handling
// RU: Ожидаемый вывод:
//   key_exists: code=<N> (Key/data pair already exists)
//   not_found: code=<N> (No matching key/data pair found)
//   busy attempt 1: failed (code <N>)
//   busy: persists after 3 attempts (expected while env is open)
//   ok: expected codes parsed, retry pattern shown
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-17-error-handling-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = ex_env_open(path, MDBX_SAFE_NOSYNC);
  MDBX_txn *txn = NULL;
  MDBX_dbi dbi;
  MDBX_val key, data = {0};
  int rc;

  rc = mdbx_txn_begin(env, NULL, 0, &txn);
  check_rc(rc, "mdbx_txn_begin");
  rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
  check_rc(rc, "mdbx_dbi_open");
  key.iov_base = (void *)"key";
  key.iov_len = strlen("key");

  // EN: MDBX_KEYEXIST — the expected code when inserting an existing key.
  // RU: MDBX_KEYEXIST — ожидаемый код при вставке существующего ключа.
  data.iov_base = (void *)"v1";
  data.iov_len = strlen("v1");
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
  check_rc(rc, "mdbx_put(NOOVERWRITE)");
  rc = mdbx_put(txn, dbi, &key, &data, MDBX_NOOVERWRITE);
  if (rc == MDBX_SUCCESS)
    die("FAIL: duplicate insert did not fail");
  if (rc != MDBX_KEYEXIST)
    die("mdbx_put(NOOVERWRITE): unexpected code %d (%s)", rc, mdbx_strerror(rc));
  printf("key_exists: code=%d (%s)\n", rc, mdbx_strerror(rc));

  // EN: MDBX_NOTFOUND — the expected code when reading an absent key.
  // RU: MDBX_NOTFOUND — ожидаемый код при чтении отсутствующего ключа.
  key.iov_base = (void *)"absent";
  key.iov_len = strlen("absent");
  rc = mdbx_get(txn, dbi, &key, &data);
  if (rc == MDBX_SUCCESS)
    die("FAIL: get of absent key did not fail");
  if (rc != MDBX_NOTFOUND)
    die("mdbx_get: unexpected code %d (%s)", rc, mdbx_strerror(rc));
  printf("not_found: code=%d (%s)\n", rc, mdbx_strerror(rc));
  rc = mdbx_txn_commit(txn);
  check_rc(rc, "mdbx_txn_commit");

  // EN: Retry strategy: trying to open the environment in exclusive mode while
  // EN: it is already open by the current process gives "busyness" (MDBX_BUSY under
  // EN: process contention or the system error EAGAIN within the process).
  // EN: Such a failure is caught and handled by a retry with a limit.
  // RU: Retry-стратегия: попытка открыть окружение в исключительном режиме, пока
  // RU: оно уже открыто текущим процессом, даёт «занятость» (MDBX_BUSY при
  // RU: конкуренции процессов или системную ошибку EAGAIN внутри процесса).
  // RU: Такой отказ перехватывается и обрабатывается повтором с ограничением.
  const int max_attempts = 3;
  int attempt = 0;
  MDBX_env *second = NULL;
  for (; attempt < max_attempts; ++attempt) {
    rc = mdbx_env_create(&second);
    check_rc(rc, "mdbx_env_create");
    rc = mdbx_env_open(second, path, MDBX_EXCLUSIVE, 0);
    if (rc == MDBX_SUCCESS) {
      mdbx_env_close(second);
      die("FAIL: exclusive open unexpectedly succeeded");
    }
    mdbx_env_close(second);
    printf("busy attempt %d: failed (code %d)\n", attempt + 1, rc);
  }
  printf("busy: persists after %d attempts (expected while env is open)\n", attempt);

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: expected codes parsed, retry pattern shown\n");
  return EXIT_SUCCESS;
}