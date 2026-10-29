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

#if IS_WINDOWS

/* Bit of madness for Windows console */

#include "mdbx-wingetopt.h"

#endif /* WINDOWS */

static char *prog;
static void usage_msg(FILE *out) {
  fprintf(out,
          "usage: %s [-V] [-v[v[v...]]] [-q] [-1..9] [-t seconds] [-f percent] [-r percent] [-s megabytes] [-c] [-u|U] "
          "[-h|--help] db_pathname\n"
          "  -V\t\tprint version and exit\n"
          "  -v\t\tmore verbose, could be repeated for extra details from debug-enabled builds.\n"
          "  -q\t\tbe quiet.\n"
          "  -1\t\tperform a single quick defragmentation cycle without two-stage moves.\n"
          "  -2 .. -9\tlimits the number of defragmentation cycles.\n"
          "  -t seconds\tlimits the duration of defragmentation in seconds.\n"
          "  -f percent\tdefragment until free up a given percentage of unused space (default 100).\n"
          "  -r percent\tacceptable undefragmented residue in percent of used space (0 by default).\n"
          "  -s megabytes\tpreferred defragmentation step/transaction size.\n"
          "  -c\t\tforce cooperative mode (don't try exclusive).\n"
          "  -u\t\twarmup database before defragmenting.\n"
          "  -U\t\twarmup and try lock database pages in memory before defragmenting.\n"
          "  -h|--help\tprint this usage message and exit.\n"
          "  db_pathname\tpath to the database.\n",
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

static MDBX_log_level_t verbosity = MDBX_LOG_WARN;
static bool quiet = false;
static bool is_console = true;
static unsigned cycles_limit;
static unsigned progress_dots;

static void logger(MDBX_log_level_t level, const char *function, int line, const char *fmt, va_list args) {
  static const char *const prefixes[] = {
      "!!!fatal: ",   // 0 fatal
      " ! ",          // 1 error
      " ~ ",          // 2 warning
      "   ",          // 3 notice
      "   // ",       // 4 verbose
      "   //// ",     // 5 verbose
      "   ////// ",   // 6 verbose
      "   //////// ", // 7 verbose
  };
  if (!quiet && level < MDBX_LOG_DEBUG) {
    if (progress_dots) {
      putchar(' ');
      putchar('\n');
      progress_dots = 0;
      if (level <= MDBX_LOG_NOTICE)
        fflush(nullptr);
    }
    FILE *out = (level < MDBX_LOG_NOTICE) ? stderr : stdout;
    if (function && line && (size_t)level < ARRAY_LENGTH(prefixes))
      fprintf(out, "%s", prefixes[level]);
    vfprintf(out, fmt, args);
    if (level <= MDBX_LOG_NOTICE)
      fflush(nullptr);
  }
}

static void defrag_report_progress(const MDBX_defrag_result_t *progress, unsigned dots) {
  if (!quiet) {
    if (progress->cycles == 0)
      printf("\r - loading the b-tree structure: %u.%u%%", progress->rough_estimation_cycle_progress_permille / 10,
             progress->rough_estimation_cycle_progress_permille % 10);
    else {
      printf("\r - cycle %u: %u.%u%%", progress->cycles, progress->rough_estimation_cycle_progress_permille / 10,
             progress->rough_estimation_cycle_progress_permille % 10);
      if (progress->pages_moved) {
        printf(", moved %zi", progress->pages_moved);
        if (progress->pages_moved < progress->pages_scheduled)
          printf(" of %zi", progress->pages_scheduled);
      } else if (progress->pages_scheduled)
        printf(", scheduled %zi", progress->pages_scheduled);
      if (progress->pages_retained)
        printf(", retained %zi", progress->pages_retained);
      if (progress->pages_shrinked)
        printf(", shrink %zi", progress->pages_shrinked);
      printf(", left %zi", progress->pages_left);
    }
    for (unsigned i = 0; i < 3 || (i < dots / 8 && i < 64); ++i)
      putchar('.');
    if (is_console && dots) {
      static char twirl[] = "\\|/-\\|/-";
      putchar(twirl[(progress->spent_time_dot16 >> 13) % (ARRAY_LENGTH(twirl) - 1)]);
      putchar('\b');
    }
    fflush(nullptr);
  }
}

static const char *stop_reason(const MDBX_defrag_result_t *progress) {
  if (progress->stopping_reasons & MDBX_defrag_error)
    return "error";
  if (progress->stopping_reasons & MDBX_defrag_aborted)
    return "aborted by user";
  if (progress->stopping_reasons & MDBX_defrag_discontinued)
    return "discontinued by user";
  if (progress->stopping_reasons & MDBX_defrag_enough_threshold)
    return "enough threshold";
  if (progress->stopping_reasons & MDBX_defrag_time_limit)
    return "time limit reached";
  if (progress->stopping_reasons & MDBX_defrag_laggard_reader)
    return "laggard reader";
  if (progress->stopping_reasons & MDBX_defrag_large_chunk)
    return "large chunk";
  if (progress->stopping_reasons & MDBX_defrag_step_size)
    return "step size";
  return "done";
}

static int defrag_notify(void *ctx, const MDBX_defrag_result_t *progress) {
  (void)ctx;
  if (!quiet) {
    static MDBX_defrag_result_t last_progress = {.cycles = UINT32_MAX};
    static uint32_t last_report_spenttime;
    bool tick = progress->spent_time_dot16 - last_report_spenttime > 65536 / 8;
    if (!ctx || last_progress.cycles != progress->cycles) {
      if (progress_dots) {
        if (last_progress.cycles == progress->cycles)
          last_progress.stopping_reasons = progress->stopping_reasons;
        defrag_report_progress(&last_progress, progress_dots);
        progress_dots = 0;
        printf(" %s%s\n", stop_reason(&last_progress), is_console ? "\033[K" : "");
        fflush(nullptr);
      }
      tick = true;
    }

    if (ctx && tick) {
      last_report_spenttime = (uint32_t)progress->spent_time_dot16;
      defrag_report_progress(progress, progress_dots++);
    }
    last_progress = *progress;
  }

  if (user_break > 1)
    return -1;

  if (cycles_limit && (progress->cycles > cycles_limit ||
                       (progress->cycles == cycles_limit && (progress->stopping_reasons & MDBX_defrag_large_chunk))))
    return 1;

  return user_break;
}

int main(int argc, char *argv[]) {
  int rc;
  MDBX_env *env = nullptr;
  const char *const progname = argv[0];
  bool warmup = false;
  size_t time_limit_seconds = 0;
  unsigned acceptable_residue_percent = 0;
  unsigned wanna_defrag_percent = 100;
  size_t step_size_MiB = 0;
  MDBX_env_flags_t env_flags = MDBX_ENV_DEFAULTS | MDBX_EXCLUSIVE;
  MDBX_warmup_flags_t warmup_flags = MDBX_warmup_default;

  prog = argv[0];
  if (argc < 2)
    usage();

  if (tool_help_requested(argc, argv, "Vvq123456789s:t:f:r:cuUh"))
    help();

  for (int i; (i = getopt(argc, argv,
                          "V"
                          "v"
                          "q"
                          "123456789"
                          "s:"
                          "t:"
                          "f:"
                          "r:"
                          "c"
                          "u"
                          "U"
                          "h")) != EOF;) {
    switch (i) {
    case 'h':
      help();
      break;
    case 'V':
      tool_version_print("defrag");
      return EXIT_SUCCESS;
    case 'v':
      if (++verbosity > 9)
        usage();
      break;
    case 'q':
      quiet = true;
      break;
    case 'c':
      env_flags = (env_flags & ~MDBX_EXCLUSIVE) | MDBX_ACCEDE;
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      cycles_limit = i - '0';
      break;
    case 't':
      if (sscanf(optarg, "%zu", &time_limit_seconds) != 1) {
        if (!quiet)
          fprintf(stderr, "%s: %s option: expecting %s, but got '%s'\n", prog, "-t", "unsigned integer value", optarg);
        return EXIT_FAILURE;
      }
      break;
    case 'f':
      if (sscanf(optarg, "%u", &wanna_defrag_percent) != 1 || wanna_defrag_percent < 1 || wanna_defrag_percent > 100) {
        if (!quiet)
          fprintf(stderr, "%s: %s option: expecting %s, but got '%s'\n", prog, "-f",
                  "unsigned integer value in range 1..100", optarg);
        return EXIT_FAILURE;
      }
      break;
    case 'r':
      if (sscanf(optarg, "%u", &acceptable_residue_percent) != 1 || acceptable_residue_percent < 1 ||
          acceptable_residue_percent > 100) {
        if (!quiet)
          fprintf(stderr, "%s: %s option: expecting %s, but got '%s'\n", prog, "-r",
                  "unsigned integer value in range 1..100", optarg);
        return EXIT_FAILURE;
      }
      break;
    case 's':
      if (sscanf(optarg, "%zu", &step_size_MiB) != 1) {
        if (!quiet)
          fprintf(stderr, "%s: %s option: expecting %s, but got '%s'\n", prog, "-s",
                  "unsigned integer value in megabytes", optarg);
        return EXIT_FAILURE;
      }
      break;
    case 'u':
      warmup = true;
      break;
    case 'U':
      warmup_flags = MDBX_warmup_force | MDBX_warmup_touchlimit | MDBX_warmup_lock;
      warmup = true;
      break;
    default:
      usage();
    }
  }

  if (optind != argc - 1)
    usage();

  tool_break_setup();
#if IS_WINDOWS
  is_console = _isatty(_fileno(stdout)) != 0;
#else
  is_console = isatty(fileno(stdout)) == 1;
#endif

  const char *const db_pathname = argv[optind];
  if (!quiet) {
    fprintf(stdout, "mdbx_defrag %s (%s, T-%s)\nRunning for %s...\n", mdbx_version.git.describe,
            mdbx_version.git.datetime, mdbx_version.git.tree, db_pathname);
    if (verbosity > MDBX_LOG_VERBOSE && MDBX_DEBUG < 1)
      printf("Verbosity level %u exposures only to"
             " a debug/extra-logging-enabled builds (MDBX_DEBUG > 0)\n",
             verbosity);
    mdbx_setup_debug(verbosity, MDBX_DBG_DONTCHANGE, logger);
    fflush(nullptr);
  }

  const char *act = "opening environment";
  rc = mdbx_env_create(&env);
  if (rc == MDBX_SUCCESS)
    rc = mdbx_env_open(env, db_pathname, env_flags, 0);
  if ((env_flags & MDBX_EXCLUSIVE) && (rc == MDBX_BUSY ||
#if IS_WINDOWS
                                       rc == ERROR_LOCK_VIOLATION || rc == ERROR_SHARING_VIOLATION
#else
                                       rc == EBUSY || rc == EAGAIN
#endif
                                       )) {
    env_flags = (env_flags & ~MDBX_EXCLUSIVE) | MDBX_ACCEDE;
    rc = mdbx_env_open(env, db_pathname, env_flags, 0);
  }

  if (rc == MDBX_SUCCESS && warmup) {
    act = "warming up";
    rc = mdbx_env_warmup(env, nullptr, warmup_flags, 3600 * 65536);
    rc = MDBX_IS_ERROR(rc) ? rc : MDBX_SUCCESS;
  }

  MDBX_txn *txn = nullptr;
  if (rc == MDBX_SUCCESS) {
    act = "preparing";
    rc = mdbx_txn_begin(env, nullptr, MDBX_TXN_READWRITE, &txn);
  }

  MDBX_envinfo info_env;
  memset(&info_env, 0, sizeof(info_env)); /* zap `uninitialized` warning */
  if (rc == MDBX_SUCCESS)
    rc = mdbx_env_info_ex(env, txn, &info_env, sizeof(info_env));

  MDBX_gc_info_t info_gc;
  memset(&info_gc, 0, sizeof(info_gc)); /* zap `uninitialized` warning */
  if (rc == MDBX_SUCCESS)
    rc = mdbx_gc_info(txn, &info_gc, sizeof(info_gc), nullptr, nullptr);

  char ratio_buffer[42];
  if (rc == MDBX_SUCCESS) {
    const size_t pages_per_MiB = MEGABYTE / info_env.mi_dxb_pagesize;
    const size_t payload_pages = info_gc.pages_allocated - info_gc.pages_gc;
    const size_t preferred_batch = (step_size_MiB > INT_MAX / pages_per_MiB) ? 0 : step_size_MiB * pages_per_MiB;
    const size_t duration_limit_dot16 =
        (time_limit_seconds > SIZE_MAX >> 16) ? /* unlimited */ 0 : time_limit_seconds << /* seconds to dot16 */ 16;
    const size_t defrag_enough =
        (wanna_defrag_percent < 100u) ? (info_gc.pages_gc * wanna_defrag_percent + 99u) / 100u : 0;
    const intptr_t acceptable_residue =
        acceptable_residue_percent ? (intptr_t)((payload_pages + 99u) / 100u * acceptable_residue_percent) : -1;

    if (!quiet) {
      printf(" - space usage in pages: %zu allocated, payload %zu, unused/GC %zu, pagesize %u\n",
             info_gc.pages_allocated, payload_pages, info_gc.pages_gc, info_env.mi_dxb_pagesize);
      printf(" - defragmentation: target %u%% (shrink %zu pages), cycles ", wanna_defrag_percent,
             defrag_enough ? defrag_enough : info_gc.pages_gc);
      if (cycles_limit)
        printf("limit %u", cycles_limit);
      else
        printf("unlimited");

      printf(", step ");
      if (preferred_batch)
        printf("%zu pages (%zu MiB)", preferred_batch, step_size_MiB);
      else
        printf("unlimited");

      if (acceptable_residue_percent)
        printf(", acceptable residue %u%% (leftover %zu pages)", acceptable_residue_percent, acceptable_residue);
      printf(", duration ");
      if (time_limit_seconds)
        printf("limit %s seconds\n",
               mdbx_ratio2digits(duration_limit_dot16, 65536, 1, ratio_buffer, sizeof(ratio_buffer)));
      else
        puts("unlimited");
      fflush(nullptr);
    }

    act = "defragmenting";
    MDBX_defrag_result_t result;
    rc = mdbx_env_defrag(env,
                         /* defrag_atleast */ 0,
                         /* time_atleast_dot16 */ 0, defrag_enough, duration_limit_dot16, acceptable_residue,
                         preferred_batch, defrag_notify, env, &result);
    defrag_notify(nullptr, &result);

    if (!MDBX_IS_ERROR(rc)) {
      if (!quiet) {
        printf("Defragmentation%s: shrink %zi pages, %u passes, moved %zu pages",
               (rc == MDBX_SUCCESS) ? " done" : " incomplete", result.pages_shrinked, result.cycles,
               result.pages_moved);
        if (result.stopping_reasons)
          printf(", stopping reasons bits 0x%x", result.stopping_reasons);
        printf(", took %s seconds, %s\n",
               mdbx_ratio2digits(result.spent_time_dot16, 65536, 3, ratio_buffer, sizeof(ratio_buffer)),
               stop_reason(&result));
      }
      rc = MDBX_SUCCESS;
    }
  }

  if (txn) {
    if (rc != MDBX_SUCCESS)
      mdbx_txn_abort(txn);
    else {
      act = "final commit";
      rc = mdbx_txn_commit(txn);
    }
    txn = nullptr;
  }

  if (!quiet && rc != MDBX_SUCCESS) {
    fflush(nullptr);
    fprintf(stderr, "%s: %s failed, error %d (%s)\n", progname, act, rc, mdbx_strerror(rc));
  }

  mdbx_env_close(env);
  fflush(nullptr);
  return (rc != MDBX_SUCCESS) ? EXIT_FAILURE : EXIT_SUCCESS;
}
