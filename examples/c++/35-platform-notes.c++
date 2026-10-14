// EN: Platform notes: tmpfs/ENOSPC, WSL1/ENOLCK, Windows notes
// EN: Textbook section: Volume V, chapter 28, "Platform specifics", §28.2/§28.3.
// EN: What it demonstrates: robust handling of geometry errors on exotic
// EN:   filesystems; the caveat snippets themselves are given in the textbook.
// RU: Платформенные заметки: tmpfs/ENOSPC, WSL1/ENOLCK, заметки о Windows
// RU: Раздел учебника: Том V, глава 28, «Платформенные нюансы», §28.2/§28.3.
// RU: Что демонстрирует: устойчивую обработку ошибок геометрии на экзотичных
// RU:   файловых системах; сами сниппеты-оговорки приведены в тексте учебника.
//
// EN: Brief notes (full snippets — in §28.2/§28.3):
// EN:   - tmpfs (e.g. /dev/shm): size is limited by memory; mdbx_env_set_geometry
// EN:     with a large upper may fail with ENOSPC/EDQUOT when the file grows.
// EN:   - WSL1: old versions do not support fcntl(F_SETLK) on some filesystems —
// EN:     MDBX_ENOLCK is possible; workaround — store the DB on ext4 inside WSL.
// EN:   - Windows: 32-bit builds require linking with the LARGEADDRESSAWARE flag
// EN:     to support large address spaces and databases > 2 GB.
// RU: Краткие заметки (полные сниппеты — в §28.2/§28.3):
// RU:   - tmpfs (например /dev/shm): размер ограничен памятью; mdbx_env_set_geometry
// RU:     с большим upper может завершиться ENOSPC/EDQUOT при росте файла.
// RU:   - WSL1: старые версии не поддерживают fcntl(F_SETLK) на некоторых ФС —
// RU:     возможен MDBX_ENOLCK; обход — хранить БД на ext4 внутри WSL.
// RU:   - Windows: 32-битные сборки требуют линковки с флагом LARGEADDRESSAWARE
// RU:     для поддержки больших адресных пространств и БД > 2 ГБ.
//
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 35-platform-notes.c++ -lmdbx
// EN: Run: ./35-platform-notes
// EN: Expected output:
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 35-platform-notes.c++ -lmdbx
// RU: Запуск: ./35-platform-notes
// RU: Ожидаемый вывод:
//   geometry probe: <result>
//   ok: platform error handling shown
// EN: On failure — message to stderr and exit 1.
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-35-platform-notes.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    // EN: Probing geometry limits must not crash: we request an "absurd"
    // EN: upper bound and handle the result carefully.
    // RU: Проверка границ геометрии не обязана падать: запрашиваем «абсурдную»
    // RU: верхнюю границу и аккуратно обрабатываем результат.
    mdbx::env::geometry probe;
#if INTPTR_MAX > 0x7fffFFFFl
    probe.size_upper = intptr_t(8) * mdbx::env::geometry::TB;
#else
    probe.size_upper = mdbx::env::geometry::maximal_value;
#endif
    try {
      env.set_geometry(probe);
      std::cout << "geometry probe: accepted\n";
    } catch (const std::exception &ex) {
      // EN: On tmpfs/WSL1/32-bit Windows this may fail —
      // EN: for the example it matters that the error is caught, not that it crashed the process.
      // RU: На tmpfs/WSL1/32-битной Windows это может завершиться ошибкой —
      // RU: для примера важно, что она перехвачена, а не уронила процесс.
      std::cout << "geometry probe: rejected (" << ex.what() << ")\n";
    }

    std::cout << "ok: platform error handling shown\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}