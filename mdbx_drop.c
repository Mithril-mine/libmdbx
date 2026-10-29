/* This file is part of the libmdbx amalgamated source code (v0.15.0-334-gb8f2dad5 at 2026-10-29T00:17:01+03:00).
 *
 * libmdbx (aka MDBX) is an extremely fast, compact, powerful, embeddedable, transactional key-value storage engine with
 * open-source code. MDBX has a specific set of properties and capabilities, focused on creating unique lightweight
 * solutions.  Please visit https://libmdbx.dqdkfa.ru for more information, changelog, documentation, C++ API description
 * and links to the original git repo with the source code.  Questions, feedback and suggestions are welcome to the
 * Telegram' group https://t.me/libmdbx, MAX' chat https://max.ru/join/dKckvyuARxp1vRK-wnPur8zYCEkbR3OUOmpPWkWxp78.
 *
 * The libmdbx code will forever remain open and with high-quality free support, as far as the life circumstances of the
 * project participants allow. Donations are welcome to ETH `0xD104d8f8B2dC312aaD74899F83EBf3EEBDC1EA3A`,
 * BTC `bc1qzvl9uegf2ea6cwlytnanrscyv8snwsvrc0xfsu`, SOL `FTCTgbHajoLVZGr8aEFWMzx3NDMyS5wXJgfeMTmJznRi`.
 * Всё будет хорошо!
 *
 * For ease of use and to eliminate potential limitations in both distribution and obstacles in technology development,
 * libmdbx is distributed as an amalgamated source code starting at the end of 2025.  The source code of the tests, as
 * well as the internal documentation, will be available only to the team directly involved in the development.
 *
 * Copyright 2015-2026 Леонид Юрьев aka Leonid Yuriev <leo@yuriev.ru>
 * SPDX-License-Identifier: Apache-2.0
 *
 * For notes about the license change, credits and acknowledgments, please refer to the COPYRIGHT file. */

/* clang-format off */

#define xMDBX_TOOLS /* Avoid using internal ASSERT(), etc */
#include "mdbx-internals.h"

#ifndef MDBX_TOOLS_COMMON_H
#define MDBX_TOOLS_COMMON_H

#include <signal.h>
#include <stdio.h>
#include <string.h>

#if IS_WINDOWS

/* Bit of madness for Windows console */
#ifndef mdbx_strerror
#define mdbx_strerror mdbx_strerror_ANSI2OEM
#define mdbx_strerror_r mdbx_strerror_r_ANSI2OEM
#endif

static volatile BOOL user_break;
static BOOL WINAPI ConsoleBreakHandlerRoutine(DWORD dwCtrlType) {
  (void)dwCtrlType;
  user_break = user_break ? 2 : 1;
  return true;
}

static inline void tool_break_setup(void) { SetConsoleCtrlHandler(ConsoleBreakHandlerRoutine, true); }

#else /* !IS_WINDOWS */

static volatile sig_atomic_t user_break;
static void signal_handler(int sig) {
  (void)sig;
  user_break = user_break ? 2 : 1;
}

static inline void tool_break_setup(void) {
#ifdef SIGPIPE
  signal(SIGPIPE, signal_handler);
#endif
#ifdef SIGHUP
  signal(SIGHUP, signal_handler);
#endif
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
}

#endif /* IS_WINDOWS */

/* Returns true if `-h` or `--help` was requested anywhere in the command
 * line, while honoring the options that consume an argument (per `optstring`),
 * so e.g. `-s --help` is NOT treated as a help request. */
static inline bool tool_help_requested(int argc, char *const argv[], const char *optstring) {
  bool consume_next = false;
  for (int i = 1; i < argc; ++i) {
    if (consume_next) {
      consume_next = false;
      continue;
    }
    const char *const arg = argv[i];
    if (arg[0] != '-' || arg[1] == '\0')
      continue; /* positional argument */
    if (strcmp(arg, "--") == 0)
      break; /* end of options */
    if (strcmp(arg, "--help") == 0)
      return true;
    if (arg[1] == '-')
      continue; /* some other long option, let getopt handle it */
    for (size_t j = 1; arg[j] != '\0'; ++j) {
      const char opt = arg[j];
      if (opt == 'h')
        return true;
      const char *const optdef = strchr(optstring, opt);
      if (!optdef)
        break; /* unknown option, getopt will report it */
      if (optdef[1] == ':') {
        if (arg[j + 1] != '\0')
          break; /* the rest of the cluster is the option argument */
        consume_next = true;
        break;
      }
    }
  }
  return false;
}

/* Whether a name contains control characters (a table name may contain any
 * byte except NUL; when emitted raw into a dump stream it could break the
 * format or reroute data, and when printed to a terminal it could leak
 * escape sequences). */
MDBX_MAYBE_UNUSED static inline bool tool_name_has_control(const char *name) {
  for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
    if (*p < 0x20 || *p == 0x7f)
      return true;
  return false;
}

MDBX_MAYBE_UNUSED static inline bool tool_data_has_control(const void *data, size_t size) {
  const unsigned char *p = (const unsigned char *)data;
  for (size_t i = 0; i < size; ++i)
    if (p[i] < 0x20 || p[i] == 0x7f)
      return true;
  return false;
}

/* Print a name replacing control characters with '?', so that no control or
 * escape sequences leak into a terminal or into a dump stream. */
