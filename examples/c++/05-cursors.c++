// EN: Cursors: FIRST/NEXT, SET_RANGE, LAST/PREV
// EN: Textbook section: Volume II, chapter 6, "Cursors",
// EN:   sections "Positioning", "Batch operations".
// EN: What it demonstrates: forward iteration (FIRST/NEXT) and backward (LAST/PREV);
// EN:   the "nearest greater-or-equal key" search (SET_RANGE / to_key_greater_or_equal).
// EN: Build: g++ -std=c++17 -I<libmdbx-include> -I../common 05-cursors.c++ -lmdbx
// EN: Run: ./05-cursors
// EN: Expected output:
// EN:   forward (10): k0 k1 k2 k3 k4 k5 k6 k7 k8 k9
// EN:   set_range(k5) -> k5
// EN:   set_range(k5x) -> k6
// EN:   backward (10): k9 k8 k7 k6 k5 k4 k3 k2 k1 k0
// EN:   ok: FIRST/NEXT, SET_RANGE, LAST/PREV iterated
// EN: On failure — message to stderr and exit 1.
// RU: Cursors: FIRST/NEXT, SET_RANGE, LAST/PREV
// RU: Раздел учебника: Том II, глава 6, «Курсоры»,
// RU:   разделы «Позиционирование», «Пакетные операции».
// RU: Что демонстрирует: итерацию вперёд (FIRST/NEXT) и назад (LAST/PREV);
// RU:   поиск «ближайшего большего ключа» (SET_RANGE / to_key_greater_or_equal).
// RU: Сборка: g++ -std=c++17 -I<libmdbx-include> -I../common 05-cursors.c++ -lmdbx
// RU: Запуск: ./05-cursors
// RU: Ожидаемый вывод:
// RU:   forward (10): k0 k1 k2 k3 k4 k5 k6 k7 k8 k9
// RU:   set_range(k5) -> k5
// RU:   set_range(k5x) -> k6
// RU:   backward (10): k9 k8 k7 k6 k5 k4 k3 k2 k1 k0
// RU:   ok: FIRST/NEXT, SET_RANGE, LAST/PREV iterated
// RU: При сбое — сообщение в stderr и exit 1.
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <string>

#include <mdbx.h++>
#include "common.h++"

int main() {
  try {
    const std::string path = std::string(example::tmpdir()) + "/mdbx-05-cursors.mdbx";
    mdbx::env::remove(path);
    mdbx::env_managed env = example::env_open(path);

    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 10; ++i) {
        const auto key = "k" + std::to_string(i);
        const auto val = "v" + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice(val));
      }
      txn.commit();
    }

    auto rtxn = env.start_read();
    auto table = rtxn.open_map(nullptr);
    auto cur = rtxn.open_cursor(table);

    // EN: Forward iteration: FIRST then NEXT until the end.
    // RU: Прямая итерация: FIRST затем NEXT до конца.
    size_t forward_count = 0;
    std::string forward;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      forward += r.key.as_string() + " ";
      ++forward_count;
    }
    std::cout << "forward (" << forward_count << "): " << forward << "\n";

    // EN: SET_RANGE: the first key not less than the given one.
    // RU: SET_RANGE: первый ключ, не меньший заданного.
    auto r = cur.to_key_greater_or_equal(mdbx::slice("k5"), false);
    std::cout << "set_range(k5) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";
    r = cur.to_key_greater_or_equal(mdbx::slice("k5x"), false);
    std::cout << "set_range(k5x) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";

    // EN: Backward iteration: LAST then PREV until the beginning.
    // RU: Обратная итерация: LAST затем PREV до начала.
    size_t backward_count = 0;
    std::string backward;
    for (auto rr = cur.to_last(); rr; rr = cur.to_previous(false)) {
      backward += rr.key.as_string() + " ";
      ++backward_count;
    }
    std::cout << "backward (" << backward_count << "): " << backward << "\n";

    rtxn.abort();

    std::cout << "ok: FIRST/NEXT, SET_RANGE, LAST/PREV iterated\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &ex) {
    std::cerr << "error: " << ex.what() << "\n";
    return EXIT_FAILURE;
  }
}