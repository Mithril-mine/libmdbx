// EN: Common helpers for the C examples of the libmdbx textbook.
// RU: Общие хелперы для C-примеров учебника libmdbx.
// SPDX-License-Identifier: Apache-2.0

#include "common.h"

#include <stdarg.h>
#include <string.h>

void check_rc(int rc, const char *what) {
  if (rc != MDBX_SUCCESS) {
    fprintf(stderr, "%s: %s\n", what, mdbx_strerror(rc));
    exit(EXIT_FAILURE);
  }
}

void die(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

MDBX_env *ex_env_open(const char *path, MDBX_env_flags_t flags) {
  MDBX_env *env = NULL;
  int rc = mdbx_env_create(&env);
  check_rc(rc, "mdbx_env_create");

  // EN: Enough named tables for all textbook examples.
  // RU: Достаточно именованных таблиц для всех примеров учебника.
  mdbx_env_set_maxdbs(env, 32);

  rc = mdbx_env_open(env, path, flags, 0640);
  if (rc != MDBX_SUCCESS) {
    mdbx_env_close(env);
    die("mdbx_env_open(%s): %s", path, mdbx_strerror(rc));
  }
  return env;
}

const char *tmpdir(void) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996) /* getenv: This function or variable may be unsafe */
#endif
  const char *dir = getenv("TMPDIR");
#if defined(_WIN32) || defined(_WIN64)
  if (!dir || !*dir)
    dir = getenv("TEMP");
  if (!dir || !*dir)
    dir = ".";
#else
  if (!dir || !*dir)
    dir = "/tmp";
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  return dir;
}