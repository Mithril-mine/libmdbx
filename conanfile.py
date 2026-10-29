import json
import os
import re
import subprocess
from conan.tools.files import rm
from conan.tools.apple import is_apple_os
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan import ConanFile
required_conan_version = '>=2.7'

def semver_parse(s):
    m = re.match(r'^v?(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(\\.(?P<tweak>0|[1-9]\d*))?(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$', s)
    return m.groupdict() if m else None

def semver_string(semver):
    s = str(semver['major']) + '.' + \
        str(semver['minor']) + '.' + str(semver['patch'])
    if semver['tweak'] is not None and semver['tweak'] != 0:
        s += '.' + str(semver['tweak'])
    if semver['prerelease'] is not None and semver['prerelease'] != '':
        s += '-' + semver['prerelease']
    return s

def semver_string_with_buildmetadata(semver):
    s = semver_string(semver)
    if semver['buildmetadata'] is not None and semver['buildmetadata'] != '':
        s += '+' + semver['buildmetadata']
    return s

class libmdbx(ConanFile):
    """Libmdbx packaged as a prebuilt binary (convenience route).

    The recommended consumption route is source-level (amalgamated sources,
    add_subdirectory or FetchContent); see README 'Packaging / Conan'. This
    package is compiled by the active Conan profile's toolchain, so consumers
    must use exactly the same compiler/stdlib/ABI, otherwise the C++ ABI can
    split between the consumer-compiled mdbx.h++ header and the packaged
    out-of-line definitions (extern template instantiations). ABI-affecting
    options: 64bit_atomic, 64bit_cas, mmap_incoherent_file_write,
    unaligned_ok, cacheline_size, without_msvc_crt, build_cxx, cxx_standard.

    Note: on the roadmap the C++ API will become the native one, with the C
    API provided as a secondary interface (an emulation facade) over it. That
    transition is expected to be long and is NOT the case yet: currently the
    C core and the C ABI are first-class, and the toolchain-match requirement
    above applies mostly to consumers of the C++ binding. Once the transition
    lands, the requirement will apply to every consumer of a prebuilt package.

    Binary identity is the hash of the active profile settings and every
    recipe option, including mdbx.lto (off/on/Fat) and mdbx.sanitize; warnings
    and the optimization level inside a build type are deliberately excluded.
    Full audit table: README 'Packaging / Conan'.
    """
    name = 'mdbx'
    package_type = 'library'
    description = 'One of the fastest embeddable key-value ACID database without WAL. libmdbx surpasses the legendary LMDB in terms of reliability, features and performance.'
    license = 'Apache-2.0'
    author = 'Leo Yuriev <leo@yuriev.ru>'
    homepage = 'https://libmdbx.dqdkfa.ru'
    url = 'https://git@git.sourcecraft.dev/dqdkfa/libmdbx.git'
    topics = ('embedded-database', 'key-value', 'btree', 'LMDB', 'storage-engine',
              'data-storage', 'nosql', 'ACID', 'MVCC', 'MDBX')
    no_copy_source = True
    test_type = 'explicit'
    build_policy = 'missing'
    revision_mode = 'scm'
    languages = 'C', 'C++'
    provides = 'libmdbx'
    implements = ['auto_shared_fpic']
    # upload_policy = 'skip'
    # exports_sources = 'LICENSE', 'NOTICE', 'CMakeLists.txt', '*.h', '*.h++', '*.c', '*.c++', 'ntdll.def', 'man1/*', 'cmake/*', 'config.h.in'

    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'mdbx.64bit_atomic': ['Auto', True, False],
        'mdbx.64bit_cas': ['Auto', True, False],
        'mdbx.apple.speed_insteadof_durability': ['Default', True, False],
        'mdbx.avoid_msync': ['Auto', True, False],
        'mdbx.build_cxx': ['Default', True, False],
        'mdbx.build_tools': ['Default', True, False],
        'mdbx.cacheline_size': ['Auto', 16, 32, 64, 128, 256],
        'mdbx.cxx_standard': ['Default', '11', '14', '17', '20', '23'],
        'mdbx.disable_gnu_source': ['Default', True, False],
        'mdbx.disable_validation': ['Default', True, False],
        'mdbx.enable_bigfoot': ['Default', True, False],
        'mdbx.enable_dbi_lockfree': ['Default', True, False],
        'mdbx.enable_dbi_sparse': ['Default', True, False],
        'mdbx.enable_pgop_stat': ['Default', True, False],
        'mdbx.enable_pget_stat': ['Default', True, False],
        'mdbx.enable_profgc': ['Default', True, False],
        'mdbx.enable_refund': ['Default', True, False],
        'mdbx.env_checkpid': ['Default', True, False],
        'mdbx.force_assertions': ['Default', True, False],
        'mdbx.have_builtin_cpu_supports': ['Auto', True, False],
        'mdbx.locking': ['Auto', 'WindowsFileLocking', 'SystemV', 'POSIX1988', 'POSIX2001', 'POSIX2008'],
        'mdbx.lto': ['Default', True, False, 'Fat'],
        'mdbx.mmap_incoherent_file_write': ['Auto', True, False],
        'mdbx.mmap_needs_jolt': ['Auto', True, False],
        'mdbx.native_seh': ['Default', True, False],
        'mdbx.sanitize': ['none', 'address', 'undefined', 'address,undefined'],
        'mdbx.trust_rtc': ['Default', True, False],
        'mdbx.txn_checkowner': ['Default', True, False],
        'mdbx.unaligned_ok': ['Auto', True, False],
        'mdbx.use_copyfilerange': ['Auto', True, False],
        'mdbx.use_mincore': ['Auto', True, False],
        'mdbx.use_ofdlocks': ['Auto', True, False],
        'mdbx.use_sendfile': ['Auto', True, False],
        'mdbx.use_fallocate': ['Auto', True, False],
        'mdbx.without_msvc_crt': ['Default', True, False],
        'shared': [True, False],
    }
    default_options = {
        'mdbx.64bit_atomic': 'Auto',
        'mdbx.64bit_cas': 'Auto',
        'mdbx.apple.speed_insteadof_durability': 'Default',
        'mdbx.avoid_msync': 'Auto',
        'mdbx.build_cxx': 'Default',
        'mdbx.build_tools': 'Default',
        'mdbx.cacheline_size': 'Auto',
        'mdbx.cxx_standard': 'Default',
        'mdbx.disable_gnu_source': 'Default',
        'mdbx.disable_validation': 'Default',
        'mdbx.enable_bigfoot': 'Default',
        'mdbx.enable_dbi_lockfree': 'Default',
        'mdbx.enable_dbi_sparse': 'Default',
        'mdbx.enable_pgop_stat': 'Default',
        'mdbx.enable_pget_stat': 'Default',
        'mdbx.enable_profgc': 'Default',
        'mdbx.enable_refund': 'Default',
        'mdbx.env_checkpid': 'Default',
        'mdbx.force_assertions': 'Default',
        'mdbx.have_builtin_cpu_supports': 'Auto',
        'mdbx.locking': 'Auto',
        'mdbx.lto': 'Default',
        'mdbx.mmap_incoherent_file_write': 'Auto',
        'mdbx.mmap_needs_jolt': 'Auto',
        'mdbx.native_seh': 'Default',
        'mdbx.sanitize': 'none',
        'mdbx.trust_rtc': 'Default',
        'mdbx.txn_checkowner': 'Default',
        'mdbx.unaligned_ok': 'Auto',
        'mdbx.use_copyfilerange': 'Auto',
        'mdbx.use_mincore': 'Auto',
        'mdbx.use_ofdlocks': 'Auto',
        'mdbx.use_sendfile': 'Auto',
        'mdbx.use_fallocate': 'Auto',
        'mdbx.without_msvc_crt': 'Default',
        'shared': True,
    }
    options_description = {
        'mdbx.64bit_atomic': 'Advanced: Assume 64-bit operations are atomic and not splitted to 32-bit halves. ',
        'mdbx.64bit_cas': 'Advanced: Assume 64-bit atomic compare-and-swap operation is available. ',
        'mdbx.apple.speed_insteadof_durability': 'Disable using `fcntl(F_FULLFSYNC)` for a performance reasons at the cost of durability on power failure. ',
        'mdbx.avoid_msync': 'Disable in-memory database updating with consequent flush-to-disk/msync syscall in `MDBX_WRITEMAP` mode. ',
        'mdbx.build_cxx': 'Build C++ portion. ',
        'mdbx.build_tools': 'Build CLI tools (mdbx_chk/stat/dump/load/copy/drop). ',
        'mdbx.cacheline_size': 'Advanced: CPU cache line size for data alignment to avoid cache line false-sharing. ',
        'mdbx.cxx_standard': 'C++ standard used to build the C++ portion (MDBX_CXX_STANDARD); Default lets CMake pick by compiler. Affects the C++ ABI, must match the consumer. ',
        'mdbx.disable_gnu_source': "Don't use GNU/Linux libc extensions. ",
        'mdbx.disable_validation': 'Disable some checks to reduce an overhead and detection probability of database corruption to a values closer to the LMDB. ',
        'mdbx.enable_bigfoot': 'Chunking long list of retired pages during huge transactions commit to avoid use sequences of pages. ',
        'mdbx.enable_dbi_lockfree': 'Support for deferred releasing and a lockfree path to quickly open DBI handles. ',
        'mdbx.enable_dbi_sparse': 'Support for sparse sets of DBI handles to reduce overhead when starting and processing transactions. ',
        'mdbx.enable_pgop_stat': 'Gathering statistics for page operations. ',
        'mdbx.enable_pget_stat': 'Gathering the number of page get operations for each transaction. ',
        'mdbx.enable_profgc': 'Profiling of GC search and updates. ',
        'mdbx.enable_refund': 'Online database zero-cost auto-compactification during write-transactions. ',
        'mdbx.env_checkpid': "Checking PID inside libmdbx's API against reuse database environment after the `fork()`. ",
        'mdbx.force_assertions': 'Forces assertion checking even for release builds. ',
        'mdbx.have_builtin_cpu_supports': 'Advanced: Assume the compiler and target system has `__builtin_cpu_supports()`. ',
        'mdbx.locking': 'Advanced: Choices the locking implementation. ',
        'mdbx.lto': 'Link-Time Optimization: Default=auto (CMake), True/False=force on/off, Fat=dual LTO+native object code (GCC/Clang -ffat-lto-objects; on MSVC behaves as True). Affects binary content and package identity. ',
        'mdbx.mmap_incoherent_file_write': "Advanced: Assume system don't have unified page cache and/or file write operations incoherent with memory-mapped files. ",
        'mdbx.mmap_needs_jolt': 'Advanced: Assume system needs explicit syscall to sync/flush/write modified mapped memory. ',
        'mdbx.native_seh': 'Controls a method for using structural exception handling (Windows). ',
        'mdbx.sanitize': 'Compiler sanitizers (AddressSanitizer/UndefinedBehaviorSanitizer). Affects ABI and package identity; a sanitized package must be consumed by a sanitized build. ',
        'mdbx.trust_rtc': 'Advanced: Does a system have battery-backed Real-Time Clock or just a fake. ',
        'mdbx.txn_checkowner': 'Checking transaction owner thread against misuse transactions from other threads. ',
        'mdbx.unaligned_ok': 'Advanced: Assume a target CPU and/or the compiler support unaligned access. ',
        'mdbx.use_copyfilerange': 'Advanced: Use `copy_file_range()` syscall. ',
        'mdbx.use_mincore': "Use Unix' `mincore()` to determine whether database pages are resident in memory. ",
        'mdbx.use_ofdlocks': 'Advanced: Use POSIX OFD-locks. ',
        'mdbx.use_sendfile': 'Advanced: Use `sendfile()` syscall. ',
        'mdbx.use_fallocate': 'Advanced: Use posix_fallocate() or fcntl(F_PREALLOCATE) on OSX. ',
        'mdbx.without_msvc_crt': 'Avoid dependence from MSVC CRT and use ntdll.dll instead. ',
    }

    build_metadata = None

    def config_options(self):
        if self.settings.get_safe('os') != 'Linux':
            self.options.rm_safe('mdbx.use_copyfilerange')
            self.options.rm_safe('mdbx.use_sendfile')
        if self.settings.get_safe('os') == 'Windows':
            self.default_options['mdbx.avoid_msync'] = True
            self.options.rm_safe('mdbx.env_checkpid')
            self.options.rm_safe('mdbx.locking')
            self.options.rm_safe('mdbx.mmap_incoherent_file_write')
            self.options.rm_safe('mdbx.use_mincore')
            self.options.rm_safe('mdbx.use_ofdlocks')
            self.options.rm_safe('mdbx.use_fallocate')
        else:
            self.options.rm_safe('mdbx.without_msvc_crt')
        if is_apple_os(self):
            self.options.rm_safe('mdbx.mmap_incoherent_file_write')
        else:
            self.options.rm_safe('mdbx.apple.speed_insteadof_durability')

    def validate(self):
        if str(self.options.get_safe('mdbx.build_cxx')) != 'False':
            self.output.warning(
                "mdbx.build_cxx is enabled: the C++ portion is compiled by the package's toolchain. "
                "Consumers must compile with the same compiler/stdlib/ABI (see README 'Packaging / Conan'); "
                "otherwise the C++ ABI splits between the consumer-compiled mdbx.h++ and the packaged "
                "out-of-line definitions (extern template instantiations).")

    def export_sources(self):
        subprocess.run(['make', '-C', self.recipe_folder, 'DIST_DIR=' +
                       self.export_sources_folder, '@dist-checked.tag'], check=True)
        rm(self, 'Makefile', self.export_sources_folder)

    def source(self):
        version_json_pathname = os.path.join(
            self.export_sources_folder, 'VERSION.json')
        with open(version_json_pathname, encoding='utf-8') as version_json_file:
            version_json = json.load(version_json_file)['semver']
        if version_json != semver_string(semver_parse(self.version)):
            self.output.error('Package/Recipe version "' + self.version +
                              '" mismatch VERSION.json "' + version_json + '"')

    def set_version(self):
        if self.build_metadata is None and self.version is not None:
            self.build_metadata = self.version
            semver = semver_parse(self.build_metadata)
            if semver:
                self.build_metadata = semver['buildmetadata']
            else:
                self.build_metadata = re.match(
                    '^[^0-9a-zA-Z]*([0-9a-zA-Z]+[-.0-9a-zA-Z]*)', self.build_metadata).group(1)
        if self.build_metadata is None:
            self.build_metadata = ''
        version_json_pathname = os.path.join(
            self.recipe_folder, 'VERSION.json')
        if os.path.exists(version_json_pathname):
            with open(version_json_pathname, encoding='utf-8') as version_json_file:
                self.version = json.load(version_json_file)['semver']
            version_from = "'" + version_json_pathname + "'"
        self.output.verbose('Fetch version from ' +
                            version_from + ': ' + self.version)
        if self.build_metadata != '':
            self.version += '+' + self.build_metadata

    def layout(self):
        cmake_layout(self)

    def handle_option(self, tc, name, define=False):
        opt = self.options.get_safe(name)
        if opt is not None:
            value = str(opt).lower()
            if value != 'auto' and value != 'default':
                name = name.upper().replace('.', '_')
                if define:
                    if value == 'false' or value == 'no' or value == 'off':
                        tc.preprocessor_definitions[name] = 0
                    elif value == 'true' or value == 'yes' or value == 'on':
                        tc.preprocessor_definitions[name] = 1
                    else:
                        tc.preprocessor_definitions[name] = int(opt)
                    self.output.highlight(
                        name + '=' + str(tc.preprocessor_definitions[name]) + ' (' + str(opt) + ')')
                else:
                    tc.cache_variables[name] = opt
                    self.output.highlight(
                        name + '=' + str(tc.cache_variables[name]) + ' (' + str(opt) + ')')

    def generate(self):
        tc = CMakeToolchain(self)
        if self.build_metadata is None:
            self.build_metadata = semver_parse(self.version)['buildmetadata']
        if self.build_metadata is not None and self.build_metadata != '':
            tc.variables['MDBX_BUILD_METADATA'] = self.build_metadata
            self.output.highlight('MDBX_BUILD_METADATA is ' +
                                  str(tc.variables['MDBX_BUILD_METADATA']))
        self.handle_option(tc, 'mdbx.64bit_atomic', True)
        self.handle_option(tc, 'mdbx.64bit_cas', True)
        self.handle_option(tc, 'mdbx.apple.speed_insteadof_durability')
        self.handle_option(tc, 'mdbx.avoid_msync')
        self.handle_option(tc, 'mdbx.build_tools')
        self.handle_option(tc, 'mdbx.build_cxx')
        self.handle_option(tc, 'mdbx.cacheline_size', True)
        self.handle_option(tc, 'mdbx.cxx_standard')
        self.handle_option(tc, 'mdbx.disable_gnu_source')
        self.handle_option(tc, 'mdbx.disable_validation')
        self.handle_option(tc, 'mdbx.enable_bigfoot')
        self.handle_option(tc, 'mdbx.enable_dbi_lockfree')
        self.handle_option(tc, 'mdbx.enable_dbi_sparse')
        self.handle_option(tc, 'mdbx.enable_pgop_stat')
        self.handle_option(tc, 'mdbx.enable_profgc')
        self.handle_option(tc, 'mdbx.enable_pget_stat')
        self.handle_option(tc, 'mdbx.enable_refund')
        self.handle_option(tc, 'mdbx.env_checkpid')
        self.handle_option(tc, 'mdbx.force_assertions')
        self.handle_option(tc, 'mdbx.have_builtin_cpu_supports', True)
        self.handle_option(tc, 'mdbx.mmap_incoherent_file_write', True)
        self.handle_option(tc, 'mdbx.mmap_needs_jolt')
        self.handle_option(tc, 'mdbx.native_seh')
        self.handle_option(tc, 'mdbx.trust_rtc')
        self.handle_option(tc, 'mdbx.txn_checkowner')
        self.handle_option(tc, 'mdbx.unaligned_ok', True)
        self.handle_option(tc, 'mdbx.use_copyfilerange', True)
        self.handle_option(tc, 'mdbx.use_mincore')
        self.handle_option(tc, 'mdbx.use_ofdlocks')
        self.handle_option(tc, 'mdbx.use_sendfile', True)
        self.handle_option(tc, 'mdbx.without_msvc_crt')
        if self.options.shared:
            tc.cache_variables['MDBX_BUILD_SHARED_LIBRARY'] = 1
        else:
            tc.cache_variables['MDBX_BUILD_SHARED_LIBRARY'] = 0
            tc.cache_variables['MDBX_INSTALL_STATIC'] = 1
            self.output.highlight(
                'MDBX_INSTALL_STATIC=1 (static-only package, shared=False)')
        opt = self.options.get_safe('mdbx.locking', 'auto')
        if opt is not None:
            value = str(opt).lower()
            if value != 'auto' and value != 'default':
                map = {'windowsfilelocking': -1, 'systemv': 5, 'posix1988': 1988,
                       'posix2001': 2001, 'posix2008': 2008}
                value = map[value]
                tc.cache_variables['MDBX_LOCKING'] = value
                self.output.highlight('MDBX_LOCKING=' +
                                      str(tc.cache_variables['MDBX_LOCKING']))
        opt = self.options.get_safe('mdbx.lto')
        if opt is not None:
            value = str(opt).lower()
            if value != 'default':
                if value == 'fat':
                    tc.cache_variables['INTERPROCEDURAL_OPTIMIZATION'] = True
                    tc.cache_variables['MDBX_LTO_FAT_OBJECTS'] = True
                    self.output.highlight(
                        'LTO: Fat (INTERPROCEDURAL_OPTIMIZATION=True, MDBX_LTO_FAT_OBJECTS=True)')
                else:
                    tc.cache_variables['INTERPROCEDURAL_OPTIMIZATION'] = (value == 'true')
                    self.output.highlight(
                        'INTERPROCEDURAL_OPTIMIZATION=' +
                        str(tc.cache_variables['INTERPROCEDURAL_OPTIMIZATION']))
        opt = self.options.get_safe('mdbx.sanitize')
        if opt is not None and str(opt) != 'none':
            sanitize_flags = '-fsanitize=' + str(opt)
            tc.cache_variables['CMAKE_C_FLAGS'] = sanitize_flags
            tc.cache_variables['CMAKE_CXX_FLAGS'] = sanitize_flags
            tc.cache_variables['CMAKE_EXE_LINKER_FLAGS'] = sanitize_flags
            tc.cache_variables['CMAKE_SHARED_LINKER_FLAGS'] = sanitize_flags
            self.output.highlight(
                'sanitize: -fsanitize=' + str(opt) + ' in CMAKE_{C,CXX}_FLAGS and linker flags')
        tc.generate()

    def package_id(self):
        # Binary identity is the full profile: all settings (os/arch/compiler/
        # build_type) and every option, including mdbx.lto and mdbx.sanitize,
        # participate in the package hash. Warnings and the optimization level
        # inside a build type are intentionally excluded; the consumer's own
        # toolchain and system library versions are invisible to Conan and
        # must be matched via the profile (README 'Packaging / Conan').
        pass

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        # CMake builds the library as `libmdbx.{so,a}` in both cases:
        # shared=True produces libmdbx.so, while a static-only package
        # (shared=False, MDBX_BUILD_SHARED_LIBRARY=0) names the archive
        # libmdbx.a (the `mdbx-static` target gets OUTPUT_NAME `mdbx`).
        self.cpp_info.libs = ['mdbx']
        self.cpp_info.set_property('cmake_target_name', 'mdbx::mdbx')
        if self.settings.os in ('Linux', 'FreeBSD'):
            self.cpp_info.system_libs.append('pthread')
        if self.settings.os == 'Windows':
            self.cpp_info.system_libs.extend(['ntdll', 'ws2_32'])
        if str(self.options.get_safe('mdbx.build_cxx')) != 'False' and self.options.shared:
            self.output.warning(
                "The C++ portion is enabled in a shared package: the consumer's header (mdbx.h++) and the "
                "packaged out-of-line definitions are compiled by different toolchains unless the active "
                "Conan profile matches the consumer's real compiler/stdlib/ABI.")
