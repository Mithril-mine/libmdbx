// EN: Crash recovery: steady state after a crashed writer, open_for_recovery
// EN: Textbook section: Volume III, chapter 20, "Durability and recovery", §20.4/§20.5.
// EN: What it demonstrates: a writer crash (_exit in the middle of a write transaction) leaves
// EN:   no traces — reopening shows the last committed state (steady recovery);
// EN:   mdbx_env_open_for_recovery() opens the DB in recovery mode;
// EN:   the mdbx_chk command for integrity checking.
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 27-recovery.c++ -lmdbx
// EN: Run: ./27-recovery
// EN: Expected output (POSIX):
// EN:   committed: key=v1
// EN:   crashed writer changes rolled back: key=v1, newkey absent
// EN:   open_for_recovery: ok
// EN:   ok: crash recovery and open_for_recovery
// EN: On Windows the example does not run and reports a skip.
// RU: Восстановление после краха: steady-состояние после упавшего писателя, open_for_recovery
// RU: Раздел учебника: Том III, глава 20, «Долговечность и восстановление», §20.4/§20.5.
// RU: Что демонстрирует: падение писателя (_exit в середине пишущей транзакции) не
// RU:   оставляет следов — повторное открытие показывает last committed состояние
// RU:   (steady-восстановление); mdbx_env_open_for_recovery() открывает БД в режиме
// RU:   восстановления; команда mdbx_chk для проверки целостности.
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 27-recovery.c++ -lmdbx
// RU: Запуск: ./27-recovery
// RU: Ожидаемый вывод (POSIX):
// RU:   committed: key=v1
// RU:   crashed writer changes rolled back: key=v1, newkey absent
// RU:   open_for_recovery: ok
// RU:   ok: crash recovery and open_for_recovery
// RU: На Windows пример не выполняется и сообщает о пропуске.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
int main() {
  std::cout << "skipped: fork() is not available on Windows\n";
  return EXIT_SUCCESS;
}
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-27-recovery.mdbx";
    mdbx::env::remove(path);

    // EN: Commit the data and close the environment.
    // RU: Коммитим данные и закрываем окружение.
    {
      auto env = example::env_open(path);
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
      std::cout << "committed: key=v1" << std::endl;
    }

    // EN: A "crashing" writer: makes changes and _exit() without commit/abort.
    // RU: Писатель-«падающий»: вносит изменения и _exit() без commit/abort.
    const pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "fork failed\n";
      return EXIT_FAILURE;
    }
    if (pid == 0) {
      auto env = example::env_open(path);
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.upsert(table, mdbx::slice("key"), mdbx::slice("crash-value"));
      txn.insert(table, mdbx::slice("newkey"), mdbx::slice("x"));
      // EN: Simulating a crash: no commit and no abort.
      // RU: Имитация краха: без commit и без abort.
      _exit(EXIT_SUCCESS);
    }
    int status = 0;
    waitpid(pid, &status, 0);

    // EN: Reopening: the uncommitted changes have been rolled back.
    // RU: Повторное открытие: незакоммиченные изменения откатились.
    {
      auto env = example::env_open(path);
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      const std::string value(txn.get(table, mdbx::slice("key")).as_string());
      std::cout << "crashed writer changes rolled back: key=" << value << ", newkey ";
      try {
        (void)txn.get(table, mdbx::slice("newkey"));
        std::cout << "present\n";
        return EXIT_FAILURE;
      } catch (const mdbx::not_found &) {
        std::cout << "absent\n";
      }
      txn.abort();
    }

    // EN: Recovery mode: open a specific meta-page (0..2).
    // RU: Режим восстановления: открыть конкретную meta-страницу (0..2).
    {
      auto recovery = mdbx::env_managed::open_for_recovery(path.c_str(), 0 /* target_meta */, true);
      std::cout << "open_for_recovery: ok\n";
    }

    // EN: Integrity check (CLI): mdbx_chk -w <path>
    // RU: Проверка целостности (CLI): mdbx_chk -w <path>
    std::cout << "hint: run 'mdbx_chk -w " << path << "' to verify integrity\n";

    std::cout << "ok: crash recovery and open_for_recovery\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}
#endif // Windows vs POSIX