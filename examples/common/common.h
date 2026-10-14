// EN: Common helpers for the C examples of the libmdbx textbook.
// EN: Textbook section: used by all C examples.
// EN: Exit-code policy: on error — diagnostic message to stderr and exit with EXIT_FAILURE.
// RU: Общие хелперы для C-примеров учебника libmdbx.
// RU: Раздел учебника: используется всеми примерами на C.
// RU: Политика exit-кодов: при ошибке — диагностика в stderr и завершение с EXIT_FAILURE.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MDBX_EXAMPLES_COMMON_C_H_
#define MDBX_EXAMPLES_COMMON_C_H_

#include <stdio.h>
#include <stdlib.h>

#include "mdbx.h"

// EN: Checks an API return code; on error prints a diagnostic to stderr and exits the process.
// RU: Проверяет код возврата API; при ошибке печатает диагностику в stderr и завершает процесс.
void check_rc(int rc, const char *what);

// EN: Prints a formatted message to stderr and exits the process with EXIT_FAILURE.
// RU: Печатает форматированное сообщение в stderr и завершает процесс с EXIT_FAILURE.
void die(const char *fmt, ...);

// EN: Opens (creating if needed) the environment at path with the given flags.
// EN: Wraps mdbx_env_create()/mdbx_env_open(); on error closes the environment and exits the process.
// RU: Открывает (создавая при необходимости) окружение по пути path с флагами flags.
// RU: Оборачивает mdbx_env_create()/mdbx_env_open(); при ошибке закрывает окружение и завершает процесс.
MDBX_env *ex_env_open(const char *path, MDBX_env_flags_t flags);

// EN: Directory for temporary DBs: $TMPDIR (on Windows — %TEMP%) or /tmp.
// RU: Каталог для временных БД: $TMPDIR (на Windows — %TEMP%) или /tmp.
const char *tmpdir(void);

#endif /* MDBX_EXAMPLES_COMMON_C_H_ */