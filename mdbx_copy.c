/* This file is part of the libmdbx amalgamated source code (v0.15.0-54-g62f07cfe at 2026-10-15T01:24:05+03:00).
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

#if IS_WINDOWS

/* Bit of madness for Windows console */

#include "mdbx-wingetopt.h"

#endif /* WINDOWS */

static const char *prog;
static void usage_msg(FILE *out) {
  fprintf(out,
          "usage: %s [-V] [-q] [-c] [-d] [-f] [-p] [-u|U] [-n] [-h|--help] src_path [dest_path]\n"
          "  -V\t\tprint version and exit\n"
          "  -q\t\tbe quiet\n"
          "  -c\t\tenable compactification (skip unused pages)\n"
          "  -f\t\tforce copying even the target file exists\n"
          "  -d\t\tenforce copy to be a dynamic size DB\n"
          "  -p\t\tusing transaction parking/ousting during copying MVCC-snapshot\n"
          "    \t\tto avoid stopping recycling and overflowing the DB\n"
          "  -u\t\twarmup database before copying\n"
          "  -U\t\twarmup and try lock database pages in memory before copying\n"
          "  -n\t\topen MDBX environment(s) which do not use subdirectories\n"
          "  -h|--help\tprint this usage message and exit\n"
          "  src_path\tsource database\n"
          "  dest_path\tdestination (stdout if not specified)\n",
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

int main(int argc, char *argv[]) {
  int rc;
  MDBX_env *env = nullptr;
  const char *act;
  unsigned flags = MDBX_RDONLY;
  unsigned cpflags = 0;
  bool quiet = false;
  bool warmup = false;
  MDBX_warmup_flags_t warmup_flags = MDBX_warmup_default;

  prog = argv[0];
  if (tool_help_requested(argc, argv, "VqcfdpnuUnh"))
    help();

  for (; argc > 1 && argv[1][0] == '-'; argc--, argv++) {
    if (argv[1][1] == 'n' && argv[1][2] == '\0')
      flags |= MDBX_NOSUBDIR;
    else if (argv[1][1] == 'c' && argv[1][2] == '\0')
      cpflags |= MDBX_CP_COMPACT;
    else if (argv[1][1] == 'd' && argv[1][2] == '\0')
      cpflags |= MDBX_CP_FORCE_DYNAMIC_SIZE;
    else if (argv[1][1] == 'p' && argv[1][2] == '\0')
      cpflags |= MDBX_CP_THROTTLE_MVCC;
    else if (argv[1][1] == 'f' && argv[1][2] == '\0')
      cpflags |= MDBX_CP_OVERWRITE;
    else if (argv[1][1] == 'q' && argv[1][2] == '\0')
      quiet = true;
    else if (argv[1][1] == 'u' && argv[1][2] == '\0')
      warmup = true;
    else if (argv[1][1] == 'U' && argv[1][2] == '\0') {
      warmup = true;
      warmup_flags = MDBX_warmup_force | MDBX_warmup_touchlimit | MDBX_warmup_lock;
    } else if (argv[1][1] == 'V' && argv[1][2] == '\0') {
      tool_version_print("copy");
      return EXIT_SUCCESS;
    } else
      argc = 0;
  }

  if (argc < 2 || argc > 3)
    usage();

  tool_break_setup();

  if (!quiet) {
    fprintf((argc == 2) ? stderr : stdout, "mdbx_copy %s (%s, T-%s)\nRunning for copy %s to %s...\n",
            mdbx_version.git.describe, mdbx_version.git.datetime, mdbx_version.git.tree, argv[1],
            (argc == 2) ? "stdout" : argv[2]);
    fflush(nullptr);
    mdbx_setup_debug(MDBX_LOG_NOTICE, MDBX_DBG_DONTCHANGE, tool_logger_default);
  }

  act = "opening environment";
  rc = mdbx_env_create(&env);
  if (rc == MDBX_SUCCESS)
    rc = mdbx_env_open(env, argv[1], flags, 0);

  if (rc == MDBX_SUCCESS && warmup) {
    act = "warming up";
    rc = mdbx_env_warmup(env, nullptr, warmup_flags, 3600 * 65536);
  }

  if (!MDBX_IS_ERROR(rc)) {
    act = "copying";
    if (argc == 2) {
      mdbx_filehandle_t fd;
#if IS_WINDOWS
      fd = GetStdHandle(STD_OUTPUT_HANDLE);
#else
      fd = fileno(stdout);
#endif
      rc = mdbx_env_copy2fd(env, fd, cpflags);
    } else
      rc = mdbx_env_copy(env, argv[2], cpflags);
  }
  if (rc)
    fprintf(stderr, "%s: %s failed, error %d (%s)\n", prog, act, rc, mdbx_strerror(rc));
  mdbx_env_close(env);

  return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
