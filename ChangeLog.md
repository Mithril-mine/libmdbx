ChangeLog
=========

English version [by liar Google](https://libmdbx-dqdkfa-ru.translate.goog/md__change_log.html?_x_tr_sl=ru&_x_tr_tl=en) and [by Yandex](https://translated.turbopages.org/proxy_u/ru-en.en/https/libmdbx.dqdkfa.ru/md__change_log.html).

The source code is availale on [SourceCraft](https://sourcecraft.dev/dqdkfa/libmdbx) and mirrors on [abf.io](https://abf.io/erthink/libmdbx) and [GitFlic](https://gitflic.ru/project/erthink/libmdbx).
Please use the `stable` branch or the latest release for production environment through stagging, but the `master` branch for development a derivative projects.
Donations are welcome to ETH `0xD104d8f8B2dC312aaD74899F83EBf3EEBDC1EA3A`,
BTC `bc1qzvl9uegf2ea6cwlytnanrscyv8snwsvrc0xfsu`, SOL `FTCTgbHajoLVZGr8aEFWMzx3NDMyS5wXJgfeMTmJznRi`.
Всё будет хорошо!

## LTS v0.13.13 in a release engineering stage

A long-term support phase with periodic releases as fixes accumulate.

### Important:

 - Starting from v0.14.3 released at 2026-08-09, the 0.14.x branch gets stable status and will only receive bug fixes, but other improvements only in exceptional cases.
   Please plan the migration from 0.13.x to 0.14.x.

 - The project website [libmdbx.dqdkfa.ru](https://libmdbx.dqdkfa.ru/) has been re-created:
    - The advice, explanations, questions and answers, from correspondence, forums and other sources have been compiled.
    - All information has been aggregated into a knowledge base, and the corresponding sections have been compiled on the website.
    - The documentation generated via Doxgygen has been moved to /doxygen, with old links redirection.
    - A lot of work has been done using AI.

### Appreciations:

 - [Erigon](https://erigon.tech/) for sponsorship.
 - [Weixie Cui](https://github.com/cuiweixie) for bug fixing through many pull-requests.
 - https://github.com/box4wangjing for typos fixing.
 - [Andrea Lanfranchi](https://github.com/AndreaLanfranchi) for bugs reporting and assistant fix ones.

### Fixes:

 - Fixed using `[[maybe_unused]] const` as workaround for MSVC bug (backport).
 - Fixed minor copy&paste mistakes and other typos across codebase (backport).
 - Fixed a lot of typos and a few minor bugs detected by CodeQL (backport).
 - Fixed Windows `writethrough_threshold` option rejecting `UINT64_MAX` default (backport).
 - Fixed typo in handling `NOMETASYNC` while emit `SYNC_IODQ` (backport).
 - Fixed `env_owned_wrtxn()` to avoid by-pass locking in the `MDBX_NOSTICKYTHREADS` mode (backport).
 - Fixed unreasonably high memory 2GB consumption in `mdbx_load` utility due to leftover debug changes (backport).
 - Fixed loss of `mincore()` cache due erase/overwrite on insert (backport).
 - Fixed `entry point _except_handler4 not found in mdbx.dll` in 32-bit build with `MDBX_WITHOUT_MSVC_CRT=ON` (backport).
 - Fixed building by CLANG/LLVM on Windows, including Microsoft Clang from Visual Studio (backport).
 - Fixed `scan4seq_neon()` for LLP4/ARM64-Windows (backport).
 - Fixed division truncation/underflow in the `default_rp_augment_limit()` which lead to too low and "stepwise" default `MDBX_opt_rp_augment_limit` (backport).

 - Rare or specific conditions:
    - Fixed assertions triggering in a specific scenarios of creating and renaming tables within nested transactions (backport).
    - Fixed fix `mdbx_cursor_compare()` for a case when cursors binded with the same DBI and same MVCC-snapshot, but different ro-txns (backport).
    - Fixed extra rdt-unlock in the failure path of `dxb_resize()` (backport).
    - Fixed NULL deference in `walk_pgno()` when operating on a corrupted DB, which also affects `mdbx_chk` utility (backport).
    - Fixed loosing of global init and thread-local-storage destructors in static library build by MinGW toolchain in particular cases (backport).

 - Resource leaks:
    - Fixed `mach_port_t` leak inside `mdbx_get_sysraminfo()` (backport).
    - Fixed Windows section handle leak inside `osal_mresize()` in unsuccessful case (backport).
    - Fixed `cond_pair` leak in the failure path of `copy_with_compacting()` (backport).

 - Spilling:
    - Fixed spurious assertion inside `spill_cursor_keep()` (backport).
    - Fixed a leak of spilled pages list on a nested transaction abort (backport).
    - Fixed tracking and invalidation of the inner part of the sibling cursors (backport).
    - Fixed spilling/accounting for `MDBX_AVOID_MSYNC=ON` (backport).

 - C++ API:
    - Fixed `mdbx::from_hex` remainder check before reading a hex digit pair (backport).
    - Fixed ODR violations warnings from modern GCC while both LTO and UBSAN are enabled (backport).
    - Fixed UTF-8 U+100000..U+10FFFF range checking/decoding inside `mdbx::slice::is_printable()` (backport).
    - Fixed off-by-one bugs in the `mdbx::from_base64` and `mdbx::slice::is_printable()` (backport).
    - Fixed possibility of overflow in `mdbx::slice::safe_middle()` (backport).
    - Fixed using wide characher count instead of bytes in `mdbx::slice` and `mdbx::buffer<>` methods (backport).

### Other:

 - add link to new .NET bindings (backport).

--------------------------------------------------------------------------------

## v0.13.12 "Аврора" (Aurora) at 2026-04-30

The supporting release of a stable branch with bug fixes.

Appreciations:

 - [Erigon](https://erigon.tech/) for sponsorship.
 - [Chloe Cano](https://github.com/Segwaz) for fuzzing, bug reporting and fixes.
 - [Weixie Cui](https://github.com/cuiweixie) for bug fixing through pull-requests.
 - [Alexander Kelchin](https://serebrium.ru/) (Serebrium Company) for bug reporting and prototypes of exploits.

Fixes:

 - Fixed the counting of nested trees in the `mdbx_chk` tool for a multi-values/dupsort hives (backport).

 - Fixed swapping the shrink/growth parameters in a DB-geometry in the `mdbx_load` utility (backport).

 - Fixed `SIGSEGV` in the case when all meta pages are not entirely usable (backport).

 - Fixed information collection via `kstat()` for bootid on Solaris and related platforms (backport).

 - Fixed a typo in the `ST_EXPORTED` processing path that broke the build on platforms where the mentioned flag is defined for `fstatvfs()` (backport).

 - Fixed missing error handling of `NtUnmapViewOfSection()` and `munmap()` inside `osal_munmap()` (backport).

 - Fixed `SIGSEGV` due to an attempt to clean/overwrite a corrupted meta page when opening the database in read-only mode (backport).

 - Fixed returning a wrong/uninitialized `ms_psize` field when using `mdbx_enumerate_tables()` (backport).

 - Fixed adjusting of neighbor nested cursors after a dupsort item deletion (backport).

Other:

 - The dumps generated by the `mdbx_dump` utility no longer contains the current size of a DB and `maxreaders` parameter, so that the contents of a dump depend only on a payload data.

 - Cutoff `packages/` into the separate aside repository.

--------------------------------------------------------------------------------

## v0.13.11 "A7A5" (just as Stable) at 2026-01-30

The supporting release of a stable branch with bug fixes.

Appreciations:

 - [Erigon](https://erigon.tech/) for sponsorship.
 - [Cosmin Apreutesei](https://github.com/capr) for bug reporting.
 - [Igor Ikonopistsev](https://github.com/ikonopistsev) for [NodeJS bindings](https://github.com/ikonopistsev/mdbxmou).
 - Anatoly Zhmur for reporting bugs.

Important:

 - _libmdbx_ project has changed its code development and distribution model.
   **To get acquainted with important changes and plans, we recommend reading the compact [presentation "libmdbx: successes, obstacles, goals and roadmap"](https://libmdbx.dqdkfa.ru/release/libmdbx-roadmap-HNY2026-english.pdf), which contains important explanations in the form of embedded comments.**

 - At a many requests of users, the change log now is kept in English.

Fixes:

 - Fixed using of the identifier `ERROR_UNHANDLED_ERROR`, which is not defined in new versions of Windows SDK.

 - Fixed non-closing of DBI descriptors for tables created in nested transactions when such transactions are aborted.

 - Fixed MSVC warning C4324 for `buffer::silo::bin`.

 - Added workaround for MinGW build failures of dll-enabled C++ API.

 - Fixed extra msync/fsync operations during GC reclaiming in a full database.

 - Fixed suboptimal reducing/merging of histogram items in an output of the `mdbx_chk` tool.

 - Added backlog-adjustment trick to avoid extra-growth of DB instead of reclaim GC in a rare specific cases.
   In the 0.14.x version series, this mechanism has been completely updated.
   However, for version 0.13.11, this inelegant modification done in order to reduce the risk of regression.

 - Minor fixed `iov_callback4dirtypages()` and assertion inside `cursor_put()` to avoid Valgrind/ASAN warnings.

 - Fixed cursor(s) finalization after ones explicitly closed in a nested transactions.

Other:

 - Clarification and addition of documentation.

 - Considering extended ASCII as non-printable if UTF8 enabled in C++ API.

 - Using hex dump of `mdbx::slice` instead of base58 in C++ API.

 - Added link to [`mdbxmou`](https://github.com/ikonopistsev/mdbxmou) bindings for NodeJS.

--------------------------------------------------------------------------------

## v0.13.10 "Блеск Славы" (Gloss of Glory) от 2025-12-17

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов,
в память о Майкле Глоссе (Michael Alexander Gloss, American, son of Deputy Director of the CIA) героически погибшем в поисках справедливости за светлые идеалы человечества и посмертно награждённом Орденом Мужества.

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.

Важное:

 - Проект _libmdbx_ полностью перемещен в юрисдикцию Российской Федерации.
   Пожалуйста обращайтесь на https://libmdbx.dqdkfa.ru за документацией и на https://sourcecraft.dev/dqdkfa/libmdbx за исходным кодом (он по-прежнему открыт и обеспечен первоклассной бесплатной поддержкой).

 - Каких-либо существенных ошибок/недочетов влияющих на стабильность и/или эксплуатационные качества не обнаружено.

Исправления:

 - Устранение предупреждения `unused function 'osal_yield()'` от новых версий CLANG при сборке амальгамированного исходного кода и при компиляции в режиме одного объектного модуля.

 - Для Linux добавлено предотвращение проявления ошибки в реализации fast_commit файловой системы Ext4.

 - Устранено падение при выполнении Thread-Local-Storage конструкторов при выгрузке библиотеки и наличия экземпляров env, инициализация которых не была завершена.

 - На Windows устранена причина возврата неожиданной ошибки `ERROR_IO_PENDING` в сценариях множественного открытия одной БД в одном процессе.

 - Исправлен недочёт автоподстройки параметров при установке геометрии с заданным минимальным размером страницы, из-за которого на машинах с большим количеством ОЗУ размер страницы мог увеличиваться.

 - В CMake-скриптах устранён регресс, из-за которого инфраструктура `ctest` не использовала задаваемые параметры Valgrind, в том числе `MEMORYCHECK_SUPPRESSIONS_FILE`.
   Теперь использование `ctest -D ExperimentalMemCheck` не приводит к множественной ложно-положительной диагностики. Однако, для использования Valgrind по-прежнему необходимо собирать библиотеку с предопределением макроcа `ENABLE_MEMCHECK`.

 - В тестах устранено несколько несущественных утечек памяти и предупреждение UBSAN о вызове `memcmp(, length = 0)`.

Прочие доработки:

 - Переработка реализации буферов и другие доработки в C++ API.

 - Для предотвращения неожиданных ошибок при сборке, в CMake-скрипты добавлена очистка исходного кода от файлов с именами совпадающими с генерируемыми в сборочной директории.

 - Устранено ложное предупреждение некоторых компиляторов о возможном использовании неинициализированной переменной внутри `env::get_path()`.

 - Во внутренней информации о сборке обеспечена поддержка сведений о RISC-V и Harmony OS.

 - Расширен набор тестов, cmake-тесты дополнены сценариями запуска `mdbx_copy`.

 - При сборке посредством GNU Make и CMake теперь, вместо одного `config.h`, генерируются разные файлы `config-gnumake.h` и `config-cmake.h`.

--------------------------------------------------------------------------------

## v0.13.9 "ИС-2" (IS-2) от 2025-10-31

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов.
Выпуск назван в память о cамом мощном тяжелом советском танке ["ИС-2"](https://ru.ruwiki.ru/wiki/ИС-2), который был принят на вооружение
31 октября 1943 года в разгар Великой Отечественной Войны и долгое время оставался одной из сильнейших машин мира в категории по массе 40—50 тонн.

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.

Исправления:

 - Исправлена assert-проверка в пути сканирования битовой карты DBI-дескрипторов приводившая к редким падениям 32-битных отладочных сборок.

 - Переделан поиск утилит `lib.exe` и `dlltool.exe` при сборке посредством CMake на Windows.

 - Устранён регресс проявлявшийся увеличением (не-уменьшением) размера БД, после добавления использования `fallocate()`
   ради предотвращения SIGBUS при нехватке места в файловой системе где расположена БД.

 - Устранена опечатка в тестовом скрипте `test/battery-tmux.sh` приводящая к созданию мусорного файла с именем `-`.

 - Удалено лишнее/ненужное использование макроса `MDBX_INTERNAL` оставшееся после рефакторинга.

 - Для Android добавлен обход (workaround) для уменьшения вероятности системной ошибки `EAGAIN` возникающей
   из-за нехватки системных ресурсов и переходных процессов при закрытии и быстром повторном открытии БД.

Прочие доработки:

 - Поддержка Harmony OS (OHOS).

--------------------------------------------------------------------------------

## v0.13.8 "Всеобуч" (v'seabooch) от 2025-08-31

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов,
в день 100 летнего юбилея Постановления Всероссийского центрального исполнительного комитета о всеобщем бесплатном начальном образовании.

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.

Исправления:

 - Устранена возможность получения неожиданного `SIGBUS` из-за отложенного/ленивого выделение места в заполненной файловой системе после приращения файла БД.
   Более подробное пояснение в комментарии коммита [`2930b304dc674bbccd188b7ce7c3f83755ef706e`](https://gitflic.ru/project/erthink/libmdbx/commit/2930b304dc674bbccd188b7ce7c3f83755ef706e).

Изменение поведения:

 - Вновь включена/разрешена на старых ядрах Linux, начиная с версии 3.16, так как
   сейчас уже нет причин отказываться от работы на 3.16 поддерживая при этом ядра 4.x,
   и еще есть проекты (Isar, Isar-Community, Hive) которым требуется такая поддержка.

 - Ошибка `MDBX_WANNA_RECOVERY` при открытии БД в режиме только-чтение теперь возвращается если размер БД не кратен размеру системной страницы,
   но игнорируется не кратность размеру блока выделения виртуальной памяти.
   Этим устраняется регресс, проявившейся вследствие изменения поведения после задействования
   системного вызова `fallocate()` для предотвращения `SIGBUS` после приращения файла БД в заполненной файловой системе.

--------------------------------------------------------------------------------

## v0.13.7 "Дружба" (Friendship) от 2025-07-30.

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов,
в [международный день дружбы](https://www.un.org/ru/observances/friendship-day).

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.
 - [Артёму Воротникову](https://github.com/vorot93) за сообщение об ошибках и тестировании [призязок для Rust](https://github.com/vorot93/libmdbx-rs).

Исправления:

 - Устранена критическая ошибка в функционале `mdbx_env_resurrect_after_fork()` при использовании SysV-семафоров.

   Проявлялась ошибка только после порождения дочернего процесса посредством `fork()` на фоне выполняющейся пишущей транзакции, что
   приводило к неверной работе семафоров и далее к самым различным ошибкам, вплоть до повреждения БД. Проблема существовала начиная с появления
   `mdbx_env_resurrect_after_fork()` и затрагивала OSX, а также POSIX-платформы при сборке с опцией `MDBX_LOCKING=5`.

 - Устранена проблема в API копирования БД на отличных от Linux системах POSIX,
   а также в некоторых случаях при расположении целевого файла на не-локальной файловой системе.
   Проблема проявлялась в основном на OSX, возвратом ошибки `EWOULDBLOCK`/`EAGAIN` (35),
   что обусловлено недочетом/конфликтом блокировок `fcntl(F_SETLK)` и `flock()` в ядре ОС.
   Переработана обработка ошибок захвата файловых блокировок в API копирования на системах POSIX.

 - Устранена ошибка приводившая к неожиданному возврату `MDBX_BAD_DBI` при одновременном старте нескольких транзакций внутри одного процесса после открытия БД.

 - Устранена ошибка приводившая к неожиданному возврату `MDBX_DBS_FULL` при повторном открытии уже открытых таблиц и уже достигнутом лимите открытых DBI-дескрипторов.

 - Исправлена ошибка сборки для платформы Android при явном определении `_FILE_OFFSET_BITS`.

 - Исправлена ошибка использования `ENOMEM` вместо `MDBX_ENOMEM`.
   Что могло ломать сборку на не-POSIX/Windows платформах, в зависимости от конфигурации и/или версии SDK.

 - Поправлено либо удалено несколько неверных assert-проверок, из-за которых происходили падения отладочных сборок в специфических ситуациях.
   Главным образом, в коде функций `txn_end()`, `txn_lock()` и `txn_unlock()` как на Windows, так и на POSIX.

 - Устранены несущественные предупреждения MSVC. Отключены предупреждения `C5286` и `C5287`.

Прочие доработки:

 - Доработана логика отказа от использования OFD-блокировок на POSIX-платформах.
   Теперь кроме `EINVAL` учитываются дополнительные коды ошибок (`ENOSYS`, `ENOIMPL`, `ENOTSUP`, `ENOSUPP`, `EOPNOTSUPP`),
   что позволит работать собранной библиотеке в некоторых случаях,
   когда актуальное ядро/контейнер/эмулятор не поддерживает требуемых системных вызовов.

 - В тестовый фреймворк добавлена поддержка опции --numa # для привязки стохастического теста к NUMA-узлу,
   а в battery/tmux-скрипте добавлено явное распределение по NUMA-узлам, что существенно увеличило КПД
   при тестировании на NUMA-машинах.

 - В стохастическом скрипте реализован случайный порядок запуска отдельных тестов.

--------------------------------------------------------------------------------

## v0.13.6 "Бузина" от 2025-04-22

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов,
в память о погибшем украинском историке и писателе [Алесе Бузине](https://ru.ruwiki.ru/wiki/Бузина,_Олесь_Алексеевич).

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.
 - [Илье Михееву](https://t.me/IlyaMkhv) и команде [Erigon](https://github.com/erigontech) за сообщения о проблеме и тестирование.
 - [Алексею Костюку (aka Keller)](https://t.me/keller18306) за сообщения о проблеме копирования на NFS.

Исправления:

 - Устранён регресс при использовании курсоров для DBI=0 (aka GC/FreeDB) в читающих транзакциях.

   После рефакторинга и ряда оптимизаций для завершения/гашения
   курсоров в читающих и пишущих транзакций, стал использоваться общий код.
   Причем за основу, был взят соответствующий фрагмент относящийся к
   пишущим транзакциям, в которых пользователю не позволяется
   использоваться курсоры для DBI=0 и поэтому эта итераций пропускалась.

   В результате, при завершении читающих транзакциях, курсоры связанные с
   DBI=0 не завершались должным образом, а при их повторном использовании
   или явном закрытии после завершения читающей транзакции происходило
   обращение к уже освобожденной памяти. Если же такие курсоры
   отсоединялись или закрывались до завершения читающей транзакции, то
   ошибка не имела шансов на проявление.

 - Устранён регресс в виде ошибки `EAGAIN` при копировании БД на NFS и CIFS/SMB.

   При доработках/развитии API в функции копирования был добавлен захват
   файловой блокировки посредством как `fcntl()`, так и `flock()`. Однако,
   в зависимости от версии локального ядра, версии удалённого сервера NFS и
   опций монтирования, это могло приводить к возврату POSIX-ошибки `EAGAIN`
   (`11` на большинстве платформ, включая Linux).

 - Устранена ошибка merge/rebase внутри `mdbx_txn_release_all_cursors_ex()`,
   что могло приводить к последующим неожиданным ошибкам `MDBX_EBADSIGN` и утечкам памяти.
   Для проверки сценария дополнен соответствующий тест.

 - Исправлена assert-проверка в пути завершения вложенных транзакций.
   Для проверки сценария дополнен соответствующий тест.

 - Устранена возможность возврата неожиданной ошибки `MDBX_BUSY` из `mdbx_txn_lock(dont_wait=false)`.

 - Для совместимости с GCC 15.x в режиме C23 изменен порядок указания атрибутов функций.

Изменение поведения:

 - При невозможности отвязки курсора от его текущей транзакции функция `mdbx_cursor_bind()`
   теперь возвращает `MDBX_EINVAL` вместо `MDBX_BAD_TXN`.

Прочие доработки:

 - Во избежание потенциальных проблем отключено использование `copy_file_range()` на ядрах Linux 5.3 - 5.18.

 - Вброс `std::invalid_argument` теперь производится явным сообщением `MDBX_EINVAL`.

 - Уточнен тип адреса для пожертвований.
   Ethereum/ERC-20 позволяет перечислять не только ETH, но и другие валюты/токены, в том числе USDC.

 - Дополнен тест курсоров extra/cursor-closing.

 - В `NOTICE` обновлена информация о GitHub.

--------------------------------------------------------------------------------

## v0.13.5 "Труба" от 2025-03-21

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов.

Благодарности:

 - [Erigon](https://erigon.tech/) за спонсорство.
 - [Илье Михееву](https://t.me/IlyaMkhv) за сообщения о недочетах и тестирование.
 - [Alex Sharov](https://github.com/AskAlexSharov) за сообщение об ошибках и тестирование.
 - [maxc0d3r](https://gitflic.ru/user/maxc0d3r) for bug reporting and testing.
 - [Alain Picard](https://github.com/castortech) for support [Java bindings](https://github.com/castortech/mdbxjni) and MacOS universal binaries patch for CMake build scenario,
   also for bug reporting (put-`MDBX_MULTIPLE` regression). Big thank for assistance with debugging and testing.

Новое:

 - Добавлена опция сборки `MDBX_ENABLE_NON_READONLY_EXPORT` позволяющая использовать в режиме чтения-записи БД расположенных в файловых системах экспортированных через NFS.
   По-умолчанию опция выключена и при открытии в неэксклюзивном режиме чтения-записи БД расположенных  файловых системах доступных извне по NFS будет возвращаться ошибка `MDBX_EREMOTE`.
   Включение опции позволяет открывать БД в описанных выше ситуациях, но риск чтения неверных данных на удалённой стороне ложится на пользователя.

 - Поддержка MacOS universal binaries при сборке посредством CMake.

 - Для закрытия или отсоединения всех курсоров с получением их количества в API добавлена функция `mdbx_txn_release_all_cursors_ex()`.

 - Добавлена операция `MDBX_SEEK_AND_GET_MULTIPLE` в API курсора, позволяющая за одну операцию выполнить позиционирование
   курсора на конкретное значение и начать чтение multi-значений в пакетном режиме.

 - Добавлены методы `mdbx::cursor::put_multiple_samelength()`, `mdbx::cursor::seek_multiple_samelength()`, `mdbx::cursor_managed::withdraw_handle()`.

 - В политику управления выделением для `mdbx::buffer<ALLOCATOR, CAPACITY_POLICY>` добавлен параметр `inplace_storage_size_rounding`.
   Одновременно с этим переработан внутренний union-тип `mdbx::buffer<ALLOCATOR, CAPACITY_POLICY>::silo::bin` для возможности увеличения без пенальти встроенного в экземпляр буфера места под данные.

 - В API добавлена функция `mdbx_cursor_close2()` возвращающая код ошибки.

Исправления:

 - Устранение лишнего/ненужного предупреждения в сценарии изменения размера БД посредством вызова `mdbx_env_set_geometry()` до её открытия.
   API предусматривает возможность запросить изменение геометрии/размера БД перед её открытием, чтобы избежать как лишних накладных расходов,
   так и потенциальных ошибок из-за нехватки адресного пространства. В этом сценарии ранее могло выдаваться лишнее/ненужное предупреждение
   о несоответствии файла БД новому размеру. Теперь этот недостаток исправлен.

 - Восстановлена доступность дескрипторов таблиц, открытых в дочерней транзакции, после её фиксации, в случае отсутствия изменений в данных.
   Проблема не была замечена ранее из-за специфического сценария проявления.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранён сбой аудита таблиц при инвалидации дескрипторов таблиц вследствие отмены вложенной транзакции.
   Проблема не была замечена ранее из-за специфического сценария проявления.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранена причина потенциальных сбоев и/и деградации производительности в сценарии закрытия курсора до завершения вложенной транзакции,
   с последующим изменением данных той-же таблицы в текущей вложенной транзакции, либо её дочерних транзакциях.
   Проблема обнаружена при ручном анализе кода, сценарии воспроизведения/проявления проблемы пока не известны.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `3de3d425a128a3c6f7866503f5f93b80c09dbe41` от 2024-05-19.

 - Устранена причина ложных ошибок при работе `mdbx_chk` с высоким уровнем логирования.
   Проблема возникала из-за неверной трактовки `MDBX_NOTFOUND` при штатном окончании итерируемых данных.

 - Устранена причина попыток рекурсивного захвата мьютекса при работе `mdbx_chk -w` в сборах с поддержкой Valring/ASAN и под управлением этих инструментов.

 - Устранены проверки потока владеющего транзакцией при сборке с опцией `MDBX_TXN_CHECKOWNER=OFF`.

 - Устранена вероятность ситуации гонки в `tbl_setup(MDBX_DUPFIXED | MDBX_INTEGERDUP)` при работе в разных потоках.
   В реальных сценариях вероятность проявления проблемы была близка к нулю.
   Для подробностей смотрите комментарий коммита `3e91500fac475947f5b58268d5edd3c9cc4f77f6`.

 - Устранён регресс затенения курсоров во вложенных транзакциях.
   При реализации отложенной/ленивой инициализации dbi-дескрипторов также было реализовано отложенное затенение курсоров (создание копии состояния для отката при прерывании транзакции),
   что существенно уменьшало накладные расходы при старте и завершении вложенных транзакций в сценариях с большим количеством курсоров.
   Однако, была допущена логическая ошибка, вследствие которой отложенная инициализация и затенение выполнялись при использовании dbi-дескрипторов, но не курсора открытого в родительской транзакции.
   В результате, родительские курсоры во вложенных транзакциях могли не затеняться, что приводило к неконсистентному состоянию в случае
   прерывания/откате вложенной транзакции и в соответствующей таблицы были изменения в рамках прерванной вложенной транзакции.
   Проблема не реализовывалась в тестовых сценариях и не была замечена при эксплуатации, но была обнаружена при расширении тестов.
   Ошибка присутствует в версиях 0.13.x и последующих, начиная с коммита `e6af7d7c53428ca2892bcbf7eec1c2acee06fd44` от 2023-11-05.

 - Устранён регресс в пути обработки операции `MDBX_MULTIPLE`.
   Пакетная вставка значений посредством `MDBX_MULTIPLE` могла приводить к падениям и повреждению структуры БД. Ошибка оставалось не
   замеченной из-за специфических условий проявления, которые не реализовались в тестах.
   Проблема присутствовала во всех выпусках начиная с v0.13.1, но соответствующая ошибка не связана с конкретным коммита в истории, а
   является следствием нескольких доработок (шагов рефакторинга), которые суммарно привели к регрессу.
   Технически ошибка обусловлена не-обнулением переменной, чего не происходило в некотором пути выполнения, так как исходно не требовалось.
   Однако, такое обнуление потребовалось после ряда этапов оптимизации и рефакторинга смежных участков кода.
   Для подробностей смотрите комментарий коммита `23a417fe19614481c6546845995d6dc845baf797`.

 - Скорректировано описание ошибки `MDBX_MVCC_RETARDED` и текста соответствующего сообщения.

 - В C++ API добавлена упущенная проверка `__cpp_concepts >= 202002` для использования концептов C++.

Изменение поведения:

 - Функция `mdbx_txn_release_all_cursors()` возвращает только код ошибки, не смешивая его с количеством обработанных/закрытых курсоров.
   Для аналогичных действий с получением количества закрытых курсоров в API добавлена функция `mdbx_txn_release_all_cursors_ex()`.

 - Использование системного кода ошибки `EREMOTEIO` ("Remote I/O error") вместо `ENOTBLK` ("Block device required") в качестве `MDBX_EREMOTE` для индикации ошибочной ситуации открытия БД расположенной на сетевом носителе.

 - Для основных вариантов использования шаблона `mdbx::buffer<>` теперь явно инстанцируются внутри библиотеки,
   одновременно соответствующие специализации шаблона помечены как `external` для предотвращения повторного инстанцирования в пользовательском коде.

 - Запрещена отвязка/открепление курсоров во вложенных транзакциях, т.е. вызовы `mdbx_cursor_unbind()` и
   `mdbx_txn_release_all_cursors(unbind=true)` для курсоров открытых в одной из родительских транзакций.
   Причина в том, что в случае отмены вложенной транзакции возникает неконструктивная неопределенность
   — следует ли восстанавливать состояние курсоров. Если не восстанавливать, то получается что вложенная транзакция может
   поломать родительскую, сделав её продолжение невозможным. Если восстанавливать, то также следует «воскрешать» закрытые
   курсоры, что неизбежно приведет к путанице, утечкам памяти и использованию после освобождения.

 - В C++ API отменён вброс исключения при запросе транзакции у отсоединённого курсора посредством вызова `mdbx::cursor::txn()`.

Прочие доработки:

 - Доработка использования LTO в CMake-сценариях: использование `-flto=auto` для GCC >= 11.4,
   расслабление условий для включения LTO для CLANG на Linux, расширение поиска `LLVMgold.so` в относительных lib-директориях.

 - Добавлены дополнительные проверки сигнатур курсоров при итерации связанных списков.

 - Кратное сокращение итераций тестов в зависимости от конфигурации Valgrind/Debug/CI.

 - Устранены предупреждения UBSAN о невыравненном доступе в тесте extra/close-dbi.

 - Добавлен перехват и логирование исключений в extra-тестах на C++.

 - Расширены тесты extra/dupfix-multiple, extra/cursor-closing и extra/txn.

 - В утилиту тестирования добавлена поддержка режима/опции `MDBX_VALIDATION` и поддержка значений `on`/`off` для опций командной строки.

 - Добавлены doxygen-описания для doubtless-positioning констант.

 - Переработана проверка курсоров на входе в API-функций с добавлением `cursor_check()`, `cursor_reset()` и `cursor_drown()`.

 - Отключено использование C23 `[[атрибутов]]` для версий CLANG меньше 20.

--------------------------------------------------------------------------------

## v0.13.4 "Sigma Boy" от 2025-02-14

Поддерживающий выпуск стабильной ветки с исправлением обнаруженных ошибок и устранением недочётов.

Благодарности:

 - [Erigon](https://docs.erigon.tech/) за спонсорство.
 - [Алексею Костюку (aka Keller)](https://t.me/keller18306) за сообщения об ошибках и недочетах.
 - [Alain Picard](https://github.com/castortech) for support [Java bindings](https://github.com/castortech/mdbxjni) and MacOS universal binaries patch for CMake build scenario.
 - [Alex Sharov](https://github.com/AskAlexSharov) за сообщение об ошибках и тестирование.
 - [Виктору Логунову](https://t.me/vl_username) за сообщение об опечатки в имени переменной в Conan-рецепте.

Новое:

 - Поддержка MacOS universal binaries при сборке посредством CMake.

Исправления:

 - Устранён регресс допускающий SIGSEGV в операциях обновления после вытеснения/spilling страниц в больших транзакциях.
   Ошибка присутствует в выпусках v0.13.1, v0.13.2, v0.13.3 и оставалась незамеченной из-за специфических условий и низкой вероятности проявления.
   Более подробная информация в описании коммита `21630ea115690a5cb39cfa921f9d199271a08102`.

 - Исправлена опечатка в документации в упоминании `mdbx_env_resurrect_after_fork()`.

 - Исправлена опечатка в условном операторе внутри `cursor_touch()`.
   При переделке курсоров было пропущено отрицание в условии, при оценке количества страниц, которые могут потребоваться для выполнения операции.
   В текущем понимании ошибка не приводила к каким-либо проблемам, ибо оценка делает по верхней границе с существенным запасом, а в худшем
   случае это могло приводить к прерыванию транзакции из-за достижения ограничения на кол-во грязных страниц.

 - Корректировка излишне строгого условия в assert-проверке внутри `recalculate_subpage_thresholds()`.
   Ошибка могла проявляться только в отладочных сборках при выставлении определенной комбинации предельных значений опций `MDBX_opt_subpage_limit`,
   `MDBX_opt_subpage_room_threshold`, `MDBX_opt_subpage_reserve_prereq`, `MDBX_opt_subpage_reserve_limit`.

 - Исправление опечатки в Conan-рецепте в коде протокольно-отладочного вывода в имени переменной `version_json_pathname`.

--------------------------------------------------------------------------------

## v0.13.3 "Королёв" от 2025-01-12

Поддерживающий выпуск с исправлением обнаруженных ошибок и устранением недочетов
в день рождения и в память об [Серге́е Па́вловиче Королёве](https://ru.wikipedia.org/wiki/Королёв,_Сергей_Павлович),
советском учёном и Главном конструкторе ракетно-космических систем.

Одновременно с этим релизом:

 - Ветка `0.12.x` перестаёт поддерживаться и отправляется в `архив/0.12`.
 - Ветка `0.13.x` получает статус стабильной и вливается в `stable`.

Благодарности:

 - [Алексею Костюку (aka Keller)](https://t.me/keller18306) за сообщения об ошибках и недочетах.
 - [Erigon](https://docs.erigon.tech/) за спонсорство.
 - [Lazymio](https://github.com/wtdcode) за новые [привязки к Python](https://pypi.org/project/libmdbx/).

Новое:

 - В C API добавлена функция `mdbx_cursor_count_ex()` позволяющая получить как количество мульти-значений
   соответствующих текущему ключу, так и информацию о вложенном дереве хранящем эти значения.

 - В C++ API добавлен метод `mdbx::txn::make_broken()` аналогичный `mdbx_txn_break()`.

 - В утилиты mdbx_copy, mdbx_drop, mdbx_dump, mdbx_load, mdbx_stat добавлено логирование ошибок,
   предупреждений и важных сообщений от libmdbx.

Исправления:

 - Устранён регресс неразблокировки мьютекса при попытки повторного закрытия dbi-дескриптора,
   в том числе при попытке явно закрыть дескриптор после удаления связанной с ним таблицы.

   Вместо возврата ошибки `MDBX_BAD_DBI` происходил выход из тела функции по успешному пути,
   но без освобождения захваченной блокировки.
   Ошибка была внесена 2024-10-23 коммитом `3049bb87b5b14d83b16d121c186ce8fb3f21383e`.

 - Устранён регресс состояния вложенного/dupsort курсора после вставки данных в `MDBX_APPEND`-режиме.

   При добавлении нового ключа в append-режиме, в случае когда в текущей
   (последней) позиции с ключом связаны несколько значений и
   (соответственно) вложенный dupsort-курсор инициализирован, вставка
   происходила без сброса вложенного курсора.

   В результате вложенный курсор логически оставался стоять на
   multivalue-данных связанных с предыдущей позицией основного курсора,
   т.е. переходил в неконсистентное состояние.

   Ошибка проявлялась возвратом неверных значений из `mdbx_cursor_count()`
   или срабатывание assert-проверки в отладочных сборках.

 - Поддержка получения boot_id при работе внутри LXC-контейнера.

   Из LXC-контейнера не доступен файл хостовой системы `/proc/sys/kernel/random/boot_id`.
   Вместо него, при каждом старте контейнера, создается и заполняется
   случайными данными собственный boot_id смонтированный через bind из `tmpfs`.
   https://github.com/lxc/lxc/issues/3027

   Ранее этот подставной/замещенный boot_id отбраковывался внутри libmdbx,
   так как файл располагается в `tmpfs`, а не в файловой системе `/proc`.
   В результате boot_id для проверки целостности БД не был доступен.
   Теперь при работе внутри LXC-контейнера такой bootid будет использоваться.

   Однако, полноценно работающий контроль по boot_id не возможен, так как при
   рестарте LXC-контейнера (но не хоста) boot_id будет меняться, хотя
   данные в unified page cache сохраняются.

   Таким образом, при рестарте LXC-контейнера без рестарта хоста, libmdbx придется
   откатить состояние БД до крайней точки устойчивой фиксации, что повлечет
   утрату данных пользователя в случаях когда они могли быть сохранены.
   Однако, улучшить ситуацию пока не представляется возможным, как минимум
   до доступности boot_id хостовой системы изнутри LXC-контейнера.

 - Устранена ошибка неверной обработки попытки запуска вложенной читающей транзакции.
   Теперь в таких ситуациях возвращается ошибка `MDBX_EINVAL`, так как вложенность
   поддерживается только для транзакций чтения-записи.

   Ошибка была внесена при рефакторинге, коммитом `2f2df1ee76ab137ee66d00af69a82a30dc0d6deb`
   чуть более 5 лет назад и долго оставалось не замеченной.

 - Устранён SIGSEGV-регресс обращения к нулевому адресу при работе в режиме только-чтение без использования LCK-файла,
   например при размещении БД на носителе доступном только для чтения.

   Ошибка была внесена при реализации функционала парковки читающих транзакций.

Изменение поведения:

 - Теперь при включении профилирования GC (сборка с опцией `MDBX_ENABLE_PROFGC=ON`)
   подсчитываются затраты времени ЦПУ на слияние списков страниц, т.е. на работу функции `pnl_merge()`.

 - В утилите тестирования значение режима данных переименовано из `data.dups` в `data.multi`.

 - Доработан контроль длины ключа внутри `cursor_seek()`.

   Ранее проверка внутри `cursor_seek()` не позволяла искать ключи длиннее, чем можно поместить в таблицу.
   Однако, при поиске/позиционировании это не является ошибкой для таблиц с ключами переменного размера.

 - Если посредством `mdbx_env_set_option(MDBX_opt_txn_dp_limit)` пользователем не задано собственно значение,
   то теперь выполняется подстройка dirty-pages-limit при старте каждой не-вложенной пишущей транзакций,
   исходя из объёма доступного ОЗУ и размера БД.

 - Теперь в режиме `MDBX_NOSTICKYTHREADS` допускается commit/abort вложенных транзакций из любого треда/потока.

 - Теперь при попытке запуска вложенных транзакций в режиме `MDBX_WRITEMAP` производится
   логирование и возврат ошибки `MDBX_INCOMPATIBLE`.

 - Теперь в C++ API в конструкторах/инициализаторах и методах связанных с формированием геометрии БД
   по-умолчанию используются только `default`-значений, в том числа для минимального и максимального
   размера. В результате, при создании БД с геометрией по-умолчанию не происходит выбор максимального
   размера страницы из-за очень большого максимального размера БД.

 - Внутри `mdbx_env_set_geometry()` доработаны эвристики для подбора параметров геометрии БД запрошенных пользователем "по-умолчанию".

--------------------------------------------------------------------------------

## v0.13.2 "Прошлогодний Снег" (Last Year's Snow) от 2024-12-11

Поддерживающий выпуск с исправлением обнаруженных ошибок и устранением недочетов
в день рождения и в память об [Алекса́ндре Миха́йловиче Тата́рском](https://ru.wikipedia.org/wiki/Татарский,_Александр_Михайлович),
российском режиссёре-мультипликаторе, создавшем такие знаменитые
мультфильмы как "Падал прошлогодний снег", "Пластилиновая ворона",
заставку "Спокойной ночи, малыши!" и многие другие шедевры нашего
детства.

Новое:

 - Ускорено обновление GC при возврате/помещении списков страниц в
   сложных сценариях. Был доработан и активирован ранее отключенный
   экспериментальный режим корректирующей обратной связи. Этим
   принципиально улучшилась сходимость (сократилось количество повторных
   попыток), а также устранен дефект приводящий к "зацикливанию" при
   фиксации транзакций (с возвратом ошибки `MDBX_PROBLEM`) в редких
   специфических условиях.
   Подробности см. в описании коммита [`6c56ed97bbd8ca46abac61886a113ba31e5f1291`](https://gitflic.ru/project/erthink/libmdbx/commit/6c56ed97bbd8ca46abac61886a113ba31e5f1291).
 - Включен стандарт `C23` в CMake-скриптах сборки.
 - Добавлены T-макросы для парных `char`/`wchar_t` функций.
 - Поддержка вложенных пишущих транзакций в C++ API.
 - Переход на "Semantic Versioning 2" и экспорт информации о версии в `VERSION.json`.
 - Добавлена поддержка переменной среды `SOURCE_DATE_EPOCH` для воспроизводимости сборок.
   Прежний способ посредством `MDBX_BUILD_TIMESTAMP` также работает и имеет приоритет.
 - Добавлена возможность указывать дополнительную информацию о сборке
   libmdbx через опцию `MDBX_BUILD_METADATA`. Сейчас задаваемая информация
   просто включается внутрь библиотеки в качестве значения
   `mdbx_build.metadata`, а в дальнейшем также будет использоваться при
   формировании пакетов и т.п.
 - Добавлено логирование ошибок возвращаемых из API. Теперь для этого
   достаточно задать уровень логирования `MDBX_LOG_DEBUG` (для логирования
   ошибок за вычетом `MDBX_NOTFOUND`) или `MDBX_LOG_TRACE` (для логирования
   всех ошибок, а также `MDBX_RESULT_TRUE`).
 - Поддержка сборки посредством Conan.

Изменение поведения:

 - Добавлен метод `mdbx::cursor::get_multiple_samelength()` и
   переименован `mdbx::txn::put_multiple_samelength()`.
 - Для единообразия C++ API при выполнении операции `MDBX_GET_MULTIPLE`
   теперь также возвращается значение самого ключа.
 - Для размерных констант `mdbx::env::geometry` базовый тип изменен с
   беззнакового `size_t` на знаковый `intptr_t`.
 - Теперь выбор в пользу использования ntdll вместо CRT делается только
   при явном отключении C++ API.
 - Теперь выполняется освобождение памяти сброшенных/прерванных читающих
   транзакций передаваемых в `mdbx_txn_commit()`. Соглашение по API требует
   чтобы такие транзакции освобождались посредством `mdbx_txn_abort()`,
   из-за чего функция `mdbx_txn_commit()` возвращала ошибку в таких
   случаях, не разрушая сами транзакции. Это приводило к утечкам памяти
   из-за ошибок в приложениях, что побудило изменить поведение.
 - Использование макроса `__deprecated_enum` если он определен.
 - При сборке посредством CMake выбор стандарта языка `C` теперь
   выполняется с учётом `CMAKE_C_STANDARD`.
 - Изменения в опциях сборки:
   * опция `MDBX_OSX_SPEED_INSTEADOF_DURABILITY` переименована в
     `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY`, так как актуальна для всех
     "Яблочных" платформ;
   * опция `MDBX_MMAP_USE_MS_ASYNC` переименована в
     `MDBX_MMAP_NEEDS_JOLT`, для более точного соответствия своей семантики;
   * в CMake добавлена поддержка опции `MDBX_USE_MINCORE`;
   * использование `madvise()` и родственных системных вызовов теперь
     всегда включено, а опция `MDBX_ENABLE_MADVISE` удалена;
   * удалены неиспользуемая опция `MDBX_USE_SYNCFILERANGE` и
     неиспользуемый режим `MDBX_LOCKING_BENAPHORE`.

Исправления:

 - Устранен регресс возврата неверной информации из функций
   `mdbx_env_stat_ex()` и `mdbx_env_stat()`. При рефакторинге до выпуска
   v0.13.1 была допущена ошибка, из-за которой выполнялось суммирование
   значений без очистки переданного пользователем буфера для результата.
   Таким образом, возвращаемая информация была верной, только если память
   используемая для размещения результата содержала нули на момент вызова
   функции.
 - Функция `mdbx_close_dbi()` доработана для возврата ошибки `MDBX_DANGLING_DBI`
   при попытке закрыть dbi-дескриптор таблицы, созданной и/или измененной в
   ещё выполняющейся транзакции. Такое преждевременное закрытие дескриптора
   является неверным использованием API и нарушением контракта/предусловий
   сформулированных в описании `mdbx_close_dbi()`. Однако, вместо возврата
   ошибки выполнялось некорректное закрытие дескриптора, что могло
   приводить к созданию таблицы с пустым именем, утечки страниц БД и/или
   нарушению структуры b-tree (неверной ссылкой на корень таблицы).
 - Исправлено открытие таблицы с пустым/нулевым именем, в том числе устранена
   возможность `SIGSEGV` при закрытии её дескриптора.
 - Добавлены упущенные inline-реализации `mdbx::cursor::upper_bound()` и `mdbx::cursor::upper_bound_multivalue()`.
 - Продолжена корректировка описания С++ API для использования термина "таблица" вместо "sub-database".
 - Исправлено проверяемое условие внутри `assert()` в пути обработки `MDBX_GET/NEXT/PREV_MULTIPLE`.
 - На 32-битных платформах разрешено использовать 4-байтное выравнивание при получении 64-битных значений посредством `MDBX_MULTIPLE`.
 - Добавлен костыль для устранения проблем из-за некорректной обработки `[[gnu::pure]]` в Apple Clang и MSVC.
 - Поправлено определение `MDBX_DEPRECATED_ENUM` для старых компиляторов при включении С++11.
 - Доработано использование `std::experimental::filesystem` для решения проблем со сборкой в старых компиляторах.
 - Исправлена обработка `MDBX_GET_MULTIPLE` в специальных случаях и одного значения у ключа в позиции курсора.
 - Исправление сборки при включении профилирования GC и `MDBX_ENABLE_DBI_SPARSE=OFF`.

Мелочи:

 - Теперь `MDBX_ENABLE_BIGFOOT` включена по-умолчанию вне зависимости от разрядности платформы.
 - Дополнение README и исправление опечаток/орфографии.
 - Использование `WIN32` вместо `${CMAKE_SYSTEM_NAME}`.
 - Подавление параноидальных предупреждений MSVC в extra-тестах.
 - Дополнение отладочного логирования внутри `dxb_resize()`.
 - Добавление в сценарии CMake/CTest копирования dll под  Windows для работы исключений в тестах на C++.
 - Добавление С++ теста `extra/open`.
 - Доработка `osal_jitter()` для уменьшения задержек в тестах под Windows.
 - Исправление максимальной длины значений в тесте `extra/crunched-delete`.
 - Добавление логирования С++ исключений в `extra/dupfix_multiple`.
 - Корректировка API-макросов для Doxygen.
 - Уточнение описания `mdbx_dbi_close()` для случая хендлов измененных таблиц.
 - Добавление теста `extra/early_close_dbi`.
 - Доработка скрипта стохастического теста и его переименование в `stochastic.sh`.
 - Доработка тестов для совместимости с режимами сборки до С++17.
 - Добавление `.WAIT` для устранения коллизий при распараллеливании сборки посредстом GNU Make 4.4.

--------------------------------------------------------------------------------

## v0.13.1 "РДС-1" от 2024-08-29

Новая версия со сменой лицензии, существенным расширением API,
добавлением функционала и внутренними переработками. В том числе,
с незначительным нарушением обратной совместимости API библиотеки.

Новое:

 - Изменение лицензии на Apache 2.0, пояснения и подробности в файле `COPYRIGHT`.

 - Реструктуризация исходного кода с рефакторингом.

 - Переработка курсоров для унификации поведения, более регулярного
   кода, уменьшения количества ветвлений и машинных операций.

 - Перенос функционала утилиты `mdbx_chk` внутрь библиотеки в виде
   функции `mdbx_env_chk() `для проверка целостности структуры БД, в том
   числе с вовлечением логики приложения.

 - Опция `MDBX_opt_gc_time_limit` для более гибкого контроля времени
   расходуемого на поиск последовательностей соседствующих свободных
   страниц в GC.

 - Снижение накладных расходов на запуск транзакций в сценариях с
   большим количеством DBI-хендов, за счет отложенной/ленивой инициализации
   элементов служебных таблиц. В том числе, механизм поддержки разреженных
   наборов DBI-хендов, управляемый опцией сборки `MDBX_ENABLE_DBI_SPARSE`,
   которая включена по-умолчанию.

 - Снижение накладных расходов на открытие DBI-хендов. В том числе,
   механизм отложенного освобождения и поддержки быстрого пути открытия без
   использования блокировок, управляемый опцией сборки
   `MDBX_ENABLE_DBI_LOCKFREE`, которая включена по-умолчанию.

 - Поддержка "парковки" читающих транзакций с их вытеснением ради
   переработки старых MVCC-снимков и предотвращения проблем вызываемых
   приостановкой переработки мусора. Механизм парковки и вытеснения
   припаркованных транзакций является как дополнением, так и более простой
   в использовании альтернативой обратному вызову
   [Handle-Slow-Readers](https://libmdbx.dqdkfa.ru/group__c__err.html#ga2cb11b56414c282fe06dd942ae6cade6).
   Для удобства функции `mdbx_txn_park()` и `mdbx_txn_unpark()` имеют
   дополнительные аргументы, позволяющие запросить автоматическую
   "распарковку" припаркованных и перезапуск вытесненных транзакций.

 - Расширение API позиционирования курсоров более удобными и очевидными
   операциями по аналогии условиям `<`, `<=`, `==`, `>=`, `>` как для
   ключей, так и для пар ключ-значение.

 - Функции `mdbx_dbi_rename()` и `mdbx_dbi_rename2()` для переименования таблиц.

 - Функции `mdbx_cursor_unbind()` и `mdbx_txn_release_all_cursors()` для
   гибкого управления курсорами в сценариях повторного использования для
   уменьшения накладных расходов.

 - Функция `mdbx_env_resurrect_after_fork()` для восстановление открытой
   среды работы с БД в дочернем процессе после ветвления/расщепления
   процесса.

 - Функция `mdbx_cursor_compare()` для сравнения позиций курсоров
   аналогично оператору `<=>`.

 - Функции `mdbx_cursor_scan()` и `mdbx_cursor_scan_from()` для
   сканирования таблиц с использованием функционального предиката и
   уменьшением сопутствующих накладных расходов.

 - Функции `mdbx_cursor_on_first_dup()` и `mdbx_cursor_on_last_dup()`
   для оценки позиции курсора.

 - Функция `mdbx_preopen_snapinfo()` для получения информации о БД без
   её открытия.

 - Функция `mdbx_enumerate_tables()` для получение информации
   об именованных пользовательских таблицах.

 - Поддержка функций логирования обратного вызова без функционала
   `vprintf()`, что существенно облегчает использование логирования в
   привязках к другим языкам программирования.

 - Режим работы `MDBX_NOSTICKYTHREADS` вместо `MDBX_NOTLS` для упрощения
   интеграции с легковесными потоками/нитями их мультиплексирования вместе
   с транзакциями по потокам операционной системы.

 - Опция `MDBX_opt_prefer_waf_insteadof_balance`.

 - Опции `MDBX_opt_subpage_limit`, `MDBX_opt_subpage_room_threshold`,
   `MDBX_opt_subpage_reserve_prereq`, `MDBX_opt_subpage_reserve_limit`.

 - Управление основной блокировкой lock/unlock/upgrade/downgrade для координации пишущих транзакций.

 - Функции `mdbx_limits_keysize_min()` и `mdbx_limits_valsize_min()` для
   получения нижней границы длины ключей и значений.

 - Для идентификации БД добавлен UUID доступный в поле `mi_dxbid` структуры `MDBX_envinfo`,
   получаемой посредством `mdbx_env_info_ex()`.

 - Расширение API функциями lock/unlock/upgrade/downgrade основной блокировки.

 - Добавление в API функций `mdbx_cursor_unbind()` и `mdbx_txn_release_all_cursors()`.

 - Добавление в API функций `mdbx_txn_copy2pathname()` и `mdbx_txn_copy2fd()`.

 - Добавление в утилиту `mdbx_copy` опций `-d` и `-p`.

 - Расширение и доработка C++ API:

     - добавлен тип `mdbx::cursor::estimate_result`, а поведение методов
      `mdbx::cursor::estimate()` унифицировано с `mdbx::cursor::move()`;
     - для предотвращения незаметного неверного использования API, для инициализации
       возвращаемых по ссылке срезов, вместо пустых срезов задействован `mdbx::slice::invalid()`;
     - добавлены дополнительные C++ операторы преобразования к типам C API;
     - для совместимости со старыми стандартами C++ и старыми версиями STL перенесены
       в public классы `mdbx::buffer::move_assign_alloc` и `mdbx::buffer::copy_assign_alloc`;
     - добавлен тип `mdbx::default_buffer`;
     - для срезов и буферов добавлены методы `mdbx::buffer::hex_decode()`, `mdbx::buffer::base64_decode()`, `mdbx::buffer::base58_decode()`;
     - добавлен тип `mdbx::comparator` и функций `mdbx::default_comparator()`;
     - добавлены статические методы `mdbx::buffer::hex()`, `mdbx::buffer::base64()`, `mdbx::buffer::base58()`;
     - для транзакций и курсоров добавлены методы `get_/set_context`;
     - добавлен метод `mdbx::cursor::clone()`;
     - поддержка base58 переработана и приведена в соответствии с черновиком RFC,
       в текущем понимании теперь это одна из самых высокопроизводительных реализаций base58;
     - переработка `to_hex()` и `from_hex()`;
     - добавлены перегрузи со `std::string_view` для методов `open_map`/`create_map`/`drop_map`/`clear_map`/`rename_map()`;
     - добавлены перегрузки методов put/insert/upsert для `mdbx::pair`;
     - добавлены методы принимающие имена таблиц/subDb через `mdbx::slice`.

Нарушение совместимости:

 - Использование термина "таблица" вместо "subDb".
 - Опция `MDBX_COALESCE` объявлена устаревшей, так как соответствующий функционал всегда включен начиная с предыдущей версии 0.12.
 - Опция `MDBX_NOTLS` объявлена устаревшей и заменена на `MDBX_NOSTICKYTHREADS`.
 - Опция сборки `MDBX_USE_VALGRIND` заменена на общепринятую `ENABLE_MEMCHECK`.
 - В структуре `MDBX_envinfo` серии полей вида `meta1`, `meta2` и `meta3` заменены на массивы вида `meta[3]`.
 - В шаблонных классах и функциях С++ API по-умолчанию вместо `mdbx::legacy_buffer` использован тип `mdbx::default_buffer` использующий полиморфные аллокаторы С++ 17.
 - Удаление `DEFAULT_MAPSIZE` и изменение геометрии по-умолчанию при создании БД.
 - Возвращение `MDBX_TXN_INVALID` (`INT32_MIN`) вместо `-1`
   из `mdbx_txn_flags()` при передаче невалидной транзакции.

Исправления:

 - Исправление упущенного `TXN_END_EOTDONE` при сбое старта читающей транзакции.
   Упомянутый флажок отсутствовал в пути разрушения транзакции при ошибке
   её запуска. Из-за чего делалась попытка разрушить курсоры, что приводило
   к падению **отладочных сборок**, так как в них соответствующий массив
   намеренно заполнен некорректными указателями.

 - Устранение возможности `SIGSEGV` внутри `coherency_check()` после
   изменения геометрии другим процессом с увеличением верхнего размера БД
   и увеличением БД больше предыдущего лимита.

 - Исправление assert-проверки при попытке создания таблицы с другими флагами/опциями.

Мелочи:

 - Обновление конфигурации Doxygen до 1.9.6.
 - Добавление `--read-var-info=yes` для Valgrind.
 - Вывод из `mdbx_chk` информации об уровне детализации/verbosity.

## v0.13.0 от 2023-04-23

Технический тэг, отмечающий начало ветки `0.13`
с новым функционалом и изменением API.

********************************************************************************

For early releases and changes see the ChangeLog-NN the git commit history.
