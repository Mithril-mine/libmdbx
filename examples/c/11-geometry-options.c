// EN: Environment configuration: geometry and runtime options (C)
// EN: Textbook section: Volume II, chapter 9, "Environment configuration",
// EN:   sections "Geometry", "Runtime options".
// EN: What it demonstrates: mdbx_env_set_geometry(); setting and reading runtime options
// EN:   via mdbx_env_set_option()/mdbx_env_get_option().
// EN: Build: gcc -std=c11 -I<libmdbx-include> -I../common 11-geometry-options.c ../common/common.c -lmdbx
// EN: Run: ./11-geometry-options
// EN: Expected output (values may differ on other platforms):
// RU: Конфигурация окружения: геометрия и runtime-опции (C)
// RU: Раздел учебника: Том II, глава 9, «Конфигурация окружения»,
// RU:   разделы «Геометрия», «Runtime options».
// RU: Что демонстрирует: mdbx_env_set_geometry(); установку и чтение runtime-опций
// RU:   через mdbx_env_set_option()/mdbx_env_get_option().
// RU: Сборка: gcc -std=c11 -I<libmdbx-include> -I../common 11-geometry-options.c ../common/common.c -lmdbx
// RU: Запуск: ./11-geometry-options
// RU: Ожидаемый вывод (значения могут отличаться на других платформах):
//   merge_threshold=21845 (~33% of page)
//   writethrough_threshold=1048576
//   prefault_write_enable=1
//   ok: geometry and runtime options set/read
// EN: On Windows writethrough_threshold is not supported (§10.5): the value
// EN: is not set and the effective one is printed (2147483647).
// EN: On failure — message to stderr and exit 1.
// RU: На Windows writethrough_threshold не поддерживается (§10.5): значение
// RU: не устанавливается и печатается эффективное (2147483647).
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

int main(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/mdbx-11-geometry-options-c.mdbx", tmpdir());
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);

  MDBX_env *env = NULL;
  int rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");
  mdbx_env_set_maxdbs(env, 32);

  // EN: Geometry: lower/current/upper bounds of the DB size, growth step,
  // EN: shrink threshold; -1 (pagesize) — the default page size.
  // RU: Геометрия: нижняя/текущая/верхняя границы размера БД, шаг роста,
  // RU: порог сжатия; -1 (pagesize) — размер страницы по умолчанию.
  const size_t MB = 1000 * 1000;
  rc = mdbx_env_set_geometry(env, 8 * MB, 16 * MB, 64 * MB, 8 * MB, 2 * MB, 0);
  check_rc(rc, "mdbx_env_set_geometry");

  rc = mdbx_env_open(env, path, MDBX_SAFE_NOSYNC, 0640);
  check_rc(rc, "mdbx_env_open");

  // EN: Runtime options: set and read them back.
  // EN: MDBX_opt_writethrough_threshold is not supported on Windows (Volume II, §10.5):
  // EN: there set_option accepts only default/0/UINT_MAX, otherwise MDBX_EINVAL.
  // RU: Runtime-опции: устанавливаем и читаем обратно.
  // RU: MDBX_opt_writethrough_threshold не поддерживается на Windows (Том II, §10.5):
  // RU: там set_option принимает только default/0/UINT_MAX, иначе MDBX_EINVAL.
#if !defined(_WIN32)
  rc = mdbx_env_set_option(env, MDBX_opt_writethrough_threshold, 1 << 20);
  check_rc(rc, "mdbx_env_set_option(writethrough_threshold)");
#endif
  rc = mdbx_env_set_option(env, MDBX_opt_merge_threshold, 65536 / 3);
  check_rc(rc, "mdbx_env_set_option(merge_threshold)");
  rc = mdbx_env_set_option(env, MDBX_opt_sync_bytes, 256 * 1024);
  check_rc(rc, "mdbx_env_set_option(sync_bytes)");
  rc = mdbx_env_set_option(env, MDBX_opt_prefault_write_enable, 1);
  check_rc(rc, "mdbx_env_set_option(prefault_write_enable)");

  uint64_t value = 0;
  rc = mdbx_env_get_option(env, MDBX_opt_merge_threshold, &value);
  check_rc(rc, "mdbx_env_get_option(merge_threshold)");
  printf("merge_threshold=%llu (~%llu%% of page)\n", (unsigned long long)value,
         (unsigned long long)(value * 100 / 65536));
  rc = mdbx_env_get_option(env, MDBX_opt_writethrough_threshold, &value);
  check_rc(rc, "mdbx_env_get_option(writethrough_threshold)");
  printf("writethrough_threshold=%llu\n", (unsigned long long)value);
  rc = mdbx_env_get_option(env, MDBX_opt_prefault_write_enable, &value);
  check_rc(rc, "mdbx_env_get_option(prefault_write_enable)");
  printf("prefault_write_enable=%llu\n", (unsigned long long)value);

  mdbx_env_close(env);
  mdbx_env_delete(path, MDBX_ENV_JUST_DELETE);
  printf("ok: geometry and runtime options set/read\n");
  return EXIT_SUCCESS;
}