MDBX_MAYBE_UNUSED static inline void tool_print_sanitized(const void *data, size_t size) {
  const unsigned char *p = (const unsigned char *)data;
  for (size_t i = 0; i < size; ++i)
    putchar((p[i] < 0x20 || p[i] == 0x7f) ? '?' : p[i]);
}

/* Uniform version banner (as printed by the `-V` option of each tool). */
static inline void tool_version_print(const char *tool) {
  printf("mdbx_%s version %d.%d.%d.%d\n"
         " - source: %s %s, commit %s, tree %s\n"
         " - anchor: %s\n"
         " - build: %s for %s by %s\n"
         " - flags: %s\n"
         " - options: %s\n",
         tool, mdbx_version.major, mdbx_version.minor, mdbx_version.patch, mdbx_version.tweak,
         mdbx_version.git.describe, mdbx_version.git.datetime, mdbx_version.git.commit, mdbx_version.git.tree,
         mdbx_sourcery_anchor, mdbx_build.datetime, mdbx_build.target, mdbx_build.compiler, mdbx_build.flags,
         mdbx_build.options);
}

/* Default logger shared by the tools that do not need custom formatting. */
MDBX_MAYBE_UNUSED static inline void tool_logger_default(MDBX_log_level_t level, const char *function, int line,
                                                         const char *fmt, va_list args) {
  static const char *const prefixes[] = {
      "!!!fatal: ", // 0 fatal
      " ! ",        // 1 error
      " ~ ",        // 2 warning
      "   ",        // 3 notice
      "   //",      // 4 verbose
  };
  if (level >= 0 && level < MDBX_LOG_DEBUG) {
    if (function && line && (size_t)level < ARRAY_LENGTH(prefixes))
      fprintf(stderr, "%s", prefixes[level]);
    vfprintf(stderr, fmt, args);
  }
}

#endif /* MDBX_TOOLS_COMMON_H */
#include <ctype.h>

#if IS_WINDOWS

/* Bit of madness for Windows console */

#include "mdbx-wingetopt.h"

#endif /* WINDOWS */

static char *prog;
static bool quiet = false;
static void usage_msg(FILE *out) {
  fprintf(out,
          "usage: %s [-V] [-q] [-d] [-s name] [-h|--help] dbpath\n"
          "  -V\t\tprint version and exit\n"
          "  -q\t\tbe quiet\n"
          "  -d\t\tdelete the specified database, don't just empty it\n"
          "  -s name\tdrop the specified named table\n"
          "  \t\tby default empty the main DB\n"
          "  -h|--help\tprint this usage message and exit\n",
          prog);
}

static void usage(void) {
  usage_msg(stderr);
  exit(EXIT_FAILURE);
}

static void help(void) {
  usage_msg(stdout);
  exit(EXIT_SUCCESS);
}

static void error(const char *func, int rc) {
  if (!quiet)
    fprintf(stderr, "%s: %s() error %d %s\n", prog, func, rc, mdbx_strerror(rc));
}

int main(int argc, char *argv[]) {
  int i, rc;
  MDBX_env *env;
  MDBX_txn *txn;
  MDBX_dbi dbi;
  char *envname = nullptr;
  char *subname = nullptr;
  int envflags = MDBX_ACCEDE;
  bool delete = false;

  prog = argv[0];
  if (argc < 2)
    usage();

  if (tool_help_requested(argc, argv, "ds:nqVh"))
    help();

  while ((i = getopt(argc, argv,
                     "d"
                     "s:"
                     "n"
                     "q"
                     "V"
                     "h")) != EOF) {
    switch (i) {
    case 'h':
      help();
      break;
    case 'V':
      tool_version_print("drop");
      return EXIT_SUCCESS;
    case 'q':
      quiet = true;
      break;
    case 'd':
      delete = true;
      break;
    case 'n':
      break;
    case 's':
      subname = optarg;
      break;
    default:
      usage();
    }
  }

  if (optind != argc - 1)
    usage();

  tool_break_setup();

  envname = argv[optind];
  if (!quiet) {
    printf("mdbx_drop %s (%s, T-%s)\nRunning for %s/%s...\n", mdbx_version.git.describe, mdbx_version.git.datetime,
           mdbx_version.git.tree, envname, subname ? subname : "@MAIN");
    fflush(nullptr);
    mdbx_setup_debug(MDBX_LOG_NOTICE, MDBX_DBG_DONTCHANGE, tool_logger_default);
  }

  rc = mdbx_env_create(&env);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_env_create", rc);
    return EXIT_FAILURE;
  }

  if (subname) {
    rc = mdbx_env_set_maxdbs(env, 2);
    if (unlikely(rc != MDBX_SUCCESS)) {
      error("mdbx_env_set_maxdbs", rc);
      goto env_close;
    }
  }

  rc = mdbx_env_open(env, envname, envflags, 0);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_env_open", rc);
    goto env_close;
  }

  rc = mdbx_txn_begin(env, nullptr, 0, &txn);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_txn_begin", rc);
    goto env_close;
  }

  rc = mdbx_dbi_open(txn, subname, MDBX_DB_ACCEDE, &dbi);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_dbi_open", rc);
    goto txn_abort;
  }

  rc = mdbx_drop(txn, dbi, delete);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_drop", rc);
    goto txn_abort;
  }

  rc = mdbx_txn_commit(txn);
  if (unlikely(rc != MDBX_SUCCESS)) {
    error("mdbx_txn_commit", rc);
    goto txn_abort;
  }
  txn = nullptr;

txn_abort:
  if (txn)
    mdbx_txn_abort(txn);
env_close:
  mdbx_env_close(env);

  return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
