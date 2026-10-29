/// This file is part of the libmdbx amalgamated source code (v0.15.0-334-gb8f2dad5 at 2026-10-29T00:17:01+03:00).
/// \file mdbx.h++
/// \brief The libmdbx C++ API header file.
///
/// \details _libmdbx_ (aka MDBX) is an extremely fast, compact, powerful, embeddable,
/// transactional [key-value store](https://en.wikipedia.org/wiki/Key-value_database),
/// with [Apache 2.0 license](./LICENSE). _MDBX_ has a specific set of properties and capabilities,
/// focused on creating unique lightweight solutions with extraordinary performance.
///
/// Please visit https://libmdbx.dqdkfa.ru for more information, documentation,
/// C++ API description and links to the origin git repo with the source code.
/// Questions, feedback and suggestions are welcome to the Telegram' group
/// https://t.me/libmdbx, MAX' chat https://max.ru/join/dKckvyuARxp1vRK-wnPur8zYCEkbR3OUOmpPWkWxp78.
///
/// Donations are welcome to ETH `0xD104d8f8B2dC312aaD74899F83EBf3EEBDC1EA3A`,
/// BTC `bc1qzvl9uegf2ea6cwlytnanrscyv8snwsvrc0xfsu`, SOL `FTCTgbHajoLVZGr8aEFWMzx3NDMyS5wXJgfeMTmJznRi`.
/// Всё будет хорошо!
///
/// The libmdbx project has been completely relocated to the jurisdiction of the Russian Federation.
/// \note _libmdbx_ is still open and provided with first-class free support.
///
/// \copyright SPDX-License-Identifier: Apache-2.0
/// \author Леонид Юрьев aka Leonid Yuriev <leo@yuriev.ru> \date 2020-2026
///

#pragma once

//
// Tested with, since 2026:
//  - Elbrus LCC >= 1.28 (http://www.mcst.ru/lcc);
//  - GNU C++ >= 11.3;
//  - CLANG >= 14.0;
//  - MSVC >= 19.44 (Visual Studio 2022 toolchain v143),
// before 2026:
//  - Elbrus LCC >= 1.23 (http://www.mcst.ru/lcc);
//  - GNU C++ >= 4.8;
//  - CLANG >= 3.9;
//  - MSVC >= 14.0 (Visual Studio 2015),
//    but 19.2x could hang due optimizer bug;
//  - AppleClang, but without C++20 concepts.
//

/* Workaround for modern libstdc++ with CLANG < 4.x */
#if defined(__SIZEOF_INT128__) && !defined(__GLIBCXX_TYPE_INT_N_0) && defined(__clang__) && __clang_major__ < 4
#define __GLIBCXX_BITSIZE_INT_N_0 128
#define __GLIBCXX_TYPE_INT_N_0 __int128
#endif /* Workaround for modern libstdc++ with CLANG < 4.x */

#if !defined(__cplusplus) || __cplusplus < 201103L
#if !defined(_MSC_VER) || _MSC_VER < 1900
#error "C++11 compiler or better is required"
#elif _MSC_VER >= 1910
#error "Please add `/Zc:__cplusplus` to MSVC compiler options to enforce it conform ISO C++"
#endif /* MSVC is mad and don't define __cplusplus properly */
#endif /* __cplusplus < 201103L */

#if (defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)) && MDBX_WITHOUT_MSVC_CRT && !defined(__CODEGEARC__)
#error "CRT is required for C++ API, the MDBX_WITHOUT_MSVC_CRT option must be disabled"
#endif /* Windows */

#ifndef __has_include
#define __has_include(header) (0)
#endif /* __has_include */

#if __has_include(<version>)
#include <version>
#endif /* <version> */

/* Disable min/max macros from C' headers */
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>   // for std::min/max
#include <cassert>     // for assert()
#include <climits>     // for CHAR_BIT
#include <cstdint>     // for std::uintptr_t, std::intptr_t
#include <cstring>     // for std::strlen, str:memcmp
#include <exception>   // for std::exception_ptr
#include <ostream>     // for std::ostream
#include <sstream>     // for std::ostringstream
#include <stdexcept>   // for std::invalid_argument
#include <string>      // for std::string
#include <type_traits> // for std::is_pod<>, etc.
#include <utility>     // for std::make_pair
#include <vector>      // for std::vector<> as template args

#if defined(__cpp_lib_memory_resource) && __cpp_lib_memory_resource >= 201603L
#include <memory_resource>
#endif

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
#include <string_view>
#endif

#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 201606L
#include <optional>
#endif

#ifndef MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM
#ifdef MDBX_USING_CXX_EXPERIMETAL_FILESYSTEM
/* backward compatibility with the old misspelled name */
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM MDBX_USING_CXX_EXPERIMETAL_FILESYSTEM
#elif defined(INCLUDE_STD_FILESYSTEM_EXPERIMENTAL)
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 1
#elif defined(__cpp_lib_filesystem) && __cpp_lib_filesystem >= 201703L && __cplusplus >= 201703L
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 0
#elif (!defined(_MSC_VER) || __cplusplus >= 201403L ||                                                                 \
       (defined(_MSC_VER) && defined(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING) && __cplusplus >= 201403L))
#if defined(__cpp_lib_experimental_filesystem) && __cpp_lib_experimental_filesystem >= 201406L
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 1
#elif defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L && __has_include(<experimental/filesystem>)
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 1
#else
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 0
#endif
#else
#define MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM 0
#endif
#endif /* MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM */

#if MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#elif defined(__cpp_lib_filesystem) && __cpp_lib_filesystem >= 201703L
#include <filesystem>
#endif

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
#include <span>
#endif

#if !defined(_MSC_VER)
#define MDBX_EXTERN_API_TEMPLATE(API_ATTRIBUTES, ...) extern template class API_ATTRIBUTES __VA_ARGS__
#define MDBX_INSTALL_API_TEMPLATE(API_ATTRIBUTES, ...) template class __VA_ARGS__
#else
#define MDBX_EXTERN_API_TEMPLATE(API_ATTRIBUTES, ...) extern template class __VA_ARGS__
#define MDBX_INSTALL_API_TEMPLATE(API_ATTRIBUTES, ...) template class API_ATTRIBUTES __VA_ARGS__
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define MDBX_MSVC_DECLSPEC_EMPTY_BASES __declspec(empty_bases)
#else
#define MDBX_MSVC_DECLSPEC_EMPTY_BASES /* nope */
#endif                                 /* _MSC_VER >= 1900 */

#if __cplusplus >= 201103L
#include <chrono>
#include <ratio>
#endif

#include "mdbx.h"

#if (defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L) ||                                                  \
    (defined(__cpp_lib_endian) && __cpp_lib_endian >= 201907L) ||                                                      \
    (defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L) ||                                                      \
    (defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 202002L)
#include <bit>
#elif !(defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__))
#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#define __ORDER_LITTLE_ENDIAN__ __LITTLE_ENDIAN
#define __ORDER_BIG_ENDIAN__ __BIG_ENDIAN
#define __BYTE_ORDER__ __BYTE_ORDER
#elif defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN)
#define __ORDER_LITTLE_ENDIAN__ _LITTLE_ENDIAN
#define __ORDER_BIG_ENDIAN__ _BIG_ENDIAN
#define __BYTE_ORDER__ _BYTE_ORDER
#else
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_BIG_ENDIAN__ 4321
#if defined(__LITTLE_ENDIAN__) || (defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)) || defined(__ARMEL__) ||          \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(__MIPSEL__) || defined(_MIPSEL) || defined(__MIPSEL) ||  \
    defined(_M_ARM) || defined(_M_ARM64) || defined(__e2k__) || defined(__elbrus_4c__) || defined(__elbrus_8c__) ||    \
    defined(__bfin__) || defined(__BFIN__) || defined(__ia64__) || defined(_IA64) || defined(__IA64__) ||              \
    defined(__ia64) || defined(_M_IA64) || defined(__itanium__) || defined(__ia32__) || defined(__CYGWIN__) ||         \
    defined(_WIN64) || defined(_WIN32) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#elif defined(__BIG_ENDIAN__) || (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || defined(__ARMEB__) ||           \
    defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(__MIPSEB__) || defined(_MIPSEB) || defined(__MIPSEB) ||  \
    defined(__m68k__) || defined(M68000) || defined(__hppa__) || defined(__hppa) || defined(__HPPA__) ||               \
    defined(__sparc__) || defined(__sparc) || defined(__370__) || defined(__THW_370__) || defined(__s390__) ||         \
    defined(__s390x__) || defined(__SYSC_ZARCH__)
#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
#endif
#endif
#endif /* Byte Order */

/** Workaround for old compilers without properly support for `C++17 constexpr` */
#if defined(DOXYGEN)
#define MDBX_CXX17_CONSTEXPR constexpr
#elif defined(__cpp_constexpr) && __cpp_constexpr >= 201603L &&                                                        \
    ((defined(_MSC_VER) && _MSC_VER >= 1915) || (defined(__clang__) && __clang_major__ > 5) ||                         \
     (defined(__GNUC__) && __GNUC__ > 7) || (!defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)))
#define MDBX_CXX17_CONSTEXPR constexpr
#else
#define MDBX_CXX17_CONSTEXPR inline
#endif /* MDBX_CXX17_CONSTEXPR */

/** Workaround for old compilers without properly support for C++20 `constexpr`. */
#if defined(DOXYGEN)
#define MDBX_CXX20_CONSTEXPR constexpr
#elif defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L &&                        \
    defined(__cpp_lib_constexpr_string) && __cpp_lib_constexpr_string >= 201907L
#define MDBX_CXX20_CONSTEXPR constexpr
#else
#define MDBX_CXX20_CONSTEXPR inline
#endif /* MDBX_CXX20_CONSTEXPR */

#if MDBX_CONSTEXPR_ENUM_FLAGS_OPERATIONS || defined(DOXYGEN)
#define MDBX_CXX01_CONSTEXPR_ENUM MDBX_CXX01_CONSTEXPR
#define MDBX_CXX11_CONSTEXPR_ENUM MDBX_CXX11_CONSTEXPR
#define MDBX_CXX14_CONSTEXPR_ENUM MDBX_CXX14_CONSTEXPR
#define MDBX_CXX17_CONSTEXPR_ENUM MDBX_CXX17_CONSTEXPR
#define MDBX_CXX20_CONSTEXPR_ENUM MDBX_CXX20_CONSTEXPR
#else
#define MDBX_CXX01_CONSTEXPR_ENUM inline
#define MDBX_CXX11_CONSTEXPR_ENUM inline
#define MDBX_CXX14_CONSTEXPR_ENUM inline
#define MDBX_CXX17_CONSTEXPR_ENUM inline
#define MDBX_CXX20_CONSTEXPR_ENUM inline
#endif /* MDBX_CONSTEXPR_ENUM_FLAGS_OPERATIONS */

/** Workaround for old compilers without support assertion inside `constexpr` functions. */
#if defined(CONSTEXPR_ASSERT)
#define MDBX_CONSTEXPR_ASSERT(expr) CONSTEXPR_ASSERT(expr)
#elif defined(NDEBUG) && (!defined(MDBX_CHECKING) || !MDBX_CHECKING) && (!defined(MDBX_DEBUG) || !MDBX_DEBUG)
#define MDBX_CONSTEXPR_ASSERT(expr) void(0)
#else
#define MDBX_CONSTEXPR_ASSERT(expr) ((expr) ? void(0) : [] { assert(!#expr); }())
#endif /* MDBX_CONSTEXPR_ASSERT */

/** Workaround for old compilers without properly support for C++20 `if constexpr`. */
#if defined(DOXYGEN)
#define MDBX_IF_CONSTEXPR constexpr
#elif defined(__cpp_if_constexpr) && __cpp_if_constexpr >= 201606L
#define MDBX_IF_CONSTEXPR constexpr
#else
#define MDBX_IF_CONSTEXPR
#endif /* MDBX_IF_CONSTEXPR */

#if defined(DOXYGEN) || (__has_cpp_attribute(fallthrough) && (!defined(__clang__) || __clang__ > 4)) ||                \
    __cplusplus >= 201703L
#define MDBX_CXX17_FALLTHROUGH [[fallthrough]]
#else
#define MDBX_CXX17_FALLTHROUGH
#endif /* MDBX_CXX17_FALLTHROUGH */

#if defined(DOXYGEN) || (__has_cpp_attribute(likely) >= 201803L && (!defined(__GNUC__) || __GNUC__ > 9))
#define MDBX_CXX20_LIKELY [[likely]]
#else
#define MDBX_CXX20_LIKELY
#endif /* MDBX_CXX20_LIKELY */

#ifndef MDBX_CXX20_UNLIKELY
#if defined(DOXYGEN) || (__has_cpp_attribute(unlikely) >= 201803L && (!defined(__GNUC__) || __GNUC__ > 9))
#define MDBX_CXX20_UNLIKELY [[unlikely]]
#else
#define MDBX_CXX20_UNLIKELY
#endif
#endif /* MDBX_CXX20_UNLIKELY */

/** Workaround for old compilers without `[[nodiscard]]` support */
#ifndef MDBX_NODISCARD
#if defined(DOXYGEN) || __cplusplus >= 201703L
#define MDBX_NODISCARD [[nodiscard]]
#else
#define MDBX_NODISCARD /* nope */
#endif
#endif /* MDBX_NODISCARD */

#ifndef MDBX_HAVE_CXX20_CONCEPTS
#if defined(__cpp_concepts) && __cpp_concepts >= 202002L && defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 202002L
#include <concepts>
#define MDBX_HAVE_CXX20_CONCEPTS 1
#elif defined(DOXYGEN)
#define MDBX_HAVE_CXX20_CONCEPTS 1
#else
#define MDBX_HAVE_CXX20_CONCEPTS 0
#endif /* <concepts> */
#endif /* MDBX_HAVE_CXX20_CONCEPTS */

#ifndef MDBX_CXX20_CONCEPT
#if MDBX_HAVE_CXX20_CONCEPTS || defined(DOXYGEN)
#define MDBX_CXX20_CONCEPT(CONCEPT, NAME) CONCEPT NAME
#else
#define MDBX_CXX20_CONCEPT(CONCEPT, NAME) typename NAME
#endif
#endif /* MDBX_CXX20_CONCEPT */

#ifndef MDBX_ASSERT_CXX20_CONCEPT_SATISFIED
#if MDBX_HAVE_CXX20_CONCEPTS || defined(DOXYGEN)
#define MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(CONCEPT, TYPE) static_assert(CONCEPT<TYPE>)
#else
#define MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(CONCEPT, NAME)                                                             \
  static_assert(true, MDBX_STRINGIFY(CONCEPT) "<" MDBX_STRINGIFY(TYPE) ">")
#endif
#endif /* MDBX_ASSERT_CXX20_CONCEPT_SATISFIED */

#ifdef _MSC_VER
#pragma warning(push, 4)
#pragma warning(disable : 4127) /* conditional expression is constant */
#pragma warning(disable : 4251) /* 'std::FOO' needs to have dll-interface to                                           \
                                   be used by clients of 'mdbx::BAR' */
#pragma warning(disable : 4275) /* non dll-interface 'std::FOO' used as                                                \
                                   base for dll-interface 'mdbx::BAR' */
/* MSVC is mad and can generate this warning for its own intermediate
 * automatically generated code, which becomes unreachable after some kinds of
 * optimization (copy elision, etc). */
#pragma warning(disable : 4702) /* unreachable code */
#endif                          /* _MSC_VER (warnings) */

#if defined(__LCC__) && __LCC__ >= 126
#pragma diagnostic push
#if __LCC__ < 127
#pragma diag_suppress 3058 /* workaround: call to is_constant_evaluated()                                              \
                              appearing in a constant expression `true` */
#pragma diag_suppress 3060 /* workaround: call to is_constant_evaluated()                                              \
                              appearing in a constant expression `false` */
#endif
#endif /* E2K LCC (warnings) */

//------------------------------------------------------------------------------
/// \brief The libmdbx C++ API namespace
/// \ingroup cxx_api
namespace mdbx {

/// \defgroup cxx_api C++ API
/// @{

/// \brief The byte-like type that don't presumes aliases for pointers as does the `char`.
/// \details Essentially, to enable all kinds of an compiler optimization, we need just
/// the `unsigned char * restrict` type in C99 terms, i.e. the non-aliasing pointer to `unsigned char`.
/// However, C++ still doesn't have `restrict` keyword not `non-aliases` type attribute, but a char-pointers may be
/// aliased.
///
/// On the other hand, while `uint8_t` is provided and `CHAR_BIT = 8` the `char8_t *` actually act the same as the C99
/// `unsigned char * restrict`. So using `char8_t` should not be an issue, since both the `CHAR_BIT = 8` and `uint8_t
/// `are required.
///
/// At the same time, the approach of using `char8_t` has several advantages:
///  - the `restrict` attribute is defined on level of the base type and is inherited by any derived pointer type;
///  - some compilers treat `__restrict` as an attribute of an instance of a type (i.e. a variable, a specific
///    pointer), but not a type attribute.
///
/// Nonetheless, I should think about switching to the `uint8_t * __restrict__` for byte pointers.
/// \note Functions whose signature depends on the `mdbx::byte` type must be strictly defined as inline!
#if defined(DOXYGEN) || (defined(__cpp_char8_t) && __cpp_char8_t >= 201811)
using byte = char8_t;
#else
// Avoid `std::byte` since it doesn't add features but inconvenient restrictions.
using byte = unsigned char;
#endif /* __cpp_char8_t >= 201811*/

#if defined(__cpp_lib_endian) && __cpp_lib_endian >= 201907L
using endian = ::std::endian;
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
enum class endian { little = __ORDER_LITTLE_ENDIAN__, big = __ORDER_BIG_ENDIAN__, native = __BYTE_ORDER__ };
#else
#error "Please use a C++ compiler provides byte order information or C++20 support"
#endif /* Byte Order enum */

/// \copydoc MDBX_version_info
using version_info = ::MDBX_version_info;
/// \brief Returns libmdbx version information.
/// \returns A reference to the constant \ref version_info instance.
MDBX_CXX11_CONSTEXPR const version_info &get_version() noexcept;
/// \copydoc MDBX_build_info
using build_info = ::MDBX_build_info;
/// \brief Returns libmdbx build information.
/// \returns A reference to the constant \ref build_info instance.
MDBX_CXX11_CONSTEXPR const build_info &get_build() noexcept;

/// \brief constexpr-enabled strlen().
static MDBX_CXX17_CONSTEXPR size_t strlen(const char *c_str) noexcept;

/// \brief constexpr-enabled memcpy().
static MDBX_CXX20_CONSTEXPR void *memcpy(void *dest, const void *src, size_t bytes) noexcept;
/// \brief constexpr-enabled memcmp().
static MDBX_CXX20_CONSTEXPR int memcmp(const void *a, const void *b, size_t bytes) noexcept;

/// \brief Legacy allocator
/// but it is recommended to use \ref polymorphic_allocator.
using legacy_allocator = ::std::string::allocator_type;

/// \brief Defined as `1` if the `mdbx::polymorphic_allocator` is available.
/// \see The `<memory_resource>` Standard C++17 library header
#if defined(DOXYGEN)
#define MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR 1
#elif !defined(MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR)
#if defined(__cpp_lib_memory_resource) && __cpp_lib_memory_resource >= 201603L &&                                      \
    (!defined(_GLIBCXX_USE_CXX11_ABI) || _GLIBCXX_USE_CXX11_ABI)
#define MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR 1
#else
#define MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR 0
#endif
#endif /* MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR */

#if defined(DOXYGEN) || MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR
/// \brief Default polymorphic allocator for modern code.
using polymorphic_allocator = ::std::pmr::string::allocator_type;
/// \brief The default allocator for \ref buffer and \ref string; resolves to
/// \ref polymorphic_allocator when the latter is available, otherwise to
/// \ref legacy_allocator.
using default_allocator = polymorphic_allocator;
#else
/// \brief The default allocator for \ref buffer and \ref string; resolves to
/// \ref polymorphic_allocator when the latter is available, otherwise to
/// \ref legacy_allocator.
using default_allocator = legacy_allocator;
#endif /* MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR */

struct slice;
struct default_capacity_policy;
template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
class MDBX_MSVC_DECLSPEC_EMPTY_BASES buffer;
class env;
class env_managed;
class txn;
class txn_managed;
class cursor;
class cursor_managed;

/// \brief Transaction ID and MVCC-snapshot number.
using txnid = uint64_t;

/// \brief Default single-byte string.
template <class ALLOCATOR = default_allocator>
using string = ::std::basic_string<char, ::std::char_traits<char>, ALLOCATOR>;

/// \brief The platform-dependent file handle type, i.e. an alias for
/// \ref mdbx_filehandle_t.
using filehandle = ::mdbx_filehandle_t;
#if MDBX_USING_CXX_EXPERIMENTAL_FILESYSTEM
#ifdef _MSC_VER
namespace filesystem = ::std::experimental::filesystem::v1;
#else
namespace filesystem = ::std::experimental::filesystem;
#endif
#define MDBX_STD_FILESYSTEM_PATH ::mdbx::filesystem::path
#elif defined(DOXYGEN) ||                                                                                              \
    (defined(__cpp_lib_filesystem) && __cpp_lib_filesystem >= 201703L && defined(__cpp_lib_string_view) &&             \
     __cpp_lib_string_view >= 201606L &&                                                                               \
     (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500) &&                       \
     (!defined(__IPHONE_OS_VERSION_MIN_REQUIRED) || __IPHONE_OS_VERSION_MIN_REQUIRED >= 130100)) &&                    \
        (!defined(_MSC_VER) || __cplusplus >= 201703L)
namespace filesystem = ::std::filesystem;
/// \brief Defined if `mdbx::filesystem::path` is available.
/// \details If defined, it is always `mdbx::filesystem::path`,
/// which in turn can refer to `std::filesystem::path` or `std::experimental::filesystem::path`.
/// Nonetheless `MDBX_STD_FILESYSTEM_PATH` is not defined if the `::mdbx::path`
/// falls back to a plain `std::string` or `std::wstring`.
#define MDBX_STD_FILESYSTEM_PATH ::mdbx::filesystem::path
#endif /* MDBX_STD_FILESYSTEM_PATH */

#ifdef MDBX_STD_FILESYSTEM_PATH
/// \brief The default path type for opening environments; resolves to
/// `mdbx::filesystem::path` when the filesystem library is available,
/// otherwise to `std::wstring` on Windows and `std::string` elsewhere.
using path = MDBX_STD_FILESYSTEM_PATH;
/// \brief The string type used by \ref path, i.e. `std::wstring` on Windows
/// and `std::string` elsewhere (or `path::string_type` for the filesystem
/// library).
using path_string = MDBX_STD_FILESYSTEM_PATH::string_type;
#elif defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
/// \brief The default path type for opening environments; resolves to
/// `mdbx::filesystem::path` when the filesystem library is available,
/// otherwise to `std::wstring` on Windows and `std::string` elsewhere.
using path = ::std::wstring;
/// \brief The string type used by \ref path.
using path_string = path;
#else
/// \brief The default path type for opening environments; resolves to
/// `mdbx::filesystem::path` when the filesystem library is available,
/// otherwise to `std::wstring` on Windows and `std::string` elsewhere.
using path = ::std::string;
/// \brief The string type used by \ref path.
using path_string = path;
#endif /* mdbx::path */
/// \brief The character type of \ref path_string, i.e. `wchar_t` on Windows
/// and `char` elsewhere.
using path_char = path_string::value_type;

#if defined(__SIZEOF_INT128__) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
#ifndef MDBX_U128_TYPE
#define MDBX_U128_TYPE __uint128_t
#endif /* MDBX_U128_TYPE */
#ifndef MDBX_I128_TYPE
#define MDBX_I128_TYPE __int128_t
#endif /* MDBX_I128_TYPE */
#endif /* __SIZEOF_INT128__ || _INTEGRAL_MAX_BITS >= 128 */

#if __cplusplus >= 201103L || defined(DOXYGEN)
/// \brief Duration in 1/65536 units of second.
using duration = ::std::chrono::duration<unsigned, ::std::ratio<1, 65536>>;
#endif /* Duration for C++11 */

/// \defgroup cxx_exceptions Exceptions and Errors
/// @{

/// \brief Transfers C++ exceptions thru C callbacks.
/// \details Implements saving exceptions before returning
/// from an C++'s environment to the intermediate C code and re-throwing after
/// returning from C to the C++'s environment.
class LIBMDBX_API_TYPE exception_thunk {
  ::std::exception_ptr captured_;

public:
  /// \brief Default constructor, creates an empty (clean) thunk.
  exception_thunk() noexcept = default;
  /// \brief Copy construction is prohibited.
  exception_thunk(const exception_thunk &) = delete;
  /// \brief Move construction is prohibited.
  exception_thunk(exception_thunk &&) = delete;
  /// \brief Copy assignment is prohibited.
  exception_thunk &operator=(const exception_thunk &) = delete;
  /// \brief Move assignment is prohibited.
  exception_thunk &operator=(exception_thunk &&) = delete;
  /// \brief Checks whether no exception is captured.
  /// \returns `true` if the thunk is empty, `false` otherwise.
  inline bool is_clean() const noexcept;
  /// \brief Captures the currently thrown exception, if any.
  inline void capture() noexcept;
  /// \brief Re-throws the captured exception, if any.
  /// \throws The captured exception.
  inline void rethrow_captured() const;
};

/// \brief Implements error information and throwing corresponding exceptions.
class LIBMDBX_API_TYPE error {
  MDBX_error_t code_;
  /// \brief Assigns the given error code to this instance.
  /// \param error_code  The error code to be assigned.
  /// \returns A reference to this instance.
  inline error &operator=(MDBX_error_t error_code) noexcept;

public:
  /// \brief Constructs an error instance from the given code.
  /// \param error_code  The error code.
  MDBX_CXX11_CONSTEXPR error(MDBX_error_t error_code) noexcept;
  /// \brief Copy constructor.
  error(const error &) = default;
  /// \brief Move constructor.
  error(error &&) = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  error &operator=(const error &) = default;
  /// \brief Move assignment operator.
  /// \returns A reference to this instance.
  error &operator=(error &&) = default;

  /// \brief Compares two error codes for equality.
  /// \param a  The left-hand error.
  /// \param b  The right-hand error.
  /// \returns `true` if the error codes are equal, `false` otherwise.
  MDBX_CXX11_CONSTEXPR friend bool operator==(const error &a, const error &b) noexcept;
  /// \brief Compares two error codes for inequality.
  /// \param a  The left-hand error.
  /// \param b  The right-hand error.
  /// \returns `true` if the error codes are not equal, `false` otherwise.
  MDBX_CXX11_CONSTEXPR friend bool operator!=(const error &a, const error &b) noexcept;

  /// \brief Checks the error is a success.
  /// \returns `true` if the error code is \ref MDBX_SUCCESS, `false` otherwise.
  MDBX_CXX11_CONSTEXPR bool is_success() const noexcept;
  /// \brief Checks the error is the "true" result of a \ref MDBX_RESULT_TRUE.
  /// \returns `true` if the error code is \ref MDBX_RESULT_TRUE, `false` otherwise.
  MDBX_CXX11_CONSTEXPR bool is_result_true() const noexcept;
  /// \brief Checks the error is the "false" result of a \ref MDBX_RESULT_FALSE.
  /// \returns `true` if the error code is \ref MDBX_RESULT_FALSE, `false` otherwise.
  MDBX_CXX11_CONSTEXPR bool is_result_false() const noexcept;
  /// \brief Checks the error is a failure.
  /// \returns `true` if the error code is neither a success nor a boolean
  ///          result, `false` otherwise.
  MDBX_CXX11_CONSTEXPR bool is_failure() const noexcept;

  /// \brief Returns error code.
  /// \returns The stored \ref MDBX_error_t code.
  MDBX_CXX11_CONSTEXPR MDBX_error_t code() const noexcept;

  /// \brief Returns message for MDBX's errors only and "SYSTEM" for others.
  /// \returns The pointer to a constant string with the error message.
  const char *what() const noexcept;

  /// \brief Returns message for any errors.
  /// \returns The error message as a `std::string`.
  ::std::string message() const;

  /// \brief Returns true for MDBX's errors.
  /// \returns `true` if the error code belongs to libmdbx, `false` otherwise.
  MDBX_CXX11_CONSTEXPR bool is_mdbx_error() const noexcept;
  /// \brief Panics on unrecoverable errors inside destructors etc.
  /// \throws The corresponding \ref mdbx::exception if the error is a failure.
  [[noreturn]] void throw_exception() const;
  /// \brief Panics on unrecoverable errors inside destructors etc.
  /// \param error_code  The error code to be checked.
  /// \throws The corresponding \ref mdbx::exception if the error is a failure.
  [[noreturn]] static inline void throw_exception(int error_code);
  /// \brief Throws the corresponding exception if this error is a failure.
  /// \throws The corresponding \ref mdbx::exception.
  inline void throw_on_failure() const;
  /// \brief Throws the corresponding exception if this error is a failure.
  /// \throws The corresponding \ref mdbx::exception.
  inline void success_or_throw() const;
  /// \brief Throws the corresponding exception, or the captured one if any.
  /// \param thunk  The \ref mdbx::exception_thunk with a captured exception.
  /// \throws The corresponding \ref mdbx::exception.
  inline void success_or_throw(const exception_thunk &thunk) const;
  /// \brief Throws the corresponding exception if the pointer is null.
  /// \param ptr         The pointer to be checked.
  /// \param error_code  The error code to be thrown on a null pointer.
  /// \throws The corresponding \ref mdbx::exception.
  static inline void throw_on_nullptr(const void *ptr, MDBX_error_t error_code);
  /// \brief Throws the corresponding exception if the code is a failure.
  /// \param error_code  The error code to be checked.
  /// \throws The corresponding \ref mdbx::exception.
  static inline void success_or_throw(MDBX_error_t error_code);
  /// \brief Throws the corresponding exception if the code is a failure.
  /// \param error_code  The error code to be checked.
  /// \throws The corresponding \ref mdbx::exception.
  static void success_or_throw(int error_code) { success_or_throw(static_cast<MDBX_error_t>(error_code)); }
  /// \brief Throws the corresponding exception if the code is a failure.
  /// \param error_code  The error code to be checked.
  /// \throws The corresponding \ref mdbx::exception.
  static inline void throw_on_failure(int error_code);
  /// \brief Throws the corresponding exception if the code is a failure.
  /// \param error_code  The error code to be checked.
  /// \returns `true` if the code is \ref MDBX_RESULT_TRUE, `false` otherwise.
  /// \throws The corresponding \ref mdbx::exception.
  static inline bool boolean_or_throw(int error_code);
  /// \brief Throws the corresponding exception, or the captured one if any.
  /// \param error_code  The error code to be checked.
  /// \param thunk       The \ref mdbx::exception_thunk with a captured exception.
  /// \throws The corresponding \ref mdbx::exception.
  static inline void success_or_throw(int error_code, const exception_thunk &thunk);
  /// \brief Throws the corresponding exception, or the captured one if any.
  /// \param error_code  The error code to be checked.
  /// \param thunk       The \ref mdbx::exception_thunk with a captured exception.
  /// \returns `true` if the code is \ref MDBX_RESULT_TRUE, `false` otherwise.
  /// \throws The corresponding \ref mdbx::exception.
  static inline bool boolean_or_throw(int error_code, const exception_thunk &thunk);
};

/// \brief Base class for all libmdbx's exceptions that correspond to libmdbx errors.
/// \see MDBX_error_t
class LIBMDBX_API_TYPE exception : public ::std::runtime_error {
  using base = ::std::runtime_error;
  ::mdbx::error error_;

public:
  /// \brief Constructs the exception from the given error.
  /// \param err  The error information.
  exception(const ::mdbx::error &err);
  /// \brief Copy constructor.
  exception(const exception &) = default;
  /// \brief Move constructor.
  exception(exception &&) = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  exception &operator=(const exception &) = default;
  /// \brief Move assignment operator.
  /// \returns A reference to this instance.
  exception &operator=(exception &&) = default;
  /// \brief Destroys the exception.
  virtual ~exception() noexcept;
  /// \brief Returns the error information.
  /// \returns The stored \ref mdbx::error instance.
  const ::mdbx::error error() const noexcept { return error_; }
};

/// \brief Fatal exception that leads to termination anyway in dangerous unrecoverable cases.
class LIBMDBX_API_TYPE fatal : public exception {
  using base = exception;

public:
  /// \brief Constructs the fatal exception from the given error.
  /// \param err  The error information.
  fatal(const ::mdbx::error &err) noexcept;
  /// \brief Constructs the fatal exception from the given exception.
  /// \param src  The source exception.
  fatal(const exception &src) noexcept : fatal(src.error()) {}
  /// \brief Constructs the fatal exception from the given exception.
  /// \param src  The source exception.
  fatal(exception &&src) noexcept : fatal(src.error()) {}
  /// \brief Copy constructor.
  /// \param src  The source exception.
  fatal(const fatal &src) noexcept : fatal(src.error()) {}
  /// \brief Move constructor.
  /// \param src  The source exception.
  fatal(fatal &&src) noexcept : fatal(src.error()) {}
  /// \brief Move assignment operator.
  /// \returns A reference to this instance.
  fatal &operator=(fatal &&) = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  fatal &operator=(const fatal &) = default;
  /// \brief Destroys the fatal exception.
  virtual ~fatal() noexcept;
};

#define MDBX_DECLARE_EXCEPTION(NAME)                                                                                   \
  struct LIBMDBX_API_TYPE NAME : public exception {                                                                    \
    /** \brief Constructs the exception from the given \ref mdbx::error. */                                            \
    NAME(const ::mdbx::error &);                                                                                       \
    /** \brief Destroys the exception. */                                                                              \
    virtual ~NAME() noexcept;                                                                                          \
  }
/// \brief Thrown on \ref MDBX_BAD_DBI, i.e. an invalid table handle was used.
MDBX_DECLARE_EXCEPTION(bad_map_id);
/// \brief Thrown on \ref MDBX_BAD_TXN, i.e. a transaction handle is invalid or
/// was used after being finished.
MDBX_DECLARE_EXCEPTION(bad_transaction);
/// \brief Thrown on \ref MDBX_BAD_VALSIZE, i.e. an unsupported size of a key
/// or a data item.
MDBX_DECLARE_EXCEPTION(bad_value_size);
/// \brief Thrown on \ref MDBX_CORRUPTED, i.e. the database is corrupted or an
/// invalid data was detected.
MDBX_DECLARE_EXCEPTION(db_corrupted);
/// \brief Thrown on \ref MDBX_MAP_FULL, i.e. the database is full and the
/// environment cannot be extended.
MDBX_DECLARE_EXCEPTION(db_full);
/// \brief Thrown on \ref MDBX_INVALID, i.e. an invalid parameter was specified
/// or the environment is already closed.
MDBX_DECLARE_EXCEPTION(db_invalid);
/// \brief Thrown on \ref MDBX_TOO_LARGE, i.e. the database is too large for
/// the current process/environment.
MDBX_DECLARE_EXCEPTION(db_too_large);
/// \brief Thrown on \ref MDBX_UNABLE_EXTEND_MAPSIZE, i.e. the requested
/// geometry change needs the mapping to be re-created but it could not be
/// applied right now.
MDBX_DECLARE_EXCEPTION(db_unable_extend);
/// \brief Thrown on \ref MDBX_VERSION_MISMATCH, i.e. the database was created
/// by an incompatible version of libmdbx.
MDBX_DECLARE_EXCEPTION(db_version_mismatch);
/// \brief Thrown on \ref MDBX_WANNA_RECOVERY, i.e. the database requires
/// recovery using `mdbx_chk` and/or `mdbx_env_open_for_recovery()`.
MDBX_DECLARE_EXCEPTION(db_wanna_write_for_recovery);
/// \brief Thrown on \ref MDBX_INCOMPATIBLE, i.e. an operation is incompatible
/// with the environment/table options.
MDBX_DECLARE_EXCEPTION(incompatible_operation);
/// \brief Thrown on \ref MDBX_PAGE_FULL, i.e. an item does not fit into a page
/// of the table.
MDBX_DECLARE_EXCEPTION(internal_page_full);
/// \brief Thrown on \ref MDBX_PROBLEM, i.e. an unexpected internal problem
/// was detected.
MDBX_DECLARE_EXCEPTION(internal_problem);
/// \brief Thrown on \ref MDBX_KEYEXIST, i.e. the key/data pair already exists
/// and \ref MDBX_NOOVERWRITE was specified.
MDBX_DECLARE_EXCEPTION(key_exists);
/// \brief Thrown on \ref MDBX_EKEYMISMATCH, i.e. an insert with the append
/// hint but the key is not the last one.
MDBX_DECLARE_EXCEPTION(key_mismatch);
/// \brief Thrown on \ref MDBX_DBS_FULL, i.e. the maximum number of table
/// handles has been reached.
MDBX_DECLARE_EXCEPTION(max_maps_reached);
/// \brief Thrown on \ref MDBX_READERS_FULL, i.e. the environment's reader
/// table is full.
MDBX_DECLARE_EXCEPTION(max_readers_reached);
/// \brief Thrown on \ref MDBX_EMULTIVAL, i.e. the operation is not supported
/// for the multi-value (dupsort) key.
MDBX_DECLARE_EXCEPTION(multivalue);
/// \brief Thrown on \ref MDBX_ENODATA, i.e. the requested data is missing.
MDBX_DECLARE_EXCEPTION(no_data);
/// \brief Thrown on \ref MDBX_NOTFOUND, i.e. the requested key/data pair was
/// not found.
MDBX_DECLARE_EXCEPTION(not_found);
/// \brief Thrown on \ref MDBX_EPERM, i.e. the operation is not permitted in
/// the current state/environment mode.
MDBX_DECLARE_EXCEPTION(operation_not_permitted);
/// \brief Thrown on \ref MDBX_EACCESS, i.e. a permission was denied or the
/// environment/data is not writable.
MDBX_DECLARE_EXCEPTION(permission_denied_or_not_writeable);
/// \brief Thrown on \ref MDBX_BAD_RSLOT, i.e. a reader slot is invalid or busy.
MDBX_DECLARE_EXCEPTION(reader_slot_busy);
/// \brief Thrown on \ref MDBX_EREMOTE, i.e. a file/locking system call failed
/// for the reason that could not be diagnosed.
MDBX_DECLARE_EXCEPTION(remote_media);
/// \brief Thrown on \ref MDBX_BUSY, i.e. some resource is busy by another
/// thread or process.
MDBX_DECLARE_EXCEPTION(something_busy);
/// \brief Thrown on \ref MDBX_THREAD_MISMATCH, i.e. the transaction/cursor is
/// used by an alien thread.
MDBX_DECLARE_EXCEPTION(thread_mismatch);
/// \brief Thrown on \ref MDBX_TXN_FULL, i.e. the transaction has too many
/// dirty pages and cannot be continued.
MDBX_DECLARE_EXCEPTION(transaction_full);
/// \brief Thrown on \ref MDBX_TXN_OVERLAPPING, i.e. a write transaction was
/// started while another one is in progress by the same process.
MDBX_DECLARE_EXCEPTION(transaction_overlapping);
/// \brief Thrown on \ref MDBX_DUPLICATED_LCK, i.e. the lock file was created in
/// a manner that conflicts with the current process' state.
MDBX_DECLARE_EXCEPTION(duplicated_lck_file);
/// \brief Thrown on \ref MDBX_DANGLING_DBI, i.e. a table handle refers to
/// a non-existent table.
MDBX_DECLARE_EXCEPTION(dangling_map_id);
/// \brief Thrown on \ref MDBX_OUSTED, i.e. the current write transaction was
/// ousted by a newer one.
MDBX_DECLARE_EXCEPTION(transaction_ousted);
/// \brief Thrown on \ref MDBX_MVCC_RETARDED, i.e. the MVCC snapshot is
/// retarded too much to continue.
MDBX_DECLARE_EXCEPTION(mvcc_retarded);
/// \brief Thrown on \ref MDBX_LAGGARD_READER, i.e. the reader is lagging
/// too much behind.
MDBX_DECLARE_EXCEPTION(laggard_reader);
#undef MDBX_DECLARE_EXCEPTION

[[noreturn]] LIBMDBX_API void throw_too_small_target_buffer();
[[noreturn]] LIBMDBX_API void throw_max_length_exceeded();
[[noreturn]] LIBMDBX_API void throw_out_range();
[[noreturn]] LIBMDBX_API void throw_allocators_mismatch();
[[noreturn]] LIBMDBX_API void throw_bad_value_size();
[[noreturn]] LIBMDBX_API void throw_incomparable_cursors();

static MDBX_CXX14_CONSTEXPR size_t check_length(size_t bytes) {
  if (MDBX_UNLIKELY(bytes > size_t(MDBX_MAXDATASIZE)))
    MDBX_CXX20_UNLIKELY throw_max_length_exceeded();
  return bytes;
}

static MDBX_CXX14_CONSTEXPR size_t check_length(size_t headroom, size_t payload) {
  return check_length(check_length(headroom) + check_length(payload));
}

MDBX_MAYBE_UNUSED static MDBX_CXX14_CONSTEXPR size_t check_length(size_t headroom, size_t payload, size_t tailroom) {
  return check_length(check_length(headroom, payload) + check_length(tailroom));
}

/// \brief The only length allowed for a null pointer: zero.
/// A null pointer with a non-zero length would dereference address zero;
/// it is rejected with \ref bad_value_size.
MDBX_MAYBE_UNUSED static MDBX_CXX14_CONSTEXPR size_t check_nullptr_length(size_t bytes) {
  if (MDBX_UNLIKELY(bytes != 0))
    MDBX_CXX20_UNLIKELY throw_bad_value_size();
  return bytes;
}

/// end of cxx_exceptions @}

//------------------------------------------------------------------------------

/// \defgroup cxx_data Slices and Buffers
/// @{

/// \brief References a data located outside the slice.
///
/// The `slice` is similar in many ways to `std::string_view`, but it
/// implements specific capabilities and manipulates with bytes but
/// not a characters.
///
/// \note A slice is non-owning: it references data allocated elsewhere and
/// never copies or releases it. The referenced data must remain valid as long
/// as the slice is used. Passing a temporary (e.g. a `std::string` converted
/// inline in a call argument, like `txn.insert(map, mdbx::slice(std::to_string(k)), ...)`)
/// is safe: a temporary lives until the end of the full-expression and libmdbx
/// copies the bytes during the call. In contrast, storing such a slice beyond
/// the statement leaves it dangling.
///
/// \copydetails MDBX_val
struct LIBMDBX_API_TYPE slice : public ::MDBX_val {
  /// \todo slice& operator<<(slice&, ...) for reading
  /// \todo key-to-value (parse/unpack) functions
  /// \todo template<class X> key(X); for decoding keys while reading

  enum : size_t { max_length = MDBX_MAXDATASIZE, npos = size_t(-1) };

  /// \brief Create an empty slice.
  MDBX_CXX11_CONSTEXPR slice() noexcept;

  /// \brief Create a slice that refers to [0,bytes-1] of memory bytes pointed by ptr.
  /// \param ptr    The address of the data to be referenced.
  /// \param bytes  The number of bytes to be referenced.
  MDBX_CXX14_CONSTEXPR slice(const void *ptr, size_t bytes);

  /// \brief Create a slice that refers to [begin,end] of memory bytes.
  /// \param begin  The address of the first byte to be referenced.
  /// \param end    The address just after the last byte to be referenced.
  MDBX_CXX14_CONSTEXPR slice(const void *begin, const void *end);

  /// \brief Create a slice that refers to text[0,strlen(text)-1].
  /// \param text  The null-terminated string literal to be referenced.
  template <size_t SIZE> MDBX_CXX14_CONSTEXPR slice(const char (&text)[SIZE]) : slice(text, SIZE - 1) {
    MDBX_CONSTEXPR_ASSERT(SIZE > 0 && text[SIZE - 1] == '\0');
  }
  /// \brief Create a slice that refers to c_str[0,strlen(c_str)-1].
  /// \param c_str  The null-terminated string to be referenced.
  explicit MDBX_CXX17_CONSTEXPR slice(const char *c_str);

  /// \brief Create a slice that refers to the contents of "str".
  /// \note 'explicit' to avoid reference to the temporary std::string instance.
  /// The slice is non-owning: it references the string's data without copying.
  /// Passing a temporary string inline in a call argument is safe (the temporary
  /// lives until the end of the full-expression and libmdbx copies the bytes
  /// during the call), but storing the resulting slice beyond the statement
  /// leaves it dangling — use mdbx::buffer to own a copy.
  /// \param str  The string which content is to be referenced.
  template <class CHAR, class T, class A>
  explicit MDBX_CXX20_CONSTEXPR slice(const ::std::basic_string<CHAR, T, A> &str)
      : slice(str.data(), str.length() * sizeof(CHAR)) {}

  /// \brief Create a slice that refers to the contents of the given \ref MDBX_val.
  /// \param src  The source \ref MDBX_val instance.
  MDBX_CXX14_CONSTEXPR slice(const MDBX_val &src);
  /// \brief Copy constructor.
  /// \param src  The source slice.
  MDBX_CXX11_CONSTEXPR slice(const slice &src) noexcept = default;
  /// \brief Takes over a temporary \ref MDBX_val instance: the slice refers to its
  ///   bytes and the moved-from `src.iov_base` is set to null, while
  ///   `src.iov_len` is kept (matching the library's own moved-from state);
  ///   the slice does not own the bytes.
  /// \param src  The source \ref MDBX_val to be moved from.
  MDBX_CXX14_CONSTEXPR slice(MDBX_val &&src);
  /// \brief Move constructor: the source slice is invalidated.
  /// \param src  The source slice to be moved from.
  MDBX_CXX14_CONSTEXPR slice(slice &&src) noexcept;

#if defined(DOXYGEN) || (defined(__cpp_lib_span) && __cpp_lib_span >= 202002L)
  /// \brief Create a slice that refers to the contents of the given `std::span`.
  /// \param span  The source span of a standard-layout type.
  template <typename POD>
  MDBX_CXX14_CONSTEXPR slice(const ::std::span<POD> &span) : slice(span.data(), span.size_bytes()) {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
  }

  /// \brief Returns a read-only `std::span` view of the referenced data.
  /// \returns The `std::span` of the slice content.
  /// \throws mdbx::bad_value_size if the content size is not a multiple of `sizeof(POD)`.
  template <typename POD> MDBX_CXX14_CONSTEXPR ::std::span<const POD> as_span() const {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
    if (MDBX_LIKELY(size() % sizeof(POD) == 0))
      MDBX_CXX20_LIKELY
    return ::std::span<const POD>(static_cast<const POD *>(data()), size() / sizeof(POD));
    throw_bad_value_size();
  }

  /// \brief Returns a mutable `std::span` view of the referenced data.
  /// \returns The `std::span` of the slice content.
  /// \throws mdbx::bad_value_size if the content size is not a multiple of `sizeof(POD)`.
  template <typename POD> MDBX_CXX14_CONSTEXPR ::std::span<POD> as_span() {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
    if (MDBX_LIKELY(size() % sizeof(POD) == 0))
      MDBX_CXX20_LIKELY
    return ::std::span<POD>(static_cast<POD *>(data()), size() / sizeof(POD));
    throw_bad_value_size();
  }

  /// \brief Returns a read-only `std::span` of bytes of the referenced data.
  /// \returns The `std::span<const byte>` view.
  MDBX_CXX14_CONSTEXPR ::std::span<const byte> bytes() const { return as_span<const byte>(); }
  /// \brief Returns a mutable `std::span` of bytes of the referenced data.
  /// \returns The `std::span<byte>` view.
  MDBX_CXX14_CONSTEXPR ::std::span<byte> bytes() { return as_span<byte>(); }
  /// \brief Returns a read-only `std::span` of chars of the referenced data.
  /// \returns The `std::span<const char>` view.
  MDBX_CXX14_CONSTEXPR ::std::span<const char> chars() const { return as_span<const char>(); }
  /// \brief Returns a mutable `std::span` of chars of the referenced data.
  /// \returns The `std::span<char>` view.
  MDBX_CXX14_CONSTEXPR ::std::span<char> chars() { return as_span<char>(); }
#endif /* __cpp_lib_span >= 202002L */

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Create a slice that refers to the same contents as "string_view"
  /// \param sv  The source string view.
  template <class CHAR, class T>
  MDBX_CXX14_CONSTEXPR slice(const ::std::basic_string_view<CHAR, T> &sv)
      : slice(sv.data(), sv.size() * sizeof(CHAR)) {}

  /// \brief Takes over a temporary string view: the slice refers to its data
  /// and the moved-from view is emptied.
  /// \param sv  The source string view to be moved from.
  template <class CHAR, class T> slice(::std::basic_string_view<CHAR, T> &&sv) : slice(sv) { sv = {}; }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Wraps a null-terminated string literal into a slice.
  /// \param text  The string literal to be referenced.
  /// \returns The slice referring to the literal without the terminating null.
  template <size_t SIZE> static MDBX_CXX14_CONSTEXPR slice wrap(const char (&text)[SIZE]) { return slice(text); }

  /// \brief Wraps an object of a standard-layout type into a slice.
  /// \param pod  The object to be referenced.
  /// \returns The slice referring to the object's bytes.
  template <typename POD> MDBX_CXX14_CONSTEXPR static slice wrap(const POD &pod) {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
    return slice(&pod, sizeof(pod));
  }

  /// \brief Assigns the slice to reference [0,bytes-1] of memory pointed by ptr.
  /// \param ptr    The address of the data to be referenced.
  /// \param bytes  The number of bytes to be referenced.
  /// \returns A reference to this instance.
  inline slice &assign(const void *ptr, size_t bytes);
  /// \brief Assigns the slice to reference the same data as the source slice.
  /// \param src  The source slice.
  /// \returns A reference to this instance.
  inline slice &assign(const slice &src) noexcept;
  /// \brief Assigns the slice to reference the contents of the given \ref MDBX_val.
  /// \param src  The source \ref MDBX_val instance.
  /// \returns A reference to this instance.
  inline slice &assign(const ::MDBX_val &src);
  /// \brief Moves the source slice into this one, invalidating the source.
  /// \param src  The source slice to be moved from.
  /// \returns A reference to this instance.
  inline slice &assign(slice &&src) noexcept;
  /// \brief Takes over a temporary \ref MDBX_val instance.
  /// \param src  The source \ref MDBX_val to be moved from.
  /// \returns A reference to this instance.
  inline slice &assign(::MDBX_val &&src);
  /// \brief Assigns the slice to reference the [begin,end) memory range.
  /// \param begin  The address of the first byte to be referenced.
  /// \param end    The address just after the last byte to be referenced.
  /// \returns A reference to this instance.
  inline slice &assign(const void *begin, const void *end);
  /// \brief Assigns the slice to reference the contents of the given string.
  /// \param str  The source string.
  /// \returns A reference to this instance.
  template <class CHAR, class T, class ALLOCATOR> slice &assign(const ::std::basic_string<CHAR, T, ALLOCATOR> &str) {
    return assign(str.data(), str.size() * sizeof(CHAR));
  }
  /// \brief Assigns the slice to reference a null-terminated string.
  /// \param c_str  The source null-terminated string.
  /// \returns A reference to this instance.
  inline slice &assign(const char *c_str);
#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Assigns the slice to reference the contents of the given string view.
  /// \param view  The source string view.
  /// \returns A reference to this instance.
  template <class CHAR, class T> slice &assign(const ::std::basic_string_view<CHAR, T> &view) {
    return assign(view.data(), view.size() * sizeof(CHAR));
  }
  /// \brief Takes over a temporary string view and assigns the slice to its contents.
  /// \param view  The source string view to be moved from.
  /// \returns A reference to this instance.
  template <class CHAR, class T> slice &assign(::std::basic_string_view<CHAR, T> &&view) {
    assign(view);
    view = {};
    return *this;
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  slice &operator=(const slice &) noexcept = default;
  /// \brief Move assignment: the source slice is invalidated.
  /// \param src  The source slice to be moved from.
  /// \returns A reference to this instance.
  inline slice &operator=(slice &&src) noexcept;
  /// \brief Takes over a temporary \ref MDBX_val instance.
  /// \param src  The source \ref MDBX_val to be moved from.
  /// \returns A reference to this instance.
  inline slice &operator=(::MDBX_val &&src);
  /// \brief Returns the pointer to the underlying \ref MDBX_val instance.
  /// \returns The mutable pointer to this instance as \ref MDBX_val.
  operator MDBX_val *() noexcept { return this; }
  /// \brief Returns the pointer to the underlying \ref MDBX_val instance.
  /// \returns The constant pointer to this instance as \ref MDBX_val.
  operator const MDBX_val *() const noexcept { return this; }

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Assigns the slice to reference the contents of the given string view.
  /// \param view  The source string view.
  /// \returns A reference to this instance.
  template <class CHAR, class T> slice &operator=(const ::std::basic_string_view<CHAR, T> &view) {
    return assign(view);
  }

  /// \brief Takes over a temporary string view and assigns the slice to its contents.
  /// \param view  The source string view to be moved from.
  /// \returns A reference to this instance.
  template <class CHAR, class T> slice &operator=(::std::basic_string_view<CHAR, T> &&view) { return assign(view); }

  /// \brief Return a string_view that references the same data as this slice.
  /// \returns The `std::basic_string_view` of the slice content.
  template <class CHAR = char, class T = ::std::char_traits<CHAR>>
  MDBX_CXX11_CONSTEXPR ::std::basic_string_view<CHAR, T> string_view() const noexcept {
    static_assert(sizeof(CHAR) == 1, "Must be single byte characters");
    return ::std::basic_string_view<CHAR, T>(char_ptr(), length());
  }

  /// \brief Return a string_view that references the same data as this slice.
  /// \returns The `std::basic_string_view` of the slice content.
  template <class CHAR, class T>
  MDBX_CXX11_CONSTEXPR explicit operator ::std::basic_string_view<CHAR, T>() const noexcept {
    return this->string_view<CHAR, T>();
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Copies the slice content into a `std::basic_string`.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with a copy of the slice content.
  template <class CHAR = char, class T = ::std::char_traits<CHAR>, class ALLOCATOR = default_allocator>
  MDBX_CXX20_CONSTEXPR ::std::basic_string<CHAR, T, ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const {
    static_assert(sizeof(CHAR) == 1, "Must be single byte characters");
    return ::std::basic_string<CHAR, T, ALLOCATOR>(char_ptr(), length(), alloc);
  }

  /// \brief Copies the slice content into a `std::basic_string`.
  /// \returns The string with a copy of the slice content.
  template <class CHAR, class T, class ALLOCATOR>
  MDBX_CXX20_CONSTEXPR explicit operator ::std::basic_string<CHAR, T, ALLOCATOR>() const {
    return as_string<CHAR, T, ALLOCATOR>();
  }

  /// \brief Returns a string with a hexadecimal dump of the slice content.
  /// \param uppercase   Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting string.
  /// \returns The string with the hexadecimal dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_hex_string(bool uppercase = false, unsigned wrap_width = 0,
                                         const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a string with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting string.
  /// \returns The string with the Base58 dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_base58_string(unsigned wrap_width = 0, const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a string with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting string.
  /// \returns The string with the Base64 dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_base64_string(unsigned wrap_width = 0, const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a hexadecimal dump of the slice content.
  /// \param uppercase   Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting buffer.
  /// \returns The buffer with the hexadecimal dump.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> encode_hex(bool uppercase = false, unsigned wrap_width = 0,
                                                       const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting buffer.
  /// \returns The buffer with the Base58 dump.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> encode_base58(unsigned wrap_width = 0,
                                                          const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the resulting buffer.
  /// \returns The buffer with the Base64 dump.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> encode_base64(unsigned wrap_width = 0,
                                                          const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes hexadecimal dump from the slice content to returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc          The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> hex_decode(bool ignore_spaces = false,
                                                       const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes [Base58](https://en.wikipedia.org/wiki/Base58) dump
  /// from the slice content to returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc          The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> base58_decode(bool ignore_spaces = false,
                                                          const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes [Base64](https://en.wikipedia.org/wiki/Base64) dump
  /// from the slice content to returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc          The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, class CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> base64_decode(bool ignore_spaces = false,
                                                          const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Checks whether the content of the slice is printable.
  /// \param [in] disable_utf8 By default if `disable_utf8` is `false` function
  /// checks that content bytes are printable ASCII-7 characters or a valid UTF8
  /// sequences. Otherwise, if `disable_utf8` is `true` function checks that
  /// content bytes are printable extended 8-bit ASCII codes.
  /// \returns `true` if the content is printable.
  MDBX_NOTHROW_PURE_FUNCTION bool is_printable(bool disable_utf8 = false) const noexcept;

  /// \brief Checks whether the content of the slice is a hexadecimal dump.
  /// \param [in] ignore_spaces If `true` function will skips spaces surrounding
  /// (before, between and after) a encoded bytes. However, spaces should not
  /// break a pair of characters encoding a single byte.
  /// \returns `true` if the content is a valid hexadecimal dump.
  MDBX_NOTHROW_PURE_FUNCTION inline bool is_hex(bool ignore_spaces = false) const noexcept;

  /// \brief Checks whether the content of the slice is a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump.
  /// \param [in] ignore_spaces If `true` function will skips spaces surrounding
  /// (before, between and after) a encoded bytes. However, spaces should not
  /// break a code group of characters.
  /// \returns `true` if the content is a valid Base58 dump.
  MDBX_NOTHROW_PURE_FUNCTION inline bool is_base58(bool ignore_spaces = false) const noexcept;

  /// \brief Checks whether the content of the slice is a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump.
  /// \param [in] ignore_spaces If `true` function will skips spaces surrounding
  /// (before, between and after) a encoded bytes. However, spaces should not
  /// break a code group of characters.
  /// \returns `true` if the content is a valid Base64 dump.
  MDBX_NOTHROW_PURE_FUNCTION inline bool is_base64(bool ignore_spaces = false) const noexcept;

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Swaps the slice content with the given string view.
  /// \param view  The string view to swap with.
  template <class CHAR, class T> void swap(::std::basic_string_view<CHAR, T> &view) noexcept {
    static_assert(sizeof(CHAR) == 1, "Must be single byte characters");
    const auto temp = ::std::basic_string_view<CHAR, T>(*this);
    *this = view;
    view = temp;
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Swaps this slice with another one.
  /// \param other  The slice to swap with.
  MDBX_CXX20_CONSTEXPR void swap(slice &other) noexcept {
    auto temp = other;
    other = *this;
    *this = temp;
  }

  /// \brief Returns casted to pointer to byte an address of data.
  /// \returns The read-only pointer to the first byte.
  MDBX_CXX11_CONSTEXPR const byte *byte_ptr() const noexcept;
  /// \brief Returns casted to pointer to byte an address of data.
  /// \returns The mutable pointer to the first byte.
  MDBX_CXX11_CONSTEXPR byte *byte_ptr() noexcept;

  /// \brief Returns casted to pointer to byte an end of data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const byte *end_byte_ptr() const noexcept;
  /// \brief Returns casted to pointer to byte an end of data.
  /// \returns The mutable pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR byte *end_byte_ptr() noexcept;

  /// \brief Returns casted to pointer to char an address of data.
  /// \returns The read-only pointer to the first byte.
  MDBX_CXX11_CONSTEXPR const char *char_ptr() const noexcept;
  /// \brief Returns casted to pointer to char an address of data.
  /// \returns The mutable pointer to the first byte.
  MDBX_CXX11_CONSTEXPR char *char_ptr() noexcept;

  /// \brief Returns casted to pointer to char an end of data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const char *end_char_ptr() const noexcept;
  /// \brief Returns casted to pointer to char an end of data.
  /// \returns The mutable pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR char *end_char_ptr() noexcept;

  /// \brief Return a pointer to the beginning of the referenced data.
  /// \returns The read-only pointer to the data.
  MDBX_CXX11_CONSTEXPR const void *data() const noexcept;
  /// \brief Return a pointer to the beginning of the referenced data.
  /// \returns The mutable pointer to the data.
  MDBX_CXX11_CONSTEXPR void *data() noexcept;

  /// \brief Return a pointer to the end of the referenced data.
  /// \details An iterator-style pointer (one past the last byte).
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const byte *end() const noexcept;
  /// \copydoc end() const
  MDBX_CXX11_CONSTEXPR byte *end() noexcept;

  /// \brief Returns the number of bytes.
  /// \returns The length of the referenced data in bytes.
  MDBX_CXX11_CONSTEXPR size_t length() const noexcept;

  /// \brief Set slice length.
  /// \param bytes  The new length of the slice.
  /// \returns A reference to this instance.
  MDBX_CXX14_CONSTEXPR slice &set_length(size_t bytes);

  /// \brief Sets the length by specifying the end of the slice data.
  /// \param ptr  The address just after the last byte of the slice.
  /// \returns A reference to this instance.
  MDBX_CXX14_CONSTEXPR slice &set_end(const void *ptr);

  /// \brief Checks whether the slice is empty.
  /// \returns `true` if the slice length is zero.
  MDBX_CXX11_CONSTEXPR bool empty() const noexcept;

  /// \brief Checks whether the slice data pointer is nullptr.
  /// \returns `true` if the slice refers to nothing.
  MDBX_CXX11_CONSTEXPR bool is_null() const noexcept;

  /// \brief Returns the number of bytes.
  /// \returns The length of the referenced data in bytes.
  MDBX_CXX11_CONSTEXPR size_t size() const noexcept;

  /// \brief Returns true if slice is not empty.
  /// \returns `true` if the slice is not null.
  MDBX_CXX11_CONSTEXPR operator bool() const noexcept;
  /// \brief Same as the const version; keeps `if (s)` away from `operator MDBX_val *()`.
  /// \returns `true` if the slice is not null.
  MDBX_CXX14_CONSTEXPR operator bool() noexcept { return !is_null(); }

  /// \brief Invalidates the slice by nullifying the data pointer while
  /// preserving the length; makes \ref is_valid() return `false` unless the
  /// slice was already of zero length.
  MDBX_CXX14_CONSTEXPR void invalidate() noexcept;

  /// \brief Makes the slice empty and referencing to nothing.
  MDBX_CXX14_CONSTEXPR void clear() noexcept;

  /// \brief Drops the first "n" bytes from this slice.
  /// \param n  The number of bytes to drop.
  /// \pre REQUIRES: `n <= size()`
  inline void remove_prefix(size_t n) noexcept;

  /// \brief Drops the last "n" bytes from this slice.
  /// \param n  The number of bytes to drop.
  /// \pre REQUIRES: `n <= size()`
  inline void remove_suffix(size_t n) noexcept;

  /// \brief Drops the first "n" bytes from this slice.
  /// \param n  The number of bytes to drop.
  /// \throws std::out_of_range if `n > size()`
  inline void safe_remove_prefix(size_t n);

  /// \brief Drops the last "n" bytes from this slice.
  /// \param n  The number of bytes to drop.
  /// \throws std::out_of_range if `n > size()`
  inline void safe_remove_suffix(size_t n);

  /// \brief Checks if the data starts with the given prefix.
  /// \param prefix  The expected prefix.
  /// \returns `true` if the slice starts with the given prefix.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool starts_with(const slice &prefix) const noexcept;

  /// \brief Checks if the data ends with the given suffix.
  /// \param suffix  The expected suffix.
  /// \returns `true` if the slice ends with the given suffix.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool ends_with(const slice &suffix) const noexcept;

  /// \brief Returns the nth byte in the referenced data.
  /// \param n  The index of the byte.
  /// \returns The nth byte.
  /// \pre REQUIRES: `n < size()`
  MDBX_CXX11_CONSTEXPR byte operator[](size_t n) const noexcept;

  /// \brief Returns the nth byte in the referenced data with bounds checking.
  /// \param n  The index of the byte.
  /// \returns The nth byte.
  /// \throws std::out_of_range if `n >= size()`
  MDBX_CXX11_CONSTEXPR byte at(size_t n) const;

  /// \brief Returns the first "n" bytes of the slice.
  /// \param n  The number of bytes.
  /// \returns The sub-slice of the first `n` bytes.
  /// \pre REQUIRES: `n <= size()`
  MDBX_CXX14_CONSTEXPR slice head(size_t n) const noexcept;

  /// \brief Returns the last "n" bytes of the slice.
  /// \param n  The number of bytes.
  /// \returns The sub-slice of the last `n` bytes.
  /// \pre REQUIRES: `n <= size()`
  MDBX_CXX14_CONSTEXPR slice tail(size_t n) const noexcept;

  /// \brief Returns the middle "n" bytes of the slice.
  /// \param from  The offset of the first byte.
  /// \param n     The number of bytes.
  /// \returns The sub-slice of `n` bytes starting at `from`.
  /// \pre REQUIRES: `from + n <= size()`
  MDBX_CXX14_CONSTEXPR slice middle(size_t from, size_t n) const noexcept;

  /// \brief Returns an iterator to the beginning of the referenced data.
  /// \returns The read-only pointer to the first byte.
  MDBX_CXX11_CONSTEXPR const byte *begin() const noexcept;
  /// \copydoc begin() const
  MDBX_CXX11_CONSTEXPR byte *begin() noexcept;

  /// \brief Returns the first byte of the referenced data.
  /// \returns The first byte.
  /// \pre REQUIRES: `!empty()`
  MDBX_CXX11_CONSTEXPR byte front() const noexcept { return byte_ptr()[0]; }

  /// \brief Returns the last byte of the referenced data.
  /// \returns The last byte.
  /// \pre REQUIRES: `!empty()`
  MDBX_CXX11_CONSTEXPR byte back() const noexcept { return byte_ptr()[size() - 1]; }

  /// \brief Returns a sub-slice of [pos, pos+count) clamped to the end.
  /// \param pos    The offset of the first byte of the sub-slice.
  /// \param count  The number of bytes, `npos` means "up to the end".
  /// \returns The sub-slice.
  /// \throws std::out_of_range if `pos > size()`.
  MDBX_CXX14_CONSTEXPR slice substr(size_t pos, size_t count = npos) const {
    if (MDBX_UNLIKELY(pos > size()))
      MDBX_CXX20_UNLIKELY throw_out_range();
    return middle(pos, (::std::min)(count, size() - pos));
  }

  /// \brief Finds the first occurrence of `needle` starting at `pos`.
  /// \param needle  The sub-slice to search for.
  /// \param pos     The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find(const slice &needle, size_t pos = 0) const noexcept;
  /// \brief Finds the first occurrence of the byte `c` starting at `pos`.
  /// \param c    The byte to search for.
  /// \param pos  The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find(byte c, size_t pos = 0) const noexcept;

  /// \brief Finds the last occurrence of `needle` not after `pos`.
  /// \param needle  The sub-slice to search for.
  /// \param pos     The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t rfind(const slice &needle, size_t pos = npos) const noexcept;
  /// \brief Finds the last occurrence of the byte `c` not after `pos`.
  /// \param c    The byte to search for.
  /// \param pos  The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t rfind(byte c, size_t pos = npos) const noexcept;

  /// \brief Finds the first byte matching any byte of `chars` starting at `pos`.
  /// \param chars  The set of bytes to search for.
  /// \param pos    The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_first_of(const slice &chars,
                                                                       size_t pos = 0) const noexcept;
  /// \brief Finds the first occurrence of the byte `c` starting at `pos`.
  /// \param c    The byte to search for.
  /// \param pos  The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_first_of(byte c, size_t pos = 0) const noexcept;

  /// \brief Finds the last byte matching any byte of `chars` not after `pos`.
  /// \param chars  The set of bytes to search for.
  /// \param pos    The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_last_of(const slice &chars,
                                                                      size_t pos = npos) const noexcept;
  /// \brief Finds the last occurrence of the byte `c` not after `pos`.
  /// \param c    The byte to search for.
  /// \param pos  The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_last_of(byte c, size_t pos = npos) const noexcept;

  /// \brief Finds the first byte not matching any byte of `chars` starting at `pos`.
  /// \param chars  The set of bytes to avoid.
  /// \param pos    The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_first_not_of(const slice &chars,
                                                                           size_t pos = 0) const noexcept;
  /// \brief Finds the first byte not equal to `c` starting at `pos`.
  /// \param c    The byte to avoid.
  /// \param pos  The position to start the search from.
  /// \returns The position of the first match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_first_not_of(byte c, size_t pos = 0) const noexcept;

  /// \brief Finds the last byte not matching any byte of `chars` not after `pos`.
  /// \param chars  The set of bytes to avoid.
  /// \param pos    The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_last_not_of(const slice &chars,
                                                                          size_t pos = npos) const noexcept;
  /// \brief Finds the last byte not equal to `c` not after `pos`.
  /// \param c    The byte to avoid.
  /// \param pos  The position to search backwards from, `npos` means the end.
  /// \returns The position of the last match or `npos`.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t find_last_not_of(byte c, size_t pos = npos) const noexcept;

  /// \brief Checks whether the referenced data contains `needle` as a sub-slice.
  /// \param needle  The sub-slice to search for.
  /// \returns `true` if the slice contains the given sub-slice.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool contains(const slice &needle) const noexcept {
    return find(needle) != npos;
  }

  /// \brief Lexicographical three-way comparison with another slice.
  /// \param other  The slice to compare with.
  /// \returns `0` if equal, `<0` if this slice is less, `>0` if greater.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR intptr_t compare(const slice &other) const noexcept {
    return compare_lexicographically(*this, other);
  }

  /// \brief Returns the first "n" bytes of the slice.
  /// \param n  The number of bytes.
  /// \returns The sub-slice of the first `n` bytes.
  /// \throws std::out_of_range if `n > size()`
  MDBX_CXX14_CONSTEXPR slice safe_head(size_t n) const;

  /// \brief Returns the last "n" bytes of the slice.
  /// \param n  The number of bytes.
  /// \returns The sub-slice of the last `n` bytes.
  /// \throws std::out_of_range if `n > size()`
  MDBX_CXX14_CONSTEXPR slice safe_tail(size_t n) const;

  /// \brief Returns the middle "n" bytes of the slice.
  /// \param from  The offset of the first byte.
  /// \param n     The number of bytes.
  /// \returns The sub-slice of `n` bytes starting at `from`.
  /// \throws std::out_of_range if `from + n > size()`
  MDBX_CXX14_CONSTEXPR slice safe_middle(size_t from, size_t n) const;

  /// \brief Returns the hash value of referenced data.
  /// \attention Function implementation and returned hash values may change
  /// from version to version, and in future the t1ha3 will be used here.
  /// Therefore values obtained from this function shouldn't be persisted
  /// anywhere.
  /// \returns The hash value of the referenced data.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t hash_value() const noexcept;

  /// \brief Three-way fast non-lexicographically length-based comparison.
  /// \details Firstly compares length and if it equal then compare content
  /// lexicographically. \return value:
  ///  `== 0` if `a` the same as `b`;
  ///   `< 0` if `a` shorter than `b`,
  ///             or the same length and lexicographically less than `b`;
  ///   `> 0` if `a` longer than `b`,
  ///             or the same length and lexicographically greater than `b`.
  /// \param a  The left-hand slice.
  /// \param b  The right-hand slice.
  /// \returns The three-way comparison result.
  MDBX_NOTHROW_PURE_FUNCTION static MDBX_CXX14_CONSTEXPR intptr_t compare_fast(const slice &a, const slice &b) noexcept;

  /// \brief Three-way lexicographically comparison.
  /// \return value:
  ///  `== 0` if `a` lexicographically equal `b`;
  ///   `< 0` if `a` lexicographically less than `b`;
  ///   `> 0` if `a` lexicographically greater than `b`.
  /// \param a  The left-hand slice.
  /// \param b  The right-hand slice.
  /// \returns The three-way comparison result.
  MDBX_NOTHROW_PURE_FUNCTION static MDBX_CXX14_CONSTEXPR intptr_t compare_lexicographically(const slice &a,
                                                                                            const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator==(const slice &a, const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator<(const slice &a, const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator>(const slice &a, const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator<=(const slice &a, const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator>=(const slice &a, const slice &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator!=(const slice &a, const slice &b) noexcept;
#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
  friend MDBX_CXX14_CONSTEXPR auto operator<=>(const slice &a, const slice &b) noexcept;
#endif /* __cpp_impl_three_way_comparison */

  /// \brief Checks the slice is valid: returns `false` only if it has a null
  /// data pointer and a non-zero length.
  /// \returns `true` if the slice is valid.
  MDBX_CXX11_CONSTEXPR bool is_valid() const noexcept { return !(iov_base == nullptr && iov_len != 0); }

  /// \brief Builds an invalid slice that has a non-zero length and refers to
  /// a null address.
  /// \returns The invalid slice instance.
  MDBX_CXX14_CONSTEXPR static slice invalid() noexcept {
    return slice(/* using special constructor without length checking */ ~size_t(0));
  }

  /// \brief Build a null slice which zero length and refers to null address.
  /// \returns The null slice instance.
  MDBX_CXX14_CONSTEXPR static slice null() noexcept { return slice(nullptr, size_t(0)); }

  /// \brief Builds a "blank" slice for a \ref MDBX_RESERVE allocation: it has
  /// a null data pointer but the requested non-zero length, which the C API
  /// fills in on return from `txn::insert_reserve()`,
  /// `txn::upsert_reserve()` and friends.
  /// \details Since a null pointer with a non-zero length is rejected by the
  /// ordinary \ref slice() constructor and `assign()` (issue #142), this
  /// explicit factory is the only way to spell the reserve form; it builds the
  /// blank via the special private constructor (like \ref invalid()) but still
  /// validates `length` against \ref MDBX_MAXDATASIZE, so an oversize request
  /// throws `std::length_error` exactly as the old `slice(nullptr, N)`
  /// spelling did.
  /// \param length  The requested length of the blank slice.
  /// \returns The blank slice for a reserve allocation.
  /// \throws std::length_error if `length` exceeds the maximum value size.
  MDBX_CXX14_CONSTEXPR static slice blank_for_put_reserve(size_t length) {
    return slice(/* validates length, then uses the unchecked special ctor */ check_length(length));
  }

  /// \brief Reinterprets the slice content as an object of a standard-layout type.
  /// \returns A copy of the object of type `POD` read from the slice bytes.
  /// \throws mdbx::bad_value_size if the slice length does not exactly match `sizeof(POD)`.
  template <typename POD> MDBX_CXX14_CONSTEXPR POD as_pod() const {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
    if (MDBX_LIKELY(size() == sizeof(POD)))
      MDBX_CXX20_LIKELY {
        POD _as_pod_local;
        memcpy(&_as_pod_local, data(), sizeof(_as_pod_local));
        return _as_pod_local;
      }
    throw_bad_value_size();
  }

#if defined(DOXYGEN) || (defined(__cpp_lib_optional) && __cpp_lib_optional >= 201606L)
  /// \brief Non-throwing counterpart of `as_pod()`: returns `std::nullopt`
  /// when the slice length does not exactly match `sizeof(POD)`.
  /// \note The returned object is a copy of the referenced bytes, so the slice
  /// need not outlive the result (issue #144).
  /// \returns The object of type `POD` or `std::nullopt` on a size mismatch.
  template <typename POD> MDBX_CXX14_CONSTEXPR ::std::optional<POD> try_as_pod() const {
    static_assert(::std::is_standard_layout<POD>::value && !::std::is_pointer<POD>::value,
                  "Must be a standard layout type!");
    if (MDBX_LIKELY(size() == sizeof(POD))) {
      POD _as_pod_local;
      memcpy(&_as_pod_local, data(), sizeof(_as_pod_local));
      return _as_pod_local;
    }
    return ::std::nullopt;
  }
#endif /* __cpp_lib_optional >= 201606L */

#ifdef MDBX_U128_TYPE
  /// \brief Reinterprets the slice as a 128-bit unsigned integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR MDBX_U128_TYPE as_uint128() const { return as_pod<MDBX_U128_TYPE>(); }
#endif /* MDBX_U128_TYPE */
  /// \brief Reinterprets the slice as a 64-bit unsigned integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR uint64_t as_uint64() const { return as_pod<uint64_t>(); }
  /// \brief Reinterprets the slice as a 32-bit unsigned integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR uint32_t as_uint32() const { return as_pod<uint32_t>(); }
  /// \brief Reinterprets the slice as a 16-bit unsigned integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR uint16_t as_uint16() const { return as_pod<uint16_t>(); }
  /// \brief Reinterprets the slice as an 8-bit unsigned integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR uint8_t as_uint8() const { return as_pod<uint8_t>(); }

#ifdef MDBX_I128_TYPE
  /// \brief Reinterprets the slice as a 128-bit signed integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR MDBX_I128_TYPE as_int128() const { return as_pod<MDBX_I128_TYPE>(); }
#endif /* MDBX_I128_TYPE */
  /// \brief Reinterprets the slice as a 64-bit signed integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR int64_t as_int64() const { return as_pod<int64_t>(); }
  /// \brief Reinterprets the slice as a 32-bit signed integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR int32_t as_int32() const { return as_pod<int32_t>(); }
  /// \brief Reinterprets the slice as a 16-bit signed integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR int16_t as_int16() const { return as_pod<int16_t>(); }
  /// \brief Reinterprets the slice as an 8-bit signed integer.
  /// \returns The value; throws mdbx::bad_value_size on a size mismatch.
  MDBX_CXX14_CONSTEXPR int8_t as_int8() const { return as_pod<int8_t>(); }

#ifdef MDBX_U128_TYPE
  /// \brief Reinterprets the slice as a 128-bit unsigned integer, adapting
  /// the value when the slice is shorter than 16 bytes.
  /// \returns The value adapted to the actual slice length.
  MDBX_U128_TYPE as_uint128_adapt() const;
#endif /* MDBX_U128_TYPE */
  /// \brief Reinterprets the slice as a 64-bit unsigned integer, adapting
  /// the value when the slice is shorter than 8 bytes.
  /// \returns The value adapted to the actual slice length.
  uint64_t as_uint64_adapt() const;
  /// \brief Reinterprets the slice as a 32-bit unsigned integer, adapting
  /// the value when the slice is shorter than 4 bytes.
  /// \returns The value adapted to the actual slice length.
  uint32_t as_uint32_adapt() const;
  /// \brief Reinterprets the slice as a 16-bit unsigned integer, adapting
  /// the value when the slice is shorter than 2 bytes.
  /// \returns The value adapted to the actual slice length.
  uint16_t as_uint16_adapt() const;
  /// \brief Reinterprets the slice as an 8-bit unsigned integer.
  /// \returns The value adapted to the actual slice length.
  uint8_t as_uint8_adapt() const;

#ifdef MDBX_I128_TYPE
  /// \brief Reinterprets the slice as a 128-bit signed integer, adapting
  /// the value when the slice is shorter than 16 bytes.
  /// \returns The value adapted to the actual slice length.
  MDBX_I128_TYPE as_int128_adapt() const;
#endif /* MDBX_I128_TYPE */
  /// \brief Reinterprets the slice as a 64-bit signed integer, adapting
  /// the value when the slice is shorter than 8 bytes.
  /// \returns The value adapted to the actual slice length.
  int64_t as_int64_adapt() const;
  /// \brief Reinterprets the slice as a 32-bit signed integer, adapting
  /// the value when the slice is shorter than 4 bytes.
  /// \returns The value adapted to the actual slice length.
  int32_t as_int32_adapt() const;
  /// \brief Reinterprets the slice as a 16-bit signed integer, adapting
  /// the value when the slice is shorter than 2 bytes.
  /// \returns The value adapted to the actual slice length.
  int16_t as_int16_adapt() const;
  /// \brief Reinterprets the slice as an 8-bit signed integer.
  /// \returns The value adapted to the actual slice length.
  int8_t as_int8_adapt() const;

protected:
  /// \brief A special constructor without length checking, for \ref invalid()
  /// and \ref blank_for_put_reserve().
  /// \param invalid_length  The length to be set with a null data pointer.
  MDBX_CXX11_CONSTEXPR slice(size_t invalid_length) noexcept : ::MDBX_val({nullptr, invalid_length}) {}
};

/// \brief Combines data slice with boolean flag to represent result of certain operations.
struct value_result {
  slice value; ///< \brief The resulting data slice.
  bool done;   ///< \brief `true` when the operation has found/produced the value.
  /// \brief Constructs the result.
  /// \param value  The resulting data slice.
  /// \param done   `true` when the operation has found/produced the value.
  value_result(const slice &value, bool done) noexcept : value(value), done(done) {}
  /// \brief Copy constructor.
  value_result(const value_result &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  value_result &operator=(const value_result &) noexcept = default;
  /// \brief Checks whether the value was found/produced.
  /// \returns The \ref done flag.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept {
    MDBX_INLINE_API_ASSERT(!done || value.is_valid());
    return done;
  }
};

/// \brief Combines pair of slices for key and value to represent result of certain operations.
struct pair {
  /// \brief An alias for `std::pair<slice, slice>`.
  using stl_pair = std::pair<slice, slice>;
  slice key;   ///< \brief The key part of the pair.
  slice value; ///< \brief The value part of the pair.
  /// \brief Constructs the pair from the given key and value slices.
  /// \param key    The key part.
  /// \param value  The value part.
  MDBX_CXX11_CONSTEXPR pair(const slice &key, const slice &value) noexcept : key(key), value(value) {}
  /// \brief Constructs the pair from a `std::pair` of slices.
  /// \param couple  The source `std::pair`.
  MDBX_CXX11_CONSTEXPR pair(const stl_pair &couple) noexcept : key(couple.first), value(couple.second) {}
  /// \brief Converts the pair to a `std::pair` of slices.
  /// \returns The `std::pair` with the same key and value.
  MDBX_CXX11_CONSTEXPR operator stl_pair() const noexcept { return stl_pair(key, value); }
  /// \brief Copy constructor.
  pair(const pair &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  pair &operator=(const pair &) noexcept = default;
  /// \brief Move assignment operator.
  /// \param couple  The source pair to be moved from.
  /// \returns A reference to this instance.
  pair &operator=(pair &&couple) noexcept {
    key.assign(std::move(couple.key));
    value.assign(std::move(couple.value));
    return *this;
  }
  /// \brief Checks whether the pair is not empty.
  /// \returns `true` if the key (and the value) slices are not null.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept {
    MDBX_INLINE_API_ASSERT(bool(key) == bool(value));
    return key;
  }
  /// \brief Builds an invalid pair of invalid slices.
  /// \returns The invalid pair instance.
  MDBX_CXX14_CONSTEXPR static pair invalid() noexcept { return pair(slice::invalid(), slice::invalid()); }

  /// \brief Assigns the pair from a `std::pair` of slices.
  /// \param couple  The source `std::pair`.
  /// \returns A reference to this instance.
  pair &operator=(const stl_pair &couple) {
    key.assign(couple.first);
    value.assign(couple.second);
    return *this;
  }
  /// \brief Move-assigns the pair from a temporary `std::pair` of slices.
  /// \param couple  The source `std::pair` to be moved from.
  /// \returns A reference to this instance.
  pair &operator=(stl_pair &&couple) {
    key.assign(std::move(couple.first));
    value.assign(std::move(couple.second));
    return *this;
  }

  /// \brief Three-way fast non-lexicographically length-based comparison.
  /// \param a  The left-hand pair.
  /// \param b  The right-hand pair.
  /// \returns The three-way comparison result.
  MDBX_NOTHROW_PURE_FUNCTION static MDBX_CXX14_CONSTEXPR intptr_t compare_fast(const pair &a, const pair &b) noexcept;

  /// \brief Three-way lexicographically comparison.
  /// \param a  The left-hand pair.
  /// \param b  The right-hand pair.
  /// \returns The three-way comparison result.
  MDBX_NOTHROW_PURE_FUNCTION static MDBX_CXX14_CONSTEXPR intptr_t compare_lexicographically(const pair &a,
                                                                                            const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator==(const pair &a, const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator<(const pair &a, const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator>(const pair &a, const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator<=(const pair &a, const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator>=(const pair &a, const pair &b) noexcept;
  friend MDBX_CXX14_CONSTEXPR bool operator!=(const pair &a, const pair &b) noexcept;
#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
  friend MDBX_CXX14_CONSTEXPR auto operator<=>(const pair &a, const pair &b) noexcept;
#endif /* __cpp_impl_three_way_comparison */
};

/// \brief Combines pair of slices for key and value with boolean flag to
/// represent result of certain operations.
struct pair_result : public pair {
  bool done; ///< \brief `true` when the operation has found/produced the pair.
  /// \brief Constructs an empty (not done) result with an invalid pair.
  MDBX_CXX11_CONSTEXPR pair_result() noexcept : pair(pair::invalid()), done(false) {}
  /// \brief Constructs the result.
  /// \param key    The key part.
  /// \param value  The value part.
  /// \param done   `true` when the operation has found/produced the pair.
  MDBX_CXX11_CONSTEXPR pair_result(const slice &key, const slice &value, bool done) noexcept
      : pair(key, value), done(done) {}
  /// \brief Copy constructor.
  pair_result(const pair_result &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  pair_result &operator=(const pair_result &) noexcept = default;
  /// \brief Checks whether the pair was found/produced.
  /// \returns The \ref done flag.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept {
    MDBX_INLINE_API_ASSERT(!done || (bool(key) && bool(value)));
    return done;
  }
};

/// \brief Value-to-Key functions to avoid using custom comparators.
///
/// \details These build keys which are comparable without a custom comparator,
/// see \ref ::mdbx_key_from_double() and the `value2key` group. The returned
/// integers should be stored with \ref key_mode::ordinal.
/// \param json_integer  The value to be converted.
/// \returns The 8-byte key comparable with keys made from `double` values.
/// \see ::mdbx_key_from_jsonInteger()
inline uint64_t key_from_jsonInteger(int64_t json_integer) noexcept {
  return ::mdbx_key_from_jsonInteger(json_integer);
}
/// \param ieee754_64bit  The value to be converted.
/// \returns The 8-byte key that keeps the original value order.
/// \see ::mdbx_key_from_double()
inline uint64_t key_from_double(double ieee754_64bit) noexcept { return ::mdbx_key_from_double(ieee754_64bit); }
/// \param ieee754_64bit  The address of the value to be converted.
/// \returns The 8-byte key that keeps the original value order.
/// \see ::mdbx_key_from_ptrdouble()
inline uint64_t key_from_ptrdouble(const double *const ieee754_64bit) noexcept {
  return ::mdbx_key_from_ptrdouble(ieee754_64bit);
}
/// \param ieee754_32bit  The value to be converted.
/// \returns The 4-byte key that keeps the original value order.
/// \see ::mdbx_key_from_float()
inline uint32_t key_from_float(float ieee754_32bit) noexcept { return ::mdbx_key_from_float(ieee754_32bit); }
/// \param ieee754_32bit  The address of the value to be converted.
/// \returns The 4-byte key that keeps the original value order.
/// \see ::mdbx_key_from_ptrfloat()
inline uint32_t key_from_ptrfloat(const float *const ieee754_32bit) noexcept {
  return ::mdbx_key_from_ptrfloat(ieee754_32bit);
}
/// \param i64  The value to be converted.
/// \returns The 8-byte key that keeps the original value order.
/// \see ::mdbx_key_from_int64()
inline uint64_t key_from_int64(int64_t i64) noexcept { return ::mdbx_key_from_int64(i64); }
/// \param i32  The value to be converted.
/// \returns The 4-byte key that keeps the original value order.
/// \see ::mdbx_key_from_int32()
inline uint32_t key_from_int32(int32_t i32) noexcept { return ::mdbx_key_from_int32(i32); }

/// \brief Key-to-Value functions to avoid using custom comparators.
/// \param key  The key made by \ref key_from_double().
/// \returns The original `double` value.
/// \see ::mdbx_double_from_key()
inline double double_from_key(const slice &key) noexcept { return ::mdbx_double_from_key(key); }
/// \param key  The key made by \ref key_from_float().
/// \returns The original `float` value.
/// \see ::mdbx_float_from_key()
inline float float_from_key(const slice &key) noexcept { return ::mdbx_float_from_key(key); }
/// \param key  The key made by \ref key_from_int32().
/// \returns The original `int32_t` value.
/// \see ::mdbx_int32_from_key()
inline int32_t int32_from_key(const slice &key) noexcept { return ::mdbx_int32_from_key(key); }
/// \param key  The key made by \ref key_from_int64().
/// \returns The original `int64_t` value.
/// \see ::mdbx_int64_from_key()
inline int64_t int64_from_key(const slice &key) noexcept { return ::mdbx_int64_from_key(key); }
/// \param key  The key made by \ref key_from_jsonInteger().
/// \returns The original `int64_t` JSON-number value.
/// \see ::mdbx_jsonInteger_from_key()
inline int64_t jsonInteger_from_key(const slice &key) noexcept { return ::mdbx_jsonInteger_from_key(key); }

/// \brief Dumps the given value to a string: as-is when printable (all bytes
/// in the range 0x20..0x7E), otherwise as a hexadecimal dump.
/// \param value  The value to be dumped.
/// \returns `<empty>` for an empty value, `<nullptr.N>` for a non-empty value
/// with a null data pointer.
/// \see ::mdbx_dump_val()
inline ::std::string dump_val(const slice &value) {
  if (value.empty())
    return "<empty>";
  ::std::string result(value.length() * 2 + 4, '\0');
  const char *const ptr = ::mdbx_dump_val(&value, &result[0], result.size());
  if (!ptr)
    return result;
  result.assign(ptr);
  return result;
}

#if MDBX_HAVE_CXX20_CONCEPTS || defined(DOXYGEN)

/** \concept MutableByteProducer
 *  \interface MutableByteProducer
 *  \brief MutableByteProducer C++20 concept */
template <typename T>
concept MutableByteProducer = requires(T a, char array[42]) {
  { a.is_empty() } -> std::same_as<bool>;
  { a.envisage_result_length() } -> std::same_as<size_t>;
  { a.write_bytes(&array[0], size_t(42)) } -> std::same_as<char *>;
};

/** \concept ImmutableByteProducer
 *  \interface ImmutableByteProducer
 *  \brief ImmutableByteProducer C++20 concept */
template <typename T>
concept ImmutableByteProducer = requires(const T &a, char array[42]) {
  { a.is_empty() } -> std::same_as<bool>;
  { a.envisage_result_length() } -> std::same_as<size_t>;
  { a.write_bytes(&array[0], size_t(42)) } -> std::same_as<char *>;
};

/** \concept SliceTranscoder
 *  \interface SliceTranscoder
 *  \brief SliceTranscoder C++20 concept */
template <typename T>
concept SliceTranscoder = ImmutableByteProducer<T> && requires(const slice &source, const T &a) {
  T(source);
  { a.is_erroneous() } -> std::same_as<bool>;
};

#endif /* MDBX_HAVE_CXX20_CONCEPTS */

/// \brief Hexadecimal encoder which satisfy \ref SliceTranscoder concept.
struct LIBMDBX_API to_hex {
  const slice source;        ///< \brief The slice to be encoded.
  const bool uppercase;      ///< \brief Use uppercase letters for hexadecimal digits.
  const unsigned wrap_width; ///< \brief The width to wrap lines with a newline, zero means no wrapping.
  /// \brief Constructs the encoder for the given slice.
  /// \param source       The slice to be encoded.
  /// \param uppercase    Use uppercase letters for hexadecimal digits.
  /// \param wrap_width   The width to wrap lines with a newline, zero means no wrapping.
  MDBX_CXX11_CONSTEXPR to_hex(const slice &source, bool uppercase = false, unsigned wrap_width = 0) noexcept
      : source(source), uppercase(uppercase), wrap_width(wrap_width) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, to_hex);
  }

  /// \brief Returns a string with a hexadecimal dump of a passed slice.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the hexadecimal dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a hexadecimal dump of a passed slice.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the hexadecimal dump.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the buffer size in bytes needed for hexadecimal
  /// dump of a passed slice.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept {
    const size_t bytes = source.length() << 1;
    return wrap_width ? bytes + bytes / wrap_width : bytes;
  }

  /// \brief Fills the buffer by hexadecimal dump of a passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Output hexadecimal dump of passed slice to the std::ostream.
  /// \param out  The output stream.
  /// \returns A reference to the given output stream.
  /// \throws std::ios_base::failure corresponding to std::ostream::write() behaviour.
  ::std::ostream &output(::std::ostream &out) const;

  /// \brief Checks whether a passed slice is empty,
  /// and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of the passed slice could be encoded;
  /// this encoder always returns `false`.
  /// \returns Always `false` for the hexadecimal encoder.
  bool is_erroneous() const noexcept { return false; }
};

/// \brief [Base58](https://en.wikipedia.org/wiki/Base58) encoder which satisfy
/// \ref SliceTranscoder concept.
struct LIBMDBX_API to_base58 {
  const slice source;        ///< \brief The slice to be encoded.
  const unsigned wrap_width; ///< \brief The width to wrap lines with a newline, zero means no wrapping.
  /// \brief Constructs the encoder for the given slice.
  /// \param source       The slice to be encoded.
  /// \param wrap_width   The width to wrap lines with a newline, zero means no wrapping.
  MDBX_CXX11_CONSTEXPR
  to_base58(const slice &source, unsigned wrap_width = 0) noexcept : source(source), wrap_width(wrap_width) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, to_base58);
  }

  /// \brief Returns a string with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of a passed slice.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the Base58 dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of a passed slice.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the Base58 dump.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the buffer size in bytes needed for
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of passed slice.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept {
    const size_t bytes = (source.length() * 11 + 7) / 8;
    return wrap_width ? bytes + bytes / wrap_width : bytes;
  }

  /// \brief Fills the buffer by [Base58](https://en.wikipedia.org/wiki/Base58) dump of passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Output [Base58](https://en.wikipedia.org/wiki/Base58) dump of passed slice to the std::ostream.
  /// \param out  The output stream.
  /// \returns A reference to the given output stream.
  /// \throws std::ios_base::failure corresponding to std::ostream::write() behaviour.
  ::std::ostream &output(::std::ostream &out) const;

  /// \brief Checks whether a passed slice is empty, and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of a passed slice could be encoded;
  /// always returns `false` for the Base58 encoder.
  /// \returns Always `false` for the Base58 encoder.
  bool is_erroneous() const noexcept { return false; }
};

/// \brief [Base64](https://en.wikipedia.org/wiki/Base64) encoder which satisfy
/// \ref SliceTranscoder concept.
struct LIBMDBX_API to_base64 {
  const slice source;        ///< \brief The slice to be encoded.
  const unsigned wrap_width; ///< \brief The width to wrap lines with a newline, zero means no wrapping.
  /// \brief Constructs the encoder for the given slice.
  /// \param source       The slice to be encoded.
  /// \param wrap_width   The width to wrap lines with a newline, zero means no wrapping.
  MDBX_CXX11_CONSTEXPR
  to_base64(const slice &source, unsigned wrap_width = 0) noexcept : source(source), wrap_width(wrap_width) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, to_base64);
  }

  /// \brief Returns a string with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of a passed slice.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the Base64 dump.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns a buffer with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of a passed slice.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the Base64 dump.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the buffer size in bytes needed for
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of passed slice.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept {
    const size_t bytes = (source.length() + 2) / 3 * 4;
    return wrap_width ? bytes + bytes / wrap_width : bytes;
  }

  /// \brief Fills the buffer by [Base64](https://en.wikipedia.org/wiki/Base64)
  /// dump of passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Output [Base64](https://en.wikipedia.org/wiki/Base64)
  /// dump of passed slice to the std::ostream.
  /// \param out  The output stream.
  /// \returns A reference to the given output stream.
  /// \throws std::ios_base::failure corresponding to std::ostream::write() behaviour.
  ::std::ostream &output(::std::ostream &out) const;

  /// \brief Checks whether a passed slice is empty,
  /// and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of the passed slice could be encoded;
  /// this encoder always returns `false`.
  /// \returns Always `false` for the Base64 encoder.
  bool is_erroneous() const noexcept { return false; }
};

/// \brief Hexadecimal decoder which satisfy \ref SliceTranscoder concept.
struct LIBMDBX_API from_hex {
  const slice source;       ///< \brief The slice with a hexadecimal dump to be decoded.
  const bool ignore_spaces; ///< \brief Skip whitespaces while decoding.
  /// \brief Constructs the decoder for the given slice.
  /// \param source          The slice with a hexadecimal dump to be decoded.
  /// \param ignore_spaces   Skip whitespaces while decoding.
  MDBX_CXX11_CONSTEXPR from_hex(const slice &source, bool ignore_spaces = false) noexcept
      : source(source), ignore_spaces(ignore_spaces) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, from_hex);
  }

  /// \brief Decodes hexadecimal dump from a passed slice to returned string.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the decoded data.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes hexadecimal dump from a passed slice to returned buffer.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the number of bytes needed for conversion
  /// hexadecimal dump from a passed slice to decoded data.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept { return source.length() >> 1; }

  /// \brief Fills the destination with data decoded from hexadecimal dump from a passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Checks whether a passed slice is empty, and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of a passed slice is a valid hexadecimal
  /// dump, and therefore can be decoded without errors.
  /// \returns `true` if the source slice is NOT a valid hexadecimal dump.
  bool is_erroneous() const noexcept;
};

/// \brief [Base58](https://en.wikipedia.org/wiki/Base58) decoder which satisfy
/// \ref SliceTranscoder concept.
struct LIBMDBX_API from_base58 {
  const slice source;       ///< \brief The slice with a Base58 dump to be decoded.
  const bool ignore_spaces; ///< \brief Skip whitespaces while decoding.
  /// \brief Constructs the decoder for the given slice.
  /// \param source          The slice with a Base58 dump to be decoded.
  /// \param ignore_spaces   Skip whitespaces while decoding.
  MDBX_CXX11_CONSTEXPR from_base58(const slice &source, bool ignore_spaces = false) noexcept
      : source(source), ignore_spaces(ignore_spaces) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, from_base58);
  }

  /// \brief Decodes [Base58](https://en.wikipedia.org/wiki/Base58) dump from a
  /// passed slice to returned string.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the decoded data.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes [Base58](https://en.wikipedia.org/wiki/Base58) dump from a
  /// passed slice to returned buffer.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the number of bytes needed for conversion
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump from a passed slice to decoded data.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept {
    return source.length() /* могут быть все нули кодируемые один-к-одному */;
  }

  /// \brief Fills the destination with data decoded from
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump from a passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Checks whether a passed slice is empty, and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of a passed slice is a valid
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump, and therefore can be
  /// decoded without errors.
  /// \returns `true` if the source slice is NOT a valid Base58 dump.
  bool is_erroneous() const noexcept;
};

/// \brief [Base64](https://en.wikipedia.org/wiki/Base64) decoder which satisfy
/// \ref SliceTranscoder concept.
struct LIBMDBX_API from_base64 {
  const slice source;       ///< \brief The slice with a Base64 dump to be decoded.
  const bool ignore_spaces; ///< \brief Skip whitespaces while decoding.
  /// \brief Constructs the decoder for the given slice.
  /// \param source          The slice with a Base64 dump to be decoded.
  /// \param ignore_spaces   Skip whitespaces while decoding.
  MDBX_CXX11_CONSTEXPR from_base64(const slice &source, bool ignore_spaces = false) noexcept
      : source(source), ignore_spaces(ignore_spaces) {
    MDBX_ASSERT_CXX20_CONCEPT_SATISFIED(SliceTranscoder, from_base64);
  }

  /// \brief Decodes [Base64](https://en.wikipedia.org/wiki/Base64) dump from a
  /// passed slice to returned string.
  /// \param alloc  The allocator for the resulting string.
  /// \returns The string with the decoded data.
  template <class ALLOCATOR = default_allocator>
  inline string<ALLOCATOR> as_string(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Decodes [Base64](https://en.wikipedia.org/wiki/Base64) dump from a
  /// passed slice to returned buffer.
  /// \param alloc  The allocator for the resulting buffer.
  /// \returns The buffer with the decoded data.
  template <class ALLOCATOR = default_allocator, typename CAPACITY_POLICY = default_capacity_policy>
  inline buffer<ALLOCATOR, CAPACITY_POLICY> as_buffer(const ALLOCATOR &alloc = ALLOCATOR()) const;

  /// \brief Returns the number of bytes needed for conversion
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump from a passed slice to decoded data.
  /// \returns The required buffer size in bytes.
  MDBX_CXX11_CONSTEXPR size_t envisage_result_length() const noexcept { return (source.length() + 3) / 4 * 3; }

  /// \brief Fills the destination with data decoded from
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump from a passed slice.
  /// \param dest       The destination buffer to be filled.
  /// \param dest_size  The size of the destination buffer.
  /// \returns The pointer just after the last written byte.
  /// \throws std::length_error if given buffer is too small.
  char *write_bytes(char *dest, size_t dest_size) const;

  /// \brief Checks whether a passed slice is empty,
  /// and therefore there will be no output bytes.
  /// \returns `true` if the source slice is empty.
  bool is_empty() const noexcept { return source.empty(); }

  /// \brief Checks whether the content of a passed slice is a valid
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump, and therefore can be
  /// decoded without errors.
  /// \returns `true` if the source slice is NOT a valid Base64 dump.
  bool is_erroneous() const noexcept;
};

namespace allocation_aware_details {

template <typename A> constexpr bool allocator_is_always_equal() noexcept {
#if defined(__cpp_lib_allocator_traits_is_always_equal) && __cpp_lib_allocator_traits_is_always_equal >= 201411L
  return ::std::allocator_traits<A>::is_always_equal::value;
#else
  return ::std::is_empty<A>::value;
#endif /* __cpp_lib_allocator_traits_is_always_equal */
}

template <typename T, typename A = typename T::allocator_type,
          bool PoCMA = ::std::allocator_traits<A>::propagate_on_container_move_assignment::value>
struct move_assign_alloc;

template <typename T, typename A> struct move_assign_alloc<T, A, false> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept { return is_always_equal(); }
  static MDBX_CXX20_CONSTEXPR bool is_moveable(T *target, T &source) noexcept {
    if MDBX_IF_CONSTEXPR (is_always_equal())
      return true;
    else
      return target->get_allocator() == source.get_allocator();
  }
  static MDBX_CXX20_CONSTEXPR void propagate(T *target, T &) noexcept { target->release(); }
};

template <typename T, typename A> struct move_assign_alloc<T, A, true> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept { return ::std::is_nothrow_move_assignable<A>::value; }
  static constexpr bool is_moveable(T *, T &) noexcept { return true; }
  static MDBX_CXX20_CONSTEXPR void propagate(T *target, T &source) noexcept {
    target->release();
    target->get_allocator() = ::std::move(source.get_allocator());
  }
};

template <typename T, typename A = typename T::allocator_type,
          bool PoCCA = ::std::allocator_traits<A>::propagate_on_container_copy_assignment::value>
struct copy_assign_alloc;

template <typename T, typename A> struct copy_assign_alloc<T, A, false> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept { return false; }
  static MDBX_CXX20_CONSTEXPR void propagate(T *, const T &) noexcept {}
};

template <typename T, typename A> struct copy_assign_alloc<T, A, true> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept {
    return is_always_equal() || ::std::is_nothrow_copy_assignable<A>::value;
  }
  static MDBX_CXX20_CONSTEXPR void propagate(T *target, const T &source) noexcept(is_nothrow()) {
    if MDBX_IF_CONSTEXPR (!is_always_equal()) {
      if (MDBX_UNLIKELY(target->get_allocator() != source.get_allocator())) {
        target->release();
        MDBX_CXX20_UNLIKELY target->get_allocator() = source.get_allocator();
      }
    } else {
      /* gag for buggy compilers */
      (void)target;
      (void)source;
    }
  }
};

template <typename T, typename A = typename T::allocator_type,
          bool PoCS = ::std::allocator_traits<A>::propagate_on_container_swap::value>
struct swap_alloc;

template <typename T, typename A> struct swap_alloc<T, A, false> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept { return is_always_equal(); }
  static MDBX_CXX20_CONSTEXPR void propagate(T &left, T &right) noexcept(is_nothrow()) {
    if MDBX_IF_CONSTEXPR (!is_always_equal()) {
      if (MDBX_UNLIKELY(left.get_allocator() != right.get_allocator()))
        MDBX_CXX20_UNLIKELY throw_allocators_mismatch();
    } else {
      /* gag for buggy compilers */
      (void)left;
      (void)right;
    }
  }
};

template <typename T, typename A> struct swap_alloc<T, A, true> {
  static constexpr bool is_always_equal() noexcept { return allocator_is_always_equal<A>(); }
  static constexpr bool is_nothrow() noexcept {
    return is_always_equal() ||
#if defined(__cpp_lib_is_swappable) && __cpp_lib_is_swappable >= 201603L
           ::std::is_nothrow_swappable<A>() ||
#endif /* __cpp_lib_is_swappable >= 201603L */
           (::std::is_nothrow_move_constructible<A>::value && ::std::is_nothrow_move_assignable<A>::value);
  }
  static MDBX_CXX20_CONSTEXPR void propagate(T &left, T &right) noexcept(is_nothrow()) {
    if MDBX_IF_CONSTEXPR (!is_always_equal())
#if !defined(__cpp_if_constexpr) || __cpp_if_constexpr < 201606L
      /* Workaround of ms-clang (clang corrupted by microsoft) */
      MDBX_CXX20_UNLIKELY
#endif
    ::std::swap(left.get_allocator(), right.get_allocator());
    else {
      /* gag for buggy compilers */
      (void)left;
      (void)right;
    }
  }
};

} // namespace allocation_aware_details

/// \brief The default capacity policy for the \ref buffer class: rounds the allocated
/// capacity and advises whether to grow, keep or shrink it.
struct default_capacity_policy {
  /// \brief Policy constants.
  enum : size_t {
    extra_inplace_storage = 0,          ///< \brief Extra bytes in the inplace storage.
    inplace_storage_size_rounding = 16, ///< \brief Rounding of the inplace storage size.
    pettiness_threshold = 64,           ///< \brief The threshold to avoid petty reallocations.
    max_reserve = 65536                 ///< \brief The maximum reserve over the requested capacity.
  };

  /// \brief Rounds the given value up to the \ref pettiness_threshold granularity.
  /// \param value  The value to be rounded.
  /// \returns The rounded value.
  static MDBX_CXX11_CONSTEXPR size_t round(const size_t value) {
    static_assert((pettiness_threshold & (pettiness_threshold - 1)) == 0, "pettiness_threshold must be a power of 2");
    static_assert(pettiness_threshold >= sizeof(uint64_t), "pettiness_threshold must be > 7");
    constexpr const auto pettiness_mask = ~size_t(pettiness_threshold - 1);
    return (value + pettiness_threshold - 1) & pettiness_mask;
  }

  /// \brief Advises the capacity for the requested size.
  /// \param current  The currently allocated capacity.
  /// \param wanna    The requested capacity.
  /// \param inplace  The size of the inplace storage available without allocation.
  /// \returns The advised capacity to be (re)allocated.
  static MDBX_CXX11_CONSTEXPR size_t advise(const size_t current, const size_t wanna, const size_t inplace) {
    static_assert(max_reserve % pettiness_threshold == 0, "max_reserve must be a multiple of pettiness_threshold");
    static_assert(max_reserve / 3 > pettiness_threshold, "max_reserve must be > pettiness_threshold * 3");

    if (wanna <= inplace && (current <= inplace || current >= std::max(inplace + inplace, size_t(pettiness_threshold))))
      return inplace;

    if (wanna > current)
      /* doubling capacity, but don't made reserve more than max_reserve */
      return round(wanna + ::std::min(size_t(max_reserve), current));

    if (current - wanna >
        /* shrink if reserve will more than half of current or max_reserve,
         * but not less than pettiness_threshold */
        ::std::min(wanna + pettiness_threshold, size_t(max_reserve)))
      return round(wanna);

    /* keep unchanged */
    return current;
  }
};

/// \brief Type tag for delivered buffer template classes.
struct buffer_tag {};

/// \brief The chunk of data stored inside the buffer or located outside it.
///
/// \note Unlike the base `slice`, a buffer by default owns its content: the
/// constructors from strings/views copy the bytes (freestanding modality), so
/// there are no lifetime requirements on the source. The `make_reference=true`
/// overloads switch to slice-like behavior: the buffer just refers to external
/// data, which must outlive the buffer (the same temporary-lifetime rules as
/// for `slice` apply). The rvalue-string constructor steals the string's
/// storage and leaves the source empty.
template <class ALLOCATOR, typename CAPACITY_POLICY>
class MDBX_MSVC_DECLSPEC_EMPTY_BASES buffer : public slice, public buffer_tag {
public:
  /// \brief The base \ref slice class.
  using inherited = slice;
#if !defined(_MSC_VER) || _MSC_VER > 1900
  /// \brief The allocator type rebound for the internal storage.
  using allocator_type = typename ::std::allocator_traits<ALLOCATOR>::template rebind_alloc<uint64_t>;
#else
  /// \brief The allocator type rebound for the internal storage.
  using allocator_type = typename ALLOCATOR::template rebind<uint64_t>::other;
#endif /* MSVC is mad */
  /// \brief The `allocator_type` traits.
  using allocator_traits = ::std::allocator_traits<allocator_type>;
  /// \brief The capacity policy type, see \ref default_capacity_policy.
  using reservation_policy = CAPACITY_POLICY;
  enum : size_t {
    max_length = MDBX_MAXDATASIZE,
    max_capacity = (max_length / 3u * 4u + 1023u) & ~size_t(1023),
    extra_inplace_storage = reservation_policy::extra_inplace_storage,
    inplace_storage_size_rounding =
        (alignof(max_align_t) * 2 > size_t(reservation_policy::inplace_storage_size_rounding))
            ? alignof(max_align_t) * 2
            : size_t(reservation_policy::inplace_storage_size_rounding),
    pettiness_threshold = reservation_policy::pettiness_threshold
  };

  /// \brief Data storage modality.
  enum class modality { reference, inplace, allocated };

private:
  friend class txn;
  struct silo /* Empty Base Class Optimization */ : public allocator_type {
    MDBX_CXX20_CONSTEXPR const allocator_type &get_allocator() const noexcept { return *this; }
    MDBX_CXX20_CONSTEXPR allocator_type &get_allocator() noexcept { return *this; }

    using allocator_pointer = typename allocator_traits::pointer;
    using allocator_const_pointer = typename allocator_traits::const_pointer;
    using move_assign_alloc = allocation_aware_details::move_assign_alloc<silo, allocator_type>;
    using copy_assign_alloc = allocation_aware_details::copy_assign_alloc<silo, allocator_type>;
    using swap_alloc = allocation_aware_details::swap_alloc<silo, allocator_type>;

    MDBX_CXX20_CONSTEXPR ::std::pair<allocator_pointer, size_t> allocate_storage(size_t bytes) {
      MDBX_INLINE_API_ASSERT(bytes >= sizeof(bin));
      constexpr size_t unit = sizeof(typename allocator_type::value_type);
      static_assert((unit & (unit - 1)) == 0, "size of ALLOCATOR::value_type should be a power of 2");
      static_assert(unit > 0, "size of ALLOCATOR::value_type must be > 0");
      const size_t n = (bytes + unit - 1) / unit;
      return ::std::make_pair(allocator_traits::allocate(get_allocator(), n), n * unit);
    }

    MDBX_CXX20_CONSTEXPR void deallocate_storage(allocator_pointer ptr, size_t bytes) {
      constexpr size_t unit = sizeof(typename allocator_type::value_type);
      MDBX_INLINE_API_ASSERT(ptr && bytes >= sizeof(bin) && bytes >= unit && bytes % unit == 0);
      allocator_traits::deallocate(get_allocator(), ptr, bytes / unit);
    }

    MDBX_CXX20_CONSTEXPR ::std::pair<allocator_pointer, size_t> provide_storage(size_t bytes) {
      const size_t capacity = bin::advise_capacity(0, bytes);
      return bin::is_suitable_for_inplace(capacity) ? ::std::pair<allocator_pointer, size_t>(nullptr, capacity)
                                                    : allocate_storage(capacity);
    }

    static MDBX_CXX17_CONSTEXPR void *to_address(allocator_pointer ptr) noexcept {
#if defined(__cpp_lib_to_address) && __cpp_lib_to_address >= 201711L
      return static_cast<void *>(::std::to_address(ptr));
#else
      return static_cast<void *>(::std::addressof(*ptr));
#endif /* __cpp_lib_to_address */
    }

    static MDBX_CXX17_CONSTEXPR const void *to_address(allocator_const_pointer ptr) noexcept {
#if defined(__cpp_lib_to_address) && __cpp_lib_to_address >= 201711L
      return static_cast<const void *>(::std::to_address(ptr));
#else
      return static_cast<const void *>(::std::addressof(*ptr));
#endif /* __cpp_lib_to_address */
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324) /* structure was padded due to alignment specifier */
#endif                          /* _MSC_VER */

    union alignas(max_align_t) bin {
      struct stub_allocated_holder /* используется только для вычисления (минимального необходимого) размера,
                                      с учетом выравнивания */
      {
        allocator_pointer stub_ptr_;
        size_t stub_capacity_bytes_;
      };

      enum : byte { lastbyte_poison = 0, lastbyte_inplace_signature = byte(~byte(lastbyte_poison)) };
      enum : size_t {
        inplace_signature_limit = size_t(lastbyte_inplace_signature)
                                  << (sizeof(size_t /* allocated::capacity_bytes_ */) - 1) * CHAR_BIT,
        inplace_size_rounding = size_t(inplace_storage_size_rounding) - 1,
        inplace_size =
            (sizeof(stub_allocated_holder) + extra_inplace_storage + inplace_size_rounding) & ~inplace_size_rounding
      };

      struct capacity_holder {
        byte pad_[inplace_size - sizeof(allocator_pointer)];
        size_t bytes_;
      };

      struct inplace_flag_holder {
        byte buffer_[inplace_size - sizeof(byte)];
        byte lastbyte_;
        MDBX_CXX11_CONSTEXPR inplace_flag_holder(byte signature) : buffer_(), lastbyte_(signature) {}
      };

      allocator_pointer allocated_ptr_;
      capacity_holder capacity_;
      inplace_flag_holder inplace_;

      static constexpr size_t inplace_capacity() noexcept { return sizeof(inplace_flag_holder::buffer_); }
      static constexpr bool is_suitable_for_inplace(size_t capacity_bytes) noexcept {
        static_assert((size_t(reservation_policy::inplace_storage_size_rounding) &
                       (size_t(reservation_policy::inplace_storage_size_rounding) - 1)) == 0,
                      "CAPACITY_POLICY::inplace_storage_size_rounding must be power of 2");
        static_assert(sizeof(bin) == sizeof(inplace_) && sizeof(bin) == sizeof(capacity_), "WTF?");
        return capacity_bytes <= inplace_capacity();
      }

      MDBX_NOTHROW_PURE_FUNCTION constexpr bool is_inplace() const noexcept {
        static_assert(size_t(inplace_signature_limit) > size_t(max_capacity), "WTF?");
        static_assert(std::numeric_limits<size_t>::max() - (std::numeric_limits<size_t>::max() >> CHAR_BIT) ==
                          inplace_signature_limit,
                      "WTF?");
        return inplace_.lastbyte_ == lastbyte_inplace_signature;
      }
      MDBX_NOTHROW_PURE_FUNCTION constexpr bool is_allocated() const noexcept { return !is_inplace(); }

      MDBX_CXX17_CONSTEXPR byte *make_inplace() noexcept {
        MDBX_CONSTEXPR_ASSERT(is_allocated());
        /* properly destroy allocator::pointer */
        allocated_ptr_.~allocator_pointer();
        inplace_.lastbyte_ = lastbyte_inplace_signature;
        MDBX_CONSTEXPR_ASSERT(is_inplace() && address() == inplace_.buffer_ && is_suitable_for_inplace(capacity()));
        return address();
      }

      MDBX_CXX17_CONSTEXPR byte *make_allocated(const ::std::pair<allocator_pointer, size_t> &pair) noexcept {
        MDBX_CONSTEXPR_ASSERT(inplace_signature_limit > pair.second);
        MDBX_CONSTEXPR_ASSERT(is_inplace());
        new (&allocated_ptr_) allocator_pointer(pair.first);
        capacity_.bytes_ = pair.second;
        MDBX_CONSTEXPR_ASSERT(is_allocated() && address() == to_address(pair.first) && capacity() == pair.second);
        return address();
      }

      MDBX_CXX11_CONSTEXPR bin() noexcept : inplace_(lastbyte_inplace_signature) {
        if (::std::is_trivial<allocator_pointer>::value)
          /* workaround for "uninitialized" warning from some compilers */
          memset(&allocated_ptr_, 0, sizeof(allocated_ptr_));
      }

      static MDBX_CXX20_CONSTEXPR size_t advise_capacity(const size_t current, const size_t wanna) {
        if (MDBX_UNLIKELY(wanna > max_capacity))
          MDBX_CXX20_UNLIKELY throw_max_length_exceeded();

        const size_t advised = reservation_policy::advise(current, wanna, inplace_capacity());
        MDBX_INLINE_API_ASSERT(advised >= wanna);
        return ::std::min(size_t(max_capacity), ::std::max(inplace_capacity(), advised));
      }

      constexpr bool is_inplace(const void *ptr) const noexcept {
        /* `<=`: an empty payload may sit at the very end of the inplace storage */
        return ptr != nullptr && size_t(static_cast<const byte *>(ptr) - inplace_.buffer_) <= inplace_capacity();
      }

      constexpr const byte *address() const noexcept {
        return is_inplace() ? inplace_.buffer_ : static_cast<const byte *>(to_address(allocated_ptr_));
      }
      MDBX_CXX17_CONSTEXPR byte *address() noexcept {
        return is_inplace() ? inplace_.buffer_ : static_cast<byte *>(to_address(allocated_ptr_));
      }
      constexpr size_t capacity() const noexcept { return is_inplace() ? inplace_capacity() : capacity_.bytes_; }
    } bin_;

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */

    constexpr bool is_inplace(const void *ptr) const noexcept { return bin_.is_inplace(ptr); }

    MDBX_CXX20_CONSTEXPR void release() noexcept {
      if (bin_.is_allocated()) {
        deallocate_storage(bin_.allocated_ptr_, bin_.capacity_.bytes_);
        /* properly destroy allocator::pointer */
        bin_.allocated_ptr_.~allocator_pointer();
        bin_.inplace_.lastbyte_ = bin::lastbyte_inplace_signature;
      }
    }

    template <bool external_content>
    MDBX_CXX20_CONSTEXPR void *reshape(const size_t wanna_capacity, const size_t wanna_headroom,
                                       const void *const content, const size_t length) {
      MDBX_INLINE_API_ASSERT(wanna_capacity >= wanna_headroom + length);
      const size_t old_capacity = bin_.capacity();
      const size_t new_capacity = bin::advise_capacity(old_capacity, wanna_capacity);
      if (MDBX_LIKELY(new_capacity == old_capacity))
        MDBX_CXX20_LIKELY {
          MDBX_INLINE_API_ASSERT(bin_.is_inplace() == bin::is_suitable_for_inplace(new_capacity));
          byte *const new_place = bin_.address() + wanna_headroom;
          if (MDBX_LIKELY(length))
            MDBX_CXX20_LIKELY {
              if (external_content)
                /* `content` may refer into the own storage, so use memmove */
                ::std::memmove(new_place, content, length);
              else {
                const size_t old_headroom = static_cast<const byte *>(content) - bin_.address();
                MDBX_INLINE_API_ASSERT(old_capacity >= old_headroom + length);
                if (MDBX_UNLIKELY(old_headroom != wanna_headroom))
                  MDBX_CXX20_UNLIKELY ::std::memmove(new_place, content, length);
              }
            }
          return new_place;
        }

      if (bin::is_suitable_for_inplace(new_capacity)) {
        MDBX_INLINE_API_ASSERT(bin_.is_allocated());
        const auto old_allocated = bin_.allocated_ptr_;
        byte *const new_place = bin_.make_inplace() + wanna_headroom;
        if (MDBX_LIKELY(length))
          MDBX_CXX20_LIKELY memcpy(new_place, content, length);
        deallocate_storage(old_allocated, old_capacity);
        return new_place;
      }

      if (bin_.is_inplace()) {
        const auto pair = allocate_storage(new_capacity);
        MDBX_INLINE_API_ASSERT(pair.second >= new_capacity);
        byte *const new_place = static_cast<byte *>(to_address(pair.first)) + wanna_headroom;
        /* Copy the content before make_allocated(), since the latter overwrites
         * the inplace storage (the source of `content` for reshape<false>). */
        if (MDBX_LIKELY(length))
          MDBX_CXX20_LIKELY memcpy(new_place, content, length);
        bin_.make_allocated(pair);
        return new_place;
      }

      auto pair = allocate_storage(new_capacity);
      std::swap(bin_.allocated_ptr_, pair.first);
      bin_.capacity_.bytes_ = pair.second;
      auto new_place = bin_.address() + wanna_headroom;
      if (MDBX_LIKELY(length))
        MDBX_CXX20_LIKELY memcpy(new_place, content, length);
      deallocate_storage(pair.first, old_capacity);
      return new_place;
    }

    MDBX_CXX20_CONSTEXPR const byte *get(size_t offset = 0) const noexcept {
      MDBX_INLINE_API_ASSERT(capacity() >= offset);
      return bin_.address() + offset;
    }
    MDBX_CXX20_CONSTEXPR byte *get(size_t offset = 0) noexcept {
      MDBX_INLINE_API_ASSERT(capacity() >= offset);
      return bin_.address() + offset;
    }
    MDBX_CXX20_CONSTEXPR byte *put(size_t offset, const void *ptr, size_t length) {
      MDBX_INLINE_API_ASSERT(capacity() >= offset + length);
      return static_cast<byte *>(memcpy(get(offset), ptr, length));
    }

    //--------------------------------------------------------------------------

    MDBX_CXX20_CONSTEXPR silo(const allocator_type &alloc = allocator_type()) noexcept : allocator_type(alloc) {}
    MDBX_CXX20_CONSTEXPR silo(size_t capacity, const allocator_type &alloc = allocator_type()) : silo(alloc) {
      if (!bin::is_suitable_for_inplace(capacity))
        bin_.make_allocated(provide_storage(capacity));
    }

    silo(silo &&other) = delete;
    MDBX_CXX20_CONSTEXPR
    silo(silo &&other, bool is_reference = false) noexcept(::std::is_nothrow_move_constructible<allocator_type>::value)
        : allocator_type(::std::move(other.get_allocator())) {
      if (!is_reference) {
        if (other.bin_.is_inplace()) {
          memcpy(&bin_, &other.bin_, sizeof(bin));
          MDBX_CONSTEXPR_ASSERT(bin_.is_inplace());
        } else {
          /* coverity[USE_AFTER_MOVE] */
          new (&bin_.allocated_ptr_) allocator_pointer(::std::move(other.bin_.allocated_ptr_));
          /* coverity[USE_AFTER_MOVE] */
          bin_.capacity_.bytes_ = other.bin_.capacity_.bytes_;
          /* properly destroy allocator::pointer.
           *
           * CoverityScan issues an erroneous warning here about using an uninitialized object. Which is not true,
           * since in C++ (unlike Rust) an object remains initialized after a move-assignment operation; Moreover,
           * a destructor will be called for such an object (this is explicitly stated in all C++ standards, starting
           * from the 11th). */
          /* coverity[USE_AFTER_MOVE] */
          other.bin_.allocated_ptr_.~allocator_pointer();
          other.bin_.inplace_.lastbyte_ = bin::lastbyte_inplace_signature;
          MDBX_CONSTEXPR_ASSERT(bin_.is_allocated() && other.bin_.is_inplace());
        }
      }
    }

    MDBX_CXX17_CONSTEXPR bool move_content(silo &other, const modality other_modality) noexcept {
      switch (other_modality) {
      case modality::inplace:
        memcpy(&bin_, &other.bin_, sizeof(bin));
        MDBX_CONSTEXPR_ASSERT(bin_.is_inplace());
        return /* buffer's slice fixup is needed */ true;
      case modality::allocated:
        new (&bin_.allocated_ptr_) allocator_pointer(::std::move(other.bin_.allocated_ptr_));
        bin_.capacity_.bytes_ = other.bin_.capacity_.bytes_;
        /* properly destroy allocator::pointer.
         *
         * CoverityScan issues an erroneous warning here about using an uninitialized object. Which is not true,
         * since in C++ (unlike Rust) an object remains initialized after a move-assignment operation; Moreover,
         * a destructor will be called for such an object (this is explicitly stated in all C++ standards, starting
         * from the 11th). */
        /* coverity[use_after_move] */
        other.bin_.allocated_ptr_.~allocator_pointer();
        other.bin_.inplace_.lastbyte_ = bin::lastbyte_inplace_signature;
        MDBX_CONSTEXPR_ASSERT(bin_.is_allocated() && other.bin_.is_inplace());
        return /* buffer's slice fixup is not needed */ false;
      default:
        MDBX_CONSTEXPR_ASSERT(other_modality == modality::reference);
        return /* buffer's slice fixup is not needed */ false;
      }
    }

    static MDBX_CXX20_CONSTEXPR std::pair<bool, bool>
    exchange(silo &left, const modality left_modality, silo &right,
             const modality right_modality) noexcept(swap_alloc::is_nothrow()) {
      swap_alloc::propagate(left, right);
      bool left_need_fixup = false, right_need_fixup = false;
      if (left_modality == modality::reference || right_modality == modality::reference) {
        /* It is Ok here to call move_content for the left and right side in any order,
         * since the move_content() does nothing when modality == reference.
         * Thus, the actual move action will perform no more than one call of move_content(),
         * so the order doesn't matter here. */
        left_need_fixup = left.move_content(right, right_modality);
        right_need_fixup = right.move_content(left, left_modality);
      } else {
        silo temp(std::move(left), false);
        left_need_fixup = left.move_content(right, right_modality);
        right_need_fixup = right.move_content(temp, left_modality);
      }
      return std::make_pair(left_need_fixup, right_need_fixup);
    }

    MDBX_CXX20_CONSTEXPR silo(size_t capacity, size_t headroom, const void *ptr, size_t length,
                              const allocator_type &alloc = allocator_type())
        : silo(capacity, alloc) {
      MDBX_INLINE_API_ASSERT(capacity >= headroom + length);
      if (length)
        put(headroom, ptr, length);
    }

    MDBX_CXX20_CONSTEXPR silo(const void *ptr, size_t length, const allocator_type &alloc = allocator_type())
        : silo(length, 0, ptr, length, alloc) {}

    ~silo() { release(); }

    //--------------------------------------------------------------------------

    MDBX_CXX20_CONSTEXPR void *assign(size_t headroom, const void *ptr, size_t length, size_t tailroom) {
      return reshape<true>(headroom + length + tailroom, headroom, ptr, length);
    }

    MDBX_CXX20_CONSTEXPR void *assign(const void *ptr, size_t length) { return assign(0, ptr, length, 0); }

    MDBX_CXX20_CONSTEXPR void *clear() { return reshape<true>(0, 0, nullptr, 0); }
    MDBX_CXX20_CONSTEXPR void *clear_and_reserve(size_t whole_capacity, size_t headroom) {
      return reshape<false>(whole_capacity, headroom, nullptr, 0);
    }

    MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX11_CONSTEXPR size_t capacity() const noexcept { return bin_.capacity(); }
    MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX11_CONSTEXPR const void *data(size_t offset = 0) const noexcept {
      return get(offset);
    }
    MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX11_CONSTEXPR void *data(size_t offset = 0) noexcept { return get(offset); }
  };

  MDBX_CXX14_CONSTEXPR void fixup_imported_inplace(const typename silo::bin &src) noexcept {
    auto ptr = inherited::byte_ptr();
    if (src.is_inplace(ptr)) {
      MDBX_CONSTEXPR_ASSERT(silo_.bin_.is_inplace());
      intptr_t offset = &silo_.bin_.inplace_.buffer_[0] - &src.inplace_.buffer_[0];
      iov_base = ptr + offset;
      MDBX_CONSTEXPR_ASSERT(is_freestanding());
    }
  }

  silo silo_;

  void insulate() {
    MDBX_INLINE_API_ASSERT(is_reference());
    iov_base = silo_.assign(iov_base, iov_len);
  }

  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR const byte *silo_begin() const noexcept {
    return static_cast<const byte *>(silo_.data());
  }

  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR const byte *silo_end() const noexcept {
    return silo_begin() + silo_.capacity();
  }

  /* `data` overlaps the own tailroom, which append_producer() overwrites in place while still reading `data` */
  MDBX_NOTHROW_PURE_FUNCTION bool reaches_tailroom(const struct slice &data) const noexcept {
    return data.length() && is_freestanding() && uintptr_t(data.byte_ptr()) < uintptr_t(silo_end()) &&
           uintptr_t(data.end_byte_ptr()) > uintptr_t(inherited::end_byte_ptr());
  }

  struct data_preserver : public exception_thunk {
    buffer data;
    data_preserver(const allocator_type &alloc) : data(alloc) {}
    static int callback(void *context, MDBX_val *target, const void *src, size_t bytes) noexcept {
      auto self = static_cast<data_preserver *>(context);
      assert(self->is_clean());
      assert(&self->data == target);
      (void)target;
      try {
        self->data.assign(src, bytes, false);
        return MDBX_RESULT_FALSE;
      } catch (... /* capture any exception to rethrow it over C code */) {
        self->capture();
        return MDBX_RESULT_TRUE;
      }
    }
    MDBX_CXX11_CONSTEXPR operator MDBX_preserve_func() const noexcept { return callback; }
  };

public:
  /// \todo buffer& operator<<(buffer&, ...) for writing
  /// \todo template<class X> key(X) for encoding keys while writing

  /// \brief The move-assignment allocator-propagation helper (internal use).
  using move_assign_alloc = typename silo::move_assign_alloc;
  /// \brief The copy-assignment allocator-propagation helper (internal use).
  using copy_assign_alloc = typename silo::copy_assign_alloc;
  /// \brief The swap allocator-propagation helper (internal use).
  using swap_alloc = typename silo::swap_alloc;
  /// \brief Checks whether the swap operation is noexcept for this allocator.
  /// \returns `true` if swapping never throws.
  static constexpr bool is_swap_nothrow() noexcept { return swap_alloc::is_nothrow(); }

  /// \brief Returns the associated allocator.
  /// \returns A copy of the allocator associated with the buffer.
  MDBX_CXX20_CONSTEXPR allocator_type get_allocator() const { return silo_.get_allocator(); }

  /// \brief Checks whether data chunk stored inside the buffer, otherwise
  /// buffer just refers to data located outside the buffer.
  /// \returns `true` if the buffer owns its data.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR bool is_freestanding() const noexcept {
    static_assert(size_t(-intptr_t(max_length)) > max_length, "WTF?");
    const auto begin_ptr = inherited::byte_ptr();
    if (MDBX_UNLIKELY(begin_ptr == nullptr))
      return false;
    return size_t(begin_ptr - silo_begin()) <= silo_.capacity();
  }

  /// \brief Checks whether data chunk stored in place within the buffer instance itself,
  /// without reference outside nor allocating additional memory resources,
  /// which also implies buffer is freestanding.
  /// \returns `true` if the data is stored inplace.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR bool is_inplace() const noexcept {
    return inherited::data() != nullptr && silo_.is_inplace(inherited::data());
  }

  /// \brief Checks whether the buffer just refers to data located outside the buffer, rather than stores it.
  /// \returns `true` if the buffer refers to external data.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR bool is_reference() const noexcept { return !is_freestanding(); }

  /// \brief Returns current modality of buffer content.
  /// \returns The \ref modality of the content storage.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR modality content_modality() const noexcept {
    return is_freestanding() ? (is_inplace() ? modality::inplace : modality::allocated) : modality::reference;
  }

  /// \brief Returns the number of bytes that can be held in currently allocated storage.
  /// \returns The capacity in bytes, zero for a referencing buffer.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR size_t capacity() const noexcept {
    return is_freestanding() ? silo_.capacity() : 0;
  }

  /// \brief Returns the number of bytes available in the currently allocated storage in front of the data.
  /// \returns The headroom in bytes, zero for a referencing buffer.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR size_t headroom() const noexcept {
    return is_freestanding() ? inherited::byte_ptr() - silo_begin() : 0;
  }

  /// \brief Returns the number of bytes that available in currently allocated storage after the data.
  /// \returns The tailroom in bytes, zero for a referencing buffer.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR size_t tailroom() const noexcept {
    return is_freestanding() ? silo_end() - inherited::end_byte_ptr() : 0;
  }

  /// \brief Returns casted to const pointer to byte an address of data.
  /// \returns The read-only pointer to the first byte.
  MDBX_CXX11_CONSTEXPR const byte *byte_ptr() const noexcept { return inherited::byte_ptr(); }

  /// \brief Returns casted to const pointer to byte an end of data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const byte *end_byte_ptr() const noexcept { return inherited::end_byte_ptr(); }

  /// \brief Returns casted to pointer to byte an address of data.
  /// \returns The mutable pointer to the first byte.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR byte *byte_ptr() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return inherited::byte_ptr();
  }

  /// \brief Returns casted to pointer to byte an end of data.
  /// \returns The mutable pointer just after the last byte.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR byte *end_byte_ptr() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return const_cast<byte *>(inherited::end_byte_ptr());
  }

  /// \brief Returns casted to const pointer to char an address of data.
  /// \returns The read-only pointer to the first byte.
  MDBX_CXX11_CONSTEXPR const char *char_ptr() const noexcept { return inherited::char_ptr(); }

  /// \brief Returns casted to const pointer to char an end of data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const char *end_char_ptr() const noexcept { return inherited::end_char_ptr(); }

  /// \brief Returns casted to pointer to char an address of data.
  /// \returns The mutable pointer to the first byte.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR char *char_ptr() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return const_cast<char *>(inherited::char_ptr());
  }

  /// \brief Returns casted to pointer to char an end of data.
  /// \returns The mutable pointer just after the last byte.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR char *end_char_ptr() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return const_cast<char *>(inherited::end_char_ptr());
  }

  /// \brief Return a const pointer to the beginning of the referenced data.
  /// \returns The read-only pointer to the data.
  MDBX_CXX11_CONSTEXPR const void *data() const noexcept { return const_data(); }

  /// \brief Return a const pointer to the end of the referenced data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const byte *end() const noexcept { return static_cast<const byte *>(const_end()); }

  /// \brief Return a const pointer to the beginning of the referenced data.
  /// \returns The read-only pointer to the data.
  MDBX_CXX11_CONSTEXPR const void *const_data() const noexcept { return inherited::data(); }

  /// \brief Return a const pointer to the end of the referenced data.
  /// \returns The read-only pointer just after the last byte.
  MDBX_CXX11_CONSTEXPR const void *const_end() const noexcept { return inherited::end(); }

  /// \brief Return a pointer to the beginning of the referenced data.
  /// \returns The mutable pointer to the data.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR void *data() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return const_cast<void *>(inherited::data());
  }

  /// \brief Return a pointer to the end of the referenced data.
  /// \returns The mutable pointer just after the last byte.
  /// \pre REQUIRES: The buffer should store data chunk, but not referenced to an external one.
  MDBX_CXX11_CONSTEXPR byte *end() noexcept {
    MDBX_CONSTEXPR_ASSERT(is_freestanding());
    return const_cast<byte *>(inherited::end_byte_ptr());
  }

  /// \brief Returns the number of bytes.
  /// \returns The length of the data in bytes.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t length() const noexcept {
    return MDBX_CONSTEXPR_ASSERT(is_reference() || inherited::end_byte_ptr() <= silo_end()), inherited::length();
  }

  /// \brief Set length of data.
  /// \param bytes  The new length of the data.
  /// \returns A reference to this instance.
  MDBX_CXX14_CONSTEXPR buffer &set_length(size_t bytes) {
    MDBX_CONSTEXPR_ASSERT(is_reference() || inherited::byte_ptr() + bytes <= silo_end());
    inherited::set_length(bytes);
    return *this;
  }

  /// \brief Resizes the buffer to contain `size` bytes, filling the added
  /// space with zeroes. STL-compatible counterpart of \ref set_length().
  /// \param size  The new length of the data.
  void resize(size_t size) { resize(size, byte(0)); }

  /// \brief Resizes the buffer to contain `size` bytes, filling the added
  /// space with `value`. STL-compatible counterpart of \ref set_length().
  /// \param size   The new length of the data.
  /// \param value  The byte to fill the added space with.
  void resize(size_t size, byte value) {
    if (MDBX_UNLIKELY(size > length())) {
      reserve_tailroom(size - length());
      memset(end_byte_ptr(), value, size - length());
    }
    inherited::set_length(size);
  }

  /// \brief Sets the length by specifying the end of the data.
  /// \param ptr  The address just after the last byte of the data.
  /// \returns A reference to this instance.
  MDBX_CXX14_CONSTEXPR buffer &set_end(const void *ptr) {
    MDBX_CONSTEXPR_ASSERT(static_cast<const char *>(ptr) >= char_ptr());
    return set_length(static_cast<const char *>(ptr) - char_ptr());
  }

  /// \brief Makes buffer owning the data.
  /// \details If buffer refers to an external data, then makes it the owner
  /// of clone by allocating storage and copying the data.
  void make_freestanding() {
    if (is_reference())
      insulate();
  }

  /// \brief Constructs an empty buffer.
  MDBX_CXX20_CONSTEXPR buffer() noexcept = default;
  /// \brief Constructs an empty buffer with the given allocator.
  /// \param alloc  The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR buffer(const allocator_type &alloc) noexcept : silo_(alloc) {}
  /// \brief Constructs a buffer from the given slice, either copying or referencing it.
  /// \param src             The source slice.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \param alloc           The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR buffer(const struct slice &src, bool make_reference,
                              const allocator_type &alloc = allocator_type());

  /// \brief Constructs a buffer from a memory range, either copying or referencing it.
  /// \param ptr             The address of the source data.
  /// \param bytes           The number of bytes of the source data.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \param alloc           The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR
  buffer(const void *ptr, size_t bytes, bool make_reference, const allocator_type &alloc = allocator_type())
      : buffer(inherited(ptr, bytes), make_reference, alloc) {}

  /// \brief Constructs a buffer from a null-terminated string, either copying or referencing it.
  /// \param c_str           The source null-terminated string.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \param alloc           The allocator for the buffer storage.
  buffer(const char *c_str, bool make_reference, const allocator_type &alloc = allocator_type())
      : buffer(inherited(c_str), make_reference, alloc) {}

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Constructs a buffer from a string view, either copying or referencing it.
  /// \param view            The source string view.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \param alloc           The allocator for the buffer storage.
  template <class CHAR, class T>
  MDBX_CXX20_CONSTEXPR buffer(const ::std::basic_string_view<CHAR, T> &view, bool make_reference,
                              const allocator_type &alloc = allocator_type())
      : buffer(inherited(view), make_reference, alloc) {}
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Constructs a buffer storing a copy of the given slice; empty or null
  /// slices are referenced instead of copied.
  /// \param src    The source slice.
  /// \param alloc  The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR
  buffer(const struct slice &src, const allocator_type &alloc = allocator_type())
      : buffer(src, src.empty() || src.is_null(), alloc) {}

  /// \brief Copy constructor.
  /// \param src  The source buffer.
  MDBX_CXX20_CONSTEXPR
  explicit buffer(const buffer &src)
      : buffer(src, allocator_traits::select_on_container_copy_construction(src.get_allocator())) {}

  /// \brief Constructs a buffer storing a copy of the given memory range.
  /// \param ptr    The address of the source data.
  /// \param bytes  The number of bytes of the source data.
  /// \param alloc  The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR
  buffer(const void *ptr, size_t bytes, const allocator_type &alloc = allocator_type())
      : buffer(inherited(ptr, bytes), alloc) {}

  /// \brief Copies `bytes` from `ptr`; keeps an integer length away from `buffer(const char *, bool)`.
  /// \param ptr  The address of the source data.
  /// \param bytes  The number of bytes of the source data.
  /// \param alloc  The allocator for the buffer storage.
  template <typename INT, typename ::std::enable_if<::std::is_integral<INT>::value && !::std::is_same<INT, bool>::value,
                                                    int>::type = 0>
  MDBX_CXX20_CONSTEXPR buffer(const char *ptr, INT bytes, const allocator_type &alloc = allocator_type())
      : buffer(static_cast<const void *>(ptr), size_t(bytes), alloc) {}

  /// \brief Takes `nullptr`, so that a literal `0` or `NULL` is ambiguous instead of a null `const char *`.
  /// \note A null pointer with a non-zero length would dereference address
  ///   zero; only `nullptr` with zero bytes (an empty buffer) is accepted.
  /// \param bytes  The number of bytes of the source data.
  /// \param alloc  The allocator for the buffer storage.
  template <typename INT, typename ::std::enable_if<::std::is_integral<INT>::value && !::std::is_same<INT, bool>::value,
                                                    int>::type = 0>
  MDBX_CXX20_CONSTEXPR buffer(::std::nullptr_t, INT bytes, const allocator_type &alloc = allocator_type())
      : buffer(static_cast<const void *>(nullptr), check_nullptr_length(size_t(bytes)), alloc) {}

  /// \brief Null pointer with an explicit `make_reference` flag.
  /// \note Exists to keep `buffer(nullptr, N, false)` unambiguous on the
  ///   Windows compilers where the generic `(const void *, size_t, bool)`
  ///   overload and the integral-length templates otherwise collide on the
  ///   `bool` third argument ([#142](https://github.com/Mithril-mine/libmdbx-devel/issues/142)).
  /// \param bytes  The number of bytes of the source data.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  template <typename INT, typename ::std::enable_if<::std::is_integral<INT>::value && !::std::is_same<INT, bool>::value,
                                                    int>::type = 0>
  MDBX_CXX20_CONSTEXPR buffer(::std::nullptr_t, INT bytes, bool make_reference)
      : buffer(static_cast<const void *>(nullptr), check_nullptr_length(size_t(bytes)), make_reference) {}

  /// \brief Creates a buffer that stores a copy of the string's content.
  /// The source string may be a temporary: the bytes are copied immediately,
  /// so there are no lifetime requirements (unlike mdbx::slice).
  /// \param str  The source string.
  /// \param alloc  The allocator for the buffer storage.
  template <class CHAR, class T, class A>
  explicit MDBX_CXX20_CONSTEXPR buffer(const ::std::basic_string<CHAR, T, A> &str,
                                       const allocator_type &alloc = allocator_type())
      : buffer(inherited(str), false, alloc) {}

  /// \brief Creates a buffer that either stores a copy of the string's content
  /// (make_reference=false, the default choice) or just refers to it
  /// (make_reference=true, then the string must outlive the buffer —
  /// the same lifetime rules as for mdbx::slice).
  /// \param str  The source string.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \param alloc  The allocator for the buffer storage.
  template <class CHAR, class T, class A>
  explicit MDBX_CXX20_CONSTEXPR buffer(const ::std::basic_string<CHAR, T, A> &str, bool make_reference,
                                       const allocator_type &alloc = allocator_type())
      : buffer(inherited(str), make_reference, alloc) {}

  /// \brief Constructs a buffer storing a copy of a null-terminated string.
  /// \param c_str  The source null-terminated string.
  /// \param alloc  The allocator for the buffer storage.
  MDBX_CXX20_CONSTEXPR
  buffer(const char *c_str, const allocator_type &alloc = allocator_type()) : buffer(inherited(c_str), alloc) {}

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Constructs a buffer storing a copy of a string view.
  /// \param view   The source string view.
  /// \param alloc  The allocator for the buffer storage.
  template <class CHAR, class T>
  MDBX_CXX20_CONSTEXPR buffer(const ::std::basic_string_view<CHAR, T> &view,
                              const allocator_type &alloc = allocator_type())
      : buffer(inherited(view), alloc) {}
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Constructs an empty buffer with the given headroom and tailroom.
  /// \param head_room  The number of bytes to reserve in front of the data.
  /// \param tail_room  The number of bytes to reserve after the data.
  /// \param alloc      The allocator for the buffer storage.
  buffer(size_t head_room, size_t tail_room, const allocator_type &alloc = allocator_type())
      : silo_(check_length(head_room, tail_room), alloc) {
    iov_base = silo_.get(head_room);
    MDBX_INLINE_API_ASSERT(iov_len == 0);
  }

  /// \brief Constructs an empty buffer with the given capacity.
  /// \param capacity  The capacity in bytes to allocate.
  /// \param alloc     The allocator for the buffer storage.
  buffer(size_t capacity, const allocator_type &alloc = allocator_type()) : silo_(check_length(capacity), alloc) {
    iov_base = silo_.get();
    MDBX_INLINE_API_ASSERT(iov_len == 0);
  }

  /// \brief Constructs a buffer storing a copy of the slice with reserved headroom/tailroom.
  /// \param head_room  The number of bytes to reserve in front of the data.
  /// \param src        The source slice.
  /// \param tail_room  The number of bytes to reserve after the data.
  /// \param alloc      The allocator for the buffer storage.
  buffer(size_t head_room, const slice &src, size_t tail_room, const allocator_type &alloc = allocator_type())
      : silo_(check_length(head_room, src.length(), tail_room), alloc) {
    iov_len = src.length();
    iov_base = memcpy(silo_.get(head_room), src.data(), iov_len);
  }

  /// \brief Constructs a buffer owning a copy of the slice when its data is dirty
  /// within the given transaction, otherwise a reference to it.
  /// \param transaction  The transaction to check the data dirtiness against.
  /// \param src          The source slice.
  /// \param alloc        The allocator for the buffer storage.
  inline buffer(const txn &transaction, const slice &src, const allocator_type &alloc = allocator_type());

  /// \brief Move constructor: the source buffer is invalidated.
  /// \param src  The source buffer to be moved from.
  buffer(buffer &&src) noexcept(::std::is_nothrow_move_constructible<allocator_type>::value)
      : inherited(/* no move here */ src), silo_(::std::move(src.silo_), src.is_reference()) {
    /* CoverityScan issues an erroneous warning here about using an uninitialized object. Which is not true,
     * since in C++ (unlike Rust) an object remains initialized after a move-assignment operation; Moreover,
     * a destructor will be called for such an object (this is explicitly stated in all C++ standards, starting from the
     * 11th). */
    /* coverity[USE_AFTER_MOVE] */
    fixup_imported_inplace(src.silo_.bin_);
    src.invalidate();
  }

  /// \brief Build a null buffer which zero length and refers to null address.
  /// \returns The null buffer instance.
  MDBX_CXX14_CONSTEXPR static buffer null() noexcept { return buffer(inherited::null()); }

  /// \brief Builds an invalid buffer that has a non-zero length and refers to
  /// a null address.
  /// \returns The invalid buffer instance.
  MDBX_CXX14_CONSTEXPR static buffer invalid() noexcept { return buffer(inherited::invalid()); }

  /// \brief Wraps an object of a standard-layout type into a buffer.
  /// \param pod             The object to be wrapped.
  /// \param make_reference  Make the buffer a reference to the object instead of copying it.
  /// \param alloc           The allocator for the buffer storage.
  /// \returns The buffer wrapping the object.
  template <typename POD>
  static buffer wrap(const POD &pod, bool make_reference = false, const allocator_type &alloc = allocator_type()) {
    return buffer(inherited::wrap(pod), make_reference, alloc);
  }

  /// \brief Returns a new buffer with a hexadecimal dump of the slice content.
  /// \param source  The source object.
  /// \param uppercase  Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a hexadecimal dump of the slice content.
  static buffer hex(const slice &source, bool uppercase = false, unsigned wrap_width = 0,
                    const allocator_type &alloc = allocator_type()) {
    return source.template encode_hex<ALLOCATOR, CAPACITY_POLICY>(uppercase, wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  /// \param source  The source object.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  static buffer base58(const slice &source, unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) {
    return source.template encode_base58<ALLOCATOR, CAPACITY_POLICY>(wrap_width, alloc);
  }
  /// \brief Returns a new buffer with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  /// \param source  The source object.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  static buffer base64(const slice &source, unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) {
    return source.template encode_base64<ALLOCATOR, CAPACITY_POLICY>(wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a hexadecimal dump of the given pod.
  /// \param pod  The object of a standard-layout type.
  /// \param uppercase  Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a hexadecimal dump of the given pod.
  template <typename POD, typename ::std::enable_if<!::std::is_constructible<slice, const POD &>::value, int>::type = 0>
  static buffer hex(const POD &pod, bool uppercase = false, unsigned wrap_width = 0,
                    const allocator_type &alloc = allocator_type()) {
    return hex(inherited::wrap(pod), uppercase, wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of the given pod.
  /// \param pod         The object of a standard-layout type.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc       The allocator for the buffer storage.
  /// \returns A new buffer with a Base58 dump of the given pod.
  template <typename POD, typename ::std::enable_if<!::std::is_constructible<slice, const POD &>::value, int>::type = 0>
  static buffer base58(const POD &pod, unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) {
    return base58(inherited::wrap(pod), wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of the given pod.
  /// \param pod  The object of a standard-layout type.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a [Base64](https://en.wikipedia.org/wiki/Base64) dump of the given pod.
  template <typename POD, typename ::std::enable_if<!::std::is_constructible<slice, const POD &>::value, int>::type = 0>
  static buffer base64(const POD &pod, unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) {
    return base64(inherited::wrap(pod), wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a hexadecimal dump of the slice content.
  /// \param uppercase  Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a hexadecimal dump of the slice content.
  buffer encode_hex(bool uppercase = false, unsigned wrap_width = 0,
                    const allocator_type &alloc = allocator_type()) const {
    return inherited::template encode_hex<ALLOCATOR, CAPACITY_POLICY>(uppercase, wrap_width, alloc);
  }

  /// \brief Returns a new buffer with a
  /// [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a [Base58](https://en.wikipedia.org/wiki/Base58) dump of the slice content.
  buffer encode_base58(unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) const {
    return inherited::template encode_base58<ALLOCATOR, CAPACITY_POLICY>(wrap_width, alloc);
  }
  /// \brief Returns a new buffer with a
  /// [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns A new buffer with a [Base64](https://en.wikipedia.org/wiki/Base64) dump of the slice content.
  buffer encode_base64(unsigned wrap_width = 0, const allocator_type &alloc = allocator_type()) const {
    return inherited::template encode_base64<ALLOCATOR, CAPACITY_POLICY>(wrap_width, alloc);
  }

  /// \brief Decodes hexadecimal dump from the slice content to returned buffer.
  /// \param source  The source object.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  static buffer hex_decode(const slice &source, bool ignore_spaces = false,
                           const allocator_type &alloc = allocator_type()) {
    return source.template hex_decode<ALLOCATOR, CAPACITY_POLICY>(ignore_spaces, alloc);
  }

  /// \brief Decodes [Base58](https://en.wikipedia.org/wiki/Base58) dump
  /// from the slice content to returned buffer.
  /// \param source  The source object.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  static buffer base58_decode(const slice &source, bool ignore_spaces = false,
                              const allocator_type &alloc = allocator_type()) {
    return source.template base58_decode<ALLOCATOR, CAPACITY_POLICY>(ignore_spaces, alloc);
  }

  /// \brief Decodes [Base64](https://en.wikipedia.org/wiki/Base64) dump
  /// from the slice content to returned buffer.
  /// \param source  The source object.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  static buffer base64_decode(const slice &source, bool ignore_spaces = false,
                              const allocator_type &alloc = allocator_type()) {
    return source.template base64_decode<ALLOCATOR, CAPACITY_POLICY>(ignore_spaces, alloc);
  }

  /// \brief Decodes hexadecimal dump
  /// from the buffer content to new returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  buffer hex_decode(bool ignore_spaces = false, const allocator_type &alloc = allocator_type()) const {
    return hex_decode(*this, ignore_spaces, alloc);
  }

  /// \brief Decodes [Base58](https://en.wikipedia.org/wiki/Base58) dump
  /// from the buffer content to new returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  buffer base58_decode(bool ignore_spaces = false, const allocator_type &alloc = allocator_type()) const {
    return base58_decode(*this, ignore_spaces, alloc);
  }

  /// \brief Decodes [Base64](https://en.wikipedia.org/wiki/Base64) dump
  /// from the buffer content to new returned buffer.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The resulting buffer.
  buffer base64_decode(bool ignore_spaces = false, const allocator_type &alloc = allocator_type()) const {
    return base64_decode(*this, ignore_spaces, alloc);
  }

  /// \brief Reserves storage space.
  /// \param wanna_headroom  The desired headroom in bytes.
  /// \param wanna_tailroom  The desired tailroom in bytes.
  void reserve(size_t wanna_headroom, size_t wanna_tailroom) {
    wanna_headroom = ::std::min(
        ::std::max(headroom(), wanna_headroom),
        (wanna_headroom < max_length - pettiness_threshold) ? wanna_headroom + pettiness_threshold : wanna_headroom);
    wanna_tailroom = ::std::min(
        ::std::max(tailroom(), wanna_tailroom),
        (wanna_tailroom < max_length - pettiness_threshold) ? wanna_tailroom + pettiness_threshold : wanna_tailroom);
    const size_t wanna_capacity = check_length(wanna_headroom, length(), wanna_tailroom);

    if (is_freestanding())
      iov_base = silo_.template reshape<false>(wanna_capacity, wanna_headroom, iov_base, iov_len);
    else if (wanna_capacity > iov_len)
      iov_base = silo_.template reshape<true>(wanna_capacity, wanna_headroom, iov_base, iov_len);

    MDBX_INLINE_API_ASSERT(headroom() >= wanna_headroom && headroom() <= wanna_headroom + pettiness_threshold);
    /* The upper bound is intentionally not checked for the tailroom: the
     * capacity-growth policy may legitimately over-provision (e.g. by
     * doubling), so the only guaranteed contract here is the lower bound. */
    MDBX_INLINE_API_ASSERT(tailroom() >= wanna_tailroom);
  }

  /// \brief Reserves space before the payload.
  /// \param wanna_headroom  The desired headroom in bytes.
  void reserve_headroom(size_t wanna_headroom) { reserve(wanna_headroom, 0); }

  /// \brief Reserves space after the payload.
  /// \param wanna_tailroom  The desired tailroom in bytes.
  void reserve_tailroom(size_t wanna_tailroom) { reserve(0, wanna_tailroom); }

  /// \brief Reserves storage space for at least `size` bytes of payload,
  /// without changing the current length. STL-compatible counterpart of
  /// \ref reserve(size_t, size_t) that keeps the headroom unchanged.
  /// \param size  The new length of the data.
  void reserve(size_t size) {
    if (size > length() && size - length() > tailroom())
      reserve(headroom(), size - length());
  }

  /// \brief Assigns the buffer to reference an external memory range without copying.
  /// \param ptr    The address of the source data.
  /// \param bytes  The number of bytes of the source data.
  /// \returns A reference to this instance.
  buffer &assign_reference(const void *ptr, size_t bytes) {
    silo_.clear();
    inherited::assign(ptr, bytes);
    return *this;
  }

  /// \brief Assigns the buffer to own a copy of an external memory range.
  /// \param ptr    The address of the source data.
  /// \param bytes  The number of bytes of the source data.
  /// \returns A reference to this instance.
  buffer &assign_freestanding(const void *ptr, size_t bytes) {
    inherited::assign(silo_.assign(static_cast<const typename silo::value_type *>(ptr), check_length(bytes)), bytes);
    return *this;
  }

  /// \brief Swaps this buffer with another one.
  /// \param other  The buffer to swap with.
  MDBX_CXX20_CONSTEXPR void swap(buffer &other) noexcept(swap_alloc::is_nothrow()) {
    const auto pair = silo::exchange(silo_, content_modality(), other.silo_, other.content_modality());
    inherited::swap(other);
    if (pair.first)
      fixup_imported_inplace(other.silo_.bin_);
    if (pair.second)
      other.fixup_imported_inplace(silo_.bin_);
  }

  /// \brief Swaps two buffers (the ADL-friendly free function).
  /// \param left   The left-hand buffer.
  /// \param right  The right-hand buffer.
  MDBX_CXX20_CONSTEXPR friend void swap(buffer &left, buffer &right) noexcept(buffer::is_swap_nothrow()) {
    left.swap(right);
  }

  /// \brief Makes a deep copy of the given buffer preserving headroom/tailroom.
  /// \param src    The source buffer.
  /// \param alloc  The allocator for the new buffer storage.
  /// \returns The cloned buffer.
  static buffer clone(const buffer &src, const allocator_type &alloc = allocator_type()) {
    return buffer(src.headroom(), src, src.tailroom(), alloc);
  }

  /// \brief Makes a lightweight copy: inplace when this buffer is inplace,
  /// otherwise a reference to the same data.
  /// \returns The new buffer instance.
  MDBX_CXX20_CONSTEXPR buffer make_inplace_or_reference() const {
    return buffer(static_cast<const struct slice &>(*this), !is_inplace(),
                  allocator_traits::select_on_container_copy_construction(get_allocator()));
  }

  /// \brief Assigns a copy of the source buffer with the given headroom/tailroom.
  /// \param headroom  The number of bytes to reserve in front of the data.
  /// \param src       The source buffer.
  /// \param tailroom  The number of bytes to reserve after the data.
  /// \returns A reference to this instance.
  MDBX_CXX20_CONSTEXPR buffer &assign(size_t headroom, const buffer &src, size_t tailroom) {
    if (MDBX_LIKELY(this != &src))
      MDBX_CXX20_LIKELY {
        const size_t whole_capacity = check_length(headroom, src.length(), tailroom);
        invalidate();
        copy_assign_alloc::propagate(&silo_, src.silo_);
        iov_base = silo_.template reshape<true>(whole_capacity, headroom, src.data(), src.length());
        iov_len = src.length();
      }
    else
      reserve(headroom, tailroom);
    return *this;
  }

  /// \brief Assigns from another buffer, either copying or referencing it.
  /// \param src             The source buffer.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  MDBX_CXX20_CONSTEXPR buffer &assign(const buffer &src, bool make_reference = false) {
    if (MDBX_LIKELY(this != &src))
      MDBX_CXX20_LIKELY {
        invalidate();
        copy_assign_alloc::propagate(&silo_, src.silo_);
        if (make_reference) {
          silo_.release();
          iov_base = src.iov_base;
        } else {
          iov_base = silo_.template reshape<true>(src.length(), 0, src.data(), src.length());
        }
        iov_len = src.length();
      }
    else if (!make_reference && is_reference())
      insulate();
    return *this;
  }

  /// \brief Move-assigns from another buffer; the source is invalidated.
  /// \param src  The source buffer to be moved from.
  /// \returns A reference to this instance.
  MDBX_CXX20_CONSTEXPR buffer &assign(buffer &&src) noexcept(move_assign_alloc::is_nothrow()) {
    if (MDBX_LIKELY(this != &src))
      MDBX_CXX20_LIKELY {
        const auto kind = src.content_modality();
        if (MDBX_UNLIKELY(kind == modality::reference && src.const_data() &&
                          size_t(static_cast<const byte *>(src.const_data()) - silo_begin()) < silo_.capacity())) {
          /* `src` refers into this buffer's own storage, which the move below would release first,
           * so just move the bytes to the start of the storage without any (re)allocation */
          MDBX_INLINE_API_ASSERT(static_cast<const byte *>(src.const_data()) + src.length() <= silo_end());
          iov_base = ::std::memmove(silo_.data(), src.const_data(), src.length());
          iov_len = src.length();
          src.invalidate();
          return *this;
        }
        const auto src_headroom = src.headroom();
        const auto src_data = src.const_data();
        const auto src_length = src.length();
        inherited::assign(std::move(src));
        if (!move_assign_alloc::is_moveable(&silo_, src.silo_) && kind == modality::allocated) {
          iov_base = silo_.template reshape<true>(src.silo_.capacity(), src_headroom, src_data, src_length);
          return *this;
        }
        move_assign_alloc::propagate(&silo_, src.silo_);
        if (silo_.move_content(src.silo_, kind))
          fixup_imported_inplace(src.silo_.bin_);
      }
    return *this;
  }

  /// \brief Assigns the buffer from a memory range, either copying or referencing it.
  /// \param ptr             The address of the source data.
  /// \param bytes           The number of bytes of the source data.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(const void *ptr, size_t bytes, bool make_reference = false) {
    /* assign_freestanding() would memcpy from `ptr` before any slice guard can
     * fire, so reject a null pointer with a non-zero length up front (issue #142). */
    if (MDBX_UNLIKELY(ptr == nullptr))
      MDBX_CXX20_UNLIKELY check_nullptr_length(bytes);
    return make_reference ? assign_reference(ptr, bytes) : assign_freestanding(ptr, bytes);
  }

  /// \brief Assigns the buffer from a char-pointer and an integral length.
  /// \param ptr             The address of the source data.
  /// \param bytes           The number of bytes of the source data.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  template <typename INT, typename ::std::enable_if<::std::is_integral<INT>::value && !::std::is_same<INT, bool>::value,
                                                    int>::type = 0>
  buffer &assign(const char *ptr, INT bytes, bool make_reference = false) {
    return assign(static_cast<const void *>(ptr), size_t(bytes), make_reference);
  }

  /// \brief Assigns the buffer to a null pointer; only a zero length is accepted.
  /// \param bytes           The number of bytes, must be zero.
  /// \param make_reference  Make the buffer a reference instead of copying.
  /// \returns A reference to this instance.
  template <typename INT, typename ::std::enable_if<::std::is_integral<INT>::value && !::std::is_same<INT, bool>::value,
                                                    int>::type = 0>
  buffer &assign(::std::nullptr_t, INT bytes, bool make_reference = false) {
    return assign(static_cast<const void *>(nullptr), check_nullptr_length(size_t(bytes)), make_reference);
  }

  /// \brief Assigns the buffer from a slice, either copying or referencing it.
  /// \param src             The source slice.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(const struct slice &src, bool make_reference = false) {
    return assign(src.data(), src.length(), make_reference);
  }

  /// \brief Assigns the buffer from a \ref MDBX_val, either copying or referencing it.
  /// \param src             The source \ref MDBX_val instance.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(const ::MDBX_val &src, bool make_reference = false) {
    return assign(src.iov_base, src.iov_len, make_reference);
  }

  /// \brief Move-assigns the buffer from a slice; the source is invalidated.
  /// \param src             The source slice to be moved from.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(struct slice &&src, bool make_reference = false) {
    assign(src.data(), src.length(), make_reference);
    src.invalidate();
    return *this;
  }

  /// \brief Takes over a temporary \ref MDBX_val instance.
  /// \param src             The source \ref MDBX_val to be moved from.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(::MDBX_val &&src, bool make_reference = false) {
    assign(src.iov_base, src.iov_len, make_reference);
    src.iov_base = nullptr;
    return *this;
  }

  /// \brief Assigns the buffer from a [begin,end) memory range.
  /// \param begin           The address of the first byte.
  /// \param end             The address just after the last byte.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(const void *begin, const void *end, bool make_reference = false) {
    return assign(begin, static_cast<const byte *>(end) - static_cast<const byte *>(begin), make_reference);
  }

  /// \brief Assigns the buffer from a string, either copying or referencing it.
  /// \param str             The source string.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  template <class CHAR, class T, class A>
  buffer &assign(const ::std::basic_string<CHAR, T, A> &str, bool make_reference = false) {
    return assign(str.data(), str.length() * sizeof(CHAR), make_reference);
  }

  /// \brief Assigns the buffer from a null-terminated string.
  /// \param c_str           The source null-terminated string.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  buffer &assign(const char *c_str, bool make_reference = false) {
    return assign(c_str, strlen(c_str), make_reference);
  }

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
  /// \brief Assigns the buffer from a string view, either copying or referencing it.
  /// \param view            The source string view.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  template <class CHAR, class T>
  buffer &assign(const ::std::basic_string_view<CHAR, T> &view, bool make_reference = false) {
    return assign(inherited(view), make_reference);
  }

  /// \brief Takes over a temporary string view.
  /// \param view            The source string view to be moved from.
  /// \param make_reference  Make the buffer a reference to the source data instead of copying it.
  /// \returns A reference to this instance.
  template <class CHAR, class T> buffer &assign(::std::basic_string_view<CHAR, T> &&view, bool make_reference = false) {
    assign(inherited(view), make_reference);
    view = {};
    return *this;
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Copy assignment operator.
  /// \param src  The source buffer.
  /// \returns A reference to this instance.
  buffer &operator=(const buffer &src) { return assign(src); }

  /// \brief Move assignment operator; the source is invalidated.
  /// \param src  The source buffer to be moved from.
  /// \returns A reference to this instance.
  buffer &operator=(buffer &&src) noexcept(move_assign_alloc::is_nothrow()) { return assign(::std::move(src)); }

  /// \brief Assigns from a slice, copying its content.
  /// \param src  The source slice.
  /// \returns A reference to this instance.
  buffer &operator=(const struct slice &src) { return assign(src); }

  /// \brief Move-assigns from a slice; the source is invalidated.
  /// \param src  The source slice to be moved from.
  /// \returns A reference to this instance.
  buffer &operator=(struct slice &&src) { return assign(::std::move(src)); }

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Assigns from a string view, copying its content.
  /// \param view  The source string view.
  /// \returns A reference to this instance.
  template <class CHAR, class T> buffer &operator=(const ::std::basic_string_view<CHAR, T> &view) {
    /* assigning a string_view copies the bytes into owned storage, so this
     * allocates and must be allowed to throw on out-of-memory (issue #78) */
    return assign(inherited(view));
  }

  /// \brief Appends a string view to the end of the buffer.
  /// \param view  The string view to append.
  /// \returns A reference to this instance.
  template <class CHAR, class T> buffer &append(const ::std::basic_string_view<CHAR, T> &view) {
    return append(view.data(), view.size() * sizeof(CHAR));
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Converts the buffer content to a `std::basic_string`.
  /// \returns The string with a copy of the buffer content.
  template <class CHAR, class T, class A>
  MDBX_CXX20_CONSTEXPR explicit operator ::std::basic_string<CHAR, T, A>() const {
    return as_string<CHAR, T, A>();
  }

  /// \brief Appends a string to the end of the buffer.
  /// \param str  The string to append.
  /// \returns A reference to this instance.
  template <class CHAR, class T, class A> buffer &append(const ::std::basic_string<CHAR, T, A> &str) {
    return append(inherited(str));
  }

  /// \brief Clears the contents and storage.
  void clear() noexcept { inherited::assign(silo_.clear(), size_t(0)); }

  /// \brief Clears the contents and reserve storage.
  /// \param whole_capacity  The whole capacity to reserve.
  /// \param headroom  The number of bytes to reserve in front of the data.
  void clear_and_reserve(size_t whole_capacity, size_t headroom = 0) {
    inherited::assign(silo_.clear_and_reserve(whole_capacity, headroom), size_t(0));
  }

  /// \brief Reduces memory usage by freeing unused storage space.
  void shrink_to_fit() {
    if (is_freestanding())
      iov_base = silo_.template reshape<false>(iov_len, 0, iov_base, iov_len);
  }

  /// \brief Appends a memory range to the end of the buffer.
  /// \param src    The address of the data to append.
  /// \param bytes  The number of bytes to append.
  /// \returns A reference to this instance.
  buffer &append(const void *src, size_t bytes) {
    if (MDBX_LIKELY(bytes))
      MDBX_CXX20_LIKELY {
        const auto begin_ptr = inherited::byte_ptr();
        const auto src_ptr = static_cast<const byte *>(src);
        /* In case `src` refers into the own storage, save its relative offset
         * since the storage may be relocated by the reserve below. */
        const bool aliased = src_ptr >= begin_ptr && src_ptr < inherited::end_byte_ptr() &&
                             bytes <= size_t(inherited::end_byte_ptr() - src_ptr);
        const size_t src_offset = aliased ? size_t(src_ptr - begin_ptr) : 0;
        if (MDBX_UNLIKELY(tailroom() < check_length(bytes)))
          MDBX_CXX20_UNLIKELY {
            if (MDBX_UNLIKELY(!aliased && size_t(src_ptr - silo_begin()) < silo_.capacity())) {
              /* `src` is in the own storage, but not wholly in the content, which alone the reserve preserves */
              const buffer chunk(src, bytes, get_allocator());
              return append(chunk.data(), chunk.size());
            }
            reserve_tailroom(bytes);
          }
        ::std::memmove(end_byte_ptr(), aliased ? byte_ptr() + src_offset : src_ptr, bytes);
        iov_len += bytes;
      }
    return *this;
  }

  /// \brief Appends a single byte to the end of the buffer.
  /// \param c  The byte to append.
  /// \returns A reference to this instance.
  buffer &append(const byte c) {
    if (MDBX_UNLIKELY(tailroom() < 1))
      MDBX_CXX20_UNLIKELY reserve_tailroom(1);
    *end_byte_ptr() = c;
    iov_len += 1;
    return *this;
  }

  /// \brief Appends a slice to the end of the buffer.
  /// \param chunk  The data chunk to append.
  /// \returns A reference to this instance.
  buffer &append(const struct slice &chunk) { return append(chunk.data(), chunk.size()); }

  /// \brief Prepends a memory range to the beginning of the buffer (into the headroom).
  /// \param src    The address of the data to prepend.
  /// \param bytes  The number of bytes to prepend.
  /// \returns A reference to this instance.
  buffer &add_header(const void *src, size_t bytes) {
    if (MDBX_LIKELY(bytes))
      MDBX_CXX20_LIKELY {
        const auto begin_ptr = inherited::byte_ptr();
        const auto src_ptr = static_cast<const byte *>(src);
        /* In case `src` refers into the own storage, save its relative offset
         * since the storage may be relocated by the reserve below. */
        const bool aliased = src_ptr >= begin_ptr && src_ptr < inherited::end_byte_ptr() &&
                             bytes <= size_t(inherited::end_byte_ptr() - src_ptr);
        const size_t src_offset = aliased ? size_t(src_ptr - begin_ptr) : 0;
        if (MDBX_UNLIKELY(headroom() < check_length(bytes)))
          MDBX_CXX20_UNLIKELY {
            if (MDBX_UNLIKELY(!aliased && size_t(src_ptr - silo_begin()) < silo_.capacity())) {
              /* `src` is in the own storage, but not wholly in the content, which alone the reserve preserves */
              const buffer chunk(src, bytes, get_allocator());
              return add_header(chunk.data(), chunk.size());
            }
            reserve_headroom(bytes);
          }
        iov_base = ::std::memmove(byte_ptr() - bytes, aliased ? byte_ptr() + src_offset : src_ptr, bytes);
        iov_len += bytes;
      }
    return *this;
  }

  /// \brief Prepends a slice to the beginning of the buffer (into the headroom).
  /// \param chunk  The data chunk to prepend.
  /// \returns A reference to this instance.
  buffer &add_header(const struct slice &chunk) { return add_header(chunk.data(), chunk.size()); }

  /// \brief Appends the bytes produced by the given producer to the end of the buffer.
  ///
  /// \pre The producer must not read from the destination region `[end_char_ptr(), silo_end())`
  /// while producing: when the result fits the tailroom, it is written in place, at
  /// `end_char_ptr()`, so a source overlapping the own tailroom would be overwritten before
  /// being read (the `dest` argument of `write_bytes()` is `__restrict`). The `append_hex()`,
  /// `append_base58()`, `append_base64()` and the `append_decoded_*()` methods handle a
  /// slice source that reaches the own tailroom themselves (by copying it out first); for a
  /// custom producer, avoiding such an overlap is the caller's responsibility.
  ///
  /// \param [in,out] producer  A mutable or immutable byte producer, e.g. `to_hex`,
  ///                           `from_base64`, or a user-provided one; it may refer into this
  ///                           buffer, but only into the content or the headroom.
  /// \returns A reference to this buffer.
  template <MDBX_CXX20_CONCEPT(MutableByteProducer, PRODUCER)> buffer &append_producer(PRODUCER &producer) {
    const size_t wanna_bytes = producer.envisage_result_length();
    if (MDBX_LIKELY(wanna_bytes))
      MDBX_CXX20_LIKELY {
        if (MDBX_UNLIKELY(tailroom() < check_length(wanna_bytes)))
          MDBX_CXX20_UNLIKELY {
            /* the producer may refer into this buffer, so produce before relocating the storage */
            buffer chunk(wanna_bytes, get_allocator());
            chunk.append_producer(producer);
            return append(chunk.data(), chunk.size());
          }
        return set_end(producer.write_bytes(end_char_ptr(), tailroom()));
      }
    return *this;
  }

  /// \brief Appends the bytes produced by the given immutable producer, see the overload above.
  /// \pre The same: the producer must not read from `[end_char_ptr(), silo_end())` while producing.
  /// \param producer  An immutable byte producer, e.g. `to_hex` or `from_base64`.
  /// \returns A reference to this buffer.
  template <MDBX_CXX20_CONCEPT(ImmutableByteProducer, PRODUCER)> buffer &append_producer(const PRODUCER &producer) {
    const size_t wanna_bytes = producer.envisage_result_length();
    if (MDBX_LIKELY(wanna_bytes))
      MDBX_CXX20_LIKELY {
        if (MDBX_UNLIKELY(tailroom() < check_length(wanna_bytes)))
          MDBX_CXX20_UNLIKELY {
            /* the producer may refer into this buffer, so produce before relocating the storage */
            buffer chunk(wanna_bytes, get_allocator());
            chunk.append_producer(producer);
            return append(chunk.data(), chunk.size());
          }
        return set_end(producer.write_bytes(end_char_ptr(), tailroom()));
      }
    return *this;
  }

  /// \brief Appends a hexadecimal dump of the given data to the buffer.
  /// \param data        The data to be encoded.
  /// \param uppercase   Use uppercase letters for hexadecimal digits.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \returns A reference to this instance.
  buffer &append_hex(const struct slice &data, bool uppercase = false, unsigned wrap_width = 0) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_hex(buffer(data, get_allocator()), uppercase, wrap_width);
    return append_producer(to_hex(data, uppercase, wrap_width));
  }

  /// \brief Appends a [Base58](https://en.wikipedia.org/wiki/Base58) dump of the given data to the buffer.
  /// \param data        The data to be encoded.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \returns A reference to this instance.
  buffer &append_base58(const struct slice &data, unsigned wrap_width = 0) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_base58(buffer(data, get_allocator()), wrap_width);
    return append_producer(to_base58(data, wrap_width));
  }

  /// \brief Appends a [Base64](https://en.wikipedia.org/wiki/Base64) dump of the given data to the buffer.
  /// \param data        The data to be encoded.
  /// \param wrap_width  The width to wrap lines with a newline, zero means no wrapping.
  /// \returns A reference to this instance.
  buffer &append_base64(const struct slice &data, unsigned wrap_width = 0) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_base64(buffer(data, get_allocator()), wrap_width);
    return append_producer(to_base64(data, wrap_width));
  }

  /// \brief Appends data decoded from a hexadecimal dump to the buffer.
  /// \param data           The hexadecimal dump to be decoded.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \returns A reference to this instance.
  buffer &append_decoded_hex(const struct slice &data, bool ignore_spaces = false) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_decoded_hex(buffer(data, get_allocator()), ignore_spaces);
    return append_producer(from_hex(data, ignore_spaces));
  }

  /// \brief Appends data decoded from a [Base58](https://en.wikipedia.org/wiki/Base58) dump to the buffer.
  /// \param data           The Base58 dump to be decoded.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \returns A reference to this instance.
  buffer &append_decoded_base58(const struct slice &data, bool ignore_spaces = false) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_decoded_base58(buffer(data, get_allocator()), ignore_spaces);
    return append_producer(from_base58(data, ignore_spaces));
  }

  /// \brief Appends data decoded from a [Base64](https://en.wikipedia.org/wiki/Base64) dump to the buffer.
  /// \param data           The Base64 dump to be decoded.
  /// \param ignore_spaces  Skip whitespaces while decoding.
  /// \returns A reference to this instance.
  buffer &append_decoded_base64(const struct slice &data, bool ignore_spaces = false) {
    if (MDBX_UNLIKELY(reaches_tailroom(data)))
      return append_decoded_base64(buffer(data, get_allocator()), ignore_spaces);
    return append_producer(from_base64(data, ignore_spaces));
  }

  /// \brief Appends an 8-bit value in little-endian order.
  /// \param u8  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u8(uint_fast8_t u8) {
    if (MDBX_UNLIKELY(tailroom() < 1))
      MDBX_CXX20_UNLIKELY reserve_tailroom(1);
    *end_byte_ptr() = uint8_t(u8);
    iov_len += 1;
    return *this;
  }

  /// \brief Appends a single byte.
  /// \param byte  The byte to append.
  /// \returns A reference to this instance.
  buffer &append_byte(uint_fast8_t byte) { return append_u8(byte); }

  /// \brief Appends a 16-bit value in little-endian order.
  /// \param u16  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u16(uint_fast16_t u16) {
    if (MDBX_UNLIKELY(tailroom() < 2))
      MDBX_CXX20_UNLIKELY reserve_tailroom(2);
    const auto ptr = end_byte_ptr();
    ptr[0] = uint8_t(u16);
    ptr[1] = uint8_t(u16 >> 8);
    iov_len += 2;
    return *this;
  }

  /// \brief Appends a 24-bit value in little-endian order.
  /// \param u24  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u24(uint_fast32_t u24) {
    if (MDBX_UNLIKELY(tailroom() < 3))
      MDBX_CXX20_UNLIKELY reserve_tailroom(3);
    const auto ptr = end_byte_ptr();
    ptr[0] = uint8_t(u24);
    ptr[1] = uint8_t(u24 >> 8);
    ptr[2] = uint8_t(u24 >> 16);
    iov_len += 3;
    return *this;
  }

  /// \brief Appends a 32-bit value in little-endian order.
  /// \param u32  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u32(uint_fast32_t u32) {
    if (MDBX_UNLIKELY(tailroom() < 4))
      MDBX_CXX20_UNLIKELY reserve_tailroom(4);
    const auto ptr = end_byte_ptr();
    ptr[0] = uint8_t(u32);
    ptr[1] = uint8_t(u32 >> 8);
    ptr[2] = uint8_t(u32 >> 16);
    ptr[3] = uint8_t(u32 >> 24);
    iov_len += 4;
    return *this;
  }

  /// \brief Appends a 48-bit value in little-endian order.
  /// \param u48  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u48(uint_fast64_t u48) {
    if (MDBX_UNLIKELY(tailroom() < 6))
      MDBX_CXX20_UNLIKELY reserve_tailroom(6);
    const auto ptr = end_byte_ptr();
    ptr[0] = uint8_t(u48);
    ptr[1] = uint8_t(u48 >> 8);
    ptr[2] = uint8_t(u48 >> 16);
    ptr[3] = uint8_t(u48 >> 24);
    ptr[4] = uint8_t(u48 >> 32);
    ptr[5] = uint8_t(u48 >> 40);
    iov_len += 6;
    return *this;
  }

  /// \brief Appends a 64-bit value in little-endian order.
  /// \param u64  The value to append.
  /// \returns A reference to this instance.
  buffer &append_u64(uint_fast64_t u64) {
    if (MDBX_UNLIKELY(tailroom() < 8))
      MDBX_CXX20_UNLIKELY reserve_tailroom(8);
    const auto ptr = end_byte_ptr();
    ptr[0] = uint8_t(u64);
    ptr[1] = uint8_t(u64 >> 8);
    ptr[2] = uint8_t(u64 >> 16);
    ptr[3] = uint8_t(u64 >> 24);
    ptr[4] = uint8_t(u64 >> 32);
    ptr[5] = uint8_t(u64 >> 40);
    ptr[6] = uint8_t(u64 >> 48);
    ptr[7] = uint8_t(u64 >> 56);
    iov_len += 8;
    return *this;
  }

  //----------------------------------------------------------------------------

  /// \brief Makes a key buffer from a C string literal.
  /// \param text            The null-terminated string literal.
  /// \param make_reference  Make the buffer a reference to the source data
  ///                          instead of copying it.
  /// \returns The key \ref buffer.
  template <size_t SIZE> static buffer key_from(const char (&text)[SIZE], bool make_reference = true) {
    return buffer(inherited(text), make_reference);
  }

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)
  /// \brief Makes a key buffer from a `std::basic_string_view`.
  /// \param src             The source string view.
  /// \param make_reference  Make the buffer a reference to the source data
  ///                          instead of copying it.
  /// \returns The key \ref buffer.
  template <class CHAR, class T>
  static buffer key_from(const ::std::basic_string_view<CHAR, T> &src, bool make_reference = false) {
    return buffer(src, make_reference);
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief Makes a key buffer from a null-terminated C string.
  /// \param src             The source null-terminated string.
  /// \param make_reference  Make the buffer a reference to the source data
  ///                          instead of copying it.
  /// \returns The key \ref buffer.
  static buffer key_from(const char *src, bool make_reference = false) { return buffer(src, make_reference); }

  /// \brief Makes a key buffer from a `std::basic_string`.
  /// \param src             The source string.
  /// \param make_reference  Make the buffer a reference to the source data
  ///                          instead of copying it.
  /// \returns The key \ref buffer.
  template <class CHAR, class T, class A>
  static buffer key_from(const ::std::basic_string<CHAR, T, A> &src, bool make_reference = false) {
    return buffer(src, make_reference);
  }

  /// \brief Makes a key buffer by moving the given buffer.
  /// \param src  The source buffer to be moved.
  /// \returns The key \ref buffer.
  static buffer key_from(buffer &&src) noexcept { return buffer(::std::move(src)); }

  /// \brief Makes a comparable key from a `double` IEEE754 value.
  /// \param ieee754_64bit  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from_double(const double ieee754_64bit) { return wrap(::mdbx_key_from_double(ieee754_64bit)); }

  /// \brief Makes a comparable key from a `double` IEEE754 value.
  /// \param ieee754_64bit  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const double ieee754_64bit) { return key_from_double(ieee754_64bit); }

  /// \brief Makes a comparable key from a `double` IEEE754 value by a pointer.
  /// \param ieee754_64bit  The address of the value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const double *ieee754_64bit) { return wrap(::mdbx_key_from_ptrdouble(ieee754_64bit)); }

  /// \brief Makes a comparable key from an unsigned `uint64_t` value.
  /// \param unsigned_int64  The value to be wrapped.
  /// \returns The key \ref buffer.
  static buffer key_from_u64(const uint64_t unsigned_int64) { return wrap(unsigned_int64); }

  /// \brief Makes a comparable key from an unsigned `uint64_t` value.
  /// \param unsigned_int64  The value to be wrapped.
  /// \returns The key \ref buffer.
  static buffer key_from(const uint64_t unsigned_int64) { return key_from_u64(unsigned_int64); }

  /// \brief Makes a comparable key from a signed `int64_t` value.
  /// \param signed_int64  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from_i64(const int64_t signed_int64) { return wrap(::mdbx_key_from_int64(signed_int64)); }

  /// \brief Makes a comparable key from a signed `int64_t` value.
  /// \param signed_int64  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const int64_t signed_int64) { return key_from_i64(signed_int64); }

  /// \brief Makes a comparable key from a signed `int64_t` JSON-number value.
  /// \param json_integer  The value to be converted.
  /// \returns The key \ref buffer, comparable with keys from `double` values.
  static buffer key_from_jsonInteger(const int64_t json_integer) {
    return wrap(::mdbx_key_from_jsonInteger(json_integer));
  }

  /// \brief Makes a comparable key from a `float` IEEE754 value.
  /// \param ieee754_32bit  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from_float(const float ieee754_32bit) { return wrap(::mdbx_key_from_float(ieee754_32bit)); }

  /// \brief Makes a comparable key from a `float` IEEE754 value.
  /// \param ieee754_32bit  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const float ieee754_32bit) { return key_from_float(ieee754_32bit); }

  /// \brief Makes a comparable key from a `float` IEEE754 value by a pointer.
  /// \param ieee754_32bit  The address of the value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const float *ieee754_32bit) { return wrap(::mdbx_key_from_ptrfloat(ieee754_32bit)); }

  /// \brief Makes a comparable key from an unsigned `uint32_t` value.
  /// \param unsigned_int32  The value to be wrapped.
  /// \returns The key \ref buffer.
  static buffer key_from_u32(const uint32_t unsigned_int32) { return wrap(unsigned_int32); }

  /// \brief Makes a comparable key from an unsigned `uint32_t` value.
  /// \param unsigned_int32  The value to be wrapped.
  /// \returns The key \ref buffer.
  static buffer key_from(const uint32_t unsigned_int32) { return key_from_u32(unsigned_int32); }

  /// \brief Makes a comparable key from a signed `int32_t` value.
  /// \param signed_int32  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from_i32(const int32_t signed_int32) { return wrap(::mdbx_key_from_int32(signed_int32)); }

  /// \brief Makes a comparable key from a signed `int32_t` value.
  /// \param signed_int32  The value to be converted.
  /// \returns The key \ref buffer.
  static buffer key_from(const int32_t signed_int32) { return key_from_i32(signed_int32); }

  /// \brief Returns the base \ref slice view of this buffer.
  /// \returns A constant reference to this buffer as a \ref slice.
  const slice &slice() const noexcept { return *this; }
};

/// \brief A pair of \ref buffer instances for a key and a value, with the same
/// allocator and capacity policy; the buffer counterpart of \ref pair.
template <typename ALLOCATOR, typename CAPACITY_POLICY> struct buffer_pair_spec {
  /// \brief The type of the key/value buffers.
  using buffer_type = buffer<ALLOCATOR, CAPACITY_POLICY>;
  /// \brief The allocator type of the buffers.
  using allocator_type = typename buffer_type::allocator_type;
  /// \brief The `allocator_type` traits.
  using allocator_traits = typename buffer_type::allocator_traits;
  /// \brief The capacity policy type, see \ref default_capacity_policy.
  using reservation_policy = CAPACITY_POLICY;
  /// \brief An alias for `std::pair<buffer_type, buffer_type>`.
  using stl_pair = ::std::pair<buffer_type, buffer_type>;
  buffer_type key;   ///< \brief The key buffer.
  buffer_type value; ///< \brief The value buffer.

  /// \brief Constructs an empty pair.
  MDBX_CXX20_CONSTEXPR buffer_pair_spec() noexcept = default;
  /// \brief Constructs an empty pair with the given allocator.
  /// \param alloc  The allocator for both buffers.
  MDBX_CXX20_CONSTEXPR
  buffer_pair_spec(const allocator_type &alloc) noexcept : key(alloc), value(alloc) {}

  /// \brief Constructs the pair from the given key and value buffers.
  /// \param key    The key buffer.
  /// \param value  The value buffer.
  /// \param alloc  The allocator for both buffers.
  buffer_pair_spec(const buffer_type &key, const buffer_type &value, const allocator_type &alloc = allocator_type())
      : key(key, alloc), value(value, alloc) {}
  /// \brief Constructs the pair from the given key and value buffers.
  /// \param key             The key buffer.
  /// \param value           The value buffer.
  /// \param make_reference  Make the buffers references instead of copies.
  /// \param alloc           The allocator for both buffers.
  buffer_pair_spec(const buffer_type &key, const buffer_type &value, bool make_reference,
                   const allocator_type &alloc = allocator_type())
      : key(key, make_reference, alloc), value(value, make_reference, alloc) {}

  /// \brief Constructs the pair from a `std::pair` of buffers.
  /// \param pair   The source `std::pair`.
  /// \param alloc  The allocator for both buffers.
  buffer_pair_spec(const stl_pair &pair, const allocator_type &alloc = allocator_type())
      : buffer_pair_spec(pair.first, pair.second, alloc) {}
  /// \brief Constructs the pair from a `std::pair` of buffers.
  /// \param pair            The source `std::pair`.
  /// \param make_reference  Make the buffers references instead of copies.
  /// \param alloc           The allocator for both buffers.
  buffer_pair_spec(const stl_pair &pair, bool make_reference, const allocator_type &alloc = allocator_type())
      : buffer_pair_spec(pair.first, pair.second, make_reference, alloc) {}

  /// \brief Constructs the pair from the given key and value slices.
  /// \param key    The key slice.
  /// \param value  The value slice.
  /// \param alloc  The allocator for both buffers.
  buffer_pair_spec(const slice &key, const slice &value, const allocator_type &alloc = allocator_type())
      : key(key, alloc), value(value, alloc) {}
  /// \brief Constructs the pair from the given key and value slices.
  /// \param key             The key slice.
  /// \param value           The value slice.
  /// \param make_reference  Make the buffers references instead of copies.
  /// \param alloc           The allocator for both buffers.
  buffer_pair_spec(const slice &key, const slice &value, bool make_reference,
                   const allocator_type &alloc = allocator_type())
      : key(key, make_reference, alloc), value(value, make_reference, alloc) {}

  /// \brief Constructs the pair from the given key-value pair of slices.
  /// \param pair   The source \ref pair.
  /// \param alloc  The allocator for both buffers.
  buffer_pair_spec(const pair &pair, const allocator_type &alloc = allocator_type())
      : buffer_pair_spec(pair.key, pair.value, alloc) {}
  /// \brief Constructs the pair from the given key-value pair of slices.
  /// \param pair            The source \ref pair.
  /// \param make_reference  Make the buffers references instead of copies.
  /// \param alloc           The allocator for both buffers.
  buffer_pair_spec(const pair &pair, bool make_reference, const allocator_type &alloc = allocator_type())
      : buffer_pair_spec(pair.key, pair.value, make_reference, alloc) {}

  /// \brief Constructs the pair owning copies of dirty data of the given transaction.
  /// \param transaction  The transaction to check the data dirtiness against.
  /// \param key          The key slice.
  /// \param value        The value slice.
  /// \param alloc        The allocator for both buffers.
  buffer_pair_spec(const txn &transaction, const slice &key, const slice &value,
                   const allocator_type &alloc = allocator_type())
      : key(transaction, key, alloc), value(transaction, value, alloc) {}
  /// \brief Constructs the pair owning copies of dirty data of the given transaction.
  /// \param transaction  The transaction to check the data dirtiness against.
  /// \param pair         The source \ref pair.
  /// \param alloc        The allocator for both buffers.
  buffer_pair_spec(const txn &transaction, const pair &pair, const allocator_type &alloc = allocator_type())
      : buffer_pair_spec(transaction, pair.key, pair.value, alloc) {}

  /// \brief Constructs the pair by moving the given buffers.
  /// \param key    The key buffer to be moved from.
  /// \param value  The value buffer to be moved from.
  buffer_pair_spec(buffer_type &&key, buffer_type &&value) noexcept(buffer_type::move_assign_alloc::is_nothrow())
      : key(::std::move(key)), value(::std::move(value)) {}
  /// \brief Copy constructor.
  buffer_pair_spec(const buffer_pair_spec &) = default;
  /// \brief Move constructor.
  /// \param pair  The source pair to be moved from.
  buffer_pair_spec(buffer_pair_spec &&pair) noexcept(buffer_type::move_assign_alloc::is_nothrow())
      : buffer_pair_spec(::std::move(pair.key), ::std::move(pair.value)) {}

  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(const buffer_pair_spec &) = default;
  /// \brief Move assignment operator.
  /// \param src  The source pair to be moved from.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(buffer_pair_spec &&src) {
    key.assign(std::move(src.key));
    value.assign(std::move(src.value));
    return *this;
  }

  /// \brief Assigns from a \ref pair of slices, copying the content.
  /// \param src  The source pair.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(const pair &src) {
    key.assign(src.key);
    value.assign(src.value);
    return *this;
  }
  /// \brief Move-assigns from a \ref pair of slices.
  /// \param src  The source pair to be moved from.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(pair &&src) {
    key.assign(std::move(src.key));
    value.assign(std::move(src.value));
    return *this;
  }
  /// \brief Assigns from a `std::pair` of slices, copying the content.
  /// \param src  The source `std::pair`.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(const stl_pair &src) {
    key.assign(src.first);
    value.assign(src.second);
    return *this;
  }
  /// \brief Move-assigns from a `std::pair` of slices.
  /// \param src  The source `std::pair` to be moved from.
  /// \returns A reference to this instance.
  buffer_pair_spec &operator=(stl_pair &&src) {
    key.assign(std::move(src.first));
    value.assign(std::move(src.second));
    return *this;
  }

  /// \brief Checks whether data chunk stored inside the buffers both, otherwise
  /// at least one of buffers just refers to data located outside.
  /// \returns `true` if both buffers own their data.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR bool is_freestanding() const noexcept {
    return key.is_freestanding() && value.is_freestanding();
  }
  /// \brief Checks whether one of the buffers just refers to data located
  /// outside the buffer, rather than stores it.
  /// \returns `true` if at least one buffer refers to external data.
  MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX20_CONSTEXPR bool is_reference() const noexcept {
    return key.is_reference() || value.is_reference();
  }
  /// \brief Makes buffers owning the data.
  /// \details If buffer refers to an external data, then makes it the owner
  /// of clone by allocating storage and copying the data.
  void make_freestanding() {
    key.make_freestanding();
    value.make_freestanding();
  }

  /// \brief Converts to a \ref pair of slices referencing the same data.
  /// \returns The pair of slices.
  operator pair() const noexcept { return pair(key, value); }
};

/// end of cxx_data @}

/// \defgroup cxx_core Environment, Transactions, Cursors, Key-value tables and map handles
/// @{

/// \brief Cache entry for the transparent read-your-writes caching, see
/// txn::get_cached() and ::mdbx_cache_get().
///
/// \details A value type wrapping \ref MDBX_cache_entry_t. Each entry is
/// associated with a key and holds a lightweight "version stamp" plus the
/// offset of the cached value inside the memory-mapped database file, so
/// subsequent lookups stop as soon as they reach an unmodified page.
///
/// An entry must be initialized (\ref reset()) before the first use. Entries
/// may be placed in shared memory and used by multiple processes, though
/// libmdbx does not provide such interaction management yet.
class cache_entry : public MDBX_cache_entry_t {
public:
  /// \brief Constructs the entry and initializes it to the "no cached info" state.
  cache_entry() noexcept { reset(); }
  /// \brief Copy constructor.
  cache_entry(const cache_entry &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  cache_entry &operator=(const cache_entry &) noexcept = default;
  /// \brief Move constructor: copies the source entry and resets it.
  /// \param other  The source entry to be moved from.
  cache_entry(cache_entry &&other) noexcept {
    *this = other;
    other.reset();
  }

  /// \brief Initializes the cache entry to the "no cached info" state.
  void reset() noexcept { mdbx_cache_init(this); }

  /// \brief Returns `true` if the entry carries a confirmed result of a
  /// previous lookup, i.e. either a cached value or a cached "not found".
  /// \returns `true` if the entry is confirmed, `false` otherwise.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept { return last_confirmed_txnid != 0; }
  /// \brief Returns `true` if the entry caches the "key not found" state.
  /// \returns `true` if the entry caches the "not found" state.
  MDBX_CXX14_CONSTEXPR bool notfound() const noexcept { return offset == 0; }
  /// \brief Returns the transaction/MVCC-snapshot ID of the internal
  /// structure that holds the cached data.
  /// \returns The transaction ID of the internal structure.
  MDBX_CXX14_CONSTEXPR uint64_t transaction_id() const noexcept { return trunk_txnid; }
  /// \brief Returns the recent transaction/MVCC-snapshot ID wherein the
  /// entry was checked and confirmed.
  /// \returns The transaction ID of the last confirmation.
  MDBX_CXX14_CONSTEXPR uint64_t last_confirmed() const noexcept { return last_confirmed_txnid; }

  /// \brief Constructs the entry as a copy of the given C API structure.
  /// \param ce  The source \ref MDBX_cache_entry_t instance.
  MDBX_CXX20_CONSTEXPR cache_entry(const MDBX_cache_entry_t &ce) noexcept { mdbx::memcpy(this, &ce, sizeof(*this)); }
  /// \brief Assigns the given C API structure to this entry.
  /// \param ce  The source \ref MDBX_cache_entry_t instance.
  /// \returns A reference to this instance.
  MDBX_CXX20_CONSTEXPR cache_entry &operator=(const MDBX_cache_entry_t &ce) noexcept {
    mdbx::memcpy(this, &ce, sizeof(*this));
    return *this;
  };

  friend MDBX_CXX14_CONSTEXPR bool operator==(const cache_entry &a, const cache_entry &b) noexcept {
    return a.trunk_txnid == b.trunk_txnid && a.last_confirmed_txnid == b.last_confirmed_txnid && a.offset == b.offset &&
           a.length == b.length;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator!=(const cache_entry &a, const cache_entry &b) noexcept { return !(a == b); }
};

//------------------------------------------------------------------------------

/// \brief Loop control constants for readers enumeration functor and other
/// cases. \see env::enumerate_readers()
enum loop_control { continue_loop = 0, exit_loop = INT32_MIN };

/// \brief Kinds of the keys and corresponding modes of comparing it.
enum class key_mode {
  usual = MDBX_DB_DEFAULTS,  ///< Usual variable length keys with byte-by-byte
                             ///< lexicographic comparison like `std::memcmp()`.
  reverse = MDBX_REVERSEKEY, ///< Variable length keys with byte-by-byte
                             ///< lexicographic comparison in reverse order,
                             ///< from the end of the keys to the beginning.
  ordinal = MDBX_INTEGERKEY, ///< Keys are binary integers in native byte order,
                             ///< either `uint32_t` or `uint64_t`, and will be
                             ///< sorted as such. The keys must all be of the
                             ///< same size and must be aligned while passing
                             ///< as arguments.
  msgpack = -1               ///< Keys are in [MessagePack](https://msgpack.org/)
                             ///< format with appropriate comparison.
                             ///< \note Not yet implemented and PRs are welcome.
};

/// \brief Checks whether the given key mode is the "usual" one.
/// \param mode  The key mode to be checked.
/// \returns `true` if the mode matches the "usual" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_usual(key_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & (MDBX_REVERSEKEY | MDBX_INTEGERKEY)) == 0;
}

/// \brief Checks whether the given key mode is the "ordinal" one.
/// \param mode  The key mode to be checked.
/// \returns `true` if the mode matches the "ordinal" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_ordinal(key_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_INTEGERKEY) != 0;
}

/// \brief Checks whether the given key mode is the "same-length" one.
/// \param mode  The key mode to be checked.
/// \returns `true` if the mode matches the "same-length" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_samelength(key_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_INTEGERKEY) != 0;
}

/// \brief Checks whether the given key mode is the "reverse" one.
/// \param mode  The key mode to be checked.
/// \returns `true` if the mode matches the "reverse" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_reverse(key_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_REVERSEKEY) != 0;
}

/// \brief Checks whether the given key mode is the "MessagePack" one.
/// \param mode  The key mode to be checked.
/// \returns `true` if the mode matches the "MessagePack" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_msgpack(key_mode mode) noexcept { return mode == key_mode::msgpack; }

/// \brief Kind of the values and sorted multi-values with corresponding comparison.
enum class value_mode {
  single = MDBX_DB_DEFAULTS, ///< Usual single value for each key. In terms of
                             ///< keys, they are unique.
  multi = MDBX_DUPSORT,      ///< A more than one data value could be associated with
                             ///< each key. Internally each key is stored once, and the
                             ///< corresponding data values are sorted by byte-by-byte
                             ///< lexicographic comparison like `std::memcmp()`.
                             ///< In terms of keys, they are not unique, i.e. has
                             ///< duplicates which are sorted by associated data values.
#if MDBX_CONSTEXPR_ENUM_FLAGS_OPERATIONS || defined(DOXYGEN)
  multi_reverse = MDBX_DUPSORT | MDBX_REVERSEDUP,  ///< A more than one data value could be associated with
                                                   ///< each key. Internally each key is stored once, and
                                                   ///< the corresponding data values are sorted by
                                                   ///< byte-by-byte lexicographic comparison in reverse
                                                   ///< order, from the end of the keys to the beginning.
                                                   ///< In terms of keys, they are not unique, i.e. has
                                                   ///< duplicates which are sorted by associated data
                                                   ///< values.
  multi_samelength = MDBX_DUPSORT | MDBX_DUPFIXED, ///< A more than one data value could be associated with
                                                   ///< each key, and all data values must be same length.
                                                   ///< Internally each key is stored once, and the
                                                   ///< corresponding data values are sorted by byte-by-byte
                                                   ///< lexicographic comparison like `std::memcmp()`. In
                                                   ///< terms of keys, they are not unique, i.e. has
                                                   ///< duplicates which are sorted by associated data values.
  multi_ordinal = MDBX_DUPSORT | MDBX_DUPFIXED | MDBX_INTEGERDUP, ///< A more than one data value could be associated
                                                                  ///< with each key, and all data values are binary
                                                                  ///< integers in native byte order, either `uint32_t`
                                                                  ///< or `uint64_t`, and will be sorted as such.
                                                                  ///< Internally each key is stored once, and the
                                                                  ///< corresponding data values are sorted. In terms of
                                                                  ///< keys, they are not unique, i.e. has duplicates
                                                                  ///< which are sorted by associated data values.
  multi_reverse_samelength =
      MDBX_DUPSORT | MDBX_REVERSEDUP | MDBX_DUPFIXED, ///< A more than one data value could be associated with
                                                      ///< each key, and all data values must be same length.
                                                      ///< Internally each key is stored once, and the
                                                      ///< corresponding data values are sorted by byte-by-byte
                                                      ///< lexicographic comparison in reverse order, from the
                                                      ///< end of the keys to the beginning. In terms of keys,
                                                      ///< they are not unique, i.e. has duplicates which are
                                                      ///< sorted by associated data values.
#else
  multi_reverse = uint32_t(MDBX_DUPSORT) | uint32_t(MDBX_REVERSEDUP),
  multi_samelength = uint32_t(MDBX_DUPSORT) | uint32_t(MDBX_DUPFIXED),
  multi_ordinal = uint32_t(MDBX_DUPSORT) | uint32_t(MDBX_DUPFIXED) | uint32_t(MDBX_INTEGERDUP),
  multi_reverse_samelength = uint32_t(MDBX_DUPSORT) | uint32_t(MDBX_REVERSEDUP) | uint32_t(MDBX_DUPFIXED),
#endif
  msgpack = -1 ///< A more than one data value could be associated with each
  ///< key. Values are in [MessagePack](https://msgpack.org/)
  ///< format with appropriate comparison. Internally each key is
  ///< stored once, and the corresponding data values are sorted.
  ///< In terms of keys, they are not unique, i.e. has duplicates
  ///< which are sorted by associated data values.
  ///< \note Not yet implemented and PRs are welcome.
};

/// \brief Checks whether the given value mode is the "usual" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "usual" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_usual(value_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & (MDBX_DUPSORT | MDBX_INTEGERDUP | MDBX_DUPFIXED | MDBX_REVERSEDUP)) == 0;
}

/// \brief Checks whether the given value mode is the "multi-value" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "multi-value" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_multi(value_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_DUPSORT) != 0;
}

/// \brief Checks whether the given value mode is the "ordinal" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "ordinal" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_ordinal(value_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_INTEGERDUP) != 0;
}

/// \brief Checks whether the given value mode is the "same-length" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "same-length" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_samelength(value_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_DUPFIXED) != 0;
}

/// \brief Checks whether the given value mode is the "reverse" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "reverse" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_reverse(value_mode mode) noexcept {
  return (MDBX_db_flags_t(mode) & MDBX_REVERSEDUP) != 0;
}

/// \brief Checks whether the given value mode is the "MessagePack" one.
/// \param mode  The value mode to be checked.
/// \returns `true` if the mode matches the "MessagePack" kind, `false` otherwise.
MDBX_CXX01_CONSTEXPR_ENUM bool is_msgpack(value_mode mode) noexcept { return mode == value_mode::msgpack; }

/// \brief A handle for an individual table (aka key-value space, maps or sub-database) in the environment.
/// \see txn::open_map() \see txn::create_map()
/// \see txn::clear_map() \see txn::drop_map()
/// \see txn::get_map_flags() \see txn::get_map_stat()
/// \see env::close_map()
/// \see cursor::map()
struct LIBMDBX_API_TYPE map_handle {
  MDBX_dbi dbi{0}; ///< \brief The table handle (DBI) number; zero for the main table.
  /// \brief Constructs an invalid (empty) handle.
  MDBX_CXX11_CONSTEXPR map_handle() noexcept {}
  /// \brief Constructs a handle from the given DBI number.
  /// \param dbi  The table handle (DBI) number.
  MDBX_CXX11_CONSTEXPR map_handle(MDBX_dbi dbi) noexcept : dbi(dbi) {}
  /// \brief Copy constructor.
  map_handle(const map_handle &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  map_handle &operator=(const map_handle &) noexcept = default;
  /// \brief Checks whether the handle is valid, i.e. not the main table.
  /// \returns `true` if the handle refers to a named table, `false` otherwise.
  operator bool() const noexcept { return dbi != 0; }
  /// \brief Returns the underlying DBI number.
  /// \returns The table handle (DBI) number.
  operator MDBX_dbi() const { return dbi; }

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
  friend MDBX_CXX11_CONSTEXPR auto operator<=>(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi <=> b.dbi;
  }
#endif /* __cpp_impl_three_way_comparison */
  friend MDBX_CXX14_CONSTEXPR bool operator==(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi == b.dbi;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator<(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi < b.dbi;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator>(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi > b.dbi;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator<=(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi <= b.dbi;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator>=(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi >= b.dbi;
  }
  friend MDBX_CXX14_CONSTEXPR bool operator!=(const map_handle &a, const map_handle &b) noexcept {
    return a.dbi != b.dbi;
  }

  /// \brief An alias for \ref MDBX_db_flags_t, i.e. the table flags.
  using flags = ::MDBX_db_flags_t;
  /// \brief An alias for \ref MDBX_dbi_state_t, i.e. the table state.
  using state = ::MDBX_dbi_state_t;
  /// \brief Flags and state of a table (aka map).
  struct LIBMDBX_API_TYPE info {
    map_handle::flags flags; ///< \brief The table flags, see \ref MDBX_db_flags_t.
    map_handle::state state; ///< \brief The table state, see \ref MDBX_dbi_state_t.
    /// \brief Constructs the info from the given flags and state.
    /// \param flags  The table flags.
    /// \param state  The table state.
    MDBX_CXX11_CONSTEXPR info(map_handle::flags flags, map_handle::state state) noexcept;
    /// \brief Copy constructor.
    info(const info &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    info &operator=(const info &) noexcept = default;
    /// \brief Returns the key comparison mode implied by the flags.
    /// \returns The \ref mdbx::key_mode of the table.
    MDBX_CXX11_CONSTEXPR_ENUM mdbx::key_mode key_mode() const noexcept;
    /// \brief Returns the value comparison mode implied by the flags.
    /// \returns The \ref mdbx::value_mode of the table.
    MDBX_CXX11_CONSTEXPR_ENUM mdbx::value_mode value_mode() const noexcept;
  };
};

/// \brief An alias for \ref MDBX_cmp_func, i.e. a comparison function pointer.
using comparator = ::MDBX_cmp_func;
/// \brief Returns the default internal key comparator for the given key mode.
/// \param mode  The key mode.
/// \returns The pointer to the default \ref comparator function.
inline comparator default_comparator(key_mode mode) noexcept {
  return ::mdbx_get_keycmp(static_cast<MDBX_db_flags_t>(mode));
}
/// \brief Returns the default internal value comparator for the given value mode.
/// \param mode  The value mode.
/// \returns The pointer to the default \ref comparator function.
inline comparator default_comparator(value_mode mode) noexcept {
  return ::mdbx_get_datacmp(static_cast<MDBX_db_flags_t>(mode));
}

/// \brief Key-value pairs put mode.
enum put_mode {
  insert_unique = MDBX_NOOVERWRITE, ///< Insert only unique keys.
  upsert = MDBX_UPSERT,             ///< Insert or update.
  update = MDBX_CURRENT,            ///< Update existing, don't insert new.
};

/// \brief Unmanaged database environment.
///
/// Like other unmanaged classes, `env` allows copying and assignment for handles as a values,
/// but does not manage nor destroys the represented underlying object from the own class destructor.
///
/// An environment supports multiple key-value tables (aka key-value maps,
/// tables or sub-databases), all residing in the same shared-memory mapped file.
class LIBMDBX_API_TYPE env {
  friend class txn;

protected:
  MDBX_env *handle_{nullptr}; ///< \brief The underlying C environment handle, null for an empty instance.
  /// \brief Constructs a non-owning environment wrapping the given C handle.
  /// \param ptr  The pointer to the underlying \ref MDBX_env object.
  MDBX_CXX11_CONSTEXPR env(MDBX_env *ptr) noexcept;

public:
  /// \brief Constructs an empty (null) environment.
  MDBX_CXX11_CONSTEXPR env() noexcept = default;
  /// \brief Copy constructor.
  env(const env &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  env &operator=(const env &) noexcept = default;
  /// \brief Move assignment operator.
  /// \param other  The source environment to be moved from.
  /// \returns A reference to this instance.
  inline env &operator=(env &&other) noexcept;
  /// \brief Move constructor: the source environment is nullified.
  /// \param other  The source environment to be moved from.
  inline env(env &&other) noexcept;
  /// \brief Destroys the wrapper; the underlying object is NOT closed
  /// since the environment is non-owning.
  inline ~env() noexcept;
  /* Slicing an owning env_managed temporary into the base handle would drop
   * ownership and never close the environment (issue #77), so these exact-match
   * overloads turn `mdbx::env e = mdbx::env_managed(...);` into a compile error,
   * while a non-owning view of an env_managed lvalue is still allowed. */
  env(env_managed &&) = delete;
  env &operator=(env_managed &&) = delete;

  /// \brief Checks whether the environment wraps a non-null handle.
  /// \returns `true` if the environment is not null.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept { return handle_ != nullptr; };
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_env object.
  MDBX_CXX14_CONSTEXPR operator const MDBX_env *() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_env object.
  MDBX_CXX14_CONSTEXPR operator MDBX_env *() noexcept { return handle_; }
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_env object.
  MDBX_CXX14_CONSTEXPR const MDBX_env *handle() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_env object.
  MDBX_CXX14_CONSTEXPR MDBX_env *handle() noexcept { return handle_; };
  friend MDBX_CXX11_CONSTEXPR bool operator==(const env &a, const env &b) noexcept { return a.handle_ == b.handle_; }
  friend MDBX_CXX11_CONSTEXPR bool operator!=(const env &a, const env &b) noexcept { return a.handle_ != b.handle_; }

  //----------------------------------------------------------------------------

  /// \brief Database geometry for size management.
  /// \see env_managed::create_parameters
  /// \see env_managed::env_managed(const ::std::string &pathname, const
  /// create_parameters &, const operate_parameters &, bool accede)

  struct LIBMDBX_API_TYPE geometry {
    enum : intptr_t {
      default_value = -1,         ///< Means "keep current or use default"
      minimal_value = 0,          ///< Means "minimal acceptable"
      maximal_value = INTPTR_MAX, ///< Means "maximal acceptable"
      kB = 1000,                  ///< \f$10^{3}\f$ bytes (0x03E8)
      MB = kB * 1000,             ///< \f$10^{6}\f$ bytes (0x000F_4240)
      GB = MB * 1000,             ///< \f$10^{9}\f$ bytes (0x3B9A_CA00)
#if INTPTR_MAX > 0x7fffFFFFl
      TB = GB * 1000,  ///< \f$10^{12}\f$ bytes (0x0000_00E8_D4A5_1000)
      PB = TB * 1000,  ///< \f$10^{15}\f$ bytes (0x0003_8D7E_A4C6_8000)
      EB = PB * 1000,  ///< \f$10^{18}\f$ bytes (0x0DE0_B6B3_A764_0000)
#endif                 /* 64-bit intptr_t */
      KiB = 1024,      ///< \f$2^{10}\f$ bytes (0x0400)
      MiB = KiB << 10, ///< \f$2^{20}\f$ bytes (0x0010_0000)
      GiB = MiB << 10, ///< \f$2^{30}\f$ bytes (0x4000_0000)
#if INTPTR_MAX > 0x7fffFFFFl
      TiB = GiB << 10, ///< \f$2^{40}\f$ bytes (0x0000_0100_0000_0000)
      PiB = TiB << 10, ///< \f$2^{50}\f$ bytes (0x0004_0000_0000_0000)
      EiB = PiB << 10, ///< \f$2^{60}\f$ bytes (0x1000_0000_0000_0000)
#endif                 /* 64-bit intptr_t */
    };

    /// \brief Tagged type for output to std::ostream
    struct size {
      intptr_t bytes; ///< \brief The size in bytes.
      /// \brief Constructs from the given number of bytes.
      /// \param bytes  The size in bytes.
      MDBX_CXX11_CONSTEXPR size(intptr_t bytes) noexcept : bytes(bytes) {}
      /// \brief Converts to the plain number of bytes.
      /// \returns The size in bytes.
      MDBX_CXX11_CONSTEXPR operator intptr_t() const noexcept { return bytes; }
    };

    /// \brief The lower bound of database size in bytes.
    intptr_t size_lower{default_value};

    /// \brief The size in bytes to setup the database size for now.
    /// \details It is recommended always pass \ref default_value in this
    /// argument except some special cases.
    intptr_t size_now{default_value};

    /// \brief The upper bound of database size in bytes.
    /// \details It is recommended to avoid change upper bound while database is
    /// used by other processes or threaded (i.e. just pass \ref default_value
    /// in this argument except absolutely necessary). Otherwise you must be
    /// ready for \ref MDBX_UNABLE_EXTEND_MAPSIZE error(s), unexpected pauses
    /// during remapping and/or system errors like "address busy", and so on. In
    /// other words, there is no way to handle a growth of the upper bound
    /// robustly because there may be a lack of appropriate system resources
    /// (which are extremely volatile in a multi-process multi-threaded
    /// environment).
    intptr_t size_upper{default_value};

    /// \brief The growth step in bytes, must be greater than zero to allow the database to grow.
    intptr_t growth_step{default_value};

    /// \brief The shrink threshold in bytes, must be greater than zero to allow the database to shrink.
    intptr_t shrink_threshold{default_value};

    /// \brief The database page size for new database creation
    /// or \ref default_value otherwise.
    /// \details Must be power of 2 in the range between \ref MDBX_MIN_PAGESIZE
    /// and \ref MDBX_MAX_PAGESIZE.
    intptr_t pagesize{default_value};

    /// \brief Makes a fixed-size geometry, i.e. with disabled growing and shrinking.
    /// \param size  The fixed database size in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &make_fixed(intptr_t size) noexcept;
    /// \brief Makes a dynamic-size geometry with the given bounds.
    /// \param lower  The lower bound of database size in bytes.
    /// \param upper  The upper bound of database size in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &make_dynamic(intptr_t lower = default_value,
                                                intptr_t upper = default_value) noexcept;

    /// \brief Sets the lower bound of database size in bytes.
    /// \param size  The lower bound in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_size_lower(intptr_t size) noexcept;
    /// \brief Sets the size in bytes to setup the database size for now.
    /// \param size  The size in bytes to setup for now.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_size_now(intptr_t size) noexcept;
    /// \brief Sets the upper bound of database size in bytes.
    /// \param size  The upper bound in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_size_upper(intptr_t size) noexcept;
    /// \brief Sets the growth step in bytes.
    /// \param step  The growth step in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_growth_step(intptr_t step) noexcept;
    /// \brief Sets the shrink threshold in bytes.
    /// \param threshold  The shrink threshold in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_shrink_threshold(intptr_t threshold) noexcept;
    /// \brief Sets the database page size for new database creation.
    /// \param size  The page size in bytes.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &set_pagesize(intptr_t size) noexcept;

    /// \brief Constructs a geometry with all values set to \ref default_value.
    MDBX_CXX11_CONSTEXPR geometry() noexcept {}
    /// \brief Copy constructor.
    MDBX_CXX11_CONSTEXPR
    geometry(const geometry &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR geometry &operator=(const geometry &) noexcept = default;

    /// \brief All `geometry::set_*` methods are noexcept by design: they only
    /// fill a value object, checking nothing and performing no C API calls. The
    /// resulting geometry is applied via \ref env::set_geometry(), which
    /// throws \ref mdbx::error on any invalid/inapplicable value (e.g. a size
    /// below the current file size or a late call after open), so configuration
    /// errors are never silently swallowed (issue #143).
    /// \param size_lower  The `size_lower` argument.
    /// \param size_now  The `size_now` argument.
    /// \param size_upper  The `size_upper` argument.
    /// \param growth_step  The `growth_step` argument.
    /// \param shrink_threshold  The `shrink_threshold` argument.
    /// \param pagesize  The database page size in bytes.
    MDBX_CXX11_CONSTEXPR geometry(intptr_t size_lower, intptr_t size_now = default_value,
                                  intptr_t size_upper = default_value, intptr_t growth_step = default_value,
                                  intptr_t shrink_threshold = default_value, intptr_t pagesize = default_value) noexcept
        : size_lower(size_lower), size_now(size_now), size_upper(size_upper), growth_step(growth_step),
          shrink_threshold(shrink_threshold), pagesize(pagesize) {}

    /// \brief Creates a fixed-size geometry, i.e. with disabled growing and shrinking.
    /// \param size  The new length of the data.
    /// \returns The result.
    static MDBX_CXX14_CONSTEXPR geometry fixed(intptr_t size) noexcept {
      geometry result;
      return result.make_fixed(size);
    }
    /// \brief Creates a dynamic-size geometry with the given bounds.
    /// \param lower  The `lower` argument.
    /// \param upper  The `upper` argument.
    /// \returns The result.
    static MDBX_CXX14_CONSTEXPR geometry dynamic(intptr_t lower = default_value,
                                                 intptr_t upper = default_value) noexcept {
      geometry result;
      return result.make_dynamic(lower, upper);
    }

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    friend auto operator<=>(const geometry &a, const geometry &b) = default;
#else
    MDBX_CXX14_CONSTEXPR bool operator==(const geometry &other) const noexcept {
      return size_lower == other.size_lower && size_now == other.size_now && size_upper == other.size_upper &&
             growth_step == other.growth_step && shrink_threshold == other.shrink_threshold &&
             pagesize == other.pagesize;
    }
    MDBX_CXX14_CONSTEXPR bool operator!=(const geometry &other) const noexcept { return !(*this == other); }
#endif
  };

  /// \brief Operation mode.
  enum class mode : unsigned {
    readonly,                           ///< \copydoc MDBX_RDONLY
    write_file_io,                      ///< Write via file I/O (not available on OpenBSD).
    write_mapped_io,                    ///< \copydoc MDBX_WRITEMAP
    nested_transactions = write_file_io ///< An alias for `write_file_io`, i.e. the mode
                                        ///< that allows nested transactions.
  };

  /// \brief Durability level.
  enum class durability : unsigned {
    robust_synchronous,         ///< \copydoc MDBX_SYNC_DURABLE
    half_synchronous_weak_last, ///< \copydoc MDBX_NOMETASYNC
    lazy_weak_tail,             ///< \copydoc MDBX_SAFE_NOSYNC
    whole_fragile               ///< \copydoc MDBX_UTTERLY_NOSYNC
  };

  /* Deprecated backward-compatibility aliases: before the mode/durability enums
   * became scoped (`enum class`), their enumerators leaked into `env::`.
   * They are kept as deprecated static constants so existing code compiles,
   * but new code should use the scoped form `env::mode::...`/`env::durability::...`. */
  [[deprecated("use mdbx::env::mode::readonly instead")]] static constexpr mode readonly = mode::readonly;
  [[deprecated("use mdbx::env::mode::write_file_io instead")]] static constexpr mode write_file_io =
      mode::write_file_io;
  [[deprecated("use mdbx::env::mode::write_mapped_io instead")]] static constexpr mode write_mapped_io =
      mode::write_mapped_io;
  [[deprecated("use mdbx::env::mode::nested_transactions instead")]] static constexpr mode nested_transactions =
      mode::nested_transactions;
  [[deprecated(
      "use mdbx::env::durability::robust_synchronous instead")]] static constexpr durability robust_synchronous =
      durability::robust_synchronous;
  [[deprecated("use mdbx::env::durability::half_synchronous_weak_last instead")]] static constexpr durability
      half_synchronous_weak_last = durability::half_synchronous_weak_last;
  [[deprecated("use mdbx::env::durability::lazy_weak_tail instead")]] static constexpr durability lazy_weak_tail =
      durability::lazy_weak_tail;
  [[deprecated("use mdbx::env::durability::whole_fragile instead")]] static constexpr durability whole_fragile =
      durability::whole_fragile;

  /// \brief Garbage reclaiming options.
  struct LIBMDBX_API_TYPE reclaiming_options {
    /// \copydoc MDBX_LIFORECLAIM
    bool lifo{false};
    /// \brief Constructs the default reclaiming options.
    MDBX_CXX11_CONSTEXPR reclaiming_options() noexcept {}
    /// \brief Copy constructor.
    MDBX_CXX11_CONSTEXPR
    reclaiming_options(const reclaiming_options &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR reclaiming_options &operator=(const reclaiming_options &) noexcept = default;
    /// \brief Sets the LIFO reclaiming mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR reclaiming_options &set_lifo(bool value = true) noexcept;

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    friend auto operator<=>(const reclaiming_options &a, const reclaiming_options &b) = default;
#else
    MDBX_CXX14_CONSTEXPR bool operator==(const reclaiming_options &other) const noexcept { return lifo == other.lifo; }
    MDBX_CXX14_CONSTEXPR bool operator!=(const reclaiming_options &other) const noexcept { return !(*this == other); }
#endif

    /// \brief Constructs from the environment flags, extracting the reclaiming bits.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    reclaiming_options(MDBX_env_flags_t flags) noexcept;
  };

  /// \brief Operate options.
  struct LIBMDBX_API_TYPE operate_options {
    /// \copydoc MDBX_NOSTICKYTHREADS
    bool no_sticky_threads{false};
    /// \brief Enables nested transactions at the cost of disabling
    /// \ref MDBX_WRITEMAP and increasing overhead.
    bool nested_transactions{false};
    /// \copydoc MDBX_EXCLUSIVE
    bool exclusive{false};
    /// \copydoc MDBX_NORDAHEAD
    bool disable_readahead{false};
    /// \copydoc MDBX_NOMEMINIT
    bool disable_clear_memory{false};
    /// \copydoc MDBX_VALIDATION
    bool enable_validation{false};
    /// \brief Constructs the default operate options.
    MDBX_CXX11_CONSTEXPR operate_options() noexcept {}
    /// \brief Copy constructor.
    MDBX_CXX11_CONSTEXPR
    operate_options(const operate_options &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_options &operator=(const operate_options &) noexcept = default;

    /// \brief Sets the "no sticky threads" mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_no_sticky_threads(bool value = true) noexcept;
    /// \brief Sets the nested transactions mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_nested_transactions(bool value = true) noexcept;
    /// \brief Sets the exclusive mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_exclusive(bool value = true) noexcept;
    /// \brief Sets the readahead disable mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_disable_readahead(bool value = true) noexcept;
    /// \brief Sets the clear-memory disable mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_disable_clear_memory(bool value = true) noexcept;
    /// \brief Sets the validation enable mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_options &set_enable_validation(bool value = true) noexcept;

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    friend auto operator<=>(const operate_options &a, const operate_options &b) = default;
#else
    MDBX_CXX14_CONSTEXPR bool operator==(const operate_options &other) const noexcept {
      return no_sticky_threads == other.no_sticky_threads && nested_transactions == other.nested_transactions &&
             exclusive == other.exclusive && disable_readahead == other.disable_readahead &&
             disable_clear_memory == other.disable_clear_memory && enable_validation == other.enable_validation;
    }
    MDBX_CXX14_CONSTEXPR bool operator!=(const operate_options &other) const noexcept { return !(*this == other); }
#endif

    /// \brief Constructs from the environment flags, extracting the option bits.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    operate_options(MDBX_env_flags_t flags) noexcept;
  };

  /// \brief Operate parameters.
  struct LIBMDBX_API_TYPE operate_parameters {
    /// \brief The maximum number of named tables/maps for the environment.
    /// Zero means default value.
    unsigned max_maps{0};
    /// \brief The maximum number of threads/reader slots for the environment.
    /// Zero means default value.
    unsigned max_readers{0};
    env::mode mode{env::mode::write_mapped_io}; ///< \brief The operation mode, see \ref env::mode.
    env::durability durability{
        env::durability::robust_synchronous}; ///< \brief The durability level, see \ref env::durability.
    env::reclaiming_options reclaiming;       ///< \brief The garbage reclaiming options.
    env::operate_options options;             ///< \brief The other operate options.

    /// \brief Constructs the default operate parameters.
    MDBX_CXX11_CONSTEXPR operate_parameters() noexcept {}
    /// \brief Constructs the operate parameters.
    /// \param max_maps    The maximum number of key-value maps (tables).
    /// \param max_readers The maximum number of concurrent readers.
    /// \param mode        The operation mode, see \ref env::mode.
    /// \param durability  The durability mode, see \ref env::durability.
    /// \param reclaiming  The reclaiming options, see \ref env::reclaiming_options.
    /// \param options     The other options, see \ref env::operate_options.
    MDBX_CXX11_CONSTEXPR
    operate_parameters(const unsigned max_maps, const unsigned max_readers = 0,
                       const env::mode mode = env::mode::write_mapped_io,
                       env::durability durability = env::durability::robust_synchronous,
                       const env::reclaiming_options &reclaiming = env::reclaiming_options(),
                       const env::operate_options &options = env::operate_options()) noexcept
        : max_maps(max_maps), max_readers(max_readers), mode(mode), durability(durability), reclaiming(reclaiming),
          options(options) {}
    /// \brief Copy constructor.
    MDBX_CXX11_CONSTEXPR
    operate_parameters(const operate_parameters &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &operator=(const operate_parameters &) noexcept = default;

    /// \brief Sets the maximum number of named tables/maps for the environment.
    /// Zero means default value.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_max_maps(unsigned value) noexcept;
    /// \brief Sets the maximum number of threads/reader slots for the environment.
    /// Zero means default value.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_max_readers(unsigned value) noexcept;
    /// \brief Sets the operation mode.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_mode(env::mode value) noexcept;
    /// \brief Sets the durability level.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_durability(env::durability value) noexcept;
    /// \brief Sets the garbage reclaiming options.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_reclaiming(const env::reclaiming_options &value) noexcept;
    /// \brief Sets the operate options.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &set_options(const env::operate_options &value) noexcept;

    /// \brief Sets the operation mode to \ref env::mode::readonly.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &readonly() noexcept;
    /// \brief Sets the operation mode to \ref env::mode::write_file_io.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &write_file_io() noexcept;
    /// \brief Sets the operation mode to \ref env::mode::write_mapped_io.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &write_mapped_io() noexcept;
    /// \brief Sets the durability level to \ref env::durability::robust_synchronous.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &robust_synchronous() noexcept;
    /// \brief Sets the durability level to \ref env::durability::half_synchronous_weak_last.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &half_synchronous_weak_last() noexcept;
    /// \brief Sets the durability level to \ref env::durability::lazy_weak_tail.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &lazy_weak_tail() noexcept;
    /// \brief Sets the durability level to \ref env::durability::whole_fragile.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR operate_parameters &whole_fragile() noexcept;
    /// \brief Sets the nested transactions mode, i.e. \ref operate_options::nested_transactions.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &nested_transactions(bool value = true) noexcept;
    /// \brief Sets the LIFO reclaiming mode, i.e. \ref reclaiming_options::lifo.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &lifo(bool value = true) noexcept;
    /// \brief Sets the "no sticky threads" mode, i.e. \ref operate_options::no_sticky_threads.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &no_sticky_threads(bool value = true) noexcept;
    /// \brief Sets the exclusive mode, i.e. \ref operate_options::exclusive.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &exclusive(bool value = true) noexcept;
    /// \brief Sets the readahead disable mode, i.e. \ref operate_options::disable_readahead.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &disable_readahead(bool value = true) noexcept;
    /// \brief Sets the clear-memory disable mode, i.e. \ref operate_options::disable_clear_memory.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &disable_clear_memory(bool value = true) noexcept;
    /// \brief Sets the validation enable mode, i.e. \ref operate_options::enable_validation.
    /// \param value  The value.
    /// \returns The result.
    MDBX_CXX14_CONSTEXPR operate_parameters &enable_validation(bool value = true) noexcept;

    /// \brief Constructs parameters for a read-only environment.
    /// \returns The constructed \ref operate_parameters instance.
    static MDBX_CXX14_CONSTEXPR operate_parameters read_only() noexcept;
    /// \brief Constructs parameters for a writable environment with
    /// \ref env::durability::robust_synchronous durability and \ref env::mode::write_mapped_io.
    /// \returns The constructed \ref operate_parameters instance.
    static MDBX_CXX14_CONSTEXPR operate_parameters safe_write() noexcept;
    /// \brief Constructs parameters for a writable environment with
    /// \ref env::durability::lazy_weak_tail durability and \ref env::mode::write_mapped_io.
    /// \returns The constructed \ref operate_parameters instance.
    static MDBX_CXX14_CONSTEXPR operate_parameters lazy_write() noexcept;
    /// \brief Constructs parameters for a writable environment with
    /// \ref env::durability::whole_fragile durability and \ref env::mode::write_mapped_io.
    /// \returns The constructed \ref operate_parameters instance.
    static MDBX_CXX14_CONSTEXPR operate_parameters fragile_write() noexcept;

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    friend auto operator<=>(const operate_parameters &a, const operate_parameters &b) = default;
#else
    MDBX_CXX14_CONSTEXPR bool operator==(const operate_parameters &other) const noexcept {
      return max_maps == other.max_maps && max_readers == other.max_readers && mode == other.mode &&
             durability == other.durability && reclaiming == other.reclaiming && options == other.options;
    }
    MDBX_CXX14_CONSTEXPR bool operator!=(const operate_parameters &other) const noexcept { return !(*this == other); }
#endif

    /// \brief Makes the \ref MDBX_env_flags_t combination from these parameters.
    /// \param accede           Allows accepting incompatible operating options
    ///                           in case the database is already being used by
    ///                           another process(es), \see MDBX_ACCEDE
    /// \param use_subdirectory Use a subdirectory to place the DB files.
    /// \returns The \ref MDBX_env_flags_t combination.
    MDBX_NODISCARD MDBX_env_flags_t make_flags(bool accede = true, bool use_subdirectory = false) const;
    /// \brief Extracts the operation mode from environment flags.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    /// \returns The corresponding \ref env::mode.
    MDBX_NODISCARD static env::mode mode_from_flags(MDBX_env_flags_t flags) noexcept;
    /// \brief Extracts the durability level from environment flags.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    /// \returns The corresponding \ref env::durability.
    MDBX_NODISCARD static env::durability durability_from_flags(MDBX_env_flags_t flags) noexcept;
    /// \brief Extracts the reclaiming options from environment flags.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    /// \returns The corresponding \ref env::reclaiming_options.
    MDBX_NODISCARD inline static env::reclaiming_options reclaiming_from_flags(MDBX_env_flags_t flags) noexcept;
    /// \brief Extracts the other options from environment flags.
    /// \param flags  The \ref MDBX_env_flags_t combination.
    /// \returns The corresponding \ref env::operate_options.
    MDBX_NODISCARD inline static env::operate_options options_from_flags(MDBX_env_flags_t flags) noexcept;
  };

  /// \brief Returns current operation parameters.
  /// \returns The \ref operate_parameters the environment was opened with.
  MDBX_NODISCARD inline env::operate_parameters get_operation_parameters() const;
  /// \brief Returns current operation mode.
  /// \returns The \ref env::mode of the environment.
  MDBX_NODISCARD inline env::mode get_mode() const;
  /// \brief Returns current durability mode.
  /// \returns The \ref env::durability of the environment.
  MDBX_NODISCARD inline env::durability get_durability() const;
  /// \brief Returns current reclaiming options.
  /// \returns The \ref reclaiming_options of the environment.
  MDBX_NODISCARD inline env::reclaiming_options get_reclaiming() const;
  /// \brief Returns current operate options.
  /// \returns The \ref operate_options of the environment.
  MDBX_NODISCARD inline env::operate_options get_options() const;

  /// \brief Returns `true` for a freshly created database,
  /// but `false` if at least one transaction was committed.
  /// \returns `true` if the database is pristine.
  bool is_pristine() const;

  /// \brief Checks whether the database is empty.
  /// \returns `true` if the database is empty.
  bool is_empty() const;

  /// \brief Returns default page size for current system/platform.
  /// \returns The default page size in bytes.
  static size_t default_pagesize() noexcept { return ::mdbx_default_pagesize(); }

  /// \brief Information about the system RAM, see \ref ::mdbx_get_sysraminfo().
  struct sysraminfo {
    intptr_t page_size{0};   ///< The size of a memory page in bytes.
    intptr_t total_pages{0}; ///< The number of all pages in the system.
    intptr_t avail_pages{0}; ///< The number of currently available pages.
  };
  /// \brief Returns information about the system RAM.
  /// \returns The \ref sysraminfo of the system.
  MDBX_NODISCARD static inline sysraminfo get_sysraminfo();

  /// \brief A namespace-like holder of static functions returning the various
  /// libmdbx limits, the C++ counterpart of the `mdbx_limits_*()` C API.
  struct limits {
    /// \brief Construction is prohibited; use the static methods only.
    limits() = delete;
    /// \brief Returns the minimal database page size in bytes.
    /// \returns The minimal page size in bytes.
    static inline size_t pagesize_min() noexcept;
    /// \brief Returns the maximal database page size in bytes.
    /// \returns The maximal page size in bytes.
    static inline size_t pagesize_max() noexcept;
    /// \brief Returns the minimal database size in bytes for specified page size.
    /// \param pagesize  The database page size in bytes.
    /// \returns The minimal database size in bytes for specified page size.
    static inline size_t dbsize_min(intptr_t pagesize);
    /// \brief Returns the maximal database size in bytes for specified page size.
    /// \param pagesize  The database page size in bytes.
    /// \returns The maximal database size in bytes for specified page size.
    static inline size_t dbsize_max(intptr_t pagesize);
    /// \brief Returns the minimal key size in bytes for specified table flags.
    /// \param flags  The table flags.
    /// \returns The minimal key size in bytes for specified table flags.
    static inline size_t key_min(MDBX_db_flags_t flags) noexcept;
    /// \brief Returns the minimal key size in bytes for specified keys mode.
    /// \param mode  The key comparison mode, see \ref mdbx::key_mode.
    /// \returns The minimal key size in bytes for specified keys mode.
    static inline size_t key_min(key_mode mode) noexcept;
    /// \brief Returns the maximal key size in bytes for specified page size and table flags.
    /// \param pagesize  The database page size in bytes.
    /// \param flags  The table flags.
    /// \returns The maximal key size in bytes for specified page size and table flags.
    static inline size_t key_max(intptr_t pagesize, MDBX_db_flags_t flags);
    /// \brief Returns the maximal key size in bytes for specified page size and keys mode.
    /// \param pagesize  The database page size in bytes.
    /// \param mode  The key comparison mode, see \ref mdbx::key_mode.
    /// \returns The maximal key size in bytes for specified page size and keys mode.
    static inline size_t key_max(intptr_t pagesize, key_mode mode);
    /// \brief Returns the maximal key size in bytes for given environment and table flags.
    /// \param env    The environment.
    /// \param flags  The table flags.
    /// \returns The maximal key size in bytes for given environment and table flags.
    static inline size_t key_max(const env &env, MDBX_db_flags_t flags);
    /// \brief Returns the maximal key size in bytes for given environment and keys mode.
    /// \param env   The environment.
    /// \param mode  The key comparison mode, see \ref mdbx::key_mode.
    /// \returns The maximal key size in bytes for given environment and keys mode.
    static inline size_t key_max(const env &env, key_mode mode);
    /// \brief Returns the minimal values size in bytes for specified table flags.
    /// \param flags  The table flags.
    /// \returns The minimal values size in bytes for specified table flags.
    static inline size_t value_min(MDBX_db_flags_t flags) noexcept;
    /// \brief Returns the minimal values size in bytes for specified values mode.
    /// \param mode  The value comparison mode, see \ref mdbx::value_mode.
    /// \returns The minimal values size in bytes for specified values mode.
    static inline size_t value_min(value_mode mode) noexcept;

    /// \brief Returns the maximal value size in bytes for specified page size and table flags.
    /// \param pagesize  The database page size in bytes.
    /// \param flags  The table flags.
    /// \returns The maximal value size in bytes for specified page size and table flags.
    static inline size_t value_max(intptr_t pagesize, MDBX_db_flags_t flags);
    /// \brief Returns the maximal value size in bytes for specified page size and values mode.
    /// \param pagesize  The database page size in bytes.
    /// \param mode      The value comparison mode, see \ref mdbx::value_mode.
    /// \returns The maximal value size in bytes for specified page size and values mode.
    static inline size_t value_max(intptr_t pagesize, value_mode mode);
    /// \brief Returns the maximal value size in bytes for given environment and table flags.
    /// \param env    The environment.
    /// \param flags  The table flags.
    /// \returns The maximal value size in bytes for given environment and table flags.
    static inline size_t value_max(const env &env, MDBX_db_flags_t flags);
    /// \brief Returns the maximal value size in bytes for given environment and values mode.
    /// \param env   The environment.
    /// \param mode  The value comparison mode, see \ref mdbx::value_mode.
    /// \returns The maximal value size in bytes for given environment and values mode.
    static inline size_t value_max(const env &env, value_mode mode);

    /// \brief Returns maximal size of key-value pair to fit in a single page
    /// for specified page size and table flags.
    /// \param pagesize  The database page size in bytes.
    /// \param flags  The table flags.
    /// \returns Maximal size of key-value pair to fit in a single page for specified page size and table flags.
    static inline size_t pairsize4page_max(intptr_t pagesize, MDBX_db_flags_t flags);
    /// \brief Returns maximal size of key-value pair to fit in a single page
    /// for specified page size and values mode.
    /// \param pagesize  The database page size in bytes.
    /// \param mode      The value comparison mode, see \ref mdbx::value_mode.
    /// \returns Maximal size of key-value pair to fit in a single page for specified page size and values mode.
    static inline size_t pairsize4page_max(intptr_t pagesize, value_mode mode);
    /// \brief Returns maximal size of key-value pair to fit in a single page
    /// for given environment and table flags.
    /// \param env    The environment.
    /// \param flags  The table flags.
    /// \returns Maximal size of key-value pair to fit in a single page for given environment and table flags.
    static inline size_t pairsize4page_max(const env &env, MDBX_db_flags_t flags);
    /// \brief Returns maximal size of key-value pair to fit in a single page
    /// for given environment and values mode.
    /// \param env   The environment.
    /// \param mode  The value comparison mode, see \ref mdbx::value_mode.
    /// \returns Maximal size of key-value pair to fit in a single page for given environment and values mode.
    static inline size_t pairsize4page_max(const env &env, value_mode mode);

    /// \brief Returns maximal data size in bytes to fit in a leaf-page or
    /// single large/overflow-page for specified size and table flags.
    /// \param pagesize  The database page size in bytes.
    /// \param flags  The table flags.
    /// \returns Maximal data size in bytes to fit in a leaf-page or single large/overflow-page for specified size and
    /// table flags.
    static inline size_t valsize4page_max(intptr_t pagesize, MDBX_db_flags_t flags);
    /// \brief Returns maximal data size in bytes to fit in a leaf-page or
    /// single large/overflow-page for specified page size and values mode.
    /// \param pagesize  The database page size in bytes.
    /// \param mode      The value comparison mode, see \ref mdbx::value_mode.
    /// \returns Maximal data size in bytes to fit in a leaf-page or single large/overflow-page for specified page size
    /// and values mode.
    static inline size_t valsize4page_max(intptr_t pagesize, value_mode mode);
    /// \brief Returns maximal data size in bytes to fit in a leaf-page or
    /// single large/overflow-page for given environment and table flags.
    /// \param env    The environment.
    /// \param flags  The table flags.
    /// \returns Maximal data size in bytes to fit in a leaf-page or single large/overflow-page for given environment
    /// and table flags.
    static inline size_t valsize4page_max(const env &env, MDBX_db_flags_t flags);
    /// \brief Returns maximal data size in bytes to fit in a leaf-page or
    /// single large/overflow-page for given environment and values mode.
    /// \param env   The environment.
    /// \param mode  The value comparison mode, see \ref mdbx::value_mode.
    /// \returns Maximal data size in bytes to fit in a leaf-page or single large/overflow-page for given environment
    /// and values mode.
    static inline size_t valsize4page_max(const env &env, value_mode mode);

    /// \brief Returns the maximal write transaction size (i.e. limit for
    /// summary volume of dirty pages) in bytes for specified page size.
    /// \param pagesize  The database page size in bytes.
    /// \returns The maximal write transaction size (i.e. limit for summary volume of dirty pages) in bytes for
    /// specified page size.
    static inline size_t transaction_size_max(intptr_t pagesize);

    /// \brief Returns the maximum opened map handles, aka DBI-handles.
    /// \returns The maximum number of map (DBI) handles.
    static inline size_t max_map_handles(void);
  };

  /// \brief Returns the minimal database size in bytes for the environment.
  /// \returns The minimal database size in bytes.
  size_t dbsize_min() const { return limits::dbsize_min(this->get_pagesize()); }
  /// \brief Returns the maximal database size in bytes for the environment.
  /// \returns The maximal database size in bytes.
  size_t dbsize_max() const { return limits::dbsize_max(this->get_pagesize()); }
  /// \brief Returns the minimal key size in bytes for specified keys mode.
  /// \param mode  The operation mode.
  /// \returns The minimal key size in bytes for specified keys mode.
  size_t key_min(key_mode mode) const noexcept { return limits::key_min(mode); }
  /// \brief Returns the maximal key size in bytes for specified keys mode.
  /// \param mode  The operation mode.
  /// \returns The maximal key size in bytes for specified keys mode.
  size_t key_max(key_mode mode) const { return limits::key_max(*this, mode); }
  /// \brief Returns the minimal value size in bytes for specified values mode.
  /// \param mode  The operation mode.
  /// \returns The minimal value size in bytes for specified values mode.
  size_t value_min(value_mode mode) const noexcept { return limits::value_min(mode); }
  /// \brief Returns the maximal value size in bytes for specified values mode.
  /// \param mode  The operation mode.
  /// \returns The maximal value size in bytes for specified values mode.
  size_t value_max(value_mode mode) const { return limits::value_max(*this, mode); }
  /// \brief Returns the maximal write transaction size (i.e. limit for summary
  /// volume of dirty pages) in bytes.
  /// \returns The maximal write transaction size in bytes.
  size_t transaction_size_max() const { return limits::transaction_size_max(this->get_pagesize()); }

  /// \brief Make a copy (backup) of an existing environment to the specified
  /// path.
  /// \param destination         The path to the copy to be made.
  /// \param compactify          Omit free space from the copy and renumber all pages sequentially.
  /// \param force_dynamic_size  Force the dynamic size (grow/shrink) of the copy.
  /// \returns A reference to this environment instance.
  /// \throws mdbx::error on failure.
#ifdef MDBX_STD_FILESYSTEM_PATH
  env &copy(const MDBX_STD_FILESYSTEM_PATH &destination, bool compactify, bool force_dynamic_size = false);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \brief Make a copy (backup) of an existing environment to the specified path.
  /// \param destination         The path to the copy to be made.
  /// \param compactify          Omit free space from the copy and renumber all pages sequentially.
  /// \param force_dynamic_size  Force the dynamic size (grow/shrink) of the copy.
  /// \returns A reference to this environment instance.
  env &copy(const ::std::wstring &destination, bool compactify, bool force_dynamic_size = false);
  /// \brief Make a copy (backup) of an existing environment to the specified path.
  /// \param destination         The path to the copy to be made.
  /// \param compactify          Omit free space from the copy and renumber all pages sequentially.
  /// \param force_dynamic_size  Force the dynamic size (grow/shrink) of the copy.
  /// \returns A reference to this environment instance.
  env &copy(const wchar_t *destination, bool compactify, bool force_dynamic_size = false);
#endif /* Windows */
  /// \brief Make a copy (backup) of an existing environment to the specified path.
  /// \param destination         The path to the copy to be made.
  /// \param compactify          Omit free space from the copy and renumber all pages sequentially.
  /// \param force_dynamic_size  Force the dynamic size (grow/shrink) of the copy.
  /// \returns A reference to this environment instance.
  env &copy(const ::std::string &destination, bool compactify, bool force_dynamic_size = false);
  /// \brief Make a copy (backup) of an existing environment to the specified path.
  /// \param destination         The path to the copy to be made.
  /// \param compactify          Omit free space from the copy and renumber all pages sequentially.
  /// \param force_dynamic_size  Force the dynamic size (grow/shrink) of the copy.
  /// \returns A reference to this environment instance.
  env &copy(const char *destination, bool compactify, bool force_dynamic_size = false);

  /// \brief Copy an environment to the specified file descriptor.
  /// \param fd  The file descriptor.
  /// \param compactify  The `compactify` argument.
  /// \param force_dynamic_size  The `force_dynamic_size` argument.
  /// \returns The environment instance.
  env &copy(filehandle fd, bool compactify, bool force_dynamic_size = false);

  /// \brief Deletion modes for \ref remove().
  enum remove_mode {
    /// \brief Just delete the environment's files and directory if any.
    /// \note On POSIX systems, processes already working with the database will
    /// continue to work without interference until it close the environment.
    /// \note On Windows, the behavior of `just_remove` is different
    /// because the system does not support deleting files that are currently
    /// memory mapped.
    just_remove = MDBX_ENV_JUST_DELETE,
    /// \brief Make sure that the environment is not being used by other
    /// processes, or return an error otherwise.
    ensure_unused = MDBX_ENV_ENSURE_UNUSED,
    /// \brief Wait until other processes closes the environment before deletion.
    wait_for_unused = MDBX_ENV_WAIT_FOR_UNUSED
  };

  /// \brief Removes the environment's files in a proper and multiprocess-safe way.
  /// \param pathname  The path to the database.
  /// \param mode  The operation mode.
  /// \returns `true` on success, `false` otherwise.
#ifdef MDBX_STD_FILESYSTEM_PATH
  static bool remove(const MDBX_STD_FILESYSTEM_PATH &pathname, const remove_mode mode = just_remove);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \brief Removes the environment's files in a proper and multiprocess-safe way.
  /// \param pathname  The path to the environment.
  /// \param mode      The removal mode, see \ref remove_mode.
  /// \returns `true` on success, `false` otherwise.
  static bool remove(const ::std::wstring &pathname, const remove_mode mode = just_remove);
  /// \brief Removes the environment's files in a proper and multiprocess-safe way.
  /// \param pathname  The path to the environment.
  /// \param mode      The removal mode, see \ref remove_mode.
  /// \returns `true` on success, `false` otherwise.
  static bool remove(const wchar_t *pathname, const remove_mode mode = just_remove);
#endif /* Windows */
  /// \brief Removes the environment's files in a proper and multiprocess-safe way.
  /// \param pathname  The path to the environment.
  /// \param mode      The removal mode, see \ref remove_mode.
  /// \returns `true` on success, `false` otherwise.
  static bool remove(const ::std::string &pathname, const remove_mode mode = just_remove);
  /// \brief Removes the environment's files in a proper and multiprocess-safe way.
  /// \param pathname  The path to the environment.
  /// \param mode      The removal mode, see \ref remove_mode.
  /// \returns `true` on success, `false` otherwise.
  static bool remove(const char *pathname, const remove_mode mode = just_remove);

  /// \brief Statistics for a database in the MDBX environment.
  using stat = ::MDBX_stat;

  /// \brief Information about the environment.
  using info = ::MDBX_envinfo;

  /// \brief Provides information about a database, including meta-page and
  /// geometry, without opening it.
  ///
  /// \param [in] pathname The path to the directory or database file.
  /// \param [in] mode The mode of processing weak checkpoints, see \ref MDBX_preopen_flags.
  ///   Defaults to `MDBX_PREOPEN_PROPERLY` (same as the legacy `get_preopen_snapinfo()`).
  /// \returns The \ref info of the database. Errors are thrown as
  /// \ref mdbx::error.
  /// \see ::mdbx_preopen_snapinfo_ex()
  MDBX_NODISCARD static inline info get_preopen_snapinfo(const ::std::string &pathname,
                                                         MDBX_preopen_flags mode = MDBX_PREOPEN_PROPERLY);
  /// \brief Provides information about a database, including meta-page and
  /// geometry, without opening it.
  ///
  /// \param [in] pathname The path to the directory or database file.
  /// \param [in] mode The mode of processing weak checkpoints, see \ref MDBX_preopen_flags.
  ///   Defaults to `MDBX_PREOPEN_PROPERLY` (same as the legacy `get_preopen_snapinfo()`).
  /// \returns The \ref info of the database. Errors are thrown as
  /// \ref mdbx::error.
  /// \see ::mdbx_preopen_snapinfo_ex()
  MDBX_NODISCARD static inline info get_preopen_snapinfo(const char *pathname,
                                                         MDBX_preopen_flags mode = MDBX_PREOPEN_PROPERLY);
#ifdef MDBX_STD_FILESYSTEM_PATH
  /// \copydoc get_preopen_snapinfo(const char *, MDBX_preopen_flags)
  MDBX_NODISCARD static inline info get_preopen_snapinfo(const MDBX_STD_FILESYSTEM_PATH &pathname,
                                                         MDBX_preopen_flags mode = MDBX_PREOPEN_PROPERLY);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \copydoc get_preopen_snapinfo(const char *, MDBX_preopen_flags)
  MDBX_NODISCARD static inline info get_preopen_snapinfo(const ::std::wstring &pathname,
                                                         MDBX_preopen_flags mode = MDBX_PREOPEN_PROPERLY);
  /// \copydoc get_preopen_snapinfo(const char *, MDBX_preopen_flags)
  MDBX_NODISCARD static inline info get_preopen_snapinfo(const wchar_t *pathname,
                                                         MDBX_preopen_flags mode = MDBX_PREOPEN_PROPERLY);
#endif /* Windows */

  /// \brief Returns snapshot statistics about the MDBX environment.
  /// \returns The \ref stat snapshot of the environment.
  inline stat get_stat() const;

  /// \brief Returns pagesize of this MDBX environment.
  /// \returns The database page size in bytes.
  size_t get_pagesize() const { return get_info().mi_dxb_pagesize; }

  /// \brief Return snapshot information about the MDBX environment.
  /// \returns The \ref info snapshot of the environment.
  inline info get_info() const;

  /// \brief Return statistics about the MDBX environment accordingly to the specified transaction.
  /// \param txn  The transaction for the snapshot.
  /// \returns The \ref stat accordingly to the given transaction.
  inline stat get_stat(const txn &txn) const;

  /// \brief Return information about the MDBX environment accordingly to the specified transaction.
  /// \param txn  The transaction for the snapshot.
  /// \returns The \ref info accordingly to the given transaction.
  inline info get_info(const txn &txn) const;

  /// \brief Returns the file descriptor for the DXB file of MDBX environment.
  /// \returns The \ref filehandle of the DXB file.
  inline filehandle get_filehandle() const;

  /// \brief Return the path that was used for opening the environment.
  /// \returns The path used for opening, or `nullptr` for an empty instance.
  const path_char *get_path() const;

  /// \brief Returns environment flags.
  /// \returns The \ref MDBX_env_flags_t combination.
  inline MDBX_env_flags_t get_flags() const;

  /// \brief Checks whether the environment is read-only.
  /// \returns `true` if the environment is read-only.
  inline bool is_readonly() const { return (get_flags() & MDBX_RDONLY) != 0; }

  /// \brief Checks whether the environment is exclusive.
  /// \returns `true` if the environment was opened in the exclusive mode.
  inline bool is_exclusive() const { return (get_flags() & MDBX_EXCLUSIVE) != 0; }

  /// \brief Checks whether the environment is cooperative, i.e. not exclusive.
  /// \returns `true` if the environment can be used concurrently by processes.
  inline bool is_cooperative() const { return !is_exclusive(); }

  /// \brief Checks whether the environment uses memory-mapped I/O for writes.
  /// \returns `true` if \ref MDBX_WRITEMAP was used.
  inline bool is_writemap() const { return (get_flags() & MDBX_WRITEMAP) != 0; }

  /// \brief Checks whether the environment is read-write.
  /// \returns `true` if the environment is read-write.
  inline bool is_readwrite() const { return !is_readonly(); }

  /// \brief Checks whether nested transactions are available.
  /// \returns `true` if nested transactions can be used.
  inline bool is_nested_transactions_available() const { return (get_flags() & (MDBX_WRITEMAP | MDBX_RDONLY)) == 0; }

  /// \brief Returns the maximum number of threads/reader slots for the environment.
  /// \returns The maximum number of readers.
  /// \see extra_runtime_option::max_readers
  inline unsigned max_readers() const;

  /// \brief Returns the maximum number of named tables for the environment.
  /// \returns The maximum number of named tables.
  /// \see extra_runtime_option::max_maps
  inline unsigned max_maps() const;

  /// \brief Returns the application context associated with the environment.
  /// \returns The application context pointer.
  inline void *get_context() const noexcept;

  /// \brief Sets the application context associated with the environment.
  /// \param your_context  The application context pointer.
  /// \returns The environment instance.
  inline env &set_context(void *your_context);

  /// \brief Sets threshold to force flush the data buffers to disk, for
  /// non-sync durability modes.
  ///
  /// \details The threshold value affects all processes which operates with
  /// given environment until the last process close environment or a new value
  /// will be settled. Data is always written to disk when \ref
  /// txn_managed::commit() is called, but the operating system may keep it
  /// buffered. MDBX always flushes the OS buffers upon commit as well, unless
  /// the environment was opened with \ref env::durability::whole_fragile,
  /// \ref env::durability::lazy_weak_tail or in part
  /// \ref env::durability::half_synchronous_weak_last.
  ///
  /// The default is 0, which means that no threshold is checked and no
  /// additional flush will be made.
  /// \see extra_runtime_option::sync_bytes
  /// \param bytes  The number of bytes of the source data.
  /// \returns The environment instance.
  inline env &set_sync_threshold(size_t bytes);

  /// \brief Gets threshold used to force flush the data buffers to disk, for
  /// non-sync durability modes.
  /// \details See \ref set_sync_threshold() for the description of the threshold.
  /// \returns The threshold in bytes.
  /// \see extra_runtime_option::sync_bytes
  inline size_t sync_threshold() const;

#if __cplusplus >= 201103L || defined(DOXYGEN)
  /// \brief Sets relative period since the last unsteady commit to force flush
  /// the data buffers to disk, for non-sync durability modes.
  ///
  /// \details The relative period value affects all processes which operates
  /// with given environment until the last process close environment or a new
  /// value will be settled. Data is always written to disk when \ref
  /// txn_managed::commit() is called, but the operating system may keep it
  /// buffered. MDBX always flushes the OS buffers upon commit as well, unless
  /// the environment was opened with \ref env::durability::whole_fragile,
  /// \ref env::durability::lazy_weak_tail or in part
  /// \ref env::durability::half_synchronous_weak_last. The settled period is not checked
  /// asynchronously, but only by the \ref txn_managed::commit() and \ref
  /// env::sync_to_disk() functions. Therefore, in cases where transactions are
  /// committed infrequently and/or irregularly, polling by \ref
  /// env::poll_sync_to_disk() may be a reasonable solution to timeout
  /// enforcement.
  ///
  /// The default is 0, which means that no timeout is checked and no additional
  /// flush will be made.
  /// \see extra_runtime_option::sync_period
  /// \param period  The period in seconds.
  /// \returns The environment instance.
  inline env &set_sync_period(const duration &period);

  /// \brief Gets relative period since the last unsteady commit that used to
  /// force flush the data buffers to disk, for non-sync durability modes.
  /// \details See \ref set_sync_period() for the description of the period.
  /// \returns The period as a \ref duration value.
  /// \see set_sync_period(const duration&)
  /// \see extra_runtime_option::sync_period
  inline duration sync_period() const;
#endif

  /// \brief Sets the flush period in 1/65536 of second units; the same as
  /// \ref set_sync_period() with the corresponding \ref duration.
  /// \param seconds_16dot16  The period in 1/65536 of second when a
  /// synchronous flush would be made since the last unsteady commit.
  /// \returns A reference to this environment instance.
  inline env &set_sync_period__seconds_16dot16(unsigned seconds_16dot16);

  /// \brief Gets the flush period in 1/65536 of second units.
  /// \returns The period in 1/65536 of second units.
  /// \see sync_period__seconds_16dot16(unsigned)
  inline unsigned sync_period__seconds_16dot16() const;

  /// \brief Sets the flush period in seconds; the same as
  /// \ref set_sync_period() with the corresponding \ref duration.
  /// \param seconds  The period in second when a synchronous flush would
  /// be made since the last unsteady commit.
  /// \returns A reference to this environment instance.
  inline env &set_sync_period__seconds_double(double seconds);

  /// \brief Gets the flush period in seconds.
  /// \returns The period in seconds.
  /// \see set_sync_period__seconds_double(double)
  inline double sync_period__seconds_double() const;

  /// \copydoc MDBX_option_t
  enum class extra_runtime_option {
    /// \copydoc MDBX_opt_max_db
    /// \see max_maps() \see env::operate_parameters::max_maps
    max_maps = MDBX_opt_max_db,
    /// \copydoc MDBX_opt_max_readers
    /// \see max_readers() \see env::operate_parameters::max_readers
    max_readers = MDBX_opt_max_readers,
    /// \copydoc MDBX_opt_sync_bytes
    /// \see sync_threshold() \see set_sync_threshold()
    sync_bytes = MDBX_opt_sync_bytes,
    /// \copydoc MDBX_opt_sync_period
    /// \see sync_period() \see set_sync_period()
    sync_period = MDBX_opt_sync_period,
    /// \copydoc MDBX_opt_rp_augment_limit
    rp_augment_limit = MDBX_opt_rp_augment_limit,
    /// \copydoc MDBX_opt_loose_limit
    loose_limit = MDBX_opt_loose_limit,
    /// \copydoc MDBX_opt_dp_reserve_limit
    dp_reserve_limit = MDBX_opt_dp_reserve_limit,
    /// \copydoc MDBX_opt_txn_dp_limit
    dp_limit = MDBX_opt_txn_dp_limit,
    /// \copydoc MDBX_opt_txn_dp_initial
    dp_initial = MDBX_opt_txn_dp_initial,
    /// \copydoc MDBX_opt_spill_max_denominator
    spill_max_denominator = MDBX_opt_spill_max_denominator,
    /// \copydoc MDBX_opt_spill_min_denominator
    spill_min_denominator = MDBX_opt_spill_min_denominator,
    /// \copydoc MDBX_opt_spill_parent4child_denominator
    spill_parent4child_denominator = MDBX_opt_spill_parent4child_denominator,
    /// \copydoc MDBX_opt_merge_threshold
    merge_threshold_dot16 = MDBX_opt_merge_threshold,
    /// \copydoc MDBX_opt_writethrough_threshold
    writethrough_threshold = MDBX_opt_writethrough_threshold,
    /// \copydoc MDBX_opt_prefault_write_enable
    prefault_write_enable = MDBX_opt_prefault_write_enable,
    /// \copydoc MDBX_opt_gc_time_limit
    gc_time_limit = MDBX_opt_gc_time_limit,
    /// \copydoc MDBX_opt_prefer_waf_insteadof_balance
    prefer_waf_insteadof_balance = MDBX_opt_prefer_waf_insteadof_balance,
    /// \copydoc MDBX_opt_subpage_limit
    subpage_limit = MDBX_opt_subpage_limit,
    /// \copydoc MDBX_opt_subpage_room_threshold
    subpage_room_threshold = MDBX_opt_subpage_room_threshold,
    /// \copydoc MDBX_opt_subpage_reserve_prereq
    subpage_reserve_prereq = MDBX_opt_subpage_reserve_prereq,
    /// \copydoc MDBX_opt_subpage_reserve_limit
    subpage_reserve_limit = MDBX_opt_subpage_reserve_limit,
    /// \copydoc MDBX_opt_split_reserve
    split_reserve = MDBX_opt_split_reserve,
    /// \copydoc MDBX_opt_presync_threshold
    presync_threshold = MDBX_opt_presync_threshold,
    /// \copydoc MDBX_opt_shrink_retry_period
    shrink_retry_period = MDBX_opt_shrink_retry_period
  };

  /// \copybrief mdbx_env_set_option()
  /// \param option  The `option` argument.
  /// \param value  The value.
  /// \returns The environment instance.
  inline env &set_extra_option(extra_runtime_option option, uint64_t value);

  /// \copybrief mdbx_env_get_option()
  /// \param option  The `option` argument.
  /// \returns The resulting value.
  inline uint64_t extra_option(extra_runtime_option option) const;

  /// \brief Alter environment flags.
  /// \param flags  The flags.
  /// \param on_off  The `on_off` argument.
  /// \returns The environment instance.
  inline env &alter_flags(MDBX_env_flags_t flags, bool on_off);

  /// \brief Set all size-related parameters of environment.
  /// \throws mdbx::error on invalid or inapplicable values (e.g. a size below
  /// the current file size or a late call after the environment was opened).
  /// \param size  The new length of the data.
  /// \returns The environment instance.
  inline env &set_geometry(const geometry &size);

  /// \brief Flush the environment data buffers.
  /// \return `True` if sync done or no data to sync; `false` if the
  /// environment is busy by another thread and `nonblock=true` is used.
  /// \param force  Force the operation even when it is not required.
  /// \param nonblock  Don't wait if a write transaction is running by another thread.
  inline bool sync_to_disk(bool force = true, bool nonblock = false);

  /// \brief Performs non-blocking polling of sync-to-disk thresholds.
  /// \return `True` if sync done or no data to sync; `false` if the
  /// environment is busy by another thread.
  bool poll_sync_to_disk() { return sync_to_disk(false, true); }

  /// \brief Close a key-value map (aka table) handle. Normally
  /// unnecessary.
  ///
  /// Closing a table handle is not necessary, but lets \ref txn::open_map()
  /// reuse the handle value. Usually it's better to set a bigger
  /// \ref env::operate_parameters::max_maps, unless that value would be
  /// large.
  ///
  /// \note Use with care.
  /// This call is synchronized via mutex with other calls \ref close_map(), but
  /// NOT with other transactions running by other threads. The "next" version
  /// of libmdbx ([MithrilDB](https://libmdbx.dqdkfa.ru/docs/en/overview/index.html#roadmap-and-the-future))
  /// will solve this issue.
  ///
  /// Handles should only be closed if no other threads are going to reference
  /// the table handle or one of its cursors any further. Do not close a
  /// handle if an existing transaction has modified its table. Doing so can
  /// cause misbehavior from database corruption to errors like
  /// \ref MDBX_BAD_DBI (since the DB name is gone).
  /// \param map  The table (map) handle to be closed.
  inline void close_map(const map_handle &map);

  /// \brief Reader information
  struct reader_info {
    int slot;                 ///< The reader lock table slot number.
    mdbx_pid_t pid;           ///< The reader process ID.
    mdbx_tid_t thread;        ///< The reader thread ID.
    uint64_t transaction_id;  ///< The ID of the transaction being read,
                              ///< i.e. the MVCC-snapshot number.
    uint64_t transaction_lag; ///< The lag from a recent MVCC-snapshot,
                              ///< i.e. the number of committed write
                              /// transactions since the current read
                              /// transaction started.
    size_t bytes_used;        ///< The number of last used page in the MVCC-snapshot
                              ///< which being read, i.e. database file can't be shrunk
                              ///< beyond this.
    size_t bytes_retained;    ///< The total size of the database pages that
                              ///< were retired by committed write transactions
                              ///< after the reader's MVCC-snapshot, i.e. the space
                              ///< which would be freed after the Reader releases
                              ///< the MVCC-snapshot for reuse by completion read
                              ///< transaction.

    /// \brief Constructs the reader information.
    /// \param slot      The reader lock table slot number.
    /// \param pid       The reader process ID.
    /// \param thread    The reader thread ID.
    /// \param txnid     The ID of the transaction being read.
    /// \param lag       The lag from a recent MVCC-snapshot.
    /// \param used      The number of last used page in the MVCC-snapshot.
    /// \param retained  The total size of retained pages in bytes.
    MDBX_CXX11_CONSTEXPR reader_info(int slot, mdbx_pid_t pid, mdbx_tid_t thread, uint64_t txnid, uint64_t lag,
                                     size_t used, size_t retained) noexcept;
  };

  /// \brief Enumerate readers.
  ///
  /// The VISITOR class must have `int operator()(const reader_info&, int serial)`
  /// which should return `continue_loop` (zero) to continue enumeration,
  /// or any non-zero value to exit.
  ///
  /// \param visitor  The visitor functor, see the description above.
  /// \returns The last value returned from visitor' functor.
  template <typename VISITOR> inline int enumerate_readers(VISITOR &visitor);

  /// \brief Checks for stale readers in the lock table and
  /// return number of cleared slots.
  /// \returns The number of cleared stale reader slots.
  inline unsigned check_readers();

  /// \brief Registers the current thread as a reader for the environment,
  /// i.e. assigns a reader slot.
  /// \returns `false` if the thread was already registered.
  inline bool thread_register();
  /// \brief Unregisters the current thread, i.e. releases its reader slot.
  /// \returns `false` if the thread was not registered.
  inline bool thread_unregister();

  /// \brief Loads into memory the given pages of the database and their
  /// on-disk neighbours in advance.
  ///
  /// \returns `MDBX_RESULT_TRUE` if the specified timeout is reached during
  /// loading data into memory, `MDBX_SUCCESS` on success.
  /// \see ::mdbx_env_warmup()
  /// \param flags  The flags.
  /// \param timeout_seconds_16dot16  The timeout in 1/65536 of second units.
  /// \param txn  The transaction handle.
  MDBX_NODISCARD inline int warmup(MDBX_warmup_flags_t flags, unsigned timeout_seconds_16dot16,
                                   const MDBX_txn *txn = nullptr) const;

  /// \brief Restores the environment after `fork()`, dropping all read-write
  /// locks and reader slots of the parent process.
#if !defined(_WIN32) && !defined(_WIN64) || defined(DOXYGEN)
  inline void resurrect_after_fork();
#endif /* !Windows */

  /// \brief Turns the database to the specified meta-page.
  ///
  /// \details Mostly an internal API for the `mdbx_chk` utility and subject to
  /// change; use only when you know what you are doing. See
  /// \ref env_managed::open_for_recovery().
  /// \see ::mdbx_env_turn_for_recovery()
  /// \param target_meta  The index of the meta-page.
  inline void turn_for_recovery(unsigned target_meta);

  /// \brief Checks the integrity of the environment (database).
  ///
  /// \details Performs a full consistency check of the database with
  /// interactive callbacks, see \ref ::mdbx_env_chk(). The caller provides an
  /// \ref MDBX_chk_context_t context where the results of the check are
  /// generated (`ctx.result`), and optionally a set of callbacks (`cb`,
  /// all fields may be `nullptr`).
  ///
  /// \note The API is not frozen yet and may be improved in future versions.
  ///
  /// \param [in,out] ctx  A zero-initialized context of the check, where the
  ///   results are generated. `ctx.result.tables` is valid only inside the
  ///   callbacks and is `nullptr` once the check returns.
  /// \param [in] cb   A set of callback functions, all optional. `nullptr`
  ///   means no callbacks.
  /// \param [in] flags  Flags/options of the check, \ref MDBX_chk_flags_t.
  /// \param [in] verbosity  The required detail level of progress and results.
  /// \param [in] timeout_seconds_16dot16  Duration limit of the check in
  ///   1/65536 fractions of a second, zero means no limit.
  ///
  /// \returns The reference to the `ctx` for chaining.
  /// \throws mdbx::error on failure.
  /// \see ::mdbx_env_chk()
  MDBX_NODISCARD inline MDBX_chk_context_t &chk(MDBX_chk_context_t &ctx, const MDBX_chk_callbacks_t *cb = nullptr,
                                                MDBX_chk_flags_t flags = MDBX_CHK_DEFAULTS,
                                                MDBX_chk_severity_t verbosity = MDBX_chk_info,
                                                unsigned timeout_seconds_16dot16 = 0) const;

  /// \brief Sets a Handle-Slow-Readers callback to resolve database
  /// full/overflow issue due to a reader(s) which prevents the old data from
  /// being recycled.
  ///
  /// Such callback will be triggered in a case where there is not enough free
  /// space in the database due to long read transaction(s) which impedes
  /// reusing the pages of an old MVCC snapshot(s).
  ///
  /// Using this callback you can choose how to resolve the situation:
  ///  - abort the write transaction with an error;
  ///  - wait for the read transaction(s) to complete;
  ///  - notify a thread performing a long-lived read transaction
  ///    and wait for an effect;
  ///  - kill the thread or whole process that performs the long-lived read
  ///    transaction;
  ///
  /// \see long-lived-read
  /// \param cb  The callback function, or `nullptr` to disable.
  /// \returns A reference to this environment instance.
  inline env &set_HandleSlowReaders(MDBX_hsr_func cb);

  /// \brief Returns the current Handle-Slow-Readers callback used to resolve
  /// database full/overflow issue due to a reader(s) which prevents the old
  /// data from being recycled.
  /// \returns The current \ref MDBX_hsr_func callback, or `nullptr`.
  /// \see set_HandleSlowReaders()
  inline MDBX_hsr_func get_HandleSlowReaders() const noexcept;

  /// \brief Starts read (read-only) transaction.
  /// \returns The started managed read transaction.
  /// \throws mdbx::error on failure.
  inline txn_managed start_read() const;

  /// \brief Creates but not start read transaction.
  /// \returns The created (not started) managed read transaction.
  /// \throws mdbx::error on failure.
  inline txn_managed prepare_read() const;

  /// \brief Starts write (read-write) transaction.
  /// \param parent  The parent transaction.
  /// \returns The managed transaction instance.
  inline txn_managed start_write(txn &parent);

  /// \brief Starts write (read-write) transaction.
  /// \param dont_wait  Don't wait if the resource is locked by another thread.
  /// \returns The managed transaction instance.
  inline txn_managed start_write(bool dont_wait = false);

  /// \brief Tries to start write (read-write) transaction without blocking.
  /// \returns The started managed write transaction.
  /// \throws mdbx::error on failure, including \ref MDBX_BUSY when the write
  /// lock is held by another thread.
  inline txn_managed try_start_write();

  /// \brief Acquires write-transaction lock.
  ///
  /// \details Provided for custom and/or complex locking scenarios, see
  /// \ref ::mdbx_txn_lock(). Callers MUST NOT hold any write transaction while
  /// holding the lock.
  ///
  /// \param [in] dont_wait  If true, does not block and returns `false`
  /// immediately when the lock is held by another thread or process.
  /// \returns `True` if the lock was acquired, `false` otherwise.
  /// \see txn_unlock() \see ::mdbx_txn_lock()
  inline bool txn_lock(bool dont_wait = false);

  /// \brief Releases write-transaction lock acquired by \ref txn_lock().
  /// \see txn_lock() \see ::mdbx_txn_unlock()
  inline void txn_unlock();

  /// \brief The result of database defragmentation, see \ref ::MDBX_defrag_result_t.
  using defrag_result = ::MDBX_defrag_result_t;

  /// \brief Control values returned by the defragmentation progress visitor.
  /// \see defrag()
  enum class defrag_control : int {
    proceed = 0,     ///< Continue defragmentation.
    abort = -1,      ///< Abort defragmentation immediately.
    discontinue = 1, ///< Discontinue with completion of scheduled operations.
  };

  /// \brief Performs database defragmentation.
  ///
  /// \details Defragmentation is the transfer of data from pages located at
  /// the end of the database to free pages closer to the beginning, so the
  /// freed tail can be cut off while reducing the size of the database file,
  /// see \ref ::mdbx_env_defrag(). It is almost always performed in several
  /// cycles, each of which ends with committing an internal transaction.
  ///
  /// \note No transactions nor cursors must be open while defragmenting.
  ///
  /// The `visitor` functor (if provided) is called time-to-time with a
  /// reference to the current \ref defrag_result and must return
  /// \ref defrag_control::proceed to continue, \ref defrag_control::abort to
  /// abort immediately, or \ref defrag_control::discontinue to stop after
  /// completing scheduled operations.
  ///
  /// \returns The final \ref defrag_result. Stopping reasons are reported via
  /// its `stopping_reasons` field and are not treated as errors (the same for
  /// \ref MDBX_RESULT_TRUE and \ref MDBX_LAGGARD_READER).
  ///
  /// \param [in,out] visitor  An optional functor with the signature
  /// `defrag_control visitor(const defrag_result &progress)`.
  /// \param [in] defrag_atleast  The required at least number of pages by
  /// which the database must be reduced, zero means no lower bound.
  /// \param [in] time_atleast_dot16  The minimum time in 1/65536 fractions of
  /// a second that should be spent to defragment more even if goals reached,
  /// zero means no lower bound.
  /// \param [in] defrag_enough  The number of pages by which it will be enough
  /// to shrink the database to finish, zero means no limit.
  /// \param [in] time_limit_dot16  The time limit in 1/65536 fractions of a
  /// second that could be spent to defragment, zero means no limit.
  /// \param [in] acceptable_backlash  Stop if the next cycle will be unable to
  /// shrink the database by more pages than this value, -1 means autopilot.
  /// \param [in] preferred_batch  The preferred maximum number of pages to be
  /// moved per defragmentation cycle, zero means no limit.
  ///
  /// \throws mdbx::error on failure (other than stopping reasons above).
  /// \see ::mdbx_env_defrag()
  template <typename VISITOR>
  inline defrag_result defrag(VISITOR &visitor, size_t defrag_atleast = 0, size_t time_atleast_dot16 = 0,
                              size_t defrag_enough = 0, size_t time_limit_dot16 = 0, intptr_t acceptable_backlash = -1,
                              intptr_t preferred_batch = 0);
  /// \brief Performs database defragmentation without a progress visitor.
  ///
  /// \details The same defragmentation as in the overload accepting a
  /// `visitor` functor, but without progress callbacks.
  ///
  /// \param [in] defrag_atleast  The required at least number of pages by
  /// which the database must be reduced, zero means no lower bound.
  /// \param [in] time_atleast_dot16  The minimum time in 1/65536 fractions of
  /// a second that should be spent to defragment more even if goals reached,
  /// zero means no lower bound.
  /// \param [in] defrag_enough  The number of pages by which it will be enough
  /// to shrink the database to finish, zero means no limit.
  /// \param [in] time_limit_dot16  The time limit in 1/65536 fractions of a
  /// second that could be spent to defragment, zero means no limit.
  /// \param [in] acceptable_backlash  Stop if the next cycle will be unable to
  /// shrink the database by more pages than this value, -1 means autopilot.
  /// \param [in] preferred_batch  The preferred maximum number of pages to be
  /// moved per defragmentation cycle, zero means no limit.
  ///
  /// \returns The final \ref defrag_result.
  /// \throws mdbx::error on failure (other than stopping reasons above).
  /// \see ::mdbx_env_defrag()
  inline defrag_result defrag(size_t defrag_atleast = 0, size_t time_atleast_dot16 = 0, size_t defrag_enough = 0,
                              size_t time_limit_dot16 = 0, intptr_t acceptable_backlash = -1,
                              intptr_t preferred_batch = 0);
};

/// \brief Managed database environment.
///
/// As other managed classes, `env_managed` destroys the represented underlying
/// object from the own class destructor, but disallows copying and assignment
/// for instances.
///
/// An environment supports multiple key-value tables (aka key-value spaces
/// or maps), all residing in the same shared-memory mapped file.
class LIBMDBX_API_TYPE env_managed : public env {
  using inherited = env;
  /// delegated constructor for RAII
  MDBX_CXX11_CONSTEXPR env_managed(MDBX_env *ptr) noexcept : inherited(ptr) {}
  void setup(unsigned max_maps, unsigned max_readers = 0);

public:
  /// \brief Constructs an empty (not opened) managed environment.
  MDBX_CXX11_CONSTEXPR env_managed() noexcept = default;

  /// \brief Open existing database.
  /// \param pathname  The path to the database.
  /// \param params    The operate parameters, see \ref operate_parameters.
  /// \param accede    Open with the parameters the database was created with,
  ///                    instead of failing on a mismatch.
  /// \throws mdbx::error on failure.
#ifdef MDBX_STD_FILESYSTEM_PATH
  env_managed(const MDBX_STD_FILESYSTEM_PATH &pathname, const operate_parameters &params, bool accede = true);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \brief Open existing database (the \ref operate_parameters overload for a wide-string path).
  /// \param pathname  The path to the database.
  /// \param params    The operate parameters, see \ref operate_parameters.
  /// \param accede    Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  env_managed(const ::std::wstring &pathname, const operate_parameters &params, bool accede = true);
  /// \brief Open existing database (the \ref operate_parameters overload for a wide-string path).
  /// \param pathname  The path to the database.
  /// \param params    The operate parameters, see \ref operate_parameters.
  /// \param accede    Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  explicit env_managed(const wchar_t *pathname, const operate_parameters &params, bool accede = true);
#endif /* Windows */
  /// \brief Open existing database (the \ref operate_parameters overload for a narrow-string path).
  /// \param pathname  The path to the database.
  /// \param params    The operate parameters, see \ref operate_parameters.
  /// \param accede    Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  env_managed(const ::std::string &pathname, const operate_parameters &params, bool accede = true);
  /// \brief Open existing database (the \ref operate_parameters overload for a C-string path).
  /// \param pathname  The path to the database.
  /// \param params    The operate parameters, see \ref operate_parameters.
  /// \param accede    Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  explicit env_managed(const char *pathname, const operate_parameters &params, bool accede = true);

  /// \brief Additional parameters for creating a new database.
  /// \see env_managed(const ::std::string &pathname, const create_parameters &,
  /// const operate_parameters &, bool accede)
  struct create_parameters {
    env::geometry geometry;           ///< \brief The database geometry, see \ref env::geometry.
    mdbx_mode_t file_mode_bits{0640}; ///< \brief The file mode bits for the created DB files.
    bool use_subdirectory{false};     ///< \brief Whether to place the DB files in a subdirectory.
    /// \brief Constructs the default create parameters.
    MDBX_CXX11_CONSTEXPR create_parameters() noexcept = default;
    /// \brief Copy constructor.
    create_parameters(const create_parameters &) noexcept = default;

    /// \brief Sets the database geometry for size management.
    /// \param value  The geometry to set.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR create_parameters &set_geometry(const env::geometry &value) noexcept;
    /// \brief Sets the file mode bits used for creation of the DB files.
    /// \param value  The file mode bits to set.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR create_parameters &set_file_mode_bits(mdbx_mode_t value) noexcept;
    /// \brief Sets whether to use a subdirectory to place the DB files.
    /// \param value  `true` to use a subdirectory.
    /// \returns A reference to this instance.
    MDBX_CXX14_CONSTEXPR create_parameters &set_use_subdirectory(bool value = true) noexcept;

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    friend auto operator<=>(const create_parameters &a, const create_parameters &b) = default;
#else
    MDBX_CXX14_CONSTEXPR bool operator==(const create_parameters &other) const noexcept {
      return geometry == other.geometry && file_mode_bits == other.file_mode_bits &&
             use_subdirectory == other.use_subdirectory;
    }
    MDBX_CXX14_CONSTEXPR bool operator!=(const create_parameters &other) const noexcept { return !(*this == other); }
#endif
  };

  /// \brief Create new or open existing database.
  /// \param pathname        The path to the database.
  /// \param create_params   The create parameters, see \ref create_parameters.
  /// \param operate_params  The operate parameters, see \ref operate_parameters.
  /// \param accede          Open with the parameters the database was created
  ///                          with, instead of failing on a mismatch.
  /// \throws mdbx::error on failure.
#ifdef MDBX_STD_FILESYSTEM_PATH
  env_managed(const MDBX_STD_FILESYSTEM_PATH &pathname, const create_parameters &create_params,
              const operate_parameters &operate_params, bool accede = true);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \brief Create new or open existing database (wide-string path overload).
  /// \param pathname        The path to the database.
  /// \param create_params   The create parameters, see \ref create_parameters.
  /// \param operate_params  The operate parameters, see \ref operate_parameters.
  /// \param accede          Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  env_managed(const ::std::wstring &pathname, const create_parameters &create_params,
              const operate_parameters &operate_params, bool accede = true);
  /// \brief Create new or open existing database (wide-string path overload).
  /// \param pathname        The path to the database.
  /// \param create_params   The create parameters, see \ref create_parameters.
  /// \param operate_params  The operate parameters, see \ref operate_parameters.
  /// \param accede          Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  explicit env_managed(const wchar_t *pathname, const create_parameters &create_params,
                       const operate_parameters &operate_params, bool accede = true);
#endif /* Windows */
  /// \brief Create new or open existing database (narrow-string path overload).
  /// \param pathname        The path to the database.
  /// \param create_params   The create parameters, see \ref create_parameters.
  /// \param operate_params  The operate parameters, see \ref operate_parameters.
  /// \param accede          Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  env_managed(const ::std::string &pathname, const create_parameters &create_params,
              const operate_parameters &operate_params, bool accede = true);
  /// \brief Create new or open existing database (C-string path overload).
  /// \param pathname        The path to the database.
  /// \param create_params   The create parameters, see \ref create_parameters.
  /// \param operate_params  The operate parameters, see \ref operate_parameters.
  /// \param accede          Open with the parameters the database was created with.
  /// \throws mdbx::error on failure.
  explicit env_managed(const char *pathname, const create_parameters &create_params,
                       const operate_parameters &operate_params, bool accede = true);

  /// \brief Opens an environment instance using a specific meta-page
  /// for checking and recovery.
  ///
  /// \details Creates a new environment and opens it in the recovery mode,
  /// bypassing the regular consistency checks. Mostly an internal API for the
  /// `mdbx_chk` utility and subject to change; use only when you know what you
  /// are doing. The opened environment may be used e.g. with \ref env::chk()
  /// and \ref env::turn_for_recovery().
  ///
  /// \param [in] pathname    The path to the database file.
  /// \param [in] target_meta The number of the meta-page to use (0..2), or
  ///                         any value for the default behavior.
  /// \param [in] writeable   Whether to open in read-write mode.
  /// \param [in] max_maps    The maximum number of named tables/maps, must be
  ///                         set before opening; zero means the library
  ///                         default. The recovery tooling uses 2 (MainDB+GC).
  ///
  /// \returns The opened managed environment.
  /// \see ::mdbx_env_open_for_recovery()
  static env_managed open_for_recovery(const char *pathname, unsigned target_meta, bool writeable,
                                       unsigned max_maps = 2);
  /// \copybrief env_managed::open_for_recovery(const char *, unsigned, bool, unsigned)
  /// \param pathname     The path to the database file.
  /// \param target_meta  The number of the meta-page to use (0..2), or any value for the default behavior.
  /// \param writeable    Whether to open in read-write mode.
  /// \param max_maps     The maximum number of named tables/maps.
  /// \returns The opened managed environment.
  static env_managed open_for_recovery(const ::std::string &pathname, unsigned target_meta, bool writeable,
                                       unsigned max_maps = 2);
#ifdef MDBX_STD_FILESYSTEM_PATH
  /// \copybrief env_managed::open_for_recovery(const char *, unsigned, bool, unsigned)
  /// \param pathname     The path to the database file.
  /// \param target_meta  The number of the meta-page to use (0..2), or any value for the default behavior.
  /// \param writeable    Whether to open in read-write mode.
  /// \param max_maps     The maximum number of named tables/maps.
  /// \returns The opened managed environment.
  static env_managed open_for_recovery(const MDBX_STD_FILESYSTEM_PATH &pathname, unsigned target_meta, bool writeable,
                                       unsigned max_maps = 2);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \copybrief env_managed::open_for_recovery(const char *, unsigned, bool, unsigned)
  /// \param pathname     The path to the database file.
  /// \param target_meta  The number of the meta-page to use (0..2), or any value for the default behavior.
  /// \param writeable    Whether to open in read-write mode.
  /// \param max_maps     The maximum number of named tables/maps.
  /// \returns The opened managed environment.
  static env_managed open_for_recovery(const wchar_t *pathname, unsigned target_meta, bool writeable,
                                       unsigned max_maps = 2);
  /// \copybrief env_managed::open_for_recovery(const char *, unsigned, bool, unsigned)
  /// \param pathname     The path to the database file.
  /// \param target_meta  The number of the meta-page to use (0..2), or any value for the default behavior.
  /// \param writeable    Whether to open in read-write mode.
  /// \param max_maps     The maximum number of named tables/maps.
  /// \returns The opened managed environment.
  static env_managed open_for_recovery(const ::std::wstring &pathname, unsigned target_meta, bool writeable,
                                       unsigned max_maps = 2);
#endif /* Windows */

  /// \brief Explicitly closes the environment and release the memory map.
  ///
  /// Only a single thread may call this function. All transactions, tables,
  /// and cursors must already be closed before calling this function. Attempts
  /// to use any such handles after calling this function will cause a
  /// `SIGSEGV`. The environment handle will be freed and must not be used again
  /// after this call.
  ///
  /// \param [in] dont_sync  The dont_sync flag, if non-zero the last checkpoint
  /// will be kept "as is" and may be still "weak" in the
  /// \ref env::durability::lazy_weak_tail or
  /// \ref env::durability::whole_fragile modes. Such "weak" checkpoint will be ignored
  /// on opening next time, and transactions since the last non-weak checkpoint
  /// (meta-page update) will rolledback for consistency guarantee.
  void close(bool dont_sync = false);

  /// \brief Move constructor.
  env_managed(env_managed &&) = default;
  /// \brief Move assignment operator.
  /// \param other  The source managed environment to be moved from.
  /// \returns A reference to this instance.
  env_managed &operator=(env_managed &&other) noexcept;
  env_managed(const env_managed &) = delete;
  env_managed &operator=(const env_managed &) = delete;
  virtual ~env_managed();
};

/// \brief Shorthand for \ref env::geometry.
using geometry = env::geometry;
/// \brief Shorthand for \ref env::reclaiming_options.
using reclaiming_options = env::reclaiming_options;
/// \brief Shorthand for \ref env::operate_options.
using operate_options = env::operate_options;
/// \brief Shorthand for \ref env::operate_parameters.
using operate_parameters = env::operate_parameters;
/// \brief Shorthand for \ref env_managed::create_parameters.
using create_parameters = env_managed::create_parameters;

/// \brief Converts a fraction to a string of decimal digits without using
/// floating-point operations. \see ::mdbx_ratio2digits()
/// \param numerator  The numerator of the fraction.
/// \param denominator  The denominator of the fraction.
/// \param precision  The number of digits after the decimal point.
/// \returns The resulting string.
inline ::std::string ratio2digits(uint64_t numerator, uint64_t denominator, int precision) {
  char buffer[64];
  return ::mdbx_ratio2digits(numerator, denominator, precision, buffer, sizeof(buffer));
}

/// \brief Converts a fraction to a percentage string without using
/// floating-point operations. \see ::mdbx_ratio2percents()
/// \param value  The value.
/// \param whole  The whole value, i.e. the denominator.
/// \returns The resulting string.
inline ::std::string ratio2percents(uint64_t value, uint64_t whole) {
  char buffer[64];
  return ::mdbx_ratio2percents(value, whole, buffer, sizeof(buffer));
}

/// \brief Quickly finds out whether readahead is reasonable for the given
/// data volume and redundancy. \see ::mdbx_is_readahead_reasonable()
/// \param volume  The volume in bytes.
/// \param redundancy  The `redundancy` argument.
/// \returns `true` on success, `false` otherwise.
inline bool is_readahead_reasonable(size_t volume, intptr_t redundancy) {
  return ::mdbx_is_readahead_reasonable(volume, redundancy) != 0;
}

/// \brief Returns the built-in data (value) comparator for the given table
/// flags, which is useful as a fallback when implementing custom comparators.
/// \see ::mdbx_get_datacmp()
/// \param flags  The flags.
/// \returns The built-in data (value) comparator for the given table flags, which is useful as a fallback when
/// implementing custom comparators. \see ::mdbx_get_datacmp().
inline MDBX_cmp_func get_datacmp(MDBX_db_flags_t flags) { return ::mdbx_get_datacmp(flags); }

/// \brief Sets up the global log-level, debug options and logger.
/// \returns A non-negative value on success (previous settings packed into
/// the low 16 bits and high 16 bits), or a negative error.
/// \see ::mdbx_setup_debug()
/// \param log_level  The log level.
/// \param debug_flags  The debug flags.
/// \param logger  The logger callback.
inline int setup_debug(MDBX_log_level_t log_level = MDBX_LOG_DONTCHANGE,
                       MDBX_debug_flags_t debug_flags = MDBX_DBG_DONTCHANGE,
                       MDBX_debug_func logger = MDBX_LOGGER_DONTCHANGE) {
  return ::mdbx_setup_debug(log_level, debug_flags, logger);
}

/// \brief Sets up the global log-level, debug options and a logger for plain
/// preformatted messages.
/// \returns A non-negative value on success, or a negative error.
/// \see ::mdbx_setup_debug_nofmt()
/// \param log_level  The log level.
/// \param debug_flags  The debug flags.
/// \param logger  The logger callback.
/// \param logger_buffer  The logger buffer.
/// \param logger_buffer_size  The size of the logger buffer.
inline int setup_debug_nofmt(MDBX_log_level_t log_level, MDBX_debug_flags_t debug_flags, MDBX_debug_func_nofmt logger,
                             char *logger_buffer, size_t logger_buffer_size) {
  return ::mdbx_setup_debug_nofmt(log_level, debug_flags, logger, logger_buffer, logger_buffer_size);
}

/// \brief Sets a callback for assertion failures, called before printing the
/// message and aborting. \see ::mdbx_set_panic()
/// \param func  The function name.
inline void set_panic(MDBX_panic_func func) { ::mdbx_set_panic(func); }

class txn_managed;

/// \brief Unmanaged database transaction.
///
/// Like other unmanaged classes, `txn` allows copying and assignment for handles as a values,
/// but does not manage nor destroys the represented underlying object from the own class destructor.
///
/// All database operations require a transaction handle. Transactions may be
/// read-only or read-write.
class LIBMDBX_API_TYPE txn {
protected:
  friend class cursor;
  MDBX_txn *handle_{nullptr}; ///< \brief The underlying C transaction handle, null for an empty instance.
  /// \brief Constructs a non-owning transaction wrapping the given C handle.
  /// \param ptr  The pointer to the underlying \ref MDBX_txn object.
  MDBX_CXX11_CONSTEXPR txn(MDBX_txn *ptr) noexcept;

public:
  /// \brief Constructs an empty (null) transaction.
  MDBX_CXX11_CONSTEXPR txn() noexcept = default;
  /// \brief Copy constructor.
  txn(const txn &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  txn &operator=(const txn &) noexcept = default;
  /// \brief Move assignment operator.
  /// \param other  The source transaction to be moved from.
  /// \returns A reference to this instance.
  inline txn &operator=(txn &&other) noexcept;
  /// \brief Move constructor: the source transaction is nullified.
  /// \param other  The source transaction to be moved from.
  inline txn(txn &&other) noexcept;
  /// \brief Destroys the wrapper; the underlying object is NOT finished
  /// since the transaction is non-owning.
  inline ~txn() noexcept;
  /* The base handle must never be move-initialized from the owning
   * txn_managed: the move would silently drop ownership, so the transaction
   * (and for a writer, the environment's writer lock) would never be released
   * (issue #77). The exact-match deleted overloads win over the base upcast
   * and turn `mdbx::txn t = env.start_write();` into a compile error.
   * Copying from an lvalue txn_managed stays allowed: the copy is a
   * non-owning view and does not change the source's ownership. */
  txn(txn_managed &&) = delete;
  txn &operator=(txn_managed &&) = delete;

  /// \brief Checks whether the transaction wraps a non-null handle.
  /// \returns `true` if the transaction is not null.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept { return handle_ != nullptr; };
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_txn object.
  MDBX_CXX14_CONSTEXPR operator const MDBX_txn *() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_txn object.
  MDBX_CXX14_CONSTEXPR operator MDBX_txn *() noexcept { return handle_; }
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_txn object.
  MDBX_CXX14_CONSTEXPR const MDBX_txn *handle() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_txn object.
  MDBX_CXX14_CONSTEXPR MDBX_txn *handle() noexcept { return handle_; };
  friend MDBX_CXX11_CONSTEXPR bool operator==(const txn &a, const txn &b) noexcept { return a.handle_ == b.handle_; }
  friend MDBX_CXX11_CONSTEXPR bool operator!=(const txn &a, const txn &b) noexcept { return a.handle_ != b.handle_; }

  /// \brief Returns the transaction's environment.
  /// \returns The \ref env instance the transaction belongs to.
  inline ::mdbx::env env() const noexcept;
  /// \brief Returns transaction's flags.
  /// \returns The \ref MDBX_txn_flags_t combination.
  inline MDBX_txn_flags_t flags() const;
  /// \brief Return the transaction's ID.
  /// \returns The transaction/MVCC-snapshot ID.
  inline uint64_t id() const;

  /// \brief Returns the application context associated with the transaction.
  /// \returns The application context pointer.
  inline void *get_context() const noexcept;

  /// \brief Sets the application context associated with the transaction.
  /// \param your_context  The application context pointer.
  /// \returns The transaction instance.
  inline txn &set_context(void *your_context);

  /// \brief Checks whether the given data is on a dirty page.
  /// \param ptr  The address of the source data.
  /// \returns `true` on success, `false` otherwise.
  inline bool is_dirty(const void *ptr) const;

  /// \brief Checks whether the given slice is on a dirty page.
  /// \param item  The `item` argument.
  /// \returns `true` on success, `false` otherwise.
  inline bool is_dirty(const slice &item) const { return item && is_dirty(item.data()); }

  /// \brief Checks whether the transaction is read-only.
  /// \returns `true` if the transaction is read-only.
  bool is_readonly() const { return (flags() & MDBX_TXN_RDONLY) != 0; }

  /// \brief Checks whether the transaction is read-write.
  /// \returns `true` if the transaction is read-write.
  bool is_readwrite() const { return (flags() & MDBX_TXN_RDONLY) == 0; }

  /// \brief An alias for \ref MDBX_txn_info.
  using info = ::MDBX_txn_info;
  /// \brief Returns information about the MDBX transaction.
  /// \param scan_reader_lock_table  The `scan_reader_lock_table` argument.
  /// \returns Information about the MDBX transaction.
  inline info get_info(bool scan_reader_lock_table = false) const;

  /// \brief Returns maximal write transaction size (i.e. limit for summary
  /// volume of dirty pages) in bytes.
  /// \returns The maximum transaction size in bytes.
  size_t size_max() const { return env().transaction_size_max(); }

  /// \brief Returns current write transaction size (i.e. summary volume of dirty pages) in bytes.
  /// \returns The current transaction size in bytes.
  size_t size_current() const {
    assert(is_readwrite());
    return size_t(get_info().txn_space_dirty);
  }

  //----------------------------------------------------------------------------

  /// \brief Reset read-only transaction.
  inline void reset_reading();

  /// \brief Renew read-only transaction.
  inline void renew_reading();

  /// \brief Refresh the read-only transaction to the most recent snapshot.
  ///
  /// \details Replaces the MVCC-snapshot of the given read-only transaction
  /// with the most recent committed one, see \ref ::mdbx_txn_refresh().
  ///
  /// \returns `True` if the transaction was already reading the most recent
  /// version and no actions were performed, or `false` if it was refreshed
  /// (i.e. advanced to a newer snapshot).
  /// \see reset_reading() \see renew_reading()
  inline bool refresh();

  /// \brief Clone read transaction.
  /// \param context  The application context.
  /// \returns The managed transaction instance.
  inline txn_managed clone(void *context = nullptr) const;

  /// \brief Renew given read transaction into clone.
  /// \param txn_for_renew_into_clone  The `txn_for_renew_into_clone` argument.
  /// \param context  The application context.
  inline void clone(txn_managed &txn_for_renew_into_clone, void *context = nullptr) const;

  /// \brief Copy an MVCC-snapshot of the database to the destination path.
  ///
  /// \details Copies the consistent state as seen by this transaction,
  /// see \ref ::mdbx_txn_copy2pathname().
  /// \see env::copy()
  /// \param destination  The `destination` argument.
  /// \param compactify  The `compactify` argument.
  /// \param force_dynamic_size  The `force_dynamic_size` argument.
  inline void copy(const char *destination, bool compactify, bool force_dynamic_size = false);
  /// \copydoc copy(const char *, bool, bool)
  inline void copy(const ::std::string &destination, bool compactify, bool force_dynamic_size = false);
#ifdef MDBX_STD_FILESYSTEM_PATH
  /// \copydoc copy(const char *, bool, bool)
  inline void copy(const MDBX_STD_FILESYSTEM_PATH &destination, bool compactify, bool force_dynamic_size = false);
#endif /* MDBX_STD_FILESYSTEM_PATH */
#if defined(_WIN32) || defined(_WIN64) || defined(DOXYGEN)
  /// \copydoc copy(const char *, bool, bool)
  inline void copy(const wchar_t *destination, bool compactify, bool force_dynamic_size = false);
  /// \copydoc copy(const char *, bool, bool)
  inline void copy(const ::std::wstring &destination, bool compactify, bool force_dynamic_size = false);
#endif /* Windows */

  /// \brief Copy an MVCC-snapshot of the database to the given file handle.
  /// \see ::mdbx_txn_copy2fd()
  /// \param fd  The file descriptor.
  /// \param compactify  The `compactify` argument.
  /// \param force_dynamic_size  The `force_dynamic_size` argument.
  inline void copy(filehandle fd, bool compactify, bool force_dynamic_size = false);

  /// \brief Marks transaction as broken to prevent further operations.
  inline void make_broken();

  /// \brief Park read-only transaction.
  /// \param autounpark  The `autounpark` argument.
  inline void park_reading(bool autounpark = true);

  /// \brief Resume parked read-only transaction.
  /// \returns True if transaction was restarted while `restart_if_ousted=true`.
  /// \param restart_if_ousted  The `restart_if_ousted` argument.
  inline bool unpark_reading(bool restart_if_ousted = true);

  /// \brief Start nested write transaction.
  /// \returns The managed nested write transaction.
  txn_managed start_nested();

  /// \brief Start nested transaction.
  /// \param readonly  Open a read-only transaction.
  /// \returns The managed transaction instance.
  txn_managed start_nested(bool readonly);

  /// \brief Opens cursor for specified key-value map handle.
  /// \param map  The table (map) handle.
  /// \returns The managed cursor instance.
  inline cursor_managed open_cursor(map_handle map) const;

  /// \brief Unbind or close all cursors.
  /// \warning With `unbind=false` it must NOT be used while any cursor of the
  ///   transaction is owned by a \ref cursor_managed instance: the cursor is closed
  ///   behind the wrapper's back, which is left owning a freed handle.
  /// \note With `unbind=true` the wrappers are left alive but detached (see
  ///   \ref unbind_all_cursors()).
  /// \param unbind  Unbind the cursor from the transaction.
  /// \returns The size in bytes.
  inline size_t release_all_cursors(bool unbind) const;

  // TODO: пересмотреть вариант реализации при переходе на двухсвязанные
  // списки курсоров.
  // \brief Close all cursors.
  // Опасный API: освобождает C-хендлы курсоров, которыми могут ещё
  // владеть живые mdbx::cursor_managed (issue #91).
  // inline size_t close_all_cursors() const { return release_all_cursors(false); }

  /// \brief Unbind all cursors.
  /// \details Unbinds (detaches) every live cursor of the transaction, the
  ///   underlying C objects stay allocated and owned. A \ref cursor_managed
  ///   wrapper, however, is not informed about the unbind: it still holds the
  ///   same C handle, so any subsequent cursor operation (seeking, moving,
  ///   reading the current entry, etc.) fails with \ref MDBX_EBADSIGN or
  ///   \ref MDBX_EINVAL at the next C API call. There is no way to detect the
  ///   detached state from the wrapper side except observing those errors.
  ///   To keep using a cursor after this call, re-bind it explicitly via
  ///   \ref cursor::bind() / \ref cursor::renew() (the cursor's `unbind()`
  ///   makes the detachment explicit and is the recommended way to invalidate
  ///   a wrapper you intend to re-use) (issue #146).
  /// \returns The number of unbound cursors.
  inline size_t unbind_all_cursors() const { return release_all_cursors(true); }

  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle open_map(const char *name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                             const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single) const;
  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle open_map(const ::std::string &name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                             const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single) const;
  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle open_map(const slice &name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                             const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single) const;

  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \returns The table (map) handle.
  inline map_handle open_map_accede(const char *name) const;
  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \returns The table (map) handle.
  inline map_handle open_map_accede(const ::std::string &name) const;
  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \returns The table (map) handle.
  inline map_handle open_map_accede(const slice &name) const;

  /// \brief Create new or open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle create_map(const char *name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                               const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single);
  /// \brief Create new or open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle create_map(const ::std::string &name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                               const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single);
  /// \brief Create new or open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle create_map(const slice &name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                               const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single);

  /// \brief Drops key-value map using handle.
  /// \param map  The table (map) handle.
  inline void drop_map(map_handle map);
  /// \brief Drops key-value map using name.
  /// \return `True` if the key-value map existed and was deleted, either
  /// `false` if the key-value map did not exist and there is nothing to delete.
  /// \param name  The table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool drop_map(const char *name, bool throw_if_absent = false);
  /// \brief Drop key-value map.
  /// \return `True` if the key-value map existed and was deleted, either
  /// `false` if the key-value map did not exist and there is nothing to delete.
  /// \param name  The table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  inline bool drop_map(const ::std::string &name, bool throw_if_absent = false);
  /// \brief Drop key-value map.
  /// \return `True` if the key-value map existed and was deleted, either
  /// `false` if the key-value map did not exist and there is nothing to delete.
  /// \param name  The table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool drop_map(const slice &name, bool throw_if_absent = false);

  /// \brief Clear key-value map.
  /// \param map  The table (map) handle.
  /// \throws mdbx::error on failure.
  inline void clear_map(map_handle map);
  /// \brief Clear key-value map by name.
  /// \param name             The table name.
  /// \param throw_if_absent  Throw \ref mdbx::not_found if the table does not exist.
  /// \return `True` if the key-value map existed and was cleared, either
  /// `false` if the key-value map did not exist and there is nothing to clear.
  bool clear_map(const char *name, bool throw_if_absent = false);
  /// \brief Clear key-value map by name.
  /// \param name             The table name.
  /// \param throw_if_absent  Throw \ref mdbx::not_found if the table does not exist.
  /// \return `True` if the key-value map existed and was cleared, either
  /// `false` if the key-value map did not exist and there is nothing to clear.
  inline bool clear_map(const ::std::string &name, bool throw_if_absent = false);
  /// \brief Clear key-value map by name.
  /// \param name             The table name.
  /// \param throw_if_absent  Throw \ref mdbx::not_found if the table does not exist.
  /// \return `True` if the key-value map existed and was cleared, either
  /// `false` if the key-value map did not exist and there is nothing to clear.
  bool clear_map(const slice &name, bool throw_if_absent = false);

  /// \brief Переименовывает таблицу ключ-значение.
  /// \param map  The table (map) handle.
  /// \param new_name  The new table name.
  inline void rename_map(map_handle map, const char *new_name);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \param map  The table (map) handle.
  /// \param new_name  The new table name.
  inline void rename_map(map_handle map, const ::std::string &new_name);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \param map  The table (map) handle.
  /// \param new_name  The new table name.
  inline void rename_map(map_handle map, const slice &new_name);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \return `True` если таблица существует и была переименована, либо
  /// `false` в случае отсутствия исходной таблицы.
  /// \param old_name  The `old_name` argument.
  /// \param new_name  The new table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool rename_map(const char *old_name, const char *new_name, bool throw_if_absent = false);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \return `True` если таблица существует и была переименована, либо
  /// `false` в случае отсутствия исходной таблицы.
  /// \param old_name  The `old_name` argument.
  /// \param new_name  The new table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool rename_map(const ::std::string &old_name, const ::std::string &new_name, bool throw_if_absent = false);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \return `True` если таблица существует и была переименована, либо
  /// `false` в случае отсутствия исходной таблицы.
  /// \param old_name  The `old_name` argument.
  /// \param new_name  The new table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool rename_map(const slice &old_name, const slice &new_name, bool throw_if_absent = false);

#if defined(DOXYGEN) || (defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L)

  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle open_map(const ::std::string_view &name, const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                             const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single) const {
    return open_map(slice(name), key_mode, value_mode);
  }
  /// \brief Open existing key-value map.
  /// \param name  The table name.
  /// \returns The table (map) handle.
  inline map_handle open_map_accede(const ::std::string_view &name) const;
  /// \brief Create new or open existing key-value map.
  /// \param name  The table name.
  /// \param key_mode  The key comparison mode, see \ref mdbx::key_mode.
  /// \param value_mode  The value comparison mode, see \ref mdbx::value_mode.
  /// \returns The table (map) handle.
  inline map_handle create_map(const ::std::string_view &name,
                               const ::mdbx::key_mode key_mode = ::mdbx::key_mode::usual,
                               const ::mdbx::value_mode value_mode = ::mdbx::value_mode::single) {
    return create_map(slice(name), key_mode, value_mode);
  }
  /// \brief Drop key-value map.
  /// \return `True` if the key-value map existed and was deleted, either
  /// `false` if the key-value map did not exist and there is nothing to delete.
  /// \param name  The table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool drop_map(const ::std::string_view &name, bool throw_if_absent = false) {
    return drop_map(slice(name), throw_if_absent);
  }
  /// \brief Clear key-value map by name.
  /// \param name             The table name.
  /// \param throw_if_absent  Throw \ref mdbx::not_found if the table does not exist.
  /// \return `True` if the key-value map existed and was cleared, either
  /// `false` if the key-value map did not exist and there is nothing to clear.
  bool clear_map(const ::std::string_view &name, bool throw_if_absent = false) {
    return clear_map(slice(name), throw_if_absent);
  }
  /// \brief Переименовывает таблицу ключ-значение.
  /// \param map  The table (map) handle.
  /// \param new_name  The new table name.
  inline void rename_map(map_handle map, const ::std::string_view &new_name);
  /// \brief Переименовывает таблицу ключ-значение.
  /// \return `True` если таблица существует и была переименована, либо
  /// `false` в случае отсутствия исходной таблицы.
  /// \param old_name  The `old_name` argument.
  /// \param new_name  The new table name.
  /// \param throw_if_absent  The `throw_if_absent` argument.
  bool rename_map(const ::std::string_view &old_name, const ::std::string_view &new_name,
                  bool throw_if_absent = false) {
    return rename_map(slice(old_name), slice(new_name), throw_if_absent);
  }
#endif /* __cpp_lib_string_view >= 201606L */

  /// \brief An alias for \ref MDBX_stat, i.e. the table statistics.
  using map_stat = ::MDBX_stat;
  /// \brief Returns statistics for a table.
  /// \param map  The table (map) handle.
  /// \returns Statistics for a table.
  inline map_stat get_map_stat(map_handle map) const;
  /// \brief Returns depth (bitmask) information of nested dupsort (multi-value)
  /// B+trees for given table.
  /// \param map  The table (map) handle.
  /// \returns Depth (bitmask) information of nested dupsort (multi-value) B+trees for given table.
  inline uint32_t get_tree_deepmask(map_handle map) const;
  /// \brief Returns information about key-value map (aka table) handle.
  /// \param map  The table (map) handle.
  /// \returns Information about key-value map (aka table) handle.
  inline map_handle::info get_map_flags(map_handle map) const;

  /// \brief Enumerates user's named tables in a database.
  ///
  /// \details Calls the `visitor` functor for each user-created named table
  /// until the named tables are exhausted, or until the visitor returns
  /// `exit_loop` (which will be returned as a result).
  ///
  /// \param [in,out] visitor  A functor with the signature
  /// `int visitor(const slice &name, MDBX_db_flags_t flags, const MDBX_stat &stat, MDBX_dbi dbi)`,
  /// which will be called for each table.
  ///
  /// \returns The last value returned by the visitor's functor.
  /// \see ::mdbx_enumerate_tables()
  template <typename VISITOR> inline int enumerate_tables(VISITOR &visitor) const;

  /// \brief An alias for \ref MDBX_gc_info_t, i.e. the garbage collection info.
  using gc_info = ::MDBX_gc_info_t;
  /// \brief Provides information of Garbage Collection and page usage.
  ///
  /// \details Scans the whole GC to summarise the GC state and page usage for
  /// the given transaction, optionally iterating GC entries by calling the
  /// `visitor` functor for each span of pages inside GC (excepting the pages
  /// forming the B-tree structure of GC itself).
  ///
  /// \param [in,out] visitor  An optional functor with the signature
  /// `int visitor(uint64_t span_txnid, size_t span_pgno, size_t span_length,
  /// bool span_is_reclaimable)` returning `continue_loop` to proceed or
  /// any other value to stop enumeration.
  ///
  /// \returns The \ref gc_info of the database. An empty (zeroed) info is
  /// returned when the GC is empty (\ref MDBX_NOTFOUND).
  /// \note This API has not been frozen yet.
  /// \see ::mdbx_gc_info()
  template <typename VISITOR> inline gc_info get_gc_info(VISITOR &visitor) const;
  /// \brief Returns information about garbage collection and page usage.
  /// \returns The \ref gc_info for the current transaction.
  inline gc_info get_gc_info() const;

  /// \brief An alias for \ref MDBX_canary.
  using canary = ::MDBX_canary;
  /// \brief Set integers markers (aka "canary") associated with the environment.
  /// \param canary  The canary markers to set.
  /// \returns A reference to this transaction.
  inline txn &put_canary(const canary &);
  /// \brief Returns the four integer markers (aka "canary") associated with the environment.
  /// \returns The canary markers.
  inline canary get_canary() const;

  /// \brief Reads sequence generator associated with a key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \returns The current sequence value.
  inline uint64_t sequence(map_handle map) const;
  /// \brief Reads and increment sequence generator associated with a key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param increment  The increment of the sequence.
  /// \returns The resulting value.
  inline uint64_t sequence(map_handle map, uint64_t increment);

  /// \brief Compare two keys according to a particular key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param a  The left-hand operand.
  /// \param b  The right-hand operand.
  /// \returns The resulting value.
  inline int compare_keys(map_handle map, const slice &a, const slice &b) const noexcept;
  /// \brief Compare two values according to a particular key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param a  The left-hand operand.
  /// \param b  The right-hand operand.
  /// \returns The resulting value.
  inline int compare_values(map_handle map, const slice &a, const slice &b) const noexcept;
  /// \brief Compare keys of two pairs according to a particular key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param a  The left-hand operand.
  /// \param b  The right-hand operand.
  /// \returns The resulting value.
  inline int compare_keys(map_handle map, const pair &a, const pair &b) const noexcept;
  /// \brief Compare values of two pairs according to a particular key-value map(aka table).
  /// \param map  The table (map) handle.
  /// \param a  The left-hand operand.
  /// \param b  The right-hand operand.
  /// \returns The resulting value.
  inline int compare_values(map_handle map, const pair &a, const pair &b) const noexcept;

  /// \brief Get value by key from a key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \returns The resulting slice.
  inline slice get(map_handle map, const slice &key) const;

  /// \brief An alias for \ref MDBX_cache_status_t, i.e. the cache lookup status.
  using cache_status = ::MDBX_cache_status_t;
  /// \brief An alias for \ref MDBX_cache_result_t, i.e. the cached lookup result.
  using cache_result = ::MDBX_cache_result_t;

  /// \brief Gets a value by key using the transparent read-your-writes cache.
  ///
  /// \details Uses the cached information to check as quickly as possible
  /// whether the data has changed or not, with early exit when searching
  /// through the DB. Supports multithreaded cases and resolves collisions
  /// in a lockfree way; the \ref MDBX_NOSTICKYTHREADS mode is required to use
  /// it from different threads.
  ///
  /// \param [in] map    The table handle.
  /// \param [in] key    The key to search for.
  /// \param [out] data  The resulting value; points into database-owned
  ///                    memory and remains valid until the transaction end.
  /// \param [in,out] entry  The cache entry corresponding to the key, must be
  ///                    initialized (\ref cache_entry::reset()) before use.
  ///
  /// \returns The \ref cache_result with the pair of error code and cache
  /// status. The caller inspects `errcode` and `status` explicitly.
  /// \see ::mdbx_cache_get()
  MDBX_NODISCARD inline cache_result get_cached(map_handle map, const slice &key, slice *data,
                                                cache_entry &entry) const;

  /// \brief Gets a value by key using the transparent read-your-writes cache,
  /// throwing exceptions on errors.
  ///
  /// \param [in] map    The table handle.
  /// \param [in] key    The key to search for.
  /// \param [in,out] entry  The cache entry corresponding to the key.
  /// \param [out] status  Optional address to receive the \ref cache_status.
  ///
  /// \returns The resulting value, or throws \ref mdbx::not_found if the key
  /// is absent (like \ref get()).
  /// \see ::mdbx_cache_get()
  MDBX_NODISCARD inline slice get_cached(map_handle map, const slice &key, cache_entry &entry,
                                         cache_status *status = nullptr) const;
  /// \brief Get first of multi-value and values count by key from a key-value multimap (aka table).
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param values_count  The number of values in the array.
  /// \returns The resulting slice.
  inline slice get(map_handle map, slice key, size_t &values_count) const;
  /// \brief Get value by key from a key-value map (aka table).
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param value_at_absence  The `value_at_absence` argument.
  /// \returns The resulting slice.
  inline slice get(map_handle map, const slice &key, const slice &value_at_absence) const;
  /// \brief Get first of multi-value and values count by key from a key-value multimap (aka table).
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param values_count  The number of values in the array.
  /// \param value_at_absence  The `value_at_absence` argument.
  /// \returns The resulting slice.
  inline slice get(map_handle map, slice key, size_t &values_count, const slice &value_at_absence) const;
  /// \brief Gets the value for an equal or greater key from a table.
  /// \return Bundle of key-value pair and boolean flag,
  /// which will be `true` if the exact key was found and `false` otherwise.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  inline pair_result get_equal_or_great(map_handle map, const slice &key) const;
  /// \brief Gets the value for an equal or greater key from a table.
  /// \return Bundle of key-value pair and boolean flag,
  /// which will be `true` if the exact key was found and `false` otherwise.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param value_at_absence  The `value_at_absence` argument.
  inline pair_result get_equal_or_great(map_handle map, const slice &key, const slice &value_at_absence) const;

  /// \brief Stores a key-value pair using the given low-level flags.
  /// \param map    The table (map) handle.
  /// \param key    The key to store.
  /// \param value  The value to store (in/out for some flags).
  /// \param flags  The \ref MDBX_put_flags_t combination.
  /// \returns An error code, zero on success (does not throw).
  inline MDBX_error_t put(map_handle map, const slice &key, slice *value, MDBX_put_flags_t flags) noexcept;
  /// \brief Stores a key-value pair in the given mode.
  /// \param map    The table (map) handle.
  /// \param key    The key to store.
  /// \param value  The value to store.
  /// \param mode   The \ref put_mode (insert/upsert/update).
  /// \throws mdbx::error on failure.
  inline void put(map_handle map, const slice &key, slice value, put_mode mode);
  /// \brief Inserts a new key-value pair; fails if the key already exists.
  /// \param map    The table (map) handle.
  /// \param key    The key to insert.
  /// \param value  The value to insert.
  /// \throws mdbx::key_exists if the key already exists.
  inline void insert(map_handle map, const slice &key, slice value);
  /// \brief Tries to insert a new key-value pair without throwing on a duplicate.
  /// \param map    The table (map) handle.
  /// \param key    The key to insert.
  /// \param value  The value to insert.
  /// \returns The \ref value_result with the success flag.
  inline value_result try_insert(map_handle map, const slice &key, slice value);
  /// \brief Reserves space for a value of the given length for a new key.
  /// \param map           The table (map) handle.
  /// \param key           The key to insert.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  /// \throws mdbx::key_exists if the key already exists.
  inline slice insert_reserve(map_handle map, const slice &key, size_t value_length);
  /// \brief Tries to reserve space for a value of the given length for a new key.
  /// \param map           The table (map) handle.
  /// \param key           The key to insert.
  /// \param value_length  The length of the value to reserve.
  /// \returns The \ref value_result with the reserved slice and the success flag.
  inline value_result try_insert_reserve(map_handle map, const slice &key, size_t value_length);

  /// \brief Inserts or updates the value for the given key.
  /// \param map    The table (map) handle.
  /// \param key    The key to store.
  /// \param value  The value to store.
  /// \throws mdbx::error on failure.
  inline void upsert(map_handle map, const slice &key, const slice &value);
  /// \brief Reserves space for a value of the given length, inserting or updating the key.
  /// \param map           The table (map) handle.
  /// \param key           The key to store.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  inline slice upsert_reserve(map_handle map, const slice &key, size_t value_length);

  /// \brief Updates the value for an existing key; fails if the key is absent.
  /// \param map    The table (map) handle.
  /// \param key    The key to update.
  /// \param value  The new value.
  /// \throws mdbx::not_found if the key does not exist.
  inline void update(map_handle map, const slice &key, const slice &value);
  /// \brief Tries to update the value for an existing key without throwing.
  /// \param map    The table (map) handle.
  /// \param key    The key to update.
  /// \param value  The new value.
  /// \returns `true` if the key was found and updated.
  inline bool try_update(map_handle map, const slice &key, const slice &value);
  /// \brief Reserves space for a value of the given length for an existing key.
  /// \param map           The table (map) handle.
  /// \param key           The key to update.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  /// \throws mdbx::not_found if the key does not exist.
  inline slice update_reserve(map_handle map, const slice &key, size_t value_length);
  /// \brief Tries to reserve space for a value of the given length for an existing key.
  /// \param map           The table (map) handle.
  /// \param key           The key to update.
  /// \param value_length  The length of the value to reserve.
  /// \returns The \ref value_result with the reserved slice and the success flag.
  inline value_result try_update_reserve(map_handle map, const slice &key, size_t value_length);

  /// \brief Stores a key-value pair in the given mode.
  /// \param map   The table (map) handle.
  /// \param kv    The key-value pair.
  /// \param mode  The \ref put_mode (insert/upsert/update).
  void put(map_handle map, const pair &kv, put_mode mode) { return put(map, kv.key, kv.value, mode); }
  /// \brief Inserts a new key-value pair; fails if the key already exists.
  /// \param map  The table (map) handle.
  /// \param kv   The key-value pair to insert.
  void insert(map_handle map, const pair &kv) { return insert(map, kv.key, kv.value); }
  /// \brief Tries to insert a new key-value pair without throwing on a duplicate.
  /// \param map  The table (map) handle.
  /// \param kv   The key-value pair to insert.
  /// \returns The \ref value_result with the success flag.
  value_result try_insert(map_handle map, const pair &kv) { return try_insert(map, kv.key, kv.value); }
  /// \brief Inserts or updates the value for the given key.
  /// \param map  The table (map) handle.
  /// \param kv   The key-value pair to store.
  void upsert(map_handle map, const pair &kv) { return upsert(map, kv.key, kv.value); }

  /// \brief Removes all values for given key.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \returns `true` on success, `false` otherwise.
  inline bool erase(map_handle map, const slice &key);

  /// \brief Removes the particular multi-value entry of the key.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param value  The value.
  /// \returns `true` on success, `false` otherwise.
  inline bool erase(map_handle map, const slice &key, const slice &value);

  /// \brief Replaces the particular multi-value of the key with a new value.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param old_value  The `old_value` argument.
  /// \param new_value  The `new_value` argument.
  inline void replace(map_handle map, const slice &key, slice &old_value, const slice &new_value);

  /// \brief Removes and return a value of the key.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The result.
  template <class BUFFER, class ALLOCATOR = typename BUFFER::allocator_type>
  inline BUFFER extract(map_handle map, const slice &key, const ALLOCATOR &alloc = ALLOCATOR());

  /// \brief Replaces and returns a value of the key with new one.
  /// \param map  The table (map) handle.
  /// \param key  The key.
  /// \param new_value  The `new_value` argument.
  /// \param alloc  The allocator for the buffer storage.
  /// \returns The result.
  template <class BUFFER, class ALLOCATOR = typename BUFFER::allocator_type>
  inline BUFFER replace(map_handle map, const slice &key, const slice &new_value, const ALLOCATOR &alloc = ALLOCATOR());

  /// \brief Replaces the value of the key reserving space for the new one, and
  /// returns the old value as a buffer.
  /// \param map                     The table (map) handle.
  /// \param key                     The key to replace.
  /// \param length                  The length of the new value to reserve.
  /// \param new_value_reservation   Out: the writable slice for the reserved new value.
  /// \param alloc                   The allocator for the returned buffer.
  /// \returns The buffer with the old value (empty if the key was absent).
  template <class BUFFER, class ALLOCATOR = typename BUFFER::allocator_type>
  inline BUFFER replace_reserve(map_handle map, const slice &key, size_t length, slice &new_value_reservation,
                                const ALLOCATOR &alloc = ALLOCATOR());

  /// \brief Adding a key-value pair, provided that ascending order of the keys
  /// and (optionally) values are preserved.
  ///
  /// Instead of splitting the full b+tree pages, the data will be placed on new
  /// ones. Thus appending is about two times faster than insertion, and the
  /// pages will be filled in completely mostly but not half as after splitting
  /// ones. On the other hand, any subsequent insertion or update with an
  /// increase in the length of the value will be twice as slow, since it will
  /// require splitting already filled pages.
  ///
  /// \param [in] map   A map handle to append
  /// \param [in] key   A key to be append
  /// \param [in] value A value to store with the key
  /// \param [in] multivalue_order_preserved
  /// If `multivalue_order_preserved == true` then the same rules applied for
  /// to pages of nested b+tree of multimap's values.
  inline void append(map_handle map, const slice &key, const slice &value, bool multivalue_order_preserved = true);
  /// \brief Appends a key-value pair, provided that the ascending order is preserved.
  /// \param map                         The table (map) handle.
  /// \param kv                          The key-value pair to append.
  /// \param multivalue_order_preserved  The same rules apply to the nested multi-value tree.
  inline void append(map_handle map, const pair &kv, bool multivalue_order_preserved = true) {
    return append(map, kv.key, kv.value, multivalue_order_preserved);
  }

  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param map            The table (map) handle.
  /// \param key            The key of the multi-value hive.
  /// \param value_length   The length of each value in bytes.
  /// \param values_array   The array of values.
  /// \param values_count   The number of values in the array.
  /// \param mode           The \ref put_mode.
  /// \param allow_partial  Allow a partial store when some values are rejected.
  /// \returns The number of stored values.
  inline size_t put_multiple_samelength(map_handle map, const slice &key, const size_t value_length,
                                        const void *values_array, size_t values_count, put_mode mode,
                                        bool allow_partial = false);
  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param map            The table (map) handle.
  /// \param key            The key of the multi-value hive.
  /// \param values_array   The array of standard-layout values.
  /// \param values_count   The number of values in the array.
  /// \param mode           The \ref put_mode.
  /// \param allow_partial  Allow a partial store when some values are rejected.
  /// \returns The number of stored values.
  template <typename VALUE>
  size_t put_multiple_samelength(map_handle map, const slice &key, const VALUE *values_array, size_t values_count,
                                 put_mode mode, bool allow_partial = false) {
    static_assert(::std::is_standard_layout<VALUE>::value && !::std::is_pointer<VALUE>::value &&
                      !::std::is_array<VALUE>::value,
                  "Must be a standard layout type!");
    return put_multiple_samelength(map, key, sizeof(VALUE), values_array, values_count, mode, allow_partial);
  }
  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param map     The table (map) handle.
  /// \param key     The key of the multi-value hive.
  /// \param vector  The vector of standard-layout values.
  /// \param mode    The \ref put_mode.
  template <typename VALUE>
  void put_multiple_samelength(map_handle map, const slice &key, const ::std::vector<VALUE> &vector, put_mode mode) {
    put_multiple_samelength(map, key, vector.data(), vector.size(), mode);
  }

  /// \brief Estimates the number of items between two key-value pairs.
  /// \param map   The table (map) handle.
  /// \param from  The "from" boundary pair.
  /// \param to    The "to" boundary pair.
  /// \returns The approximate distance; a negative value means `to` is before `from`.
  inline ptrdiff_t estimate(map_handle map, const pair &from, const pair &to) const;
  /// \brief Estimates the number of items between two keys.
  /// \param map   The table (map) handle.
  /// \param from  The "from" boundary key.
  /// \param to    The "to" boundary key.
  /// \returns The approximate distance; a negative value means `to` is before `from`.
  inline ptrdiff_t estimate(map_handle map, const slice &from, const slice &to) const;
  /// \brief Estimates the number of items from the first one up to the given key.
  /// \param map  The table (map) handle.
  /// \param to   The "to" boundary key.
  /// \returns The approximate distance.
  inline ptrdiff_t estimate_from_first(map_handle map, const slice &to) const;
  /// \brief Estimates the number of items from the given key up to the last one.
  /// \param map   The table (map) handle.
  /// \param from  The "from" boundary key.
  /// \returns The approximate distance.
  inline ptrdiff_t estimate_to_last(map_handle map, const slice &from) const;
};

/// \brief Managed database transaction.
///
/// As other managed classes, `txn_managed` destroys the represented underlying
/// object from the own class destructor, but disallows copying and assignment
/// for instances.
///
/// All database operations require a transaction handle. Transactions may be
/// read-only or read-write.
class LIBMDBX_API_TYPE txn_managed : public txn {
  using inherited = txn;
  friend class env;
  friend class txn;
  /// delegated constructor for RAII
  MDBX_CXX11_CONSTEXPR txn_managed(MDBX_txn *ptr) noexcept : inherited(ptr) {}

public:
  MDBX_CXX11_CONSTEXPR txn_managed() noexcept = default;
  /// \brief Move constructor.
  txn_managed(txn_managed &&) = default;
  /// \brief Move assignment operator.
  /// \param other  The source managed transaction to be moved from.
  /// \returns A reference to this instance.
  txn_managed &operator=(txn_managed &&other) noexcept;
  txn_managed(const txn_managed &) = delete;
  txn_managed &operator=(const txn_managed &) = delete;
  ~txn_managed();

  //----------------------------------------------------------------------------
  /// \brief An alias for \ref MDBX_commit_latency, i.e. the commit latency info.
  using finalization_latency = MDBX_commit_latency;

  /// \brief Abandon all the operations of the transaction instead of saving ones.
  void abort();
  /// \brief Abandon all the operations of the transaction instead of saving ones with collecting latencies information.
  void abort(finalization_latency *);
  /// \brief Abandon all the operations of the transaction instead of saving ones with collecting latencies information.
  /// \param latency  The latency structure to fill.
  void abort(finalization_latency &latency) { return abort(&latency); }
  /// \brief Abandon all the operations of the transaction instead of saving ones with collecting latencies information.
  /// \returns latency information of abort stages.
  finalization_latency abort_get_latency() {
    finalization_latency result;
    abort(&result);
    return result;
  }

  /// \brief Commits all changes of the transaction into a database with collecting latencies information.
  void commit();
  /// \brief Commits all changes of the transaction into a database with collecting latencies information.
  void commit(finalization_latency *);
  /// \brief Commits all changes of the transaction into a database with collecting latencies information.
  /// \param latency  The latency structure to fill.
  void commit(finalization_latency &latency) { return commit(&latency); }
  /// \brief Commits all changes of the transaction into a database and return latency information.
  /// \returns latency information of commit stages.
  finalization_latency commit_get_latency() {
    finalization_latency result;
    commit(&result);
    return result;
  }

  /// \brief Commits all the operations of the transaction and immediately starts next without releasing any locks.
  /// \returns `true` on success, `false` otherwise.
  bool checkpoint();
  /// \brief Commits all the operations of the transaction and immediately starts next without releasing any locks.
  /// \param latency  The latency structure to fill.
  /// \returns `true` on success, `false` otherwise.
  bool checkpoint(finalization_latency *latency);
  /// \brief Commits all the operations of the transaction and immediately starts next without releasing any locks.
  /// \param latency  The latency structure to fill.
  /// \returns `true` on success, `false` otherwise.
  bool checkpoint(finalization_latency &latency) { return checkpoint(&latency); }
  /// \brief Commits all the operations of the transaction and immediately starts next without releasing any locks.
  /// \returns latency information of commit stages.
  std::pair<bool, finalization_latency> checkpoint_get_latency() {
    finalization_latency latency;
    bool result = checkpoint(&latency);
    return std::make_pair(result, latency);
  }

  /// \brief Commits all the operations of a transaction into the database and then start read transaction.
  void commit_embark_read();
  /// \brief Commits all the operations of a transaction into the database and then start read transaction.
  /// \param latency  The latency structure to fill.
  void commit_embark_read(finalization_latency *latency);
  /// \brief Commits all the operations of a transaction into the database and then start read transaction.
  /// \param latency  The latency structure to fill.
  void commit_embark_read(finalization_latency &latency) { return commit_embark_read(&latency); }
  /// \brief Commits all the operations of a transaction into the database and then start read transaction.
  /// \returns latency information of commit stages.
  finalization_latency commit_embark_read_get_latency() {
    finalization_latency result;
    commit_embark_read(&result);
    return result;
  }

  /// \brief Starts a writing transaction to amending data in the MVCC-snapshot used by the read-only transaction.
  /// \returns The `true` if writing transaction successfully started and `false` if read-only one still continue.
  /// \param dont_wait  Don't wait if the resource is locked by another thread.
  bool amend(bool dont_wait = false);
};

/// \brief Unmanaged cursor.
///
/// Like other unmanaged classes, `cursor` allows copying and assignment for handles as a values,
/// but does not manage nor destroys the represented underlying object from the own class destructor.
///
/// \copydetails MDBX_cursor
class LIBMDBX_API_TYPE cursor {
protected:
  MDBX_cursor *handle_{nullptr}; ///< \brief The underlying C cursor handle, null for an empty instance.

public:
  /// \brief Constructs a non-owning cursor wrapping the given C handle.
  /// \param ptr  The pointer to the underlying \ref MDBX_cursor object.
  MDBX_CXX11_CONSTEXPR cursor(MDBX_cursor *ptr) noexcept;
  /// \brief Constructs an empty (null) cursor.
  MDBX_CXX11_CONSTEXPR cursor() noexcept = default;
  /// \brief Copy constructor.
  cursor(const cursor &) noexcept = default;
  /// \brief Copy assignment operator.
  /// \returns A reference to this instance.
  cursor &operator=(const cursor &) noexcept = default;
  /// \brief Move assignment operator.
  /// \param other  The source cursor to be moved from.
  /// \returns A reference to this instance.
  inline cursor &operator=(cursor &&other) noexcept;
  /// \brief Move constructor: the source cursor is nullified.
  /// \param other  The source cursor to be moved from.
  inline cursor(cursor &&other) noexcept;
  /// \brief Destroys the wrapper; the underlying object is NOT closed
  /// since the cursor is non-owning.
  inline ~cursor() noexcept;
  /* Slicing an owning cursor_managed temporary into the base handle would drop
   * ownership and leak the cursor (issue #77), so these exact-match overloads
   * turn `mdbx::cursor c = txn.open_cursor(map);` into a compile error, while
   * a non-owning view of a cursor_managed lvalue is still allowed. */
  cursor(cursor_managed &&) = delete;
  cursor &operator=(cursor_managed &&) = delete;
  /// \brief Creates a new managed cursor with the same position and context.
  /// \param your_context  An application context to be associated with the clone.
  /// \returns The new \ref cursor_managed instance.
  /// \throws mdbx::error on failure.
  cursor_managed clone(void *your_context = nullptr) const;
  /// \brief Assigns this cursor to refer to the same handle as the source.
  /// \param src  The source cursor.
  /// \returns A reference to this instance.
  inline cursor &assign(const cursor &);
  /// \brief Checks whether the cursor wraps a non-null handle.
  /// \returns `true` if the cursor is not null.
  MDBX_CXX14_CONSTEXPR operator bool() const noexcept { return handle_ != nullptr; };
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_cursor object.
  MDBX_CXX14_CONSTEXPR operator const MDBX_cursor *() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_cursor object.
  MDBX_CXX14_CONSTEXPR operator MDBX_cursor *() noexcept { return handle_; }
  /// \brief Returns the underlying C handle (read-only).
  /// \returns The constant pointer to the \ref MDBX_cursor object.
  MDBX_CXX14_CONSTEXPR const MDBX_cursor *handle() const noexcept { return handle_; }
  /// \brief Returns the underlying C handle.
  /// \returns The mutable pointer to the \ref MDBX_cursor object.
  MDBX_CXX14_CONSTEXPR MDBX_cursor *handle() noexcept { return handle_; };
  friend MDBX_CXX11_CONSTEXPR bool operator==(const cursor &a, const cursor &b) noexcept {
    return a.handle_ == b.handle_;
  }
  friend MDBX_CXX11_CONSTEXPR bool operator!=(const cursor &a, const cursor &b) noexcept {
    return a.handle_ != b.handle_;
  }

  friend inline int compare_position_nothrow(const cursor &left, const cursor &right, bool ignore_nested) noexcept;
  friend inline int compare_position(const cursor &left, const cursor &right, bool ignore_nested);

  /// \brief Checks whether this cursor is before the given one.
  /// \param other          The cursor to compare the position with.
  /// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
  /// \returns `true` if this cursor position is less than the other one.
  /// \throws `std::logic_error` if the cursors are incomparable.
  bool is_before_than(const cursor &other, bool ignore_nested = false) const {
    return compare_position(*this, other, ignore_nested) < 0;
  }

  /// \brief Checks whether this cursor is at the same position or before the given one.
  /// \param other          The cursor to compare the position with.
  /// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
  /// \returns `true` if this cursor position is less than or equal to the other one.
  /// \throws `std::logic_error` if the cursors are incomparable.
  bool is_same_or_before_than(const cursor &other, bool ignore_nested = false) const {
    return compare_position(*this, other, ignore_nested) <= 0;
  }

  /// \brief Checks whether this cursor is at the same position as the given one.
  /// \param other          The cursor to compare the position with.
  /// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
  /// \returns `true` if this cursor position is equal to the other one.
  /// \throws `std::logic_error` if the cursors are incomparable.
  bool is_same_position(const cursor &other, bool ignore_nested = false) const {
    return compare_position(*this, other, ignore_nested) == 0;
  }

  /// \brief Checks whether this cursor is after the given one.
  /// \param other          The cursor to compare the position with.
  /// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
  /// \returns `true` if this cursor position is greater than the other one.
  /// \throws `std::logic_error` if the cursors are incomparable.
  bool is_after_than(const cursor &other, bool ignore_nested = false) const {
    return compare_position(*this, other, ignore_nested) > 0;
  }

  /// \brief Checks whether this cursor is at the same position or after the given one.
  /// \param other          The cursor to compare the position with.
  /// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
  /// \returns `true` if this cursor position is greater than or equal to the other one.
  /// \throws `std::logic_error` if the cursors are incomparable.
  bool is_same_or_after_than(const cursor &other, bool ignore_nested = false) const {
    return compare_position(*this, other, ignore_nested) >= 0;
  }

  /// \brief Returns the application context associated with the cursor.
  /// \returns The application context pointer.
  inline void *get_context() const noexcept;

  /// \brief Sets the application context associated with the cursor.
  /// \param your_context  The application context pointer.
  /// \returns A reference to this instance.
  inline cursor &set_context(void *your_context);

  /// \brief Cursor positioning operations, mirrors \ref MDBX_cursor_op.
  enum move_operation {
    first = MDBX_FIRST,
    last = MDBX_LAST,
    next = MDBX_NEXT,
    previous = MDBX_PREV,
    get_current = MDBX_GET_CURRENT,

    multi_prevkey_lastvalue = MDBX_PREV_NODUP,
    multi_currentkey_firstvalue = MDBX_FIRST_DUP,
    multi_currentkey_prevvalue = MDBX_PREV_DUP,
    multi_currentkey_nextvalue = MDBX_NEXT_DUP,
    multi_currentkey_lastvalue = MDBX_LAST_DUP,
    multi_nextkey_firstvalue = MDBX_NEXT_NODUP,

    multi_find_pair = MDBX_GET_BOTH,
    multi_exactkey_lowerboundvalue = MDBX_GET_BOTH_RANGE,

    seek_key = MDBX_SET,
    key_exact = MDBX_SET_KEY,
    key_lowerbound = MDBX_SET_RANGE,

    /* Doubtless cursor positioning at a specified key. */
    key_lesser_than = MDBX_TO_KEY_LESSER_THAN,
    key_lesser_or_equal = MDBX_TO_KEY_LESSER_OR_EQUAL,
    key_equal = MDBX_TO_KEY_EQUAL,
    key_greater_or_equal = MDBX_TO_KEY_GREATER_OR_EQUAL,
    key_greater_than = MDBX_TO_KEY_GREATER_THAN,

    /* Doubtless cursor positioning at a specified key-value pair
     * for dupsort/multi-value hives. */
    multi_exactkey_value_lesser_than = MDBX_TO_EXACT_KEY_VALUE_LESSER_THAN,
    multi_exactkey_value_lesser_or_equal = MDBX_TO_EXACT_KEY_VALUE_LESSER_OR_EQUAL,
    multi_exactkey_value_equal = MDBX_TO_EXACT_KEY_VALUE_EQUAL,
    multi_exactkey_value_greater_or_equal = MDBX_TO_EXACT_KEY_VALUE_GREATER_OR_EQUAL,
    multi_exactkey_value_greater = MDBX_TO_EXACT_KEY_VALUE_GREATER_THAN,

    pair_lesser_than = MDBX_TO_PAIR_LESSER_THAN,
    pair_lesser_or_equal = MDBX_TO_PAIR_LESSER_OR_EQUAL,
    pair_equal = MDBX_TO_PAIR_EQUAL,
    pair_exact = pair_equal,
    pair_greater_or_equal = MDBX_TO_PAIR_GREATER_OR_EQUAL,
    pair_greater_than = MDBX_TO_PAIR_GREATER_THAN,

    batch_samelength = MDBX_GET_MULTIPLE,
    batch_samelength_next = MDBX_NEXT_MULTIPLE,
    batch_samelength_previous = MDBX_PREV_MULTIPLE,
    seek_and_batch_samelength = MDBX_SEEK_AND_GET_MULTIPLE
  };

  // TODO: добавить легковесный proxy-класс для замещения параметра throw_notfound более сложным набором опций,
  //       в том числе с explicit-конструктором из bool, чтобы защититься от неявной конвертации ключей поиска
  //       и других параметров в bool-throw_notfound.

  /// \brief The result of a cursor move/seek operation: the found key-value
  /// pair plus the success flag.
  struct move_result : public pair_result {
    /// \brief Reads the current position of the given cursor.
    /// \param cursor         The cursor to read the position from.
    /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
    inline move_result(const cursor &cursor, bool throw_notfound);
    /// \brief Performs the given positioning operation.
    /// \param cursor         The cursor to be moved.
    /// \param operation      The positioning operation.
    /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
    move_result(cursor &cursor, move_operation operation, bool throw_notfound)
        : move_result(cursor, operation, slice::invalid(), slice::invalid(), throw_notfound) {}
    /// \brief Performs the given positioning operation at the specified key.
    /// \param cursor         The cursor to be moved.
    /// \param operation      The positioning operation.
    /// \param key            The key to position at.
    /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
    move_result(cursor &cursor, move_operation operation, const slice &key, bool throw_notfound)
        : move_result(cursor, operation, key, slice::invalid(), throw_notfound) {}
    /// \brief Performs the given positioning operation at the specified key-value pair.
    /// \param cursor         The cursor to be moved.
    /// \param operation      The positioning operation.
    /// \param key            The key to position at.
    /// \param value          The value to position at (for dupsort tables).
    /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
    inline move_result(cursor &cursor, move_operation operation, const slice &key, const slice &value,
                       bool throw_notfound);
    /// \brief Copy constructor.
    move_result(const move_result &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    move_result &operator=(const move_result &) noexcept = default;
  };

  /// \brief The result of a range estimation: the boundary key-value pair plus
  /// the approximate number of items.
  struct estimate_result : public pair {
    ptrdiff_t approximate_quantity; ///< \brief The approximate number of items in the range.
    /// \brief Estimates the number of items from the current position per the given operation.
    /// \param cursor     The positioned cursor.
    /// \param operation  The positioning operation.
    estimate_result(const cursor &cursor, move_operation operation)
        : estimate_result(cursor, operation, slice::invalid(), slice::invalid()) {}
    /// \brief Estimates the number of items from the given key per the given operation.
    /// \param cursor     The positioned cursor.
    /// \param operation  The positioning operation.
    /// \param key        The boundary key.
    estimate_result(const cursor &cursor, move_operation operation, const slice &key)
        : estimate_result(cursor, operation, key, slice::invalid()) {}
    /// \brief Estimates the number of items from the given key-value pair per the given operation.
    /// \param cursor     The positioned cursor.
    /// \param operation  The positioning operation.
    /// \param key        The boundary key.
    /// \param value      The boundary value (for dupsort tables).
    inline estimate_result(const cursor &cursor, move_operation operation, const slice &key, const slice &value);
    /// \brief Copy constructor.
    estimate_result(const estimate_result &) noexcept = default;
    /// \brief Copy assignment operator.
    /// \returns A reference to this instance.
    estimate_result &operator=(const estimate_result &) noexcept = default;
  };

protected:
  /* fake `const`, but for specific move/get operations */
  /// \brief The low-level positioning primitive.
  /// \param operation      The positioning operation.
  /// \param key            The key argument (in/out).
  /// \param value          The value argument (in/out).
  /// \param throw_notfound Throw \ref mdbx::not_found instead of returning `false`.
  /// \returns `true` if the cursor was positioned successfully.
  /// \throws The corresponding \ref mdbx::exception on failure.
  inline bool move(move_operation operation, MDBX_val *key, MDBX_val *value, bool throw_notfound) const;

  /// \brief The low-level range estimation primitive.
  /// \param operation  The positioning operation.
  /// \param key        The boundary key (in/out).
  /// \param value      The boundary value (in/out).
  /// \returns The approximate number of items in the range.
  /// \throws The corresponding \ref mdbx::exception on failure.
  inline ptrdiff_t estimate(move_operation operation, MDBX_val *key, MDBX_val *value) const;

public:
  /// \brief Scans items calling the predicate until it returns `true`.
  /// \param predicate  The callable invoked for each key-value pair; returning
  ///                     `true` stops the scan.
  /// \param start      The initial positioning operation.
  /// \param turn       The operation for each next step.
  /// \returns `true` if the predicate returned `true` for some item.
  /// \throws The captured predicate exception, if any.
  template <typename CALLABLE_PREDICATE>
  bool scan_until(CALLABLE_PREDICATE predicate, move_operation start = first, move_operation turn = next) {
    struct wrapper : public exception_thunk {
      static int probe(void *context, MDBX_val *key, MDBX_val *value, void *arg) noexcept {
        auto thunk = static_cast<wrapper *>(context);
        assert(thunk->is_clean());
        auto &predicate = *static_cast<CALLABLE_PREDICATE *>(arg);
        try {
          return predicate(pair(*key, *value)) ? MDBX_RESULT_TRUE : MDBX_RESULT_FALSE;
        } catch (... /* capture any exception to rethrow it over C code */) {
          thunk->capture();
          return MDBX_RESULT_TRUE;
        }
      }
    } thunk;
    return error::boolean_or_throw(
        ::mdbx_cursor_scan(handle_, wrapper::probe, &thunk, MDBX_cursor_op(start), MDBX_cursor_op(turn), &predicate),
        thunk);
  }

  /// \brief Scans the whole table calling the predicate until it returns `true`.
  /// \param predicate  The callable invoked for each key-value pair; returning
  ///                     `true` stops the scan.
  /// \param backward   Scan in the reverse order.
  /// \returns `true` if the predicate returned `true` for some item.
  /// \throws The captured predicate exception, if any.
  template <typename CALLABLE_PREDICATE> bool fullscan(CALLABLE_PREDICATE predicate, bool backward = false) {
    return scan_until(std::move(predicate), backward ? last : first, backward ? previous : next);
  }

  /// \brief Scans items starting from the given key, calling the predicate until
  /// it returns `true`.
  /// \param predicate  The callable invoked for each key-value pair; returning
  ///                     `true` stops the scan.
  /// \param from       The starting key; updated to the position where the scan stopped.
  /// \param start      The initial positioning operation.
  /// \param turn       The operation for each next step.
  /// \returns `true` if the predicate returned `true` for some item.
  /// \throws The captured predicate exception, if any.
  template <typename CALLABLE_PREDICATE>
  bool scan_until_from(CALLABLE_PREDICATE predicate, slice &from, move_operation start = key_greater_or_equal,
                       move_operation turn = next) {
    struct wrapper : public exception_thunk {
      static int probe(void *context, MDBX_val *key, MDBX_val *value, void *arg) noexcept {
        auto thunk = static_cast<wrapper *>(context);
        assert(thunk->is_clean());
        auto &predicate = *static_cast<CALLABLE_PREDICATE *>(arg);
        try {
          return predicate(pair(*key, *value)) ? MDBX_RESULT_TRUE : MDBX_RESULT_FALSE;
        } catch (... /* capture any exception to rethrow it over C code */) {
          thunk->capture();
          return MDBX_RESULT_TRUE;
        }
      }
    } thunk;
    return error::boolean_or_throw(::mdbx_cursor_scan_from(handle_, wrapper::probe, &thunk, MDBX_cursor_op(start),
                                                           &from, nullptr, MDBX_cursor_op(turn), &predicate),
                                   thunk);
  }

  /// \brief Scans items starting from the given key-value pair, calling the
  /// predicate until it returns `true`.
  /// \param predicate  The callable invoked for each key-value pair; returning
  ///                     `true` stops the scan.
  /// \param from       The starting pair; updated to the position where the scan stopped.
  /// \param start      The initial positioning operation.
  /// \param turn       The operation for each next step.
  /// \returns `true` if the predicate returned `true` for some item.
  /// \throws The captured predicate exception, if any.
  template <typename CALLABLE_PREDICATE>
  bool scan_until_from(CALLABLE_PREDICATE predicate, pair &from, move_operation start = pair_greater_or_equal,
                       move_operation turn = next) {
    struct wrapper : public exception_thunk {
      static int probe(void *context, MDBX_val *key, MDBX_val *value, void *arg) noexcept {
        auto thunk = static_cast<wrapper *>(context);
        assert(thunk->is_clean());
        auto &predicate = *static_cast<CALLABLE_PREDICATE *>(arg);
        try {
          return predicate(pair(*key, *value)) ? MDBX_RESULT_TRUE : MDBX_RESULT_FALSE;
        } catch (... /* capture any exception to rethrow it over C code */) {
          thunk->capture();
          return MDBX_RESULT_TRUE;
        }
      }
    } thunk;
    return error::boolean_or_throw(::mdbx_cursor_scan_from(handle_, wrapper::probe, &thunk, MDBX_cursor_op(start),
                                                           &from.key, &from.value, MDBX_cursor_op(turn), &predicate),
                                   thunk);
  }

  /// \brief Performs the given positioning operation.
  /// \param operation      The positioning operation.
  /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result move(move_operation operation, bool throw_notfound) {
    return move_result(*this, operation, throw_notfound);
  }
  /// \brief Performs the given positioning operation at the specified key.
  /// \param operation      The positioning operation.
  /// \param key            The key to position at.
  /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result move(move_operation operation, const slice &key, bool throw_notfound) {
    return move_result(*this, operation, key, slice::invalid(), throw_notfound);
  }
  /// \brief Performs the given positioning operation at the specified key-value pair.
  /// \param operation      The positioning operation.
  /// \param key            The key to position at.
  /// \param value          The value to position at (for dupsort tables).
  /// \param throw_notfound Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result move(move_operation operation, const slice &key, const slice &value, bool throw_notfound) {
    return move_result(*this, operation, key, value, throw_notfound);
  }
  /// \brief Deleted: `throw_notfound` must be specified explicitly to avoid
  /// accidental boolean conversions.
  move_result move(move_operation operation, const slice &key, const slice &value) = delete;
  /// \brief Performs the given positioning operation updating the key and value slices.
  /// \param operation      The positioning operation.
  /// \param key            The key slice to be updated with the found key.
  /// \param value          The value slice to be updated with the found value.
  /// \param throw_notfound Throw \ref mdbx::not_found instead of returning `false`.
  /// \returns `true` if the cursor was positioned successfully.
  template <class SLICE, typename ::std::enable_if<::std::is_same<SLICE, slice>::value, int>::type = 0>
  bool move(move_operation operation, SLICE &key, SLICE &value, bool throw_notfound) {
    return move(operation, &key, &value, throw_notfound);
  }

  /// \brief Position the cursor at the first key/data item.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_first(bool throw_notfound = true) { return move(first, throw_notfound); }
  /// \brief Position the cursor at the previous key/data item.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_previous(bool throw_notfound = true) { return move(previous, throw_notfound); }
  /// \brief Position the cursor at the last multi-value of the previous key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_previous_last_multi(bool throw_notfound = true) {
    return move(multi_prevkey_lastvalue, throw_notfound);
  }
  /// \brief Position the cursor at the first multi-value of the current key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_current_first_multi(bool throw_notfound = true) {
    return move(multi_currentkey_firstvalue, throw_notfound);
  }
  /// \brief Position the cursor at the previous multi-value of the current key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_current_prev_multi(bool throw_notfound = true) {
    return move(multi_currentkey_prevvalue, throw_notfound);
  }
  /// \brief Read the current key/data item without moving.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result current(bool throw_notfound = true) const { return move_result(*this, throw_notfound); }
  /// \brief Position the cursor at the next multi-value of the current key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_current_next_multi(bool throw_notfound = true) {
    return move(multi_currentkey_nextvalue, throw_notfound);
  }
  /// \brief Position the cursor at the last multi-value of the current key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_current_last_multi(bool throw_notfound = true) {
    return move(multi_currentkey_lastvalue, throw_notfound);
  }
  /// \brief Position the cursor at the first multi-value of the next key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_next_first_multi(bool throw_notfound = true) { return move(multi_nextkey_firstvalue, throw_notfound); }
  /// \brief Position the cursor at the next key/data item.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_next(bool throw_notfound = true) { return move(next, throw_notfound); }
  /// \brief Position the cursor at the last key/data item.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_last(bool throw_notfound = true) { return move(last, throw_notfound); }

  /// \brief Position the cursor at the key lesser than the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_lesser_than(const slice &key, bool throw_notfound = true) {
    return move(key_lesser_than, key, throw_notfound);
  }
  /// \brief Position the cursor at the key lesser than or equal to the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_lesser_or_equal(const slice &key, bool throw_notfound = true) {
    return move(key_lesser_or_equal, key, throw_notfound);
  }
  /// \brief Position the cursor at the key equal to the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_equal(const slice &key, bool throw_notfound = true) {
    return move(key_equal, key, throw_notfound);
  }
  /// \brief Position the cursor at the key exactly matching the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_exact(const slice &key, bool throw_notfound = true) {
    return move(key_exact, key, throw_notfound);
  }
  /// \brief Position the cursor at the key greater than or equal to the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_greater_or_equal(const slice &key, bool throw_notfound = true) {
    return move(key_greater_or_equal, key, throw_notfound);
  }
  /// \brief Position the cursor at the key greater than the given one.
  /// \param key             The key to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_key_greater_than(const slice &key, bool throw_notfound = true) {
    return move(key_greater_than, key, throw_notfound);
  }

  /// \brief Position the cursor at the exact key and the multi-value lesser than the given one.
  /// \param key             The exact key to position at.
  /// \param value           The multi-value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_exact_key_value_lesser_than(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(multi_exactkey_value_lesser_than, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the exact key and the multi-value lesser than or equal to the given one.
  /// \param key             The exact key to position at.
  /// \param value           The multi-value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_exact_key_value_lesser_or_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(multi_exactkey_value_lesser_or_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the exact key and the multi-value equal to the given one.
  /// \param key             The exact key to position at.
  /// \param value           The multi-value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_exact_key_value_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(multi_exactkey_value_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the exact key and the multi-value greater than or equal to the given one.
  /// \param key             The exact key to position at.
  /// \param value           The multi-value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_exact_key_value_greater_or_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(multi_exactkey_value_greater_or_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the exact key and the multi-value greater than the given one.
  /// \param key             The exact key to position at.
  /// \param value           The multi-value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_exact_key_value_greater_than(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(multi_exactkey_value_greater, key, value, throw_notfound);
  }

  /// \brief Position the cursor at the key-value pair lesser than the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_lesser_than(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_lesser_than, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the key-value pair lesser than or equal to the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_lesser_or_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_lesser_or_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the key-value pair equal to the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the key-value pair exactly matching the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_exact(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_exact, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the key-value pair greater than or equal to the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_greater_or_equal(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_greater_or_equal, key, value, throw_notfound);
  }
  /// \brief Position the cursor at the key-value pair greater than the given one.
  /// \param key             The key to position against.
  /// \param value           The value to position against.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  move_result to_pair_greater_than(const slice &key, const slice &value, bool throw_notfound = true) {
    return move(pair_greater_than, key, value, throw_notfound);
  }

  /// \brief Seeks the cursor to the exact key.
  /// \param key  The key to seek.
  /// \returns `true` if the exact key was found.
  inline bool seek(const slice &key);
  /// \brief Finds the exact key.
  /// \param key             The key to find.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result find(const slice &key, bool throw_notfound = true);
  /// \brief Finds the exact key or the first one greater than it.
  /// \param key             The key to find.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result lower_bound(const slice &key, bool throw_notfound = false);
  /// \brief Finds the first key greater than the given one.
  /// \param key             The key to find the upper bound for.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result upper_bound(const slice &key, bool throw_notfound = false);

  /// \brief Return count of duplicates for current key.
  /// \returns The number of multi-values at the current key.
  inline size_t count_multivalue() const;
  /// \brief Return count of duplicates for current key, along with basic
  /// statistics of the table. \see ::mdbx_cursor_count_ex()
  /// \param stat  Optional address of an \ref MDBX_stat structure to be filled.
  /// \returns The number of multi-values at the current key.
  inline size_t count_multivalue(MDBX_stat *stat) const;

  /// \brief Reads a batch of key-value pairs.
  ///
  /// \details Fetches up to `max_pairs` key-value pairs starting from the
  /// current cursor position, advancing the cursor. The returned slices
  /// reference database-owned memory and remain valid while the transaction
  /// and cursor are alive.
  ///
  /// \param [in] max_pairs  The maximum number of key-value pairs to fetch.
  /// \param [in] op         The positioning operation, either
  ///                        `move_operation::first` or `move_operation::next`.
  /// \param [out] is_last   Optional flag set to `true` when the returned chunk
  ///                        is the last one (no pairs left after it).
  ///
  /// \returns The vector of fetched key-value pairs.
  /// \see ::mdbx_cursor_get_batch()
  inline ::std::vector<pair> get_batch(size_t max_pairs, move_operation op, bool *is_last = nullptr) const;

  /// \brief Disables the control of the order of keys when reading database
  /// pages for this cursor.
  ///
  /// \details Useful when reading a database whose user-defined comparison
  /// functions are unavailable, otherwise such reads return \ref MDBX_CORRUPTED.
  /// \see ::mdbx_cursor_ignord()
  inline void ignore_key_order();

  /// \brief Finds the exact key-value pair in a dupsort table.
  /// \param key             The key to find.
  /// \param value           The multi-value to find.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result find_multivalue(const slice &key, const slice &value, bool throw_notfound = true);
  /// \brief Finds the exact key and the first multi-value greater-or-equal to the given one.
  /// \param key             The key to find.
  /// \param value           The multi-value lower bound.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result lower_bound_multivalue(const slice &key, const slice &value, bool throw_notfound = false);
  /// \brief Finds the exact key and the first multi-value greater than the given one.
  /// \param key             The key to find.
  /// \param value           The multi-value to find the upper bound for.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the found pair and the success flag.
  inline move_result upper_bound_multivalue(const slice &key, const slice &value, bool throw_notfound = false);

  /// \brief Seeks and returns a batch of same-length multi-values of the given key.
  /// \param key             The key of the multi-value hive.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the batch and the success flag.
  inline move_result seek_multiple_samelength(const slice &key, bool throw_notfound = true) {
    return move(seek_and_batch_samelength, key, throw_notfound);
  }

  /// \brief Returns a batch of same-length multi-values at the current key.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the batch and the success flag.
  inline move_result get_multiple_samelength(bool throw_notfound = false) {
    return move(batch_samelength, throw_notfound);
  }

  /// \brief Moves to the next key and returns its batch of same-length multi-values.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the batch and the success flag.
  inline move_result next_multiple_samelength(bool throw_notfound = false) {
    return move(batch_samelength_next, throw_notfound);
  }

  /// \brief Moves to the previous key and returns its batch of same-length multi-values.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning an unsuccessful result.
  /// \returns The \ref move_result with the batch and the success flag.
  inline move_result previous_multiple_samelength(bool throw_notfound = false) {
    return move(batch_samelength_previous, throw_notfound);
  }

  /// \brief Checks whether the cursor is at the end-of-table (not positioned).
  /// \returns `true` if the cursor is not positioned on any item.
  inline bool eof() const;
  /// \brief Checks whether the cursor is on the first item of the table.
  /// \returns `true` if the cursor is on the first item.
  inline bool on_first() const;
  /// \brief Checks whether the cursor is on the last item of the table.
  /// \returns `true` if the cursor is on the last item.
  inline bool on_last() const;
  /// \brief Checks whether the cursor is on the first multi-value of the current key.
  /// \returns `true` if the cursor is on the first multi-value.
  inline bool on_first_multival() const;
  /// \brief Checks whether the cursor is on the last multi-value of the current key.
  /// \returns `true` if the cursor is on the last multi-value.
  inline bool on_last_multival() const;
  /// \brief Estimates the number of items between the current position and the given key-value pair.
  /// \param key    The boundary key.
  /// \param value  The boundary value.
  /// \returns The \ref estimate_result with the approximate quantity.
  inline estimate_result estimate(const slice &key, const slice &value) const;
  /// \brief Estimates the number of items between the current position and the given key.
  /// \param key  The boundary key.
  /// \returns The \ref estimate_result with the approximate quantity.
  inline estimate_result estimate(const slice &key) const;
  /// \brief Estimates the number of items up to the boundary defined by the operation.
  /// \param operation  The positioning operation defining the boundary.
  /// \returns The \ref estimate_result with the approximate quantity.
  inline estimate_result estimate(move_operation operation) const;
  /// \brief Estimates the number of items up to the boundary defined by the operation and key.
  /// \param operation  The positioning operation defining the boundary.
  /// \param key        The boundary key.
  /// \returns The \ref estimate_result with the approximate quantity.
  inline estimate_result estimate(move_operation operation, const slice &key) const;

  /// \brief Estimates the distance (number of items) between two cursor positions.
  /// \param from      The cursor at the "from" position, null means the table start.
  /// \param to        The cursor at the "to" position, null means the table end.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns The approximate distance; a negative value means `to` is before `from`.
  static inline ptrdiff_t distance_between(const cursor from, const cursor to,
                                           unsigned deepness = /* enough to cover whole tree height */ 42);
  /// \brief Estimates the distance from the given cursor position to this one.
  /// \param from      The cursor at the "from" position, null means the table start.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns The approximate distance.
  inline ptrdiff_t distance_from(const cursor from,
                                 unsigned deepness = /* enough to cover whole tree height */ 42) const {
    return distance_between(from, *this, deepness);
  }
  /// \brief Estimates the distance from this cursor position to the given one.
  /// \param to        The cursor at the "to" position, null means the table end.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns The approximate distance.
  inline ptrdiff_t distance_to(const cursor to, unsigned deepness = /* enough to cover whole tree height */ 42) const {
    return distance_between(*this, to, deepness);
  }
  /// \brief Estimates the distance from the first item of the table to this cursor position.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns The approximate distance.
  inline ptrdiff_t distance_from_first(unsigned deepness = /* enough to cover whole tree height */ 42) const {
    return distance_between(nullptr, *this, deepness);
  }
  /// \brief Estimates the distance from this cursor position to the end of the table.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns The approximate distance.
  inline ptrdiff_t distance_to_end(unsigned deepness = /* enough to cover whole tree height */ 42) const {
    return distance_between(*this, nullptr, deepness);
  }

  /// \brief Scrolls the cursor to the given distance from the current position.
  /// \param distable        The signed number of items to scroll by.
  /// \param deepness        The maximum depth of the B+tree inspection.
  /// \param throw_notfound  Throw \ref mdbx::not_found instead of returning `false`.
  /// \returns `true` if the cursor was positioned successfully.
  inline bool scroll(intptr_t distable, unsigned deepness = /* enough to cover whole tree height */ 42,
                     bool throw_notfound = true);

  /// \brief Distributes cursors evenly over the range between two positions.
  /// \param from               The cursor at the start of the range, null means the table start.
  /// \param to                 The cursor at the end of the range, null means the table end.
  /// \param cursors_array      The array of cursors to be positioned.
  /// \param cursors_array_size The number of cursors in the array.
  /// \param deepness           The maximum depth of the B+tree inspection.
  /// \returns `true` on success.
  static inline bool distribute(const cursor from, const cursor to, cursor *cursors_array, intptr_t cursors_array_size,
                                unsigned deepness = /* enough to cover whole tree height */ 42);

  /// \brief Distributes cursors evenly over the range between two positions.
  /// \param from      The cursor at the start of the range, null means the table start.
  /// \param to        The cursor at the end of the range, null means the table end.
  /// \param cursors   The vector of cursors to be positioned.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns `true` on success.
  static inline bool distribute(const cursor from, const cursor to, const std::vector<cursor> &cursors,
                                unsigned deepness = /* enough to cover whole tree height */ 42);

  /// \brief Distributes cursors evenly over the range between two positions.
  /// \param from      The cursor at the start of the range, null means the table start.
  /// \param to        The cursor at the end of the range, null means the table end.
  /// \param cursors   The vector of managed cursors to be positioned.
  /// \param deepness  The maximum depth of the B+tree inspection.
  /// \returns `true` on success.
  static inline bool distribute(const cursor from, const cursor to, const std::vector<cursor_managed> &cursors,
                                unsigned deepness = /* enough to cover whole tree height */ 42);

  //----------------------------------------------------------------------------

  /// \brief Renew/bind a cursor with a new transaction and previously used key-value map handle.
  /// \param txn  The new transaction to attach the cursor to.
  /// \throws mdbx::error on failure. After \ref unbind() or after
  ///   `txn::unbind_all_cursors()` this is how the cursor becomes usable again.
  inline void renew(::mdbx::txn &txn);

  /// \brief Bind/renew a cursor with a new transaction and specified key-value map handle.
  /// \param txn          The new transaction to attach the cursor to.
  /// \param map_handle   The key-value map (table) handle.
  /// \throws mdbx::error on failure (issue #146).
  inline void bind(::mdbx::txn &txn, ::mdbx::map_handle map_handle);

  /// \brief Unbind cursor from a transaction.
  /// \details Detaches the cursor from its transaction without closing the
  ///   underlying C object: the wrapper keeps the handle and the destructor
  ///   still closes it. While unbound, any seek/move/read operation fails with
  ///   \ref MDBX_EBADSIGN or \ref MDBX_EINVAL; call \ref renew() / \ref bind()
  ///   to attach the cursor to a transaction again (issue #146).
  inline void unbind();

  /// \brief Returns the cursor's transaction.
  /// \returns The transaction the cursor belongs to.
  inline ::mdbx::txn txn() const;
  /// \brief Returns the cursor's table (map) handle.
  /// \returns The \ref map_handle the cursor is associated with.
  inline map_handle map() const;

  /// \brief Converts the cursor to its transaction.
  /// \returns The transaction the cursor belongs to.
  inline operator ::mdbx::txn() const { return txn(); }
  /// \brief Converts the cursor to its table handle.
  /// \returns The \ref map_handle the cursor is associated with.
  inline operator ::mdbx::map_handle() const { return map(); }

  /// \brief Stores a key-value pair using the given low-level flags.
  /// \param key    The key to store.
  /// \param value  The value to store (in/out for some flags).
  /// \param flags  The \ref MDBX_put_flags_t combination.
  /// \returns An error code, zero on success (does not throw).
  inline MDBX_error_t put(const slice &key, slice *value, MDBX_put_flags_t flags) noexcept;
  /// \brief Stores a key-value pair in the given mode.
  /// \param key    The key to store.
  /// \param value  The value to store.
  /// \param mode   The \ref put_mode (insert/upsert/update).
  /// \throws mdbx::error on failure.
  inline void put(const slice &key, slice value, put_mode mode);
  /// \brief Inserts a new key-value pair; fails if the key already exists.
  /// \param key    The key to insert.
  /// \param value  The value to insert.
  /// \throws mdbx::key_exists if the key already exists.
  inline void insert(const slice &key, slice value);
  /// \brief Tries to insert a new key-value pair without throwing on a duplicate.
  /// \param key    The key to insert.
  /// \param value  The value to insert.
  /// \returns The \ref value_result with the success flag.
  inline value_result try_insert(const slice &key, slice value);
  /// \brief Reserves space for a value of the given length for a new key.
  /// \param key           The key to insert.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  /// \throws mdbx::key_exists if the key already exists.
  inline slice insert_reserve(const slice &key, size_t value_length);
  /// \brief Tries to reserve space for a value of the given length for a new key.
  /// \param key           The key to insert.
  /// \param value_length  The length of the value to reserve.
  /// \returns The \ref value_result with the reserved slice and the success flag.
  inline value_result try_insert_reserve(const slice &key, size_t value_length);

  /// \brief Inserts or updates the value for the given key.
  /// \param key    The key to store.
  /// \param value  The value to store.
  /// \throws mdbx::error on failure.
  inline void upsert(const slice &key, const slice &value);
  /// \brief Reserves space for a value of the given length, inserting or updating the key.
  /// \param key           The key to store.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  inline slice upsert_reserve(const slice &key, size_t value_length);

  /// \brief Updates value associated with a key at the current cursor position.
  /// \param value  The new value.
  void update_current(const slice &value);
  /// \brief Reserves and returns the space for storing a value associated with a key at the current cursor position.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  slice reserve_current(size_t value_length);
  /// \brief Deprecated misspelling of \ref reserve_current().
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  slice reverse_current(size_t value_length) { return reserve_current(value_length); }

  /// \brief Updates the value for an existing key; fails if the key is absent.
  /// \param key    The key to update.
  /// \param value  The new value.
  /// \throws mdbx::not_found if the key does not exist.
  inline void update(const slice &key, const slice &value);
  /// \brief Tries to update the value for an existing key without throwing.
  /// \param key    The key to update.
  /// \param value  The new value.
  /// \returns `true` if the key was found and updated.
  inline bool try_update(const slice &key, const slice &value);
  /// \brief Reserves space for a value of the given length for an existing key.
  /// \param key           The key to update.
  /// \param value_length  The length of the value to reserve.
  /// \returns The writable slice for the reserved value.
  /// \throws mdbx::not_found if the key does not exist.
  inline slice update_reserve(const slice &key, size_t value_length);
  /// \brief Tries to reserve space for a value of the given length for an existing key.
  /// \param key           The key to update.
  /// \param value_length  The length of the value to reserve.
  /// \returns The \ref value_result with the reserved slice and the success flag.
  inline value_result try_update_reserve(const slice &key, size_t value_length);

  /// \brief Stores a key-value pair in the given mode.
  /// \param kv    The key-value pair.
  /// \param mode  The \ref put_mode (insert/upsert/update).
  void put(const pair &kv, put_mode mode) { return put(kv.key, kv.value, mode); }
  /// \brief Inserts a new key-value pair; fails if the key already exists.
  /// \param kv  The key-value pair to insert.
  void insert(const pair &kv) { return insert(kv.key, kv.value); }
  /// \brief Tries to insert a new key-value pair without throwing on a duplicate.
  /// \param kv  The key-value pair to insert.
  /// \returns The \ref value_result with the success flag.
  value_result try_insert(const pair &kv) { return try_insert(kv.key, kv.value); }
  /// \brief Inserts or updates the value for the given key.
  /// \param kv  The key-value pair to store.
  void upsert(const pair &kv) { return upsert(kv.key, kv.value); }

  /// \brief Removes single key-value pair or all multi-values at the current cursor position.
  /// \param whole_multivalue  Remove all multi-values of the current key.
  /// \returns `true` if something was removed.
  inline bool erase(bool whole_multivalue = false);

  /// \brief Seeks and removes first value or whole multi-value of the given key.
  /// \param key               The key to remove.
  /// \param whole_multivalue  Remove all multi-values of the key.
  /// \return `True` if the key is found and a value(s) is removed.
  inline bool erase(const slice &key, bool whole_multivalue = true);

  /// \brief Seeks and removes the particular multi-value entry of the key.
  /// \param key    The key of the entry.
  /// \param value  The multi-value to remove.
  /// \return `True` if the given key-value pair is found and removed.
  inline bool erase(const slice &key, const slice &value);

  /// \brief Overloads for C strings, which otherwise bind to the `bool` parameters above.
  /// \param key               The key to remove.
  /// \param whole_multivalue  Remove all multi-values of the key.
  /// \returns `true` if the key was found and removed.
  template <typename CHARS, typename BOOL = bool,
            typename std::enable_if<(std::is_same<CHARS, const char *>::value || std::is_same<CHARS, char *>::value) &&
                                        std::is_same<BOOL, bool>::value,
                                    int>::type = 0>
  bool erase(CHARS key, BOOL whole_multivalue = true) {
    return erase(slice(key), whole_multivalue);
  }
  /// \brief Removes the particular multi-value entry given by C string literals.
  /// \param key    The key of the entry.
  /// \param value  The multi-value to remove.
  /// \returns `true` if the key-value pair was found and removed.
  template <size_t SIZE> bool erase(const slice &key, const char (&value)[SIZE]) { return erase(key, slice(value)); }
  /// \brief Removes the particular multi-value entry given by C string literals.
  /// \param key    The key of the entry.
  /// \param value  The multi-value to remove.
  /// \returns `true` if the key-value pair was found and removed.
  template <size_t KEY_SIZE, size_t VALUE_SIZE>
  bool erase(const char (&key)[KEY_SIZE], const char (&value)[VALUE_SIZE]) {
    return erase(slice(key), slice(value));
  }

  /// \brief Modes for deleting bunches of neighboring items, see
  /// \ref ::MDBX_bunch_action_t(). The EXCLUDING and INCLUDING suffixes mean
  /// correspondingly excluding and including deletion items in the current
  /// cursor position.
  enum bunch_delete {
    delete_current_value = MDBX_DELETE_CURRENT_VALUE,
    delete_current_multival_before_excluding = MDBX_DELETE_CURRENT_MULTIVAL_BEFORE_EXCLUDING,
    delete_current_multival_before_including = MDBX_DELETE_CURRENT_MULTIVAL_BEFORE_INCLUDING,
    delete_current_multival_after_including = MDBX_DELETE_CURRENT_MULTIVAL_AFTER_INCLUDING,
    delete_current_multival_after_excluding = MDBX_DELETE_CURRENT_MULTIVAL_AFTER_EXCLUDING,
    delete_current_multival_all = MDBX_DELETE_CURRENT_MULTIVAL_ALL,
    delete_before_excluding = MDBX_DELETE_BEFORE_EXCLUDING,
    delete_before_including = MDBX_DELETE_BEFORE_INCLUDING,
    delete_after_including = MDBX_DELETE_AFTER_INCLUDING,
    delete_after_excluding = MDBX_DELETE_AFTER_EXCLUDING,
    delete_whole = MDBX_DELETE_WHOLE,
  };

  /// \brief Quickly removes bunches of neighboring items much faster by
  /// cutting out entire pages and branches from the B+ tree structure.
  ///
  /// \param action  The \ref bunch_delete mode defining what to remove.
  /// \returns The number of removed items.
  /// \see erase_range() \see ::mdbx_cursor_bunch_delete()
  inline size_t erase_bunch(bunch_delete action);

  /// \brief Quickly removes a range of items between the current cursor
  /// position and the position of `end` much faster by cutting out entire
  /// pages and branches from the B+ tree structure.
  ///
  /// \param [in] end  A positioned cursor defining the end of the range.
  /// \param [in] end_including  Whether the `end` position itself should be
  /// included in the range to be deleted.
  /// \returns The number of removed items.
  /// \see erase_bunch() \see ::mdbx_cursor_delete_range()
  inline size_t erase_range(const cursor &end, bool end_including);

  /// \brief Resets the cursor state.
  ///
  /// \details As a result of the reset, the cursor becomes unpositioned and
  /// does not allow relative positioning operations, getting or changing data
  /// until the cursor is set to a position independent of the current one.
  /// \see ::mdbx_cursor_reset()
  inline void reset();

  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param key             The key of the multi-value hive.
  /// \param value_length    The length of each value in bytes.
  /// \param values_array    The array of values.
  /// \param values_count    The number of values in the array.
  /// \param mode            The \ref put_mode.
  /// \param allow_partial   Allow partial store when some values are rejected.
  /// \returns The number of stored values.
  inline size_t put_multiple_samelength(const slice &key, const size_t value_length, const void *values_array,
                                        size_t values_count, put_mode mode, bool allow_partial = false);
  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param key             The key of the multi-value hive.
  /// \param values_array    The array of standard-layout values.
  /// \param values_count    The number of values in the array.
  /// \param mode            The \ref put_mode.
  /// \param allow_partial   Allow partial store when some values are rejected.
  /// \returns The number of stored values.
  template <typename VALUE>
  size_t put_multiple_samelength(const slice &key, const VALUE *values_array, size_t values_count, put_mode mode,
                                 bool allow_partial = false) {
    static_assert(::std::is_standard_layout<VALUE>::value && !::std::is_pointer<VALUE>::value &&
                      !::std::is_array<VALUE>::value,
                  "Must be a standard layout type!");
    return put_multiple_samelength(key, sizeof(VALUE), values_array, values_count, mode, allow_partial);
  }
  /// \brief Stores a bunch of same-length multi-values for the given key.
  /// \param key     The key of the multi-value hive.
  /// \param vector  The vector of standard-layout values.
  /// \param mode    The \ref put_mode.
  template <typename VALUE>
  void put_multiple_samelength(const slice &key, const ::std::vector<VALUE> &vector, put_mode mode) {
    put_multiple_samelength(key, vector.data(), vector.size(), mode);
  }
};

/// \brief Managed cursor.
///
/// As other managed classes, `cursor_managed` destroys the represented
/// underlying object from the own class destructor, but disallows copying and
/// assignment for instances.
///
/// \copydetails MDBX_cursor
class LIBMDBX_API_TYPE cursor_managed : public cursor {
  using inherited = cursor;
  friend class txn;
  /// \brief Delegated constructor for RAII, wraps the given C handle.
  /// \param ptr  The pointer to the underlying \ref MDBX_cursor object.
  MDBX_CXX11_CONSTEXPR cursor_managed(MDBX_cursor *ptr) noexcept : inherited(ptr) {}

public:
  /// \brief Creates a new managed cursor with underlying object.
  /// \param your_context  An application context to be associated with the cursor.
  /// \throws mdbx::error on failure to allocate the underlying object.
  cursor_managed(void *your_context = nullptr) : cursor_managed(::mdbx_cursor_create(your_context)) {
    if (MDBX_UNLIKELY(!handle_))
      MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_ENOMEM);
  }

  /// \brief Explicitly closes the cursor.
  void close();

  /// \brief Move constructor.
  cursor_managed(cursor_managed &&) = default;
  /// \brief Move assignment operator.
  /// \param other  The source managed cursor to be moved from.
  /// \returns A reference to this instance.
  cursor_managed &operator=(cursor_managed &&other) noexcept;

  /// \brief Withdraws (takes over) the underlying C handle; this instance
  /// becomes empty and will not close the cursor.
  /// \returns The pointer to the underlying \ref MDBX_cursor object.
  inline MDBX_cursor *withdraw_handle() noexcept {
    MDBX_cursor *handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  /// \brief Copy construction is prohibited.
  cursor_managed(const cursor_managed &) = delete;
  /// \brief Copy assignment is prohibited.
  cursor_managed &operator=(const cursor_managed &) = delete;
  /// \brief Destroys the managed cursor, closing the underlying object.
  ~cursor_managed();
};

/// end of cxx_core @}

//==============================================================================
//
// Inline body of the libmdbx C++ API
//

MDBX_CXX11_CONSTEXPR const version_info &get_version() noexcept { return ::mdbx_version; }
MDBX_CXX11_CONSTEXPR const build_info &get_build() noexcept { return ::mdbx_build; }

static MDBX_CXX17_CONSTEXPR size_t strlen(const char *c_str) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
  if (::std::is_constant_evaluated()) {
    size_t i = 0;
    if (c_str)
      while (c_str[i])
        ++i;
    return i;
  }
#endif /* __cpp_lib_is_constant_evaluated >= 201811 */
#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
  return c_str ? ::std::string_view(c_str).length() : 0;
#else
  return c_str ? ::std::strlen(c_str) : 0;
#endif
}

MDBX_MAYBE_UNUSED static MDBX_CXX20_CONSTEXPR void *memcpy(void *dest, const void *src, size_t bytes) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
  if (::std::is_constant_evaluated()) {
    for (size_t i = 0; i < bytes; ++i)
      static_cast<byte *>(dest)[i] = static_cast<const byte *>(src)[i];
    return dest;
  } else
#endif /* __cpp_lib_is_constant_evaluated >= 201811 */
    return ::std::memcpy(dest, src, bytes);
}

static MDBX_CXX20_CONSTEXPR int memcmp(const void *a, const void *b, size_t bytes) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
  if (::std::is_constant_evaluated()) {
    for (size_t i = 0; i < bytes; ++i) {
      const int diff = int(static_cast<const byte *>(a)[i]) - int(static_cast<const byte *>(b)[i]);
      if (diff)
        return diff;
    }
    return 0;
  } else
#endif /* __cpp_lib_is_constant_evaluated >= 201811 */
    return bytes ? ::std::memcmp(a, b, bytes) : 0;
}

//------------------------------------------------------------------------------

MDBX_CXX11_CONSTEXPR map_handle::info::info(map_handle::flags flags, map_handle::state state) noexcept
    : flags(flags), state(state) {}

MDBX_CXX11_CONSTEXPR_ENUM mdbx::key_mode map_handle::info::key_mode() const noexcept {
  return ::mdbx::key_mode(flags & (MDBX_REVERSEKEY | MDBX_INTEGERKEY));
}

MDBX_CXX11_CONSTEXPR_ENUM mdbx::value_mode map_handle::info::value_mode() const noexcept {
  return ::mdbx::value_mode(flags & (MDBX_DUPSORT | MDBX_REVERSEDUP | MDBX_DUPFIXED | MDBX_INTEGERDUP));
}

inline bool exception_thunk::is_clean() const noexcept { return !captured_; }

inline void exception_thunk::capture() noexcept {
  assert(is_clean());
  captured_ = ::std::current_exception();
}

inline void exception_thunk::rethrow_captured() const {
  if (captured_)
    MDBX_CXX20_UNLIKELY ::std::rethrow_exception(captured_);
}

//------------------------------------------------------------------------------

MDBX_CXX11_CONSTEXPR error::error(MDBX_error_t error_code) noexcept : code_(error_code) {}

inline error &error::operator=(MDBX_error_t error_code) noexcept {
  code_ = error_code;
  return *this;
}

MDBX_CXX11_CONSTEXPR bool operator==(const error &a, const error &b) noexcept { return a.code_ == b.code_; }

MDBX_CXX11_CONSTEXPR bool operator!=(const error &a, const error &b) noexcept { return !(a == b); }

MDBX_CXX11_CONSTEXPR bool error::is_success() const noexcept { return code_ == MDBX_SUCCESS; }

MDBX_CXX11_CONSTEXPR bool error::is_result_true() const noexcept { return code_ == MDBX_RESULT_TRUE; }

MDBX_CXX11_CONSTEXPR bool error::is_result_false() const noexcept { return code_ == MDBX_RESULT_FALSE; }

MDBX_CXX11_CONSTEXPR bool error::is_failure() const noexcept {
  return code_ != MDBX_SUCCESS && code_ != MDBX_RESULT_TRUE;
}

MDBX_CXX11_CONSTEXPR MDBX_error_t error::code() const noexcept { return code_; }

MDBX_CXX11_CONSTEXPR bool error::is_mdbx_error() const noexcept {
  return (code() >= MDBX_FIRST_LMDB_ERRCODE && code() <= MDBX_LAST_LMDB_ERRCODE) ||
         (code() >= MDBX_FIRST_ADDED_ERRCODE && code() <= MDBX_LAST_ADDED_ERRCODE);
}

inline void error::throw_exception(int error_code) {
  const error trouble(static_cast<MDBX_error_t>(error_code));
  trouble.throw_exception();
}

inline void error::throw_on_failure() const {
  if (MDBX_UNLIKELY(is_failure()))
    MDBX_CXX20_UNLIKELY throw_exception();
}

inline void error::success_or_throw() const {
  if (MDBX_UNLIKELY(!is_success()))
    MDBX_CXX20_UNLIKELY throw_exception();
}

inline void error::success_or_throw(const exception_thunk &thunk) const {
  assert(thunk.is_clean() || code() != MDBX_SUCCESS);
  if (MDBX_UNLIKELY(!is_success())) {
    MDBX_CXX20_UNLIKELY if (MDBX_UNLIKELY(!thunk.is_clean())) thunk.rethrow_captured();
    else throw_exception();
  }
}

inline void error::throw_on_nullptr(const void *ptr, MDBX_error_t error_code) {
  if (MDBX_UNLIKELY(ptr == nullptr))
    MDBX_CXX20_UNLIKELY error(error_code).throw_exception();
}

inline void error::throw_on_failure(int error_code) {
  error rc(static_cast<MDBX_error_t>(error_code));
  rc.throw_on_failure();
}

inline void error::success_or_throw(MDBX_error_t error_code) {
  error rc(error_code);
  rc.success_or_throw();
}

inline bool error::boolean_or_throw(int error_code) {
  switch (error_code) {
  case MDBX_RESULT_FALSE:
    return false;
  case MDBX_RESULT_TRUE:
    return true;
  default:
    MDBX_CXX20_UNLIKELY throw_exception(error_code);
  }
}

inline void error::success_or_throw(int error_code, const exception_thunk &thunk) {
  error rc(static_cast<MDBX_error_t>(error_code));
  rc.success_or_throw(thunk);
}

inline bool error::boolean_or_throw(int error_code, const exception_thunk &thunk) {
  if (MDBX_UNLIKELY(!thunk.is_clean()))
    MDBX_CXX20_UNLIKELY thunk.rethrow_captured();
  return boolean_or_throw(error_code);
}

MDBX_CXX11_CONSTEXPR slice::slice() noexcept : ::MDBX_val({nullptr, 0}) {}

MDBX_CXX14_CONSTEXPR slice::slice(const void *ptr, size_t bytes)
    : ::MDBX_val({const_cast<void *>(ptr), ptr ? check_length(bytes) : check_nullptr_length(bytes)}) {}

MDBX_CXX14_CONSTEXPR slice::slice(const void *begin, const void *end)
    : slice(begin, static_cast<const byte *>(end) - static_cast<const byte *>(begin)) {}

MDBX_CXX17_CONSTEXPR slice::slice(const char *c_str) : slice(c_str, strlen(c_str)) {}

MDBX_CXX14_CONSTEXPR slice::slice(const MDBX_val &src) : slice(src.iov_base, src.iov_len) {}

MDBX_CXX14_CONSTEXPR slice::slice(MDBX_val &&src) : slice(src) { src.iov_base = nullptr; }

MDBX_CXX14_CONSTEXPR slice::slice(slice &&src) noexcept : slice(src) { src.invalidate(); }

inline slice &slice::assign(const void *ptr, size_t bytes) {
  iov_base = const_cast<void *>(ptr);
  iov_len = ptr ? check_length(bytes) : check_nullptr_length(bytes);
  return *this;
}

inline slice &slice::assign(const slice &src) noexcept {
  iov_base = src.iov_base;
  iov_len = src.iov_len;
  return *this;
}

inline slice &slice::assign(const ::MDBX_val &src) { return assign(src.iov_base, src.iov_len); }

slice &slice::assign(slice &&src) noexcept {
  assign(src);
  src.invalidate();
  return *this;
}

inline slice &slice::assign(::MDBX_val &&src) {
  assign(src.iov_base, src.iov_len);
  src.iov_base = nullptr;
  return *this;
}

inline slice &slice::assign(const void *begin, const void *end) {
  return assign(begin, static_cast<const byte *>(end) - static_cast<const byte *>(begin));
}

inline slice &slice::assign(const char *c_str) { return assign(c_str, strlen(c_str)); }

inline slice &slice::operator=(slice &&src) noexcept { return assign(::std::move(src)); }

inline slice &slice::operator=(::MDBX_val &&src) { return assign(::std::move(src)); }

MDBX_CXX11_CONSTEXPR const byte *slice::byte_ptr() const noexcept { return static_cast<const byte *>(iov_base); }

MDBX_CXX11_CONSTEXPR const byte *slice::end_byte_ptr() const noexcept { return byte_ptr() + length(); }

MDBX_CXX11_CONSTEXPR byte *slice::byte_ptr() noexcept { return static_cast<byte *>(iov_base); }

MDBX_CXX11_CONSTEXPR byte *slice::end_byte_ptr() noexcept { return byte_ptr() + length(); }

MDBX_CXX11_CONSTEXPR const char *slice::char_ptr() const noexcept { return static_cast<const char *>(iov_base); }

MDBX_CXX11_CONSTEXPR const char *slice::end_char_ptr() const noexcept { return char_ptr() + length(); }

MDBX_CXX11_CONSTEXPR char *slice::char_ptr() noexcept { return static_cast<char *>(iov_base); }

MDBX_CXX11_CONSTEXPR char *slice::end_char_ptr() noexcept { return char_ptr() + length(); }

MDBX_CXX11_CONSTEXPR const void *slice::data() const noexcept { return iov_base; }

MDBX_CXX11_CONSTEXPR const byte *slice::end() const noexcept { return end_byte_ptr(); }

MDBX_CXX11_CONSTEXPR void *slice::data() noexcept { return iov_base; }

MDBX_CXX11_CONSTEXPR byte *slice::end() noexcept { return end_byte_ptr(); }

MDBX_CXX11_CONSTEXPR size_t slice::length() const noexcept { return iov_len; }

MDBX_CXX14_CONSTEXPR slice &slice::set_length(size_t bytes) {
  iov_len = check_length(bytes);
  return *this;
}

MDBX_CXX14_CONSTEXPR slice &slice::set_end(const void *ptr) {
  MDBX_CONSTEXPR_ASSERT(static_cast<const char *>(ptr) >= char_ptr());
  return set_length(static_cast<const char *>(ptr) - char_ptr());
}

MDBX_CXX11_CONSTEXPR bool slice::empty() const noexcept { return length() == 0; }

MDBX_CXX11_CONSTEXPR bool slice::is_null() const noexcept { return data() == nullptr; }

MDBX_CXX11_CONSTEXPR size_t slice::size() const noexcept { return length(); }

MDBX_CXX11_CONSTEXPR slice::operator bool() const noexcept { return !is_null(); }

MDBX_CXX14_CONSTEXPR void slice::invalidate() noexcept { iov_base = nullptr; }

MDBX_CXX14_CONSTEXPR void slice::clear() noexcept {
  iov_base = nullptr;
  iov_len = 0;
}

inline void slice::remove_prefix(size_t n) noexcept {
  assert(n <= size());
  iov_base = static_cast<byte *>(iov_base) + n;
  iov_len -= n;
}

inline void slice::safe_remove_prefix(size_t n) {
  if (MDBX_UNLIKELY(n > size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  remove_prefix(n);
}

inline void slice::remove_suffix(size_t n) noexcept {
  assert(n <= size());
  iov_len -= n;
}

inline void slice::safe_remove_suffix(size_t n) {
  if (MDBX_UNLIKELY(n > size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  remove_suffix(n);
}

MDBX_CXX14_CONSTEXPR bool slice::starts_with(const slice &prefix) const noexcept {
  return length() >= prefix.length() && memcmp(data(), prefix.data(), prefix.length()) == 0;
}

MDBX_CXX14_CONSTEXPR bool slice::ends_with(const slice &suffix) const noexcept {
  return length() >= suffix.length() &&
         memcmp(byte_ptr() + length() - suffix.length(), suffix.data(), suffix.length()) == 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::hash_value() const noexcept {
  size_t h = length() * 3977471;
  for (size_t i = 0; i < length(); ++i)
    h = (h ^ static_cast<const uint8_t *>(data())[i]) * 1664525 + 1013904223;
  return h ^ 3863194411 * (h >> 11);
}

MDBX_CXX11_CONSTEXPR byte slice::operator[](size_t n) const noexcept {
  MDBX_CONSTEXPR_ASSERT(n < size());
  return byte_ptr()[n];
}

MDBX_CXX11_CONSTEXPR byte slice::at(size_t n) const {
  if (MDBX_UNLIKELY(n >= size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  return byte_ptr()[n];
}

MDBX_CXX14_CONSTEXPR slice slice::head(size_t n) const noexcept {
  MDBX_CONSTEXPR_ASSERT(n <= size());
  return slice(data(), n);
}

MDBX_CXX14_CONSTEXPR slice slice::tail(size_t n) const noexcept {
  MDBX_CONSTEXPR_ASSERT(n <= size());
  return slice(char_ptr() + size() - n, n);
}

MDBX_CXX14_CONSTEXPR slice slice::middle(size_t from, size_t n) const noexcept {
  MDBX_CONSTEXPR_ASSERT(from + n <= size());
  return slice(char_ptr() + from, n);
}

MDBX_CXX11_CONSTEXPR const byte *slice::begin() const noexcept { return byte_ptr(); }

MDBX_CXX11_CONSTEXPR byte *slice::begin() noexcept { return byte_ptr(); }

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find(const slice &needle, size_t pos) const noexcept {
  if (MDBX_UNLIKELY(needle.empty()))
    return pos <= length() ? pos : npos;
  if (needle.length() > length() || pos > length() - needle.length())
    return npos;
  for (size_t i = pos;; ++i) {
    if (MDBX_LIKELY(memcmp(byte_ptr() + i, needle.data(), needle.length()) == 0))
      return i;
    if (i == length() - needle.length())
      return npos;
  }
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find(byte c, size_t pos) const noexcept {
  return find(slice(&c, sizeof(c)), pos);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::rfind(const slice &needle, size_t pos) const noexcept {
  if (MDBX_UNLIKELY(needle.empty())) {
    if (pos > length())
      return length();
    return pos;
  }
  if (needle.length() > length())
    return npos;
  if (pos > length() - needle.length())
    pos = length() - needle.length();
  for (size_t i = pos + 1; i > 0; --i) {
    if (MDBX_LIKELY(memcmp(byte_ptr() + i - 1, needle.data(), needle.length()) == 0))
      return i - 1;
  }
  return npos;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::rfind(byte c, size_t pos) const noexcept {
  return rfind(slice(&c, sizeof(c)), pos);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_first_of(const slice &chars,
                                                                            size_t pos) const noexcept {
  if (MDBX_UNLIKELY(chars.empty()))
    return npos;
  for (size_t i = pos; i < length(); ++i) {
    if (MDBX_LIKELY(chars.find(byte_ptr()[i]) != npos))
      return i;
  }
  return npos;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_first_of(byte c, size_t pos) const noexcept {
  return find_first_of(slice(&c, sizeof(c)), pos);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_last_of(const slice &chars,
                                                                           size_t pos) const noexcept {
  if (MDBX_UNLIKELY(chars.empty() || empty()))
    return npos;
  if (pos >= length())
    pos = length() - 1;
  for (size_t i = pos + 1; i > 0; --i) {
    if (MDBX_LIKELY(chars.find(byte_ptr()[i - 1]) != npos))
      return i - 1;
  }
  return npos;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_last_of(byte c, size_t pos) const noexcept {
  return find_last_of(slice(&c, sizeof(c)), pos);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_first_not_of(const slice &chars,
                                                                                size_t pos) const noexcept {
  for (size_t i = pos; i < length(); ++i) {
    if (MDBX_LIKELY(chars.find(byte_ptr()[i]) == npos))
      return i;
  }
  return npos;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_first_not_of(byte c, size_t pos) const noexcept {
  return find_first_not_of(slice(&c, sizeof(c)), pos);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_last_not_of(const slice &chars,
                                                                               size_t pos) const noexcept {
  if (MDBX_UNLIKELY(empty()))
    return npos;
  if (pos >= length())
    pos = length() - 1;
  for (size_t i = pos + 1; i > 0; --i) {
    if (MDBX_LIKELY(chars.find(byte_ptr()[i - 1]) == npos))
      return i - 1;
  }
  return npos;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR size_t slice::find_last_not_of(byte c, size_t pos) const noexcept {
  return find_last_not_of(slice(&c, sizeof(c)), pos);
}

MDBX_CXX14_CONSTEXPR slice slice::safe_head(size_t n) const {
  if (MDBX_UNLIKELY(n > size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  return head(n);
}

MDBX_CXX14_CONSTEXPR slice slice::safe_tail(size_t n) const {
  if (MDBX_UNLIKELY(n > size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  return tail(n);
}

MDBX_CXX14_CONSTEXPR slice slice::safe_middle(size_t from, size_t n) const {
  if (MDBX_UNLIKELY(n > max_length))
    MDBX_CXX20_UNLIKELY throw_max_length_exceeded();
  if (MDBX_UNLIKELY(from > max_length || from + n > size()))
    MDBX_CXX20_UNLIKELY throw_out_range();
  return middle(from, n);
}

MDBX_CXX14_CONSTEXPR intptr_t slice::compare_fast(const slice &a, const slice &b) noexcept {
  const intptr_t diff = intptr_t(a.length()) - intptr_t(b.length());
  return diff                                                     ? diff
         : MDBX_UNLIKELY(a.length() == 0 || a.data() == b.data()) ? 0
                                                                  : memcmp(a.data(), b.data(), a.length());
}

MDBX_CXX14_CONSTEXPR intptr_t slice::compare_lexicographically(const slice &a, const slice &b) noexcept {
  const size_t shortest = ::std::min(a.length(), b.length());
  if (MDBX_LIKELY(shortest > 0))
    MDBX_CXX20_LIKELY {
      const intptr_t diff = memcmp(a.data(), b.data(), shortest);
      if (MDBX_LIKELY(diff != 0))
        MDBX_CXX20_LIKELY return diff;
    }
  return intptr_t(a.length()) - intptr_t(b.length());
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator==(const slice &a, const slice &b) noexcept {
  return slice::compare_fast(a, b) == 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator<(const slice &a, const slice &b) noexcept {
  return slice::compare_lexicographically(a, b) < 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator>(const slice &a, const slice &b) noexcept {
  return slice::compare_lexicographically(a, b) > 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator<=(const slice &a, const slice &b) noexcept {
  return slice::compare_lexicographically(a, b) <= 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator>=(const slice &a, const slice &b) noexcept {
  return slice::compare_lexicographically(a, b) >= 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator!=(const slice &a, const slice &b) noexcept {
  return slice::compare_fast(a, b) != 0;
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR auto operator<=>(const slice &a, const slice &b) noexcept {
  const auto cmp = slice::compare_lexicographically(a, b);
  return (cmp < 0)   ? std::strong_ordering::less
         : (cmp > 0) ? std::strong_ordering::greater
                     : std::strong_ordering::equal;
}
#endif /* __cpp_impl_three_way_comparison */

template <class ALLOCATOR, typename CAPACITY_POLICY>
MDBX_CXX20_CONSTEXPR buffer<ALLOCATOR, CAPACITY_POLICY>::buffer(const struct slice &src, bool make_reference,
                                                                const allocator_type &alloc)
    : inherited(src), silo_(alloc) {
  if (!make_reference)
    insulate();
}

/// \brief Constructs a buffer owning a clone of the given slice when its data
/// is dirty within the given transaction, otherwise a reference to it.
/// \param transaction  The transaction to check the data dirtiness against.
/// \param src          The source slice.
/// \param alloc        The allocator for the owned storage.
template <class ALLOCATOR, typename CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY>::buffer(const txn &transaction, const struct slice &src,
                                                  const allocator_type &alloc)
    : buffer(src, src.data() && ::mdbx_is_dirty(transaction, src.data()) == MDBX_RESULT_FALSE, alloc) {}

template <class ALLOCATOR, typename CAPACITY_POLICY>
inline ::std::ostream &operator<<(::std::ostream &out, const buffer<ALLOCATOR, CAPACITY_POLICY> &it) {
  return (it.is_freestanding() ? out << "buf-" << it.headroom() << "." << it.tailroom() : out << "ref-") << it.slice();
}

#if !(defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)) || defined(LIBMDBX_EXPORTS)
MDBX_EXTERN_API_TEMPLATE(LIBMDBX_API_TYPE, buffer<legacy_allocator, default_capacity_policy>);

#if MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR
MDBX_EXTERN_API_TEMPLATE(LIBMDBX_API_TYPE, buffer<polymorphic_allocator, default_capacity_policy>);
#endif /* MDBX_CXX_HAS_POLYMORPHIC_ALLOCATOR */
#endif /* !MinGW || MDBX_EXPORTS */

/// \brief Default buffer.
using default_buffer = buffer<default_allocator, default_capacity_policy>;

//------------------------------------------------------------------------------

/// \brief Produces a buffer from a mutable byte-producer (see the
/// \ref MutableByteProducer concept).
/// \param producer  The producer of bytes.
/// \param alloc     The allocator for the resulting buffer.
/// \returns The produced \ref buffer.
template <class ALLOCATOR, class CAPACITY_POLICY, MDBX_CXX20_CONCEPT(MutableByteProducer, PRODUCER)>
inline buffer<ALLOCATOR, CAPACITY_POLICY> make_buffer(PRODUCER &producer, const ALLOCATOR &alloc) {
  if (MDBX_LIKELY(!producer.is_empty()))
    MDBX_CXX20_LIKELY {
      buffer<ALLOCATOR, CAPACITY_POLICY> result(producer.envisage_result_length(), alloc);
      result.set_end(producer.write_bytes(result.end_char_ptr(), result.tailroom()));
      return result;
    }
  return buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

/// \brief Produces a buffer from an immutable byte-producer (see the
/// \ref ImmutableByteProducer concept).
/// \param producer  The producer of bytes.
/// \param alloc     The allocator for the resulting buffer.
/// \returns The produced \ref buffer.
template <class ALLOCATOR, class CAPACITY_POLICY, MDBX_CXX20_CONCEPT(ImmutableByteProducer, PRODUCER)>
inline buffer<ALLOCATOR, CAPACITY_POLICY> make_buffer(const PRODUCER &producer, const ALLOCATOR &alloc) {
  if (MDBX_LIKELY(!producer.is_empty()))
    MDBX_CXX20_LIKELY {
      buffer<ALLOCATOR, CAPACITY_POLICY> result(producer.envisage_result_length(), alloc);
      result.set_end(producer.write_bytes(result.end_char_ptr(), result.tailroom()));
      return result;
    }
  return buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

/// \brief Produces a string from a mutable byte-producer (see the
/// \ref MutableByteProducer concept).
/// \param producer  The producer of bytes.
/// \param alloc     The allocator for the resulting string.
/// \returns The produced \ref string.
template <class ALLOCATOR, MDBX_CXX20_CONCEPT(MutableByteProducer, PRODUCER)>
inline string<ALLOCATOR> make_string(PRODUCER &producer, const ALLOCATOR &alloc) {
  string<ALLOCATOR> result(alloc);
  if (MDBX_LIKELY(!producer.is_empty()))
    MDBX_CXX20_LIKELY {
      result.resize(producer.envisage_result_length());
      result.resize(producer.write_bytes(const_cast<char *>(result.data()), result.capacity()) - result.data());
    }
  return result;
}

/// \brief Produces a string from an immutable byte-producer (see the
/// \ref ImmutableByteProducer concept).
/// \param producer  The producer of bytes.
/// \param alloc     The allocator for the resulting string.
/// \returns The produced \ref string.
template <class ALLOCATOR, MDBX_CXX20_CONCEPT(ImmutableByteProducer, PRODUCER)>
inline string<ALLOCATOR> make_string(const PRODUCER &producer, const ALLOCATOR &alloc) {
  string<ALLOCATOR> result(alloc);
  if (MDBX_LIKELY(!producer.is_empty()))
    MDBX_CXX20_LIKELY {
      result.resize(producer.envisage_result_length());
      result.resize(producer.write_bytes(const_cast<char *>(result.data()), result.capacity()) - result.data());
    }
  return result;
}

template <class ALLOCATOR> string<ALLOCATOR> to_hex::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> to_hex::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR> string<ALLOCATOR> to_base58::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> to_base58::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR> string<ALLOCATOR> to_base64::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> to_base64::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR> string<ALLOCATOR> from_hex::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> from_hex::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR> string<ALLOCATOR> from_base58::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> from_base58::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR> string<ALLOCATOR> from_base64::as_string(const ALLOCATOR &alloc) const {
  return make_string<ALLOCATOR>(*this, alloc);
}

template <class ALLOCATOR, typename CAPACITY_POLICY>
buffer<ALLOCATOR, CAPACITY_POLICY> from_base64::as_buffer(const ALLOCATOR &alloc) const {
  return make_buffer<ALLOCATOR, CAPACITY_POLICY>(*this, alloc);
}

template <class ALLOCATOR>
inline string<ALLOCATOR> slice::as_hex_string(bool uppercase, unsigned wrap_width, const ALLOCATOR &alloc) const {
  return to_hex(*this, uppercase, wrap_width).as_string<ALLOCATOR>(alloc);
}

template <class ALLOCATOR>
inline string<ALLOCATOR> slice::as_base58_string(unsigned wrap_width, const ALLOCATOR &alloc) const {
  return to_base58(*this, wrap_width).as_string<ALLOCATOR>(alloc);
}

template <class ALLOCATOR>
inline string<ALLOCATOR> slice::as_base64_string(unsigned wrap_width, const ALLOCATOR &alloc) const {
  return to_base64(*this, wrap_width).as_string<ALLOCATOR>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::encode_hex(bool uppercase, unsigned wrap_width,
                                                            const ALLOCATOR &alloc) const {
  return to_hex(*this, uppercase, wrap_width).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::encode_base58(unsigned wrap_width, const ALLOCATOR &alloc) const {
  return to_base58(*this, wrap_width).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::encode_base64(unsigned wrap_width, const ALLOCATOR &alloc) const {
  return to_base64(*this, wrap_width).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::hex_decode(bool ignore_spaces, const ALLOCATOR &alloc) const {
  return from_hex(*this, ignore_spaces).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::base58_decode(bool ignore_spaces, const ALLOCATOR &alloc) const {
  return from_base58(*this, ignore_spaces).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

template <class ALLOCATOR, class CAPACITY_POLICY>
inline buffer<ALLOCATOR, CAPACITY_POLICY> slice::base64_decode(bool ignore_spaces, const ALLOCATOR &alloc) const {
  return from_base64(*this, ignore_spaces).as_buffer<ALLOCATOR, CAPACITY_POLICY>(alloc);
}

//------------------------------------------------------------------------------

MDBX_CXX14_CONSTEXPR intptr_t pair::compare_fast(const pair &a, const pair &b) noexcept {
  const auto diff = slice::compare_fast(a.key, b.key);
  return diff ? diff : slice::compare_fast(a.value, b.value);
}

MDBX_CXX14_CONSTEXPR intptr_t pair::compare_lexicographically(const pair &a, const pair &b) noexcept {
  const auto diff = slice::compare_lexicographically(a.key, b.key);
  return diff ? diff : slice::compare_lexicographically(a.value, b.value);
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator==(const pair &a, const pair &b) noexcept {
  return a.key.length() == b.key.length() && a.value.length() == b.value.length() &&
         memcmp(a.key.data(), b.key.data(), a.key.length()) == 0 &&
         memcmp(a.value.data(), b.value.data(), a.value.length()) == 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator<(const pair &a, const pair &b) noexcept {
  return pair::compare_lexicographically(a, b) < 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator>(const pair &a, const pair &b) noexcept {
  return pair::compare_lexicographically(a, b) > 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator<=(const pair &a, const pair &b) noexcept {
  return pair::compare_lexicographically(a, b) <= 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator>=(const pair &a, const pair &b) noexcept {
  return pair::compare_lexicographically(a, b) >= 0;
}

MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR bool operator!=(const pair &a, const pair &b) noexcept {
  return a.key.length() != b.key.length() || a.value.length() != b.value.length() ||
         memcmp(a.key.data(), b.key.data(), a.key.length()) != 0 ||
         memcmp(a.value.data(), b.value.data(), a.value.length()) != 0;
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
MDBX_NOTHROW_PURE_FUNCTION MDBX_CXX14_CONSTEXPR auto operator<=>(const pair &a, const pair &b) noexcept {
  return pair::compare_lexicographically(a, b);
}
#endif /* __cpp_impl_three_way_comparison */

/// \brief Combines pair of buffers for key and value to hold an operands for certain operations.
template <typename BUFFER>
using buffer_pair = buffer_pair_spec<typename BUFFER::allocator_type, typename BUFFER::reservation_policy>;

/// \brief Default pair of buffers.
using default_buffer_pair = buffer_pair<default_buffer>;

//------------------------------------------------------------------------------

inline ::std::ostream &operator<<(::std::ostream &out, const to_hex &wrapper) { return wrapper.output(out); }
inline ::std::ostream &operator<<(::std::ostream &out, const to_base58 &wrapper) { return wrapper.output(out); }
inline ::std::ostream &operator<<(::std::ostream &out, const to_base64 &wrapper) { return wrapper.output(out); }

MDBX_NOTHROW_PURE_FUNCTION inline bool slice::is_hex(bool ignore_spaces) const noexcept {
  return !from_hex(*this, ignore_spaces).is_erroneous();
}

MDBX_NOTHROW_PURE_FUNCTION inline bool slice::is_base58(bool ignore_spaces) const noexcept {
  return !from_base58(*this, ignore_spaces).is_erroneous();
}

MDBX_NOTHROW_PURE_FUNCTION inline bool slice::is_base64(bool ignore_spaces) const noexcept {
  return !from_base64(*this, ignore_spaces).is_erroneous();
}

//------------------------------------------------------------------------------

MDBX_CXX11_CONSTEXPR env::env(MDBX_env *ptr) noexcept : handle_(ptr) {}

inline env &env::operator=(env &&other) noexcept {
  if (this != &other) {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

inline env::env(env &&other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }

inline env::~env() noexcept {
#if (defined(MDBX_CHECKING) && MDBX_CHECKING > 0) || (defined(MDBX_DEBUG) && MDBX_DEBUG > 0)
  handle_ = reinterpret_cast<MDBX_env *>(uintptr_t(0xDeadBeef));
#endif
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::make_fixed(intptr_t size) noexcept {
  size_lower = size_now = size_upper = size;
  growth_step = shrink_threshold = 0;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::make_dynamic(intptr_t lower, intptr_t upper) noexcept {
  size_now = size_lower = lower;
  size_upper = upper;
  growth_step = shrink_threshold = default_value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_size_lower(intptr_t size) noexcept {
  size_lower = size;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_size_now(intptr_t size) noexcept {
  size_now = size;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_size_upper(intptr_t size) noexcept {
  size_upper = size;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_growth_step(intptr_t step) noexcept {
  growth_step = step;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_shrink_threshold(intptr_t threshold) noexcept {
  shrink_threshold = threshold;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::geometry &env::geometry::set_pagesize(intptr_t size) noexcept {
  pagesize = size;
  return *this;
}

inline env::reclaiming_options env::operate_parameters::reclaiming_from_flags(MDBX_env_flags_t flags) noexcept {
  return reclaiming_options(flags);
}

inline env::operate_options env::operate_parameters::options_from_flags(MDBX_env_flags_t flags) noexcept {
  return operate_options(flags);
}

MDBX_CXX14_CONSTEXPR
env_managed::create_parameters &env_managed::create_parameters::set_geometry(const env::geometry &value) noexcept {
  geometry = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env_managed::create_parameters &env_managed::create_parameters::set_file_mode_bits(mdbx_mode_t value) noexcept {
  file_mode_bits = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env_managed::create_parameters &env_managed::create_parameters::set_use_subdirectory(bool value) noexcept {
  use_subdirectory = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_max_maps(unsigned value) noexcept {
  max_maps = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_max_readers(unsigned value) noexcept {
  max_readers = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_mode(env::mode value) noexcept {
  mode = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_durability(env::durability value) noexcept {
  durability = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_reclaiming(const env::reclaiming_options &value) noexcept {
  reclaiming = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::set_options(const env::operate_options &value) noexcept {
  options = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::readonly() noexcept {
  mode = env::mode::readonly;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::write_file_io() noexcept {
  mode = env::mode::write_file_io;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::write_mapped_io() noexcept {
  mode = env::mode::write_mapped_io;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::robust_synchronous() noexcept {
  durability = env::durability::robust_synchronous;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::half_synchronous_weak_last() noexcept {
  durability = env::durability::half_synchronous_weak_last;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::lazy_weak_tail() noexcept {
  durability = env::durability::lazy_weak_tail;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::whole_fragile() noexcept {
  durability = env::durability::whole_fragile;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::nested_transactions(bool value) noexcept {
  options.nested_transactions = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::lifo(bool value) noexcept {
  reclaiming.lifo = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::no_sticky_threads(bool value) noexcept {
  options.no_sticky_threads = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::exclusive(bool value) noexcept {
  options.exclusive = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::disable_readahead(bool value) noexcept {
  options.disable_readahead = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::disable_clear_memory(bool value) noexcept {
  options.disable_clear_memory = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters &env::operate_parameters::enable_validation(bool value) noexcept {
  options.enable_validation = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters env::operate_parameters::read_only() noexcept {
  operate_parameters op;
  op.set_mode(env::mode::readonly);
  return op;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters env::operate_parameters::safe_write() noexcept {
  operate_parameters op;
  op.write_mapped_io().robust_synchronous();
  return op;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters env::operate_parameters::lazy_write() noexcept {
  operate_parameters op;
  op.write_mapped_io().lazy_weak_tail();
  return op;
}

MDBX_CXX14_CONSTEXPR
env::operate_parameters env::operate_parameters::fragile_write() noexcept {
  operate_parameters op;
  op.write_mapped_io().whole_fragile();
  return op;
}

MDBX_CXX14_CONSTEXPR
env::reclaiming_options &env::reclaiming_options::set_lifo(bool value) noexcept {
  lifo = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_no_sticky_threads(bool value) noexcept {
  no_sticky_threads = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_nested_transactions(bool value) noexcept {
  nested_transactions = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_exclusive(bool value) noexcept {
  exclusive = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_disable_readahead(bool value) noexcept {
  disable_readahead = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_disable_clear_memory(bool value) noexcept {
  disable_clear_memory = value;
  return *this;
}

MDBX_CXX14_CONSTEXPR
env::operate_options &env::operate_options::set_enable_validation(bool value) noexcept {
  enable_validation = value;
  return *this;
}

inline size_t env::limits::pagesize_min() noexcept { return MDBX_MIN_PAGESIZE; }

inline size_t env::limits::pagesize_max() noexcept { return MDBX_MAX_PAGESIZE; }

inline size_t env::limits::dbsize_min(intptr_t pagesize) {
  const intptr_t result = mdbx_limits_dbsize_min(pagesize);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::dbsize_max(intptr_t pagesize) {
  const intptr_t result = mdbx_limits_dbsize_max(pagesize);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::key_min(MDBX_db_flags_t flags) noexcept { return (flags & MDBX_INTEGERKEY) ? 4 : 0; }

inline size_t env::limits::key_min(key_mode mode) noexcept { return key_min(MDBX_db_flags_t(mode)); }

inline size_t env::limits::key_max(intptr_t pagesize, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_limits_keysize_max(pagesize, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::key_max(intptr_t pagesize, key_mode mode) {
  return key_max(pagesize, MDBX_db_flags_t(mode));
}

inline size_t env::limits::key_max(const env &env, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_env_get_maxkeysize_ex(env, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::key_max(const env &env, key_mode mode) { return key_max(env, MDBX_db_flags_t(mode)); }

inline size_t env::limits::value_min(MDBX_db_flags_t flags) noexcept { return (flags & MDBX_INTEGERDUP) ? 4 : 0; }

inline size_t env::limits::value_min(value_mode mode) noexcept { return value_min(MDBX_db_flags_t(mode)); }

inline size_t env::limits::value_max(intptr_t pagesize, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_limits_valsize_max(pagesize, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::value_max(intptr_t pagesize, value_mode mode) {
  return value_max(pagesize, MDBX_db_flags_t(mode));
}

inline size_t env::limits::value_max(const env &env, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_env_get_maxvalsize_ex(env, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::value_max(const env &env, value_mode mode) { return value_max(env, MDBX_db_flags_t(mode)); }

inline size_t env::limits::pairsize4page_max(intptr_t pagesize, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_limits_pairsize4page_max(pagesize, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::pairsize4page_max(intptr_t pagesize, value_mode mode) {
  return pairsize4page_max(pagesize, MDBX_db_flags_t(mode));
}

inline size_t env::limits::pairsize4page_max(const env &env, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_env_get_pairsize4page_max(env, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::pairsize4page_max(const env &env, value_mode mode) {
  return pairsize4page_max(env, MDBX_db_flags_t(mode));
}

inline size_t env::limits::valsize4page_max(intptr_t pagesize, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_limits_valsize4page_max(pagesize, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::valsize4page_max(intptr_t pagesize, value_mode mode) {
  return valsize4page_max(pagesize, MDBX_db_flags_t(mode));
}

inline size_t env::limits::valsize4page_max(const env &env, MDBX_db_flags_t flags) {
  const intptr_t result = mdbx_env_get_valsize4page_max(env, flags);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::valsize4page_max(const env &env, value_mode mode) {
  return valsize4page_max(env, MDBX_db_flags_t(mode));
}

inline size_t env::limits::transaction_size_max(intptr_t pagesize) {
  const intptr_t result = mdbx_limits_txnsize_max(pagesize);
  if (result < 0)
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return static_cast<size_t>(result);
}

inline size_t env::limits::max_map_handles(void) { return MDBX_MAX_DBI; }

inline env::operate_parameters env::get_operation_parameters() const {
  const auto flags = get_flags();
  return operate_parameters(max_maps(), max_readers(), operate_parameters::mode_from_flags(flags),
                            operate_parameters::durability_from_flags(flags),
                            operate_parameters::reclaiming_from_flags(flags),
                            operate_parameters::options_from_flags(flags));
}

inline env::mode env::get_mode() const { return operate_parameters::mode_from_flags(get_flags()); }

inline env::durability env::get_durability() const {
  return env::operate_parameters::durability_from_flags(get_flags());
}

inline env::reclaiming_options env::get_reclaiming() const {
  return env::operate_parameters::reclaiming_from_flags(get_flags());
}

inline env::operate_options env::get_options() const {
  return env::operate_parameters::options_from_flags(get_flags());
}

inline env::stat env::get_stat() const {
  env::stat r;
  error::success_or_throw(::mdbx_env_stat_ex(handle_, nullptr, &r, sizeof(r)));
  return r;
}

inline env::stat env::get_stat(const txn &txn) const {
  env::stat r;
  error::success_or_throw(::mdbx_env_stat_ex(handle_, txn, &r, sizeof(r)));
  return r;
}

inline env::info env::get_info() const {
  env::info r;
  error::success_or_throw(::mdbx_env_info_ex(handle_, nullptr, &r, sizeof(r)));
  return r;
}

inline env::info env::get_info(const txn &txn) const {
  env::info r;
  error::success_or_throw(::mdbx_env_info_ex(handle_, txn, &r, sizeof(r)));
  return r;
}

inline filehandle env::get_filehandle() const {
  filehandle fd;
  error::success_or_throw(::mdbx_env_get_fd(handle_, &fd));
  return fd;
}

inline MDBX_env_flags_t env::get_flags() const {
  unsigned bits = 0;
  error::success_or_throw(::mdbx_env_get_flags(handle_, &bits));
  return MDBX_env_flags_t(bits);
}

inline unsigned env::max_readers() const {
  unsigned r;
  error::success_or_throw(::mdbx_env_get_maxreaders(handle_, &r));
  return r;
}

inline unsigned env::max_maps() const {
  unsigned r;
  error::success_or_throw(::mdbx_env_get_maxdbs(handle_, &r));
  return r;
}

inline void *env::get_context() const noexcept { return mdbx_env_get_userctx(handle_); }

inline env &env::set_context(void *ptr) {
  error::success_or_throw(::mdbx_env_set_userctx(handle_, ptr));
  return *this;
}

inline env &env::set_sync_threshold(size_t bytes) {
  error::success_or_throw(::mdbx_env_set_syncbytes(handle_, bytes));
  return *this;
}

inline size_t env::sync_threshold() const {
  size_t bytes;
  error::success_or_throw(::mdbx_env_get_syncbytes(handle_, &bytes));
  return bytes;
}

inline env &env::set_sync_period__seconds_16dot16(unsigned seconds_16dot16) {
  error::success_or_throw(::mdbx_env_set_syncperiod(handle_, seconds_16dot16));
  return *this;
}

inline unsigned env::sync_period__seconds_16dot16() const {
  unsigned seconds_16dot16;
  error::success_or_throw(::mdbx_env_get_syncperiod(handle_, &seconds_16dot16));
  return seconds_16dot16;
}

inline env &env::set_sync_period__seconds_double(double seconds) {
  if (MDBX_UNLIKELY(!(seconds >= 0 && seconds * 65536 <= double(UINT_MAX))))
    MDBX_CXX20_UNLIKELY error::throw_exception(MDBX_EINVAL);
  return set_sync_period__seconds_16dot16(unsigned(seconds * 65536));
}

inline double env::sync_period__seconds_double() const { return sync_period__seconds_16dot16() / 65536.0; }

#if __cplusplus >= 201103L
inline env &env::set_sync_period(const duration &period) { return set_sync_period__seconds_16dot16(period.count()); }

inline duration env::sync_period() const { return duration(sync_period__seconds_16dot16()); }
#endif

inline env &env::set_extra_option(enum env::extra_runtime_option option, uint64_t value) {
  error::success_or_throw(::mdbx_env_set_option(handle_, ::MDBX_option_t(option), value));
  return *this;
}

inline uint64_t env::extra_option(enum env::extra_runtime_option option) const {
  uint64_t value;
  error::success_or_throw(::mdbx_env_get_option(handle_, ::MDBX_option_t(option), &value));
  return value;
}

inline env &env::alter_flags(MDBX_env_flags_t flags, bool on_off) {
  error::success_or_throw(::mdbx_env_set_flags(handle_, flags, on_off));
  return *this;
}

inline env &env::set_geometry(const geometry &geo) {
  error::success_or_throw(::mdbx_env_set_geometry(handle_, geo.size_lower, geo.size_now, geo.size_upper,
                                                  geo.growth_step, geo.shrink_threshold, geo.pagesize));
  return *this;
}

inline bool env::sync_to_disk(bool force, bool nonblock) {
  const int err = ::mdbx_env_sync_ex(handle_, force, nonblock);
  switch (err) {
  case MDBX_SUCCESS /* flush done */:
  case MDBX_RESULT_TRUE /* no data pending for flush to disk */:
    return true;
  case MDBX_BUSY /* the environment is used by other thread */:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void env::close_map(const map_handle &handle) { error::success_or_throw(::mdbx_dbi_close(*this, handle.dbi)); }

MDBX_CXX11_CONSTEXPR
env::reader_info::reader_info(int slot, mdbx_pid_t pid, mdbx_tid_t thread, uint64_t txnid, uint64_t lag, size_t used,
                              size_t retained) noexcept
    : slot(slot), pid(pid), thread(thread), transaction_id(txnid), transaction_lag(lag), bytes_used(used),
      bytes_retained(retained) {}

template <typename VISITOR> inline int env::enumerate_readers(VISITOR &visitor) {
  struct reader_visitor_thunk : public exception_thunk {
    VISITOR &visitor_;
    static int cb(void *ctx, int number, int slot, mdbx_pid_t pid, mdbx_tid_t thread, uint64_t txnid, uint64_t lag,
                  size_t used, size_t retained) noexcept {
      reader_visitor_thunk *thunk = static_cast<reader_visitor_thunk *>(ctx);
      assert(thunk->is_clean());
      try {
        const reader_info info(slot, pid, thread, txnid, lag, used, retained);
        return loop_control(thunk->visitor_(info, number));
      } catch (... /* capture any exception to rethrow it over C code */) {
        thunk->capture();
        return loop_control::exit_loop;
      }
    }
    MDBX_CXX11_CONSTEXPR reader_visitor_thunk(VISITOR &visitor) noexcept : visitor_(visitor) {}
  };
  reader_visitor_thunk thunk(visitor);
  const auto rc = ::mdbx_reader_list(*this, thunk.cb, &thunk);
  thunk.rethrow_captured();
  return rc;
}

inline unsigned env::check_readers() {
  int dead_count;
  error::throw_on_failure(::mdbx_reader_check(*this, &dead_count));
  MDBX_INLINE_API_ASSERT(dead_count >= 0);
  return static_cast<unsigned>(dead_count);
}

/** Zeroed set of check callbacks for a "silent" integrity check. */
static const MDBX_chk_callbacks_t silent_chk_callbacks = {};

inline MDBX_chk_context_t &env::chk(MDBX_chk_context_t &ctx, const MDBX_chk_callbacks_t *cb, MDBX_chk_flags_t flags,
                                    MDBX_chk_severity_t verbosity, unsigned timeout_seconds_16dot16) const {
  if (!cb)
    cb = &silent_chk_callbacks;
  const int err = ::mdbx_env_chk(handle_, cb, &ctx, flags, verbosity, timeout_seconds_16dot16);
  /* MDBX_PROBLEM with counted problems is a check result reported in `ctx`, not a failure of the check itself */
  if (err != MDBX_SUCCESS && !(err == MDBX_PROBLEM && ctx.result.total_problems))
    error::throw_exception(err);
  return ctx;
}

inline env::sysraminfo env::get_sysraminfo() {
  sysraminfo result;
  error::success_or_throw(::mdbx_get_sysraminfo(&result.page_size, &result.total_pages, &result.avail_pages));
  return result;
}

inline env::info env::get_preopen_snapinfo(const ::std::string &pathname, MDBX_preopen_flags mode) {
  return get_preopen_snapinfo(pathname.c_str(), mode);
}

inline env::info env::get_preopen_snapinfo(const char *pathname, MDBX_preopen_flags mode) {
  env::info result;
  error::success_or_throw(::mdbx_preopen_snapinfo_ex(pathname, mode, &result, sizeof(result)));
  return result;
}

#ifdef MDBX_STD_FILESYSTEM_PATH
inline env::info env::get_preopen_snapinfo(const MDBX_STD_FILESYSTEM_PATH &pathname, MDBX_preopen_flags mode) {
  return get_preopen_snapinfo(pathname.native().c_str(), mode);
}
#endif /* MDBX_STD_FILESYSTEM_PATH */

#if defined(_WIN32) || defined(_WIN64)
inline env::info env::get_preopen_snapinfo(const ::std::wstring &pathname, MDBX_preopen_flags mode) {
  return get_preopen_snapinfo(pathname.c_str(), mode);
}

inline env::info env::get_preopen_snapinfo(const wchar_t *pathname, MDBX_preopen_flags mode) {
  env::info result;
  error::success_or_throw(::mdbx_preopen_snapinfo_exW(pathname, mode, &result, sizeof(result)));
  return result;
}
#endif /* Windows */

inline bool env::thread_register() { return !error::boolean_or_throw(::mdbx_thread_register(handle_)); }

inline bool env::thread_unregister() { return !error::boolean_or_throw(::mdbx_thread_unregister(handle_)); }

inline int env::warmup(MDBX_warmup_flags_t flags, unsigned timeout_seconds_16dot16, const MDBX_txn *txn) const {
  return ::mdbx_env_warmup(handle_, txn, flags, timeout_seconds_16dot16);
}

#if !defined(_WIN32) && !defined(_WIN64)
inline void env::resurrect_after_fork() { error::success_or_throw(::mdbx_env_resurrect_after_fork(handle_)); }
#endif /* !Windows */

inline void env::turn_for_recovery(unsigned target_meta) {
  error::success_or_throw(::mdbx_env_turn_for_recovery(handle_, target_meta));
}

inline env &env::set_HandleSlowReaders(MDBX_hsr_func cb) {
  error::success_or_throw(::mdbx_env_set_hsr(handle_, cb));
  return *this;
}

inline MDBX_hsr_func env::get_HandleSlowReaders() const noexcept { return ::mdbx_env_get_hsr(handle_); }

inline txn_managed env::start_read() const {
  ::MDBX_txn *ptr;
  error::success_or_throw(::mdbx_txn_begin(handle_, nullptr, MDBX_TXN_RDONLY, &ptr));
  MDBX_INLINE_API_ASSERT(ptr != nullptr);
  return txn_managed(ptr);
}

inline txn_managed env::prepare_read() const {
  ::MDBX_txn *ptr;
  error::success_or_throw(::mdbx_txn_begin(handle_, nullptr, MDBX_TXN_RDONLY_PREPARE, &ptr));
  MDBX_INLINE_API_ASSERT(ptr != nullptr);
  return txn_managed(ptr);
}

inline txn_managed env::start_write(bool dont_wait) {
  ::MDBX_txn *ptr;
  error::success_or_throw(
      ::mdbx_txn_begin(handle_, nullptr, dont_wait ? MDBX_TXN_READWRITE | MDBX_TXN_TRY : MDBX_TXN_READWRITE, &ptr));
  MDBX_INLINE_API_ASSERT(ptr != nullptr || dont_wait);
  return txn_managed(ptr);
}

inline txn_managed env::start_write(txn &parent) {
  ::MDBX_txn *ptr;
  error::success_or_throw(::mdbx_txn_begin(handle_, parent, MDBX_TXN_READWRITE, &ptr));
  MDBX_INLINE_API_ASSERT(ptr != nullptr);
  return txn_managed(ptr);
}

inline txn_managed env::try_start_write() {
  ::MDBX_txn *ptr;
  const int err = ::mdbx_txn_begin(handle_, nullptr, MDBX_TXN_READWRITE | MDBX_TXN_TRY, &ptr);
  if (err == MDBX_BUSY)
    return txn_managed();
  error::success_or_throw(err);
  return txn_managed(ptr);
}

inline bool env::txn_lock(bool dont_wait) {
  const int err = ::mdbx_txn_lock(handle_, dont_wait);
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY return true;
  case MDBX_BUSY:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void env::txn_unlock() { error::success_or_throw(::mdbx_txn_unlock(handle_)); }

template <typename VISITOR>
inline env::defrag_result env::defrag(VISITOR &visitor, size_t defrag_atleast, size_t time_atleast_dot16,
                                      size_t defrag_enough, size_t time_limit_dot16, intptr_t acceptable_backlash,
                                      intptr_t preferred_batch) {
  struct defrag_thunk : public exception_thunk {
    VISITOR &visitor_;
    static int cb(void *ctx, const MDBX_defrag_result_t *progress) noexcept {
      defrag_thunk *thunk = static_cast<defrag_thunk *>(ctx);
      if (MDBX_UNLIKELY(!thunk->is_clean()))
        /* the C side notifies once more (final milestone) after the visitor has thrown */
        return static_cast<int>(defrag_control::abort);
      try {
        return static_cast<int>(thunk->visitor_(*progress));
      } catch (... /* capture any exception to rethrow it over C code */) {
        thunk->capture();
        return static_cast<int>(defrag_control::abort);
      }
    }
    MDBX_CXX11_CONSTEXPR defrag_thunk(VISITOR &visitor) noexcept : visitor_(visitor) {}
  };
  defrag_thunk thunk(visitor);
  defrag_result result;
  const int rc = ::mdbx_env_defrag(handle_, defrag_atleast, time_atleast_dot16, defrag_enough, time_limit_dot16,
                                   acceptable_backlash, preferred_batch, thunk.cb, &thunk, &result);
  thunk.rethrow_captured();
  if (rc != MDBX_SUCCESS && rc != MDBX_RESULT_TRUE && rc != MDBX_LAGGARD_READER)
    error::success_or_throw(rc);
  return result;
}

inline env::defrag_result env::defrag(size_t defrag_atleast, size_t time_atleast_dot16, size_t defrag_enough,
                                      size_t time_limit_dot16, intptr_t acceptable_backlash, intptr_t preferred_batch) {
  struct noop_visitor {
    defrag_control operator()(const defrag_result &) { return defrag_control::proceed; }
  } visitor;
  return defrag(visitor, defrag_atleast, time_atleast_dot16, defrag_enough, time_limit_dot16, acceptable_backlash,
                preferred_batch);
}

MDBX_CXX11_CONSTEXPR txn::txn(MDBX_txn *ptr) noexcept : handle_(ptr) {}

inline txn &txn::operator=(txn &&other) noexcept {
  if (this != &other) {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

inline txn::txn(txn &&other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }

inline txn::~txn() noexcept {
#if (defined(MDBX_CHECKING) && MDBX_CHECKING > 0) || (defined(MDBX_DEBUG) && MDBX_DEBUG > 0)
  handle_ = reinterpret_cast<MDBX_txn *>(uintptr_t(0xDeadBeef));
#endif
}

inline void *txn::get_context() const noexcept { return mdbx_txn_get_userctx(handle_); }

inline txn &txn::set_context(void *ptr) {
  error::success_or_throw(::mdbx_txn_set_userctx(handle_, ptr));
  return *this;
}

inline bool txn::is_dirty(const void *ptr) const {
  int err = ::mdbx_is_dirty(handle_, ptr);
  switch (err) {
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  case MDBX_RESULT_TRUE:
    return true;
  case MDBX_RESULT_FALSE:
    return false;
  }
}

inline env txn::env() const noexcept { return ::mdbx_txn_env(handle_); }

inline MDBX_txn_flags_t txn::flags() const {
  const int bits = mdbx_txn_flags(handle_);
  error::throw_on_failure((bits != static_cast<int>(MDBX_TXN_INVALID)) ? MDBX_SUCCESS : MDBX_BAD_TXN);
  return static_cast<MDBX_txn_flags_t>(bits);
}

inline uint64_t txn::id() const {
  const uint64_t txnid = mdbx_txn_id(handle_);
  error::throw_on_failure(txnid ? MDBX_SUCCESS : MDBX_BAD_TXN);
  return txnid;
}

inline void txn::reset_reading() { error::success_or_throw(::mdbx_txn_reset(handle_)); }

inline void txn::make_broken() { error::success_or_throw(::mdbx_txn_break(handle_)); }

inline void txn::renew_reading() { error::success_or_throw(::mdbx_txn_renew(handle_)); }

inline bool txn::refresh() {
  const int err = ::mdbx_txn_refresh(handle_);
  switch (err) {
  case MDBX_SUCCESS:
    return false;
  case MDBX_RESULT_TRUE:
    return true;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void txn::copy(const char *destination, bool compactify, bool force_dynamic_size) {
  error::success_or_throw(::mdbx_txn_copy2pathname(
      handle_, destination,
      MDBX_copy_flags_t((compactify ? MDBX_CP_COMPACT : MDBX_CP_DEFAULTS) |
                        (force_dynamic_size ? MDBX_CP_FORCE_DYNAMIC_SIZE : MDBX_CP_DEFAULTS))));
}

inline void txn::copy(const ::std::string &destination, bool compactify, bool force_dynamic_size) {
  copy(destination.c_str(), compactify, force_dynamic_size);
}

#if defined(_WIN32) || defined(_WIN64)
inline void txn::copy(const wchar_t *destination, bool compactify, bool force_dynamic_size) {
  error::success_or_throw(::mdbx_txn_copy2pathnameW(
      handle_, destination,
      MDBX_copy_flags_t((compactify ? MDBX_CP_COMPACT : MDBX_CP_DEFAULTS) |
                        (force_dynamic_size ? MDBX_CP_FORCE_DYNAMIC_SIZE : MDBX_CP_DEFAULTS))));
}

inline void txn::copy(const ::std::wstring &destination, bool compactify, bool force_dynamic_size) {
  copy(destination.c_str(), compactify, force_dynamic_size);
}
#endif /* Windows */

#ifdef MDBX_STD_FILESYSTEM_PATH
inline void txn::copy(const MDBX_STD_FILESYSTEM_PATH &destination, bool compactify, bool force_dynamic_size) {
  copy(destination.native(), compactify, force_dynamic_size);
}
#endif /* MDBX_STD_FILESYSTEM_PATH */

inline void txn::copy(filehandle fd, bool compactify, bool force_dynamic_size) {
  error::success_or_throw(
      ::mdbx_txn_copy2fd(handle_, fd,
                         MDBX_copy_flags_t((compactify ? MDBX_CP_COMPACT : MDBX_CP_DEFAULTS) |
                                           (force_dynamic_size ? MDBX_CP_FORCE_DYNAMIC_SIZE : MDBX_CP_DEFAULTS))));
}

inline txn_managed txn::clone(void *context) const {
  MDBX_txn *ptr = nullptr;
  error::success_or_throw(::mdbx_txn_clone(handle_, &ptr, context));
  MDBX_INLINE_API_ASSERT(ptr != nullptr);
  return txn_managed(ptr);
}

inline void txn::clone(txn_managed &txn_for_renew_into_clone, void *context) const {
  error::throw_on_nullptr(txn_for_renew_into_clone.handle_, MDBX_BAD_TXN);
  error::success_or_throw(::mdbx_txn_clone(handle_, &txn_for_renew_into_clone.handle_, context));
  MDBX_INLINE_API_ASSERT(txn_for_renew_into_clone.handle_ != nullptr);
}

inline void txn::park_reading(bool autounpark) { error::success_or_throw(::mdbx_txn_park(handle_, autounpark)); }

inline bool txn::unpark_reading(bool restart_if_ousted) {
  return error::boolean_or_throw(::mdbx_txn_unpark(handle_, restart_if_ousted));
}

inline txn::info txn::get_info(bool scan_reader_lock_table) const {
  txn::info r;
  error::success_or_throw(::mdbx_txn_info(handle_, &r, scan_reader_lock_table));
  return r;
}

inline cursor_managed txn::open_cursor(map_handle map) const {
  MDBX_cursor *ptr;
  error::success_or_throw(::mdbx_cursor_open(handle_, map.dbi, &ptr));
  return cursor_managed(ptr);
}

inline size_t txn::release_all_cursors(bool unbind) const {
  size_t count;
  error::success_or_throw(::mdbx_txn_release_all_cursors_ex(handle_, unbind, &count));
  return count;
}

inline map_handle txn::open_map(const slice &name, const ::mdbx::key_mode key_mode,
                                const ::mdbx::value_mode value_mode) const {
  map_handle map;
  error::success_or_throw(
      ::mdbx_dbi_open2(handle_, name, MDBX_db_flags_t(key_mode) | MDBX_db_flags_t(value_mode), &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline map_handle txn::open_map(const char *name, const ::mdbx::key_mode key_mode,
                                const ::mdbx::value_mode value_mode) const {
  map_handle map;
  error::success_or_throw(
      ::mdbx_dbi_open(handle_, name, MDBX_db_flags_t(key_mode) | MDBX_db_flags_t(value_mode), &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline map_handle txn::open_map_accede(const slice &name) const {
  map_handle map;
  error::success_or_throw(::mdbx_dbi_open2(handle_, name, MDBX_DB_ACCEDE, &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline map_handle txn::open_map_accede(const char *name) const {
  map_handle map;
  error::success_or_throw(::mdbx_dbi_open(handle_, name, MDBX_DB_ACCEDE, &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline map_handle txn::create_map(const slice &name, const ::mdbx::key_mode key_mode,
                                  const ::mdbx::value_mode value_mode) {
  map_handle map;
  error::success_or_throw(
      ::mdbx_dbi_open2(handle_, name, MDBX_CREATE | MDBX_db_flags_t(key_mode) | MDBX_db_flags_t(value_mode), &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline map_handle txn::create_map(const char *name, const ::mdbx::key_mode key_mode,
                                  const ::mdbx::value_mode value_mode) {
  map_handle map;
  error::success_or_throw(
      ::mdbx_dbi_open(handle_, name, MDBX_CREATE | MDBX_db_flags_t(key_mode) | MDBX_db_flags_t(value_mode), &map.dbi));
  MDBX_INLINE_API_ASSERT(map.dbi != 0);
  return map;
}

inline void txn::drop_map(map_handle map) { error::success_or_throw(::mdbx_drop(handle_, map.dbi, true)); }

inline void txn::clear_map(map_handle map) { error::success_or_throw(::mdbx_drop(handle_, map.dbi, false)); }

inline void txn::rename_map(map_handle map, const char *new_name) {
  error::success_or_throw(::mdbx_dbi_rename(handle_, map, new_name));
}

inline void txn::rename_map(map_handle map, const slice &new_name) {
  error::success_or_throw(::mdbx_dbi_rename2(handle_, map, new_name));
}

inline map_handle txn::open_map(const ::std::string &name, const ::mdbx::key_mode key_mode,
                                const ::mdbx::value_mode value_mode) const {
  return open_map(slice(name), key_mode, value_mode);
}

inline map_handle txn::open_map_accede(const ::std::string &name) const { return open_map_accede(slice(name)); }

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
inline map_handle txn::open_map_accede(const ::std::string_view &name) const { return open_map_accede(slice(name)); }
#endif /* __cpp_lib_string_view >= 201606L */

inline map_handle txn::create_map(const ::std::string &name, const ::mdbx::key_mode key_mode,
                                  const ::mdbx::value_mode value_mode) {
  return create_map(slice(name), key_mode, value_mode);
}

inline bool txn::drop_map(const ::std::string &name, bool throw_if_absent) {
  return drop_map(slice(name), throw_if_absent);
}

inline bool txn::clear_map(const ::std::string &name, bool throw_if_absent) {
  return clear_map(slice(name), throw_if_absent);
}

inline void txn::rename_map(map_handle map, const ::std::string &new_name) { return rename_map(map, slice(new_name)); }

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
inline void txn::rename_map(map_handle map, const ::std::string_view &new_name) {
  return rename_map(map, slice(new_name));
}
#endif /* __cpp_lib_string_view >= 201606L */

inline txn::map_stat txn::get_map_stat(map_handle map) const {
  txn::map_stat r;
  error::success_or_throw(::mdbx_dbi_stat(handle_, map.dbi, &r, sizeof(r)));
  return r;
}

inline uint32_t txn::get_tree_deepmask(map_handle map) const {
  uint32_t r = 0;
  /* MDBX_RESULT_TRUE: not a multi-value table, so there are no nested trees */
  error::boolean_or_throw(::mdbx_dbi_dupsort_depthmask(handle_, map.dbi, &r));
  return r;
}

inline map_handle::info txn::get_map_flags(map_handle map) const {
  unsigned flags, state;
  error::success_or_throw(::mdbx_dbi_flags_ex(handle_, map.dbi, &flags, &state));
  return map_handle::info(MDBX_db_flags_t(flags), MDBX_dbi_state_t(state));
}

template <typename VISITOR> inline int txn::enumerate_tables(VISITOR &visitor) const {
  struct tables_enum_thunk : public exception_thunk {
    VISITOR &visitor_;
    static int cb(void *ctx, const MDBX_txn *, const MDBX_val *name, MDBX_db_flags_t flags, const MDBX_stat *stat,
                  MDBX_dbi dbi) noexcept {
      tables_enum_thunk *thunk = static_cast<tables_enum_thunk *>(ctx);
      assert(thunk->is_clean());
      try {
        const slice table_name(*name);
        return loop_control(thunk->visitor_(table_name, flags, *stat, dbi));
      } catch (... /* capture any exception to rethrow it over C code */) {
        thunk->capture();
        return loop_control::exit_loop;
      }
    }
    MDBX_CXX11_CONSTEXPR tables_enum_thunk(VISITOR &visitor) noexcept : visitor_(visitor) {}
  };
  tables_enum_thunk thunk(visitor);
  const auto rc = ::mdbx_enumerate_tables(handle_, thunk.cb, &thunk);
  thunk.rethrow_captured();
  return rc;
}

template <typename VISITOR> inline txn::gc_info txn::get_gc_info(VISITOR &visitor) const {
  struct gc_iter_thunk : public exception_thunk {
    VISITOR &visitor_;
    static int cb(void *ctx, const MDBX_txn *, uint64_t span_txnid, size_t span_pgno, size_t span_length,
                  bool span_is_reclaimable) noexcept {
      gc_iter_thunk *thunk = static_cast<gc_iter_thunk *>(ctx);
      if (MDBX_UNLIKELY(!thunk->is_clean()))
        /* do not call the visitor again after it has thrown */
        return loop_control::exit_loop;
      try {
        return loop_control(thunk->visitor_(span_txnid, span_pgno, span_length, span_is_reclaimable));
      } catch (... /* capture any exception to rethrow it over C code */) {
        thunk->capture();
        return loop_control::exit_loop;
      }
    }
    MDBX_CXX11_CONSTEXPR gc_iter_thunk(VISITOR &visitor) noexcept : visitor_(visitor) {}
  };
  gc_iter_thunk thunk(visitor);
  gc_info info;
  const auto rc = ::mdbx_gc_info(handle_, &info, sizeof(info), thunk.cb, &thunk);
  thunk.rethrow_captured();
  /* The MDBX_NOTFOUND just means the GC is empty, while the C function
   * still fills the geometry-related fields (pages_total, pages_allocated,
   * etc.), which must be preserved. */
  if (rc != MDBX_NOTFOUND)
    error::success_or_throw(rc);
  return info;
}

inline txn::gc_info txn::get_gc_info() const {
  struct noop_visitor {
    int operator()(uint64_t, size_t, size_t, bool) { return loop_control::continue_loop; }
  } visitor;
  return get_gc_info(visitor);
}

inline txn &txn::put_canary(const txn::canary &canary) {
  error::success_or_throw(::mdbx_canary_put(handle_, &canary));
  return *this;
}

inline txn::canary txn::get_canary() const {
  txn::canary r;
  error::success_or_throw(::mdbx_canary_get(handle_, &r));
  return r;
}

inline uint64_t txn::sequence(map_handle map) const {
  uint64_t result;
  error::success_or_throw(::mdbx_dbi_sequence(handle_, map.dbi, &result, 0));
  return result;
}

inline uint64_t txn::sequence(map_handle map, uint64_t increment) {
  uint64_t result;
  if (error::boolean_or_throw(::mdbx_dbi_sequence(handle_, map.dbi, &result, increment)))
    throw ::std::overflow_error("mdbx::txn::sequence(): the increment overflows the sequence");
  return result;
}

inline int txn::compare_keys(map_handle map, const slice &a, const slice &b) const noexcept {
  return ::mdbx_cmp(handle_, map.dbi, &a, &b);
}

inline int txn::compare_values(map_handle map, const slice &a, const slice &b) const noexcept {
  return ::mdbx_dcmp(handle_, map.dbi, &a, &b);
}

inline int txn::compare_keys(map_handle map, const pair &a, const pair &b) const noexcept {
  return compare_keys(map, a.key, b.key);
}

inline int txn::compare_values(map_handle map, const pair &a, const pair &b) const noexcept {
  return compare_values(map, a.value, b.value);
}

inline slice txn::get(map_handle map, const slice &key) const {
  slice result;
  error::success_or_throw(::mdbx_get(handle_, map.dbi, &key, &result));
  return result;
}

inline txn::cache_result txn::get_cached(map_handle map, const slice &key, slice *data, cache_entry &entry) const {
  return ::mdbx_cache_get(handle_, map.dbi, &key, data, &entry);
}

inline slice txn::get_cached(map_handle map, const slice &key, cache_entry &entry, cache_status *status) const {
  slice value;
  const auto result = get_cached(map, key, &value, entry);
  if (status)
    *status = result.status;
  error::success_or_throw(result.errcode);
  return value;
}

inline slice txn::get(map_handle map, slice key, size_t &values_count) const {
  slice result;
  error::success_or_throw(::mdbx_get_ex(handle_, map.dbi, &key, &result, &values_count));
  return result;
}

inline slice txn::get(map_handle map, const slice &key, const slice &value_at_absence) const {
  slice result;
  const int err = ::mdbx_get(handle_, map.dbi, &key, &result);
  switch (err) {
  case MDBX_SUCCESS:
    return result;
  case MDBX_NOTFOUND:
    return value_at_absence;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline slice txn::get(map_handle map, slice key, size_t &values_count, const slice &value_at_absence) const {
  slice result;
  const int err = ::mdbx_get_ex(handle_, map.dbi, &key, &result, &values_count);
  switch (err) {
  case MDBX_SUCCESS:
    return result;
  case MDBX_NOTFOUND:
    return value_at_absence;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline pair_result txn::get_equal_or_great(map_handle map, const slice &key) const {
  pair result(key, slice());
  error::boolean_or_throw(::mdbx_get_equal_or_great(handle_, map.dbi, &result.key, &result.value));
  /* for a multimap MDBX_SUCCESS needs the value to match too, while here only the key is given */
  return pair_result(result.key, result.value, result.key == key);
}

inline pair_result txn::get_equal_or_great(map_handle map, const slice &key, const slice &value_at_absence) const {
  pair result{key, slice()};
  const int err = ::mdbx_get_equal_or_great(handle_, map.dbi, &result.key, &result.value);
  switch (err) {
  case MDBX_SUCCESS:
    return pair_result{result.key, result.value, result.key == key};
  case MDBX_RESULT_TRUE:
    return pair_result{result.key, result.value, result.key == key};
  case MDBX_NOTFOUND:
    return pair_result{key, value_at_absence, false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline MDBX_error_t txn::put(map_handle map, const slice &key, slice *value, MDBX_put_flags_t flags) noexcept {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  return MDBX_error_t(::mdbx_put(handle_, map.dbi, &key, value, flags));
}

inline void txn::put(map_handle map, const slice &key, slice value, put_mode mode) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, &value, MDBX_put_flags_t(mode)));
}

inline void txn::insert(map_handle map, const slice &key, slice value) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, &value /* takes the present value in case MDBX_KEYEXIST */,
                              MDBX_put_flags_t(put_mode::insert_unique)));
}

inline value_result txn::try_insert(map_handle map, const slice &key, slice value) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  const int err = put(map, key, &value /* takes the present value in case MDBX_KEYEXIST */,
                      MDBX_put_flags_t(put_mode::insert_unique));
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{slice(), true};
  case MDBX_KEYEXIST:
    return value_result{value, false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline slice txn::insert_reserve(map_handle map, const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, &result /* takes the present value in case MDBX_KEYEXIST */,
                              MDBX_put_flags_t(put_mode::insert_unique) | MDBX_RESERVE));
  return result;
}

inline value_result txn::try_insert_reserve(map_handle map, const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  const int err = put(map, key, &result /* takes the present value in case MDBX_KEYEXIST */,
                      MDBX_put_flags_t(put_mode::insert_unique) | MDBX_RESERVE);
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{result, true};
  case MDBX_KEYEXIST:
    return value_result{result, false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void txn::upsert(map_handle map, const slice &key, const slice &value) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::upsert)));
}

inline slice txn::upsert_reserve(map_handle map, const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, &result, MDBX_put_flags_t(put_mode::upsert) | MDBX_RESERVE));
  return result;
}

inline void txn::update(map_handle map, const slice &key, const slice &value) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::update)));
}

inline bool txn::try_update(map_handle map, const slice &key, const slice &value) {
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  const int err = put(map, key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::update));
  switch (err) {
  case MDBX_SUCCESS:
    return true;
  case MDBX_NOTFOUND:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline slice txn::update_reserve(map_handle map, const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  error::success_or_throw(put(map, key, &result, MDBX_put_flags_t(put_mode::update) | MDBX_RESERVE));
  return result;
}

inline value_result txn::try_update_reserve(map_handle map, const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  /* LY: ложные предупреждения coverity возникают из-за поддержки вставки массивов значений в режиме MDBX_MULTIPLE */
  /* coverity[OVERRUN] */
  const int err = put(map, key, &result, MDBX_put_flags_t(put_mode::update) | MDBX_RESERVE);
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{result, true};
  case MDBX_NOTFOUND:
    return value_result{slice(), false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline bool txn::erase(map_handle map, const slice &key) {
  const int err = ::mdbx_del(handle_, map.dbi, &key, nullptr);
  switch (err) {
  case MDBX_SUCCESS:
    return true;
  case MDBX_NOTFOUND:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline bool txn::erase(map_handle map, const slice &key, const slice &value) {
  const int err = ::mdbx_del(handle_, map.dbi, &key, &value);
  switch (err) {
  case MDBX_SUCCESS:
    return true;
  case MDBX_NOTFOUND:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void txn::replace(map_handle map, const slice &key, slice &old_value, const slice &new_value) {
  error::success_or_throw(::mdbx_replace_ex(handle_, map.dbi, &key, const_cast<slice *>(&new_value), &old_value,
                                            MDBX_CURRENT | MDBX_NOOVERWRITE, nullptr, nullptr));
}

template <class BUFFER, typename ALLOCATOR>
inline BUFFER txn::extract(map_handle map, const slice &key, const ALLOCATOR &alloc) {
  typename BUFFER::data_preserver result(alloc);
  error::success_or_throw(
      ::mdbx_replace_ex(handle_, map.dbi, &key, nullptr, &result.data, MDBX_CURRENT, result, &result), result);
  if (result.data.is_freestanding())
    /* already a private copy made by the preserver: hand it over with its allocator */
    return BUFFER(::std::move(result.data));
  return BUFFER(result.data.slice(), false, alloc);
}

template <class BUFFER, typename ALLOCATOR>
inline BUFFER txn::replace(map_handle map, const slice &key, const slice &new_value, const ALLOCATOR &alloc) {
  typename BUFFER::data_preserver result(alloc);
  error::success_or_throw(::mdbx_replace_ex(handle_, map.dbi, &key, const_cast<slice *>(&new_value), &result.data,
                                            MDBX_CURRENT, result, &result),
                          result);
  if (result.data.is_freestanding())
    /* already a private copy made by the preserver: hand it over with its allocator */
    return BUFFER(::std::move(result.data));
  return BUFFER(result.data.slice(), false, alloc);
}

template <class BUFFER, typename ALLOCATOR>
inline BUFFER txn::replace_reserve(map_handle map, const slice &key, size_t length, slice &new_value_reservation,
                                   const ALLOCATOR &alloc) {
  new_value_reservation = slice::blank_for_put_reserve(length);
  typename BUFFER::data_preserver result(alloc);
  error::success_or_throw(::mdbx_replace_ex(handle_, map.dbi, &key, &new_value_reservation, &result.data,
                                            MDBX_CURRENT | MDBX_RESERVE, result, &result),
                          result);
  if (result.data.is_freestanding())
    /* already a private copy made by the preserver: hand it over with its allocator */
    return BUFFER(::std::move(result.data));
  return BUFFER(result.data.slice(), false, alloc);
}

inline void txn::append(map_handle map, const slice &key, const slice &value, bool multivalue_order_preserved) {
  error::success_or_throw(::mdbx_put(handle_, map.dbi, const_cast<slice *>(&key), const_cast<slice *>(&value),
                                     multivalue_order_preserved ? (MDBX_APPEND | MDBX_APPENDDUP) : MDBX_APPEND));
}

inline size_t txn::put_multiple_samelength(map_handle map, const slice &key, const size_t value_length,
                                           const void *values_array, size_t values_count, put_mode mode,
                                           bool allow_partial) {
  MDBX_val args[2] = {{const_cast<void *>(values_array), value_length}, {nullptr, values_count}};
  const int err = ::mdbx_put(handle_, map.dbi, const_cast<slice *>(&key), args, MDBX_put_flags_t(mode) | MDBX_MULTIPLE);
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY break;
  case MDBX_KEYEXIST:
    if (allow_partial)
      break;
    mdbx_txn_break(handle_);
    MDBX_CXX17_FALLTHROUGH /* fallthrough */;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
  return args[1].iov_len /* done item count */;
}

inline ptrdiff_t txn::estimate(map_handle map, const pair &from, const pair &to) const {
  ptrdiff_t result;
  error::success_or_throw(mdbx_estimate_range(handle_, map.dbi, &from.key, &from.value, &to.key, &to.value, &result));
  return result;
}

inline ptrdiff_t txn::estimate(map_handle map, const slice &from, const slice &to) const {
  ptrdiff_t result;
  error::success_or_throw(mdbx_estimate_range(handle_, map.dbi, &from, nullptr, &to, nullptr, &result));
  return result;
}

inline ptrdiff_t txn::estimate_from_first(map_handle map, const slice &to) const {
  ptrdiff_t result;
  error::success_or_throw(mdbx_estimate_range(handle_, map.dbi, nullptr, nullptr, &to, nullptr, &result));
  return result;
}

inline ptrdiff_t txn::estimate_to_last(map_handle map, const slice &from) const {
  ptrdiff_t result;
  error::success_or_throw(mdbx_estimate_range(handle_, map.dbi, &from, nullptr, nullptr, nullptr, &result));
  return result;
}

MDBX_CXX11_CONSTEXPR cursor::cursor(MDBX_cursor *ptr) noexcept : handle_(ptr) {}

inline cursor &cursor::assign(const cursor &src) {
  error::success_or_throw(::mdbx_cursor_copy(src.handle_, handle_));
  return *this;
}

inline cursor &cursor::operator=(cursor &&other) noexcept {
  if (this != &other) {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

inline cursor::cursor(cursor &&other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }

inline cursor::~cursor() noexcept {
#if (defined(MDBX_CHECKING) && MDBX_CHECKING > 0) || (defined(MDBX_DEBUG) && MDBX_DEBUG > 0)
  handle_ = reinterpret_cast<MDBX_cursor *>(uintptr_t(0xDeadBeef));
#endif
}

inline void *cursor::get_context() const noexcept { return mdbx_cursor_get_userctx(handle_); }

inline cursor &cursor::set_context(void *ptr) {
  error::success_or_throw(::mdbx_cursor_set_userctx(handle_, ptr));
  return *this;
}

/// \brief Compares the positions of two cursors without throwing.
/// \param left           The left-hand cursor.
/// \param right          The right-hand cursor.
/// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
/// \returns A negative value if `left` is before `right`, a positive value if
///          after, zero if at the same position, or \ref MDBX_RESULT_TRUE
///          when the cursors are incomparable.
inline int compare_position_nothrow(const cursor &left, const cursor &right, bool ignore_nested = false) noexcept {
  return mdbx_cursor_compare(left.handle_, right.handle_, ignore_nested);
}

/// \brief Compares the positions of two cursors.
/// \param left           The left-hand cursor.
/// \param right          The right-hand cursor.
/// \param ignore_nested  Ignore the position within a nested (dupsort) tree.
/// \returns A negative value if `left` is before `right`, a positive value if
///          after, zero if at the same position.
/// \throws `std::logic_error` if the cursors are incomparable.
inline int compare_position(const cursor &left, const cursor &right, bool ignore_nested = false) {
  const auto diff = compare_position_nothrow(left, right, ignore_nested);
  MDBX_INLINE_API_ASSERT(compare_position_nothrow(right, left, ignore_nested) == -diff);
  if (MDBX_LIKELY(int16_t(diff) == diff))
    MDBX_CXX20_LIKELY return int(diff);
  else
    throw_incomparable_cursors();
}

inline cursor::move_result::move_result(const cursor &cursor, bool throw_notfound) : pair_result() {
  done = cursor.move(get_current, &this->key, &this->value, throw_notfound);
}

inline cursor::move_result::move_result(cursor &cursor, move_operation operation, const slice &key, const slice &value,
                                        bool throw_notfound)
    : pair_result(key, value, false) {
  this->done = cursor.move(operation, &this->key, &this->value, throw_notfound);
}

inline bool cursor::move(move_operation operation, MDBX_val *key, MDBX_val *value, bool throw_notfound) const {
  const int err = ::mdbx_cursor_get(handle_, key, value, MDBX_cursor_op(operation));
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY return true;
  case MDBX_RESULT_TRUE:
    return false;
  case MDBX_NOTFOUND:
    if (!throw_notfound)
      return false;
    MDBX_CXX17_FALLTHROUGH /* fallthrough */;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline cursor::estimate_result::estimate_result(const cursor &cursor, move_operation operation, const slice &key,
                                                const slice &value)
    : pair(key, value), approximate_quantity(PTRDIFF_MIN) {
  approximate_quantity = cursor.estimate(operation, &this->key, &this->value);
}

inline ptrdiff_t cursor::estimate(move_operation operation, MDBX_val *key, MDBX_val *value) const {
  ptrdiff_t result;
  error::success_or_throw(::mdbx_estimate_move(*this, key, value, MDBX_cursor_op(operation), &result));
  return result;
}

/// \brief Estimates the distance (number of items) between two cursor positions.
/// \param from  The cursor at the "from" position.
/// \param to    The cursor at the "to" position.
/// \returns The approximate distance; a negative value means the `to` position
///          is before the `from` one.
/// \throws The corresponding \ref mdbx::exception on failure.
inline ptrdiff_t estimate(const cursor &from, const cursor &to) {
  ptrdiff_t result;
  error::success_or_throw(mdbx_estimate_distance(from, to, &result));
  return result;
}

inline cursor::move_result cursor::find(const slice &key, bool throw_notfound) {
  return move(key_exact, key, throw_notfound);
}

inline cursor::move_result cursor::lower_bound(const slice &key, bool throw_notfound) {
  return move(key_lowerbound, key, throw_notfound);
}

inline cursor::move_result cursor::upper_bound(const slice &key, bool throw_notfound) {
  return move(key_greater_than, key, throw_notfound);
}

inline cursor::move_result cursor::find_multivalue(const slice &key, const slice &value, bool throw_notfound) {
  return move(multi_find_pair, key, value, throw_notfound);
}

inline cursor::move_result cursor::lower_bound_multivalue(const slice &key, const slice &value, bool throw_notfound) {
  return move(multi_exactkey_lowerboundvalue, key, value, throw_notfound);
}

inline cursor::move_result cursor::upper_bound_multivalue(const slice &key, const slice &value, bool throw_notfound) {
  return move(multi_exactkey_value_greater, key, value, throw_notfound);
}

inline bool cursor::seek(const slice &key) { return move(seek_key, const_cast<slice *>(&key), nullptr, false); }

inline size_t cursor::count_multivalue() const {
  size_t result;
  error::success_or_throw(::mdbx_cursor_count(*this, &result));
  return result;
}

inline size_t cursor::count_multivalue(MDBX_stat *stat) const {
  size_t result;
  error::success_or_throw(::mdbx_cursor_count_ex(*this, &result, stat, sizeof(*stat)));
  return result;
}

inline ::std::vector<pair> cursor::get_batch(size_t max_pairs, move_operation op, bool *is_last) const {
  if (is_last)
    *is_last = false;
  ::std::vector<pair> result;
  if (!max_pairs)
    return result;

  const size_t limit = max_pairs * 2;
  ::std::vector<MDBX_val> pairs(limit);
  size_t count = 0;
  int rc = ::mdbx_cursor_get_batch(handle_, &count, pairs.data(), limit, MDBX_cursor_op(op));
  if (rc == MDBX_NOTFOUND || rc == MDBX_ENODATA) {
    if (is_last)
      *is_last = true;
    return result;
  }
  if (rc == MDBX_RESULT_TRUE) {
    if (is_last)
      *is_last = true;
    rc = MDBX_SUCCESS;
  }
  error::success_or_throw(rc);

  MDBX_INLINE_API_ASSERT(count % 2 == 0 && count <= limit);
  result.reserve(count / 2);
  for (size_t i = 0; i + 1 < count; i += 2)
    result.emplace_back(slice(pairs[i]), slice(pairs[i + 1]));
  return result;
}

inline void cursor::ignore_key_order() { error::success_or_throw(::mdbx_cursor_ignord(handle_)); }

inline bool cursor::eof() const { return error::boolean_or_throw(::mdbx_cursor_eof(*this)); }

inline bool cursor::on_first() const { return error::boolean_or_throw(::mdbx_cursor_on_first(*this)); }

inline bool cursor::on_last() const { return error::boolean_or_throw(::mdbx_cursor_on_last(*this)); }

inline bool cursor::on_first_multival() const { return error::boolean_or_throw(::mdbx_cursor_on_first_dup(*this)); }

inline bool cursor::on_last_multival() const { return error::boolean_or_throw(::mdbx_cursor_on_last_dup(*this)); }

inline cursor::estimate_result cursor::estimate(const slice &key, const slice &value) const {
  return estimate_result(*this, multi_exactkey_lowerboundvalue, key, value);
}

inline cursor::estimate_result cursor::estimate(const slice &key) const {
  return estimate_result(*this, key_lowerbound, key);
}

inline cursor::estimate_result cursor::estimate(move_operation operation) const {
  return estimate_result(*this, operation);
}

inline cursor::estimate_result cursor::estimate(move_operation operation, const slice &key) const {
  return estimate_result(*this, operation, key);
}

inline void cursor::renew(::mdbx::txn &txn) { error::success_or_throw(::mdbx_cursor_renew(txn, handle_)); }

inline void cursor::bind(::mdbx::txn &txn, ::mdbx::map_handle map_handle) {
  error::success_or_throw(::mdbx_cursor_bind(txn, handle_, map_handle.dbi));
}

inline void cursor::unbind() { error::success_or_throw(::mdbx_cursor_unbind(handle_)); }

inline txn cursor::txn() const {
  MDBX_txn *txn = ::mdbx_cursor_txn(handle_);
  return ::mdbx::txn(txn);
}

inline map_handle cursor::map() const {
  const MDBX_dbi dbi = ::mdbx_cursor_dbi(handle_);
  if (MDBX_UNLIKELY(dbi > MDBX_MAX_DBI))
    error::throw_exception(MDBX_EINVAL);
  return map_handle(dbi);
}

inline MDBX_error_t cursor::put(const slice &key, slice *value, MDBX_put_flags_t flags) noexcept {
  return MDBX_error_t(::mdbx_cursor_put(handle_, &key, value, flags));
}

inline void cursor::put(const slice &key, slice value, put_mode mode) {
  error::success_or_throw(put(key, &value, MDBX_put_flags_t(mode)));
}

inline void cursor::insert(const slice &key, slice value) {
  error::success_or_throw(
      put(key, &value /* takes the present value in case MDBX_KEYEXIST */, MDBX_put_flags_t(put_mode::insert_unique)));
}

inline value_result cursor::try_insert(const slice &key, slice value) {
  const int err =
      put(key, &value /* takes the present value in case MDBX_KEYEXIST */, MDBX_put_flags_t(put_mode::insert_unique));
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{slice(), true};
  case MDBX_KEYEXIST:
    return value_result{value, false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline slice cursor::insert_reserve(const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  error::success_or_throw(put(key, &result /* takes the present value in case MDBX_KEYEXIST */,
                              MDBX_put_flags_t(put_mode::insert_unique) | MDBX_RESERVE));
  return result;
}

inline value_result cursor::try_insert_reserve(const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  const int err = put(key, &result /* takes the present value in case MDBX_KEYEXIST */,
                      MDBX_put_flags_t(put_mode::insert_unique) | MDBX_RESERVE);
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{result, true};
  case MDBX_KEYEXIST:
    return value_result{result, false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline void cursor::upsert(const slice &key, const slice &value) {
  error::success_or_throw(put(key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::upsert)));
}

inline slice cursor::upsert_reserve(const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  error::success_or_throw(put(key, &result, MDBX_put_flags_t(put_mode::upsert) | MDBX_RESERVE));
  return result;
}

inline void cursor::update(const slice &key, const slice &value) {
  error::success_or_throw(put(key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::update)));
}

inline bool cursor::try_update(const slice &key, const slice &value) {
  const int err = put(key, const_cast<slice *>(&value), MDBX_put_flags_t(put_mode::update));
  switch (err) {
  case MDBX_SUCCESS:
    return true;
  case MDBX_NOTFOUND:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline slice cursor::update_reserve(const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  error::success_or_throw(put(key, &result, MDBX_put_flags_t(put_mode::update) | MDBX_RESERVE));
  return result;
}

inline value_result cursor::try_update_reserve(const slice &key, size_t value_length) {
  slice result = slice::blank_for_put_reserve(value_length);
  const int err = put(key, &result, MDBX_put_flags_t(put_mode::update) | MDBX_RESERVE);
  switch (err) {
  case MDBX_SUCCESS:
    return value_result{result, true};
  case MDBX_NOTFOUND:
    return value_result{slice(), false};
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline bool cursor::erase(bool whole_multivalue) {
  const int err = ::mdbx_cursor_del(handle_, whole_multivalue ? MDBX_ALLDUPS : MDBX_CURRENT);
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY return true;
  case MDBX_NOTFOUND:
    return false;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline bool cursor::erase(const slice &key, bool whole_multivalue) {
  bool found = seek(key);
  return found ? erase(whole_multivalue) : found;
}

inline bool cursor::erase(const slice &key, const slice &value) {
  move_result data = find_multivalue(key, value, false);
  return data.done && erase();
}

inline size_t cursor::erase_bunch(bunch_delete action) {
  uint64_t affected;
  error::success_or_throw(::mdbx_cursor_bunch_delete(handle_, MDBX_bunch_action_t(action), &affected));
  return size_t(affected);
}

inline size_t cursor::erase_range(const cursor &end, bool end_including) {
  uint64_t affected;
  error::success_or_throw(::mdbx_cursor_delete_range(handle_, end.handle_, end_including, &affected));
  return size_t(affected);
}

inline void cursor::reset() { error::success_or_throw(::mdbx_cursor_reset(handle_)); }

inline size_t cursor::put_multiple_samelength(const slice &key, const size_t value_length, const void *values_array,
                                              size_t values_count, put_mode mode, bool allow_partial) {
  MDBX_val args[2] = {{const_cast<void *>(values_array), value_length}, {nullptr, values_count}};
  const int err = ::mdbx_cursor_put(handle_, const_cast<slice *>(&key), args, MDBX_put_flags_t(mode) | MDBX_MULTIPLE);
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY break;
  case MDBX_KEYEXIST:
    if (allow_partial)
      break;
    mdbx_txn_break(txn());
    MDBX_CXX17_FALLTHROUGH /* fallthrough */;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
  return args[1].iov_len /* done item count */;
}

inline ptrdiff_t cursor::distance_between(const cursor from, const cursor to, unsigned deepness) {
  intptr_t distance = PTRDIFF_MIN;
  error::success_or_throw(mdbx_cursor_distance(from, to, &distance, deepness));
  return distance;
}

inline bool cursor::scroll(intptr_t distance, unsigned deepness, bool throw_notfound) {
  const int err = ::mdbx_cursor_scroll(handle_, distance, deepness);
  switch (err) {
  case MDBX_SUCCESS:
    MDBX_CXX20_LIKELY return true;
  case MDBX_NOTFOUND:
    if (!throw_notfound)
      return false;
    MDBX_CXX17_FALLTHROUGH /* fallthrough */;
  default:
    MDBX_CXX20_UNLIKELY error::throw_exception(err);
  }
}

inline bool cursor::distribute(const cursor from, const cursor to, cursor *cursors_array, intptr_t cursors_array_size,
                               unsigned deepness) {
  static_assert(sizeof(cursors_array[0]) == sizeof(MDBX_cursor *), "oops");
  const int err = ::mdbx_cursor_distribute(from, to, &cursors_array[0].handle_, cursors_array_size, deepness);
  return error::boolean_or_throw(err);
}

inline bool cursor::distribute(const cursor from, const cursor to, const std::vector<cursor> &cursors_array,
                               unsigned deepness) {
  static_assert(sizeof(cursor) == sizeof(MDBX_cursor *), "oops");
  const int err = MDBX_LIKELY(!cursors_array.empty())
                      ? ::mdbx_cursor_distribute(from, to, const_cast<MDBX_cursor **>(&cursors_array[0].handle_),
                                                 cursors_array.size(), deepness)
                      : MDBX_EINVAL;
  return error::boolean_or_throw(err);
}

inline bool cursor::distribute(const cursor from, const cursor to, const std::vector<cursor_managed> &cursors_array,
                               unsigned deepness) {
  static_assert(sizeof(cursor_managed) == sizeof(MDBX_cursor *), "oops");
  const int err = MDBX_LIKELY(!cursors_array.empty())
                      ? ::mdbx_cursor_distribute(from, to, const_cast<MDBX_cursor **>(&cursors_array[0].handle_),
                                                 cursors_array.size(), deepness)
                      : MDBX_EINVAL;
  return error::boolean_or_throw(err);
}

//------------------------------------------------------------------------------

LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const slice &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const pair &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const pair_result &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::geometry::size &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::geometry &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::operate_parameters &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::mode &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::durability &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::reclaiming_options &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env::operate_options &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const env_managed::create_parameters &);

LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const MDBX_log_level_t &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const MDBX_debug_flags_t &);
LIBMDBX_API ::std::ostream &operator<<(::std::ostream &, const error &);

inline ::std::ostream &operator<<(::std::ostream &out, const MDBX_error_t &errcode) { return out << error(errcode); }

/// end cxx_api @}

} // namespace mdbx

//------------------------------------------------------------------------------

/// \brief The `std` namespace part of libmdbx C++ API
/// \ingroup cxx_api
namespace std {

/// \brief Converts the given \ref mdbx::slice to a string.
/// \param value  The slice to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::slice &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::buffer to a string.
/// \param buffer  The buffer to be converted.
/// \returns The string representation.
template <class ALLOCATOR, typename CAPACITY_POLICY>
inline string to_string(const ::mdbx::buffer<ALLOCATOR, CAPACITY_POLICY> &buffer) {
  ostringstream out;
  out << buffer;
  return out.str();
}

/// \brief Converts the given \ref mdbx::pair to a string.
/// \param value  The key-value pair to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::pair &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::geometry to a string.
/// \param value  The geometry to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::geometry &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::operate_parameters to a string.
/// \param value  The operate parameters to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::operate_parameters &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::mode to a string.
/// \param value  The mode to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::mode &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::durability to a string.
/// \param value  The durability to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::durability &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::reclaiming_options to a string.
/// \param value  The reclaiming options to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::reclaiming_options &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env::operate_options to a string.
/// \param value  The operate options to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env::operate_options &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::env_managed::create_parameters to a string.
/// \param value  The create parameters to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::env_managed::create_parameters &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref MDBX_log_level_t to a string.
/// \param value  The log level to be converted.
/// \returns The string representation.
inline string to_string(const ::MDBX_log_level_t &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref MDBX_debug_flags_t to a string.
/// \param value  The debug flags to be converted.
/// \returns The string representation.
inline string to_string(const ::MDBX_debug_flags_t &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref mdbx::error to a string.
/// \param value  The error to be converted.
/// \returns The string representation.
inline string to_string(const ::mdbx::error &value) {
  ostringstream out;
  out << value;
  return out.str();
}

/// \brief Converts the given \ref MDBX_error_t code to a string.
/// \param errcode  The error code to be converted.
/// \returns The string representation.
inline string to_string(const ::MDBX_error_t &errcode) { return to_string(::mdbx::error(errcode)); }

template <> struct hash<::mdbx::slice> {
  MDBX_CXX14_CONSTEXPR size_t operator()(const ::mdbx::slice &slice) const noexcept { return slice.hash_value(); }
};

template <> struct hash<::mdbx::map_handle> {
  MDBX_CXX11_CONSTEXPR size_t operator()(const ::mdbx::map_handle &handle) const noexcept { return handle.dbi; }
};

template <> struct hash<::mdbx::env> : public std::hash<const MDBX_env *> {
  using inherited = std::hash<const MDBX_env *>;
  size_t operator()(const ::mdbx::env &env) const noexcept { return inherited::operator()(env); }
};

template <> struct hash<::mdbx::txn> : public std::hash<const MDBX_txn *> {
  using inherited = std::hash<const MDBX_txn *>;
  size_t operator()(const ::mdbx::txn &txn) const noexcept { return inherited::operator()(txn); }
};

template <> struct hash<::mdbx::cursor> : public std::hash<const MDBX_cursor *> {
  using inherited = std::hash<const MDBX_cursor *>;
  size_t operator()(const ::mdbx::cursor &cursor) const noexcept { return inherited::operator()(cursor); }
};

template <class ALLOCATOR, typename CAPACITY_POLICY> struct hash<::mdbx::buffer<ALLOCATOR, CAPACITY_POLICY>> {
  size_t operator()(::mdbx::buffer<ALLOCATOR, CAPACITY_POLICY> const &buffer) const noexcept {
    return buffer.hash_value();
  }
};

} // namespace std

#if defined(__LCC__) && __LCC__ >= 126
#pragma diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
