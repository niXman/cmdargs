#pragma once

// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2026 niXman (github dot nixman at pm dot me)
// This file is part of CmdArgs(github.com/niXman/cmdargs) project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ----------------------------------------------------------------------------

//#include <iostream> // TODO: comment out

#include <algorithm>
#include <charconv>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <tuple>
#include <array>
#include <string>
#include <string_view>
#include <type_traits>
#include <exception>
#include <optional>
#include <utility>
#include <memory>
#include <functional>

#include <cstdint>
#include <cassert>

#ifndef CMDARGS_MAX_OPTION_DEPS
#define CMDARGS_MAX_OPTION_DEPS 3
#endif

#define __CMDARGS__STRINGIZE_I(x) #x
#define __CMDARGS__STRINGIZE(x) __CMDARGS__STRINGIZE_I(x)

#define __CMDARGS_CAT_I(l, r) l ## r
#define __CMDARGS_CAT(l, r) __CMDARGS_CAT_I(l, r)

// CMDARGS_VERSION_HEX >> 24 - is the major version
// CMDARGS_VERSION_HEX >> 16 - is the minor version
// CMDARGS_VERSION_HEX >> 8  - is the bugfix level

#define CMDARGS_VERSION_MAJOR 1
#define CMDARGS_VERSION_MINOR 0
#define CMDARGS_VERSION_BUGFIX 0

#define CMDARGS_VERSION_HEX \
    static_cast<std::uint32_t>((CMDARGS_VERSION_MAJOR << 24) \
        | (CMDARGS_VERSION_MINOR << 16) \
        | (CMDARGS_VERSION_BUGFIX << 8))

#define CMDARGS_VERSION_GET_MAJOR(x)  static_cast<std::uint8_t>(x >> 24)
#define CMDARGS_VERSION_GET_MINOR(x)  static_cast<std::uint8_t>(x >> 16)
#define CMDARGS_VERSION_GET_BUGFIX(x) static_cast<std::uint8_t>(x >> 8 )

#define CMDARGS_VERSION_STRING \
    __CMDARGS__STRINGIZE(CMDARGS_VERSION_MAJOR) \
    "." __CMDARGS__STRINGIZE(CMDARGS_VERSION_MINOR) \
    "." __CMDARGS__STRINGIZE(CMDARGS_VERSION_BUGFIX)

/*************************************************************************************************/

#ifndef CMDARGS_MAX_OPTIONS_SIZE
#   define CMDARGS_MAX_OPTIONS_SIZE 16
#endif

namespace cmdargs {

struct kwords_group;

/*************************************************************************************************/

struct invalid_argument: std::exception {
    invalid_argument(const invalid_argument &) = default;
    invalid_argument& operator= (const invalid_argument &) = default;
    invalid_argument(invalid_argument &&) = default;
    invalid_argument& operator= (invalid_argument &&) = default;

    invalid_argument(std::string msg)
        :m_msg{std::move(msg)}
    {}
    virtual ~invalid_argument() = default;
    virtual const char* what() const noexcept { return m_msg.c_str(); }

private:
    std::string m_msg;
};

/*************************************************************************************************/

namespace details {

constexpr inline char endl = '\n';
#ifdef _WIN32
constexpr inline char path_separator = '\\';
#else
constexpr inline char path_separator = '/';
#endif // _WIN32

/*************************************************************************************************/
// type name
// based on https://bitwizeshift.github.io/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/

template<std::size_t...Idxs>
constexpr auto substring_as_array(std::string_view str, std::index_sequence<Idxs...>) {
    return std::array{str[Idxs]...};
}

template <typename T>
constexpr auto type_name_array() noexcept {
#if defined(__clang__)
    constexpr auto prefix   = std::string_view{"[T = "};
    constexpr auto suffix   = std::string_view{"]"};
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
    constexpr auto prefix   = std::string_view{"with T = "};
    constexpr auto suffix   = std::string_view{"]"};
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
    constexpr auto prefix   = std::string_view{"type_name_array<"};
    constexpr auto suffix   = std::string_view{">(void)"};
    constexpr auto function = std::string_view{__FUNCSIG__};
#else
#   error Unsupported compiler
#endif
    constexpr auto double_colon = std::string_view{"::"};

    constexpr auto start = function.find(prefix) + prefix.size();
    constexpr auto end = function.rfind(suffix);
    static_assert(start < end);

    constexpr auto name = function.substr(start, (end - start));
    constexpr auto last_dc = name.rfind(double_colon);
    if constexpr ( last_dc != std::string_view::npos ) {
        constexpr auto name2 = name.substr(last_dc + double_colon.size());

        return substring_as_array(name2, std::make_index_sequence<name2.size()>{});
    } else {
        return substring_as_array(name, std::make_index_sequence<name.size()>{});
    }
}

template <typename T>
struct type_name_holder {
    static inline constexpr auto value = type_name_array<T>();
};

template <typename T>
constexpr auto type_name() noexcept -> std::string_view {
    // because of "std::basic_string"/"basic_string_view"
    if constexpr ( std::is_same_v<T, std::string> ) {
        return {"std::string"};
    } else if ( std::is_same_v<T, std::string_view> ) {
        return {"std::string_view"};
    }

    constexpr auto &value = type_name_holder<T>::value;
    return std::string_view{value.data(), value.size()};
}

/*************************************************************************************************/
// to_tuple

struct any_type { template<class T> constexpr operator T(); };

#if defined(__GNUC__) && !defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

template<class T, class Seq, class = void>
struct is_braces_constructible_n_impl : std::false_type {};

template<class T, std::size_t... I>
struct is_braces_constructible_n_impl<
    T
    ,std::index_sequence<I...>
    ,std::void_t<decltype(T{(I, std::declval<any_type>())...})>
> : std::true_type {};

#if defined(__GNUC__) && !defined(__clang__)
#   pragma GCC diagnostic pop
#elif defined(__clang__)
#   pragma clang diagnostic pop
#endif

template<class T, std::size_t N>
struct is_braces_constructible_n
    : std::conditional_t<
        (N == 0u)
        ,std::false_type
        ,is_braces_constructible_n_impl<T, std::make_index_sequence<N>>
    >
{};

template<class T, std::size_t L = 0u, std::size_t R = sizeof(T) + 1u>
constexpr std::size_t to_tuple_size_impl() {
    constexpr std::size_t M = (L + R) / 2u;
    if constexpr ( M == 0 ) {
        static_assert(std::is_empty_v<T>, "Unable to determine number of elements");

        return 0u;
    } else if constexpr ( L == M ) {
        return M;
    } else if constexpr ( is_braces_constructible_n<T, M>{} ) {
        return to_tuple_size_impl<T, M, R>();
    } else {
        return to_tuple_size_impl<T, L, M>();
    }
}

template<typename T>
using to_tuple_size = std::integral_constant<std::size_t, to_tuple_size_impl<T>()-1>;

template<typename T>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, 0>) noexcept {
    return std::make_tuple();
}

template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 1>) noexcept {
    const auto& [p0] = object;
    return std::tie(p0);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 2>) noexcept {
    const auto& [p0, p1] = object;
    return std::tie(p0, p1);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 3>) noexcept {
    const auto& [p0, p1, p2] = object;
    return std::tie(p0, p1, p2);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 4>) noexcept {
    const auto& [p0, p1, p2, p3] = object;
    return std::tie(p0, p1, p2, p3);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 5>) noexcept {
    const auto& [p0, p1, p2, p3, p4] = object;
    return std::tie(p0, p1, p2, p3, p4);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 6>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5] = object;
    return std::tie(p0, p1, p2, p3, p4, p5);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 7>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 8>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 9>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 10>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 11>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 12>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 13>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 14>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 15>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 16>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
}
#if CMDARGS_MAX_OPTIONS_SIZE >= 17
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 17>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 18
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 18>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 19
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 19>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 20
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 20>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 21
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 21>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 22
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 22>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 23
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 23>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 24
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 24>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 25
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 25>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 26
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 26>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 27
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 27>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 28
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 28>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 29
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 29>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 30
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 30>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 31
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 31>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30);
}
#endif
#if CMDARGS_MAX_OPTIONS_SIZE >= 32
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 32>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31] = object;
    return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31);
}
#endif

template<typename T, std::size_t N>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, N>) noexcept {
    static_assert(N < 33, "Please increase the number of placeholders");
}

template<
     typename T
    ,typename = std::enable_if_t<std::is_class_v<T>>
    ,typename S = to_tuple_size<std::decay_t<T>>
>
auto to_tuple(const T &kw) noexcept {
    return to_tuple_impl(kw, S{});
}

/*************************************************************************************************/
// string ops

inline std::string_view ltrim(std::string_view s, std::string_view ws) noexcept
{ return s.substr(s.find_first_not_of(ws)); }

inline std::string_view rtrim(std::string_view s, std::string_view ws) noexcept
{ return s.substr(0, s.find_last_not_of(ws) + 1); }

inline std::string_view trim(std::string_view s, std::string_view ws = " \t\n\r") noexcept
{ auto res = ltrim(s, ws); res = rtrim(res, ws); return res; }

template<
     template<typename, typename> typename Sequence
    ,typename T
    ,typename A
>
inline bool split(Sequence<T, A> &result, std::string_view str, char delim) {
    std::size_t start = 0;
    std::size_t found = str.find(delim);
    for ( ; found != std::string_view::npos; found = str.find(delim, start) ) {
        result.emplace_back(str.begin() + start, found - start);
        start = found + sizeof(delim);
    }

    if ( start != str.size() ) {
        result.emplace_back(str.begin() + start, str.length() - start);
    }

    return !result.empty();
}

inline std::string cat_vector(
     const char *pref
    ,const std::vector<std::string_view> &names
    ,bool double_quoted = false) noexcept
{
    std::string res;
    for ( auto it = names.begin(); it != names.end(); ++it ) {
        if ( double_quoted ) {
            res += "\"";
        }
        res += (pref ? pref : "");
        res += (*it);
        if ( double_quoted ) {
            res += "\"";
        }
        if ( std::next(it) != names.end() ) {
            res += ", ";
        }
    }

    return res;
}

template<typename T>
void from_string_impl(T *val, std::string_view str) {
    if constexpr ( std::is_same_v<T, std::string> ) {
        *val = str;
    } else if constexpr ( std::is_same_v<T, std::string_view> ) {
        *val = str;
    } else if constexpr ( std::is_same_v<T, bool> ) {
        *val = (str == "true" || str == "1");
    } else if constexpr ( std::is_integral_v<T> ) {
        int base = 10;
        if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            base = 16;
            str.remove_prefix(2);
        }
        auto [_, ec] = std::from_chars(str.begin(), str.end(), *val, base);
        if ( ec != std::errc{} ) {
            throw invalid_argument(
                "invalid argument received in cmdargs::details::from_string_impl(), line "
                __CMDARGS__STRINGIZE(__LINE__)
            );
        }
    } else if constexpr ( std::is_floating_point_v<T> ) {
        auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), *val);
        if ( ec != std::errc{} ) {
            throw invalid_argument(
                "invalid argument received in cmdargs::details::from_string_impl(), line "
                __CMDARGS__STRINGIZE(__LINE__)
            );
        }
    } else if constexpr ( std::is_enum_v<T> ) {
        std::underlying_type_t<T> tmp{};
        from_string_impl(&tmp, str);
        *val = static_cast<T>(tmp);
    } else if constexpr ( std::is_pointer_v<T> ) {
        *val = nullptr;
    }
}

/*************************************************************************************************/
// contains and filter

template<
     template<typename, typename> typename Pred
    ,typename T
    ,typename ...Types
>
struct contains
    :std::disjunction<Pred<T, Types>...>
{};

template<
     template<typename, typename> typename Pred
    ,typename T
    ,typename ...Types
>
struct contains<Pred, T, std::tuple<Types...>>
    :contains<Pred, T, Types...>
{};

template<
     template<typename, typename> typename Pred
    ,typename Out
    ,typename In
>
struct filter;

template<
     template<typename, typename> typename Pred
    ,typename... Out
    ,typename InCar
    ,typename... InCdr
>
struct filter<Pred, std::tuple<Out...>, std::tuple<InCar, InCdr...>> {
    using type = typename std::conditional_t<
         contains<Pred, InCar, Out...>::value
        ,filter<Pred, std::tuple<Out...>, std::tuple<InCdr...>>
        ,filter<Pred, std::tuple<Out..., InCar>, std::tuple<InCdr...>>
    >::type;
};

template<
     template<typename, typename> typename Pred
    ,typename Out
>
struct filter<Pred, Out, std::tuple<>> {
    using type = Out;
};

template<
     template<typename, typename> typename Pred
    ,typename T
>
using without_duplicates = typename filter<Pred, std::tuple<>, T>::type;

/*************************************************************************************************/
// is callable

template<typename F>
using has_operator_call_t = decltype(&F::operator());

template<typename F, typename = void>
struct is_callable: std::false_type
{};

template<typename F>
struct is_callable<
     F
    ,std::void_t<has_operator_call_t<typename std::decay_t<F>>>
>: std::true_type
{};

/*************************************************************************************************/
// callable traits

template<typename F>
struct callable_traits: callable_traits<decltype(&F::operator())>
{};

template<typename R, typename... Args>
struct callable_traits_base {
    using signature = R(Args...);
    using function  = std::function<R(Args...)>;
    static constexpr std::size_t size = sizeof...(Args);
};

template<typename Sig>
struct signature_first_arg_decay;

template<typename R, typename A0, typename ...Rest>
struct signature_first_arg_decay<R(A0, Rest...)> {
    using type = std::decay_t<A0>;
};

template<typename R, typename... Args>
struct callable_traits<R(*)(Args...)> :callable_traits_base<R, Args...>
{};

template<typename R, typename... Args>
struct callable_traits<R(&)(Args...)> :callable_traits_base<R, Args...>
{};

template<typename Obj, typename R, typename... Args>
struct callable_traits<R(Obj::*)(Args...) const> :callable_traits_base<R, Args...>
{};

template<typename F, typename ...Opts>
inline constexpr bool validator_with_deps_invocable_v = std::is_invocable_r_v<
    bool
    ,F
    ,std::string_view
    ,const std::optional<typename std::decay_t<Opts>::value_type>&...
>;

template<typename T>
inline constexpr bool always_false_v = false;

template<typename F, typename = void>
struct callable_operator_addressable_impl: std::false_type
{};

template<typename F>
struct callable_operator_addressable_impl<
    F
    ,std::void_t<decltype(&std::decay_t<F>::operator())>
>: std::true_type
{};

template<typename F>
inline constexpr bool callable_operator_addressable_v
    = callable_operator_addressable_impl<std::decay_t<F>>::value;

template<typename F, typename V, typename ...Opts>
inline constexpr bool converter_with_deps_invocable_v = std::is_invocable_r_v<
    bool
    ,F
    ,V &
    ,std::string_view
    ,const std::optional<typename std::decay_t<Opts>::value_type>&...
>;

/*************************************************************************************************/
// relations

enum class e_relation_type { AND, OR, NOT };

inline const char* relation_str(e_relation_type r) noexcept {
    static const char *arr[] = {
         "AND"
        ,"OR"
        ,"NOT"
        ,"UNKNOWN"
    };

    return arr[static_cast<unsigned>(r)];
}

template<e_relation_type E, typename ...Types>
struct relations_list {
    std::array<std::string_view, sizeof...(Types)> list;
};

template<e_relation_type E, std::size_t N>
struct relations_name_list {
    std::array<std::string_view, N> list;
};

// and
template<typename Unused, typename T>
struct relation_pred_and: std::false_type
{};

template<typename Unused, typename ...Types>
struct relation_pred_and<Unused, relations_list<e_relation_type::AND, Types...>>
    :std::true_type
{};

// or
template<typename Unused, typename T>
struct relation_pred_or: std::false_type
{};

template<typename Unused, typename ...Types>
struct relation_pred_or<Unused, relations_list<e_relation_type::OR, Types...>>
    :std::true_type
{};

// not
template<typename Unused, typename T>
struct relation_pred_not: std::false_type
{};

template<typename Unused, typename ...Types>
struct relation_pred_not<Unused, relations_list<e_relation_type::NOT, Types...>>
    :std::true_type
{};

template<typename Unused, e_relation_type E, std::size_t N>
struct relation_pred_not<Unused, relations_name_list<E, N>>
    :std::bool_constant<E == e_relation_type::NOT>
{};

// generalized
template<typename T>
struct is_relation_type
    :std::disjunction<
         relation_pred_and<char, T>
        ,relation_pred_or<char, T>
        ,relation_pred_not<char, T>
    >
{};

// helpers
template<typename ...Types>
struct contains_and
    :contains<relation_pred_and, char, Types...>
{};

template<typename ...Types>
struct contains_or
    :contains<relation_pred_or, char, Types...>
{};

template<typename ...Types>
struct contains_not
    :contains<relation_pred_not, char, Types...>
{};

template<typename T, typename = void>
struct is_option_ref_for_relation : std::false_type
{};

template<typename T>
struct is_option_ref_for_relation<
     T
    ,std::void_t<
         typename T::value_type
        ,decltype(std::declval<const T &>().name())
    >
>: std::true_type
{};

template<class T>
struct type_identity {
    using type = T;
};

// get by relation
template<
     template<typename, typename> typename Pred
    ,typename ...Rest
>
struct get_relation_list;

template<
     template<typename, typename> typename Pred
    ,typename T
    ,typename ...Rest
>
struct get_relation_list<Pred, T, Rest...> {
    using type = typename std::conditional_t<
         Pred<char, T>::value
        ,type_identity<T>
        ,get_relation_list<Pred, Rest...>
    >::type;
};

template<
    template<typename, typename> typename Pred
>
struct get_relation_list<Pred> {};

/*************************************************************************************************/
// default

template<typename T>
struct default_t {
    T val;
};

template<typename Unused, typename T>
struct is_default_pred: std::false_type
{};

template<typename Unused, typename T>
struct is_default_pred<Unused, default_t<T>>
    :std::true_type
{};

template<typename ...Types>
struct contains_default
    :contains<is_default_pred, char, Types...>
{};

/*************************************************************************************************/

struct optional_option_t {};

/*************************************************************************************************/

struct ext_none {};

template<typename... Opts>
struct validator_with_deps;

template<typename V, typename... Opts>
struct converter_with_deps;

template<typename T>
struct is_validator_with_deps: std::false_type
{};

template<typename... Opts>
struct is_validator_with_deps<validator_with_deps<Opts...>>: std::true_type
{};

template<typename T>
inline constexpr bool is_validator_with_deps_v = is_validator_with_deps<std::decay_t<T>>::value;

template<typename T>
struct is_converter_with_deps: std::false_type
{};

template<typename V, typename... Opts>
struct is_converter_with_deps<converter_with_deps<V, Opts...>>: std::true_type
{};

template<typename T>
inline constexpr bool is_converter_with_deps_v = is_converter_with_deps<std::decay_t<T>>::value;

template<typename... Ts>
struct first_validator_with_deps_or_none;

template<>
struct first_validator_with_deps_or_none<> {
    using type = ext_none;
};

template<typename Head, typename... Tail>
struct first_validator_with_deps_or_none<Head, Tail...> {
    using type = std::conditional_t<
         is_validator_with_deps_v<Head>
        ,Head
        ,typename first_validator_with_deps_or_none<Tail...>::type
    >;
};

template<typename... Ts>
struct first_converter_with_deps_or_none;

template<>
struct first_converter_with_deps_or_none<> {
    using type = ext_none;
};

template<typename Head, typename... Tail>
struct first_converter_with_deps_or_none<Head, Tail...> {
    using type = std::conditional_t<
         is_converter_with_deps_v<Head>
        ,Head
        ,typename first_converter_with_deps_or_none<Tail...>::type
    >;
};

template<typename Tuple>
struct tuple_ext_v_t_impl;

template<typename... Ts>
struct tuple_ext_v_t_impl<std::tuple<Ts...>> {
    using type = typename first_validator_with_deps_or_none<Ts...>::type;
};

template<typename Tuple>
struct tuple_ext_c_t_impl;

template<typename... Ts>
struct tuple_ext_c_t_impl<std::tuple<Ts...>> {
    using type = typename first_converter_with_deps_or_none<Ts...>::type;
};

template<typename Tuple>
using tuple_ext_v_t = typename tuple_ext_v_t_impl<std::decay_t<Tuple>>::type;

template<typename Tuple>
using tuple_ext_c_t = typename tuple_ext_c_t_impl<std::decay_t<Tuple>>::type;

template<typename... Ts>
inline constexpr std::size_t count_validator_with_deps_v
    = (0u + ... + (is_validator_with_deps_v<Ts> ? 1u : 0u));

template<typename Validator, typename... Ts>
inline constexpr std::size_t count_plain_validators_v
    = (0u + ... + (std::is_same_v<std::decay_t<Ts>, Validator> ? 1u : 0u));

template<typename... Ts>
inline constexpr std::size_t count_converter_with_deps_v
    = (0u + ... + (is_converter_with_deps_v<Ts> ? 1u : 0u));

template<typename Converter, typename... Ts>
inline constexpr std::size_t count_plain_converters_v
    = (0u + ... + (std::is_same_v<std::decay_t<Ts>, Converter> ? 1u : 0u));

/*************************************************************************************************/

} // ns details

/*************************************************************************************************/

template<typename ...Args>
struct args_pack;

#define __CMDARGS__OPTION_SUFFIX _tag

namespace details {

struct deps_storage_base {
    virtual ~deps_storage_base() = default;
    virtual std::unique_ptr<deps_storage_base> clone() const = 0;
    virtual const std::type_info &storage_type() const noexcept = 0;
    virtual void *storage_obj_void() noexcept = 0;
};

template<typename T>
struct typed_deps_storage final : deps_storage_base {
    T body;

    explicit typed_deps_storage(T &&b)
        : body{std::move(b)}
    {}

    std::unique_ptr<deps_storage_base> clone() const override {
        return std::make_unique<typed_deps_storage>(T{body});
    }

    const std::type_info &storage_type() const noexcept override {
        return typeid(T);
    }

    void *storage_obj_void() noexcept override {
        return static_cast<void *>(&body);
    }
};

struct option_ext_registry {
    struct entry {
        const void *key;
        std::unique_ptr<deps_storage_base> validator;
        std::unique_ptr<deps_storage_base> converter;
    };

    std::vector<entry> m_entries;

    void register_at(
         const void *key
        ,std::unique_ptr<deps_storage_base> v
        ,std::unique_ptr<deps_storage_base> c
    ) {
        if ( !v && !c ) {
            return;
        }

        m_entries.push_back(entry{key, std::move(v), std::move(c)});
    }

    void unregister_at(const void *key) noexcept {
        for ( auto it = m_entries.begin(); it != m_entries.end(); ++it ) {
            if ( it->key == key ) {
                m_entries.erase(it);

                return;
            }
        }
    }

    bool try_clone_for(
         const void *key
        ,std::unique_ptr<deps_storage_base> *out_v
        ,std::unique_ptr<deps_storage_base> *out_c
    ) const {
        for ( const auto &e : m_entries ) {
            if ( e.key != key ) {
                continue;
            }
            if ( e.validator && out_v ) {
                *out_v = e.validator->clone();
            }
            if ( e.converter && out_c ) {
                *out_c = e.converter->clone();
            }

            return true;
        }

        return false;
    }

    void retarget_key(const void *old_key, const void *new_key) noexcept {
        for ( auto &e : m_entries ) {
            if ( e.key == old_key ) {
                e.key = new_key;

                return;
            }
        }
    }
};

option_ext_registry *hook_option_ext_for_kwords(
     ::cmdargs::kwords_group *g
    ,const void *opt_key
    ,std::unique_ptr<deps_storage_base> v
    ,std::unique_ptr<deps_storage_base> c
);

template<typename Tuple, std::size_t N, std::size_t ...I>
inline void init_pack_dep_arrays_from_registry(
     std::array<std::unique_ptr<deps_storage_base>, N> &va
    ,std::array<std::unique_ptr<deps_storage_base>, N> &ca
    ,Tuple &&tup
    ,std::index_sequence<I...>
) {
    (void)std::initializer_list<int>{(
        (void)([&] {
            auto &opt = std::get<I>(tup);
            if ( auto *const reg = opt.deps_registry() ) {
                const void *const k = static_cast<const void *>(&opt);
                (void)reg->try_clone_for(k, &va[I], &ca[I]);
            }
        }()),
        0
    )...};
}

template<typename ...P>
bool rebind_validator_storage_into(
     deps_storage_base *stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
);

template<typename V, typename ...P>
bool rebind_converter_storage_into(
     deps_storage_base *stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
);

} // namespace details

template<typename ID, typename V>
struct option final {
    using value_type = V;
    using optional_type  = std::optional<value_type>;
    using validator_type = std::function<bool(std::string_view str)>;
    using converter_type = std::function<bool(value_type &dst, std::string_view str)>;

private:
    template<typename ...Args>
    friend struct args_pack;
    template<typename... Os>
    friend struct ::cmdargs::details::validator_with_deps;
    template<typename Vx, typename... Os>
    friend struct ::cmdargs::details::converter_with_deps;

    const std::string_view m_type_name;
    const std::string_view m_description;
    const bool m_is_required;
    const bool m_uses_custom_validator;
    validator_type m_validator;
    const bool m_uses_custom_converter;
    converter_type m_converter;
    const std::vector<std::string_view> m_relation_and;
    const std::vector<std::string_view> m_relation_or;
    const std::vector<std::string_view> m_relation_not;
    const optional_type m_default_value;
    optional_type m_value;
    details::option_ext_registry *m_deps_reg{};

public:
    details::option_ext_registry *deps_registry() const noexcept { return m_deps_reg; }

    option& operator= (const option &) = delete;
    option& operator= (option &&) = delete;
    option(const option &o)
        :m_type_name{o.m_type_name}
        ,m_description{o.m_description}
        ,m_is_required{o.m_is_required}
        ,m_uses_custom_validator{o.m_uses_custom_validator}
        ,m_validator{o.m_validator}
        ,m_uses_custom_converter{o.m_uses_custom_converter}
        ,m_converter{o.m_converter}
        ,m_relation_and{o.m_relation_and}
        ,m_relation_or{o.m_relation_or}
        ,m_relation_not{o.m_relation_not}
        ,m_default_value{o.m_default_value}
        ,m_value{o.m_value}
        ,m_deps_reg{}
    {}

    option(option &&o) noexcept
        :m_type_name{o.m_type_name}
        ,m_description{o.m_description}
        ,m_is_required{o.m_is_required}
        ,m_uses_custom_validator{o.m_uses_custom_validator}
        ,m_validator{std::move(o.m_validator)}
        ,m_uses_custom_converter{o.m_uses_custom_converter}
        ,m_converter{std::move(o.m_converter)}
        ,m_relation_and{o.m_relation_and}
        ,m_relation_or{o.m_relation_or}
        ,m_relation_not{o.m_relation_not}
        ,m_default_value{o.m_default_value}
        ,m_value{std::move(o.m_value)}
        ,m_deps_reg{o.m_deps_reg}
    {
        if ( m_deps_reg ) {
            m_deps_reg->retarget_key(static_cast<const void *>(&o), static_cast<const void *>(this));
            o.m_deps_reg = nullptr;
        }
    }

    template<typename ...Args>
    option(kwords_group *owner, const char *descr, std::tuple<Args...> as_tuple)
        :m_type_name{details::type_name<value_type>()}
        ,m_description{descr}
        ,m_is_required{!details::contains<std::is_same, details::optional_option_t, Args...>::value}
        ,m_uses_custom_validator{
            has_visitor<validator_type>(as_tuple)
            || !std::is_same_v<
                details::tuple_ext_v_t<std::tuple<Args...>>
                ,details::ext_none
            >
        }
        ,m_validator{init_visitor<validator_type>(as_tuple)}
        ,m_uses_custom_converter{
            has_visitor<converter_type>(as_tuple)
            || !std::is_same_v<
                details::tuple_ext_c_t<std::tuple<Args...>>
                ,details::ext_none
            >
        }
        ,m_converter{init_visitor<converter_type>(as_tuple)}
        ,m_relation_and{init_cond_list<details::e_relation_type::AND>(as_tuple)}
        ,m_relation_or{init_cond_list<details::e_relation_type::OR>(as_tuple)}
        ,m_relation_not{init_cond_list<details::e_relation_type::NOT>(as_tuple)}
        ,m_default_value{get_default_value(as_tuple)}
        ,m_value{}
    {
        using ext_validator_type = details::tuple_ext_v_t<std::tuple<Args...>>;
        using ext_converter_type = details::tuple_ext_c_t<std::tuple<Args...>>;
        static_assert(details::count_validator_with_deps_v<Args...> <= 1u);
        static_assert(details::count_converter_with_deps_v<Args...> <= 1u);
        static_assert(
            (details::count_validator_with_deps_v<Args...> == 0u)
            || (details::count_plain_validators_v<validator_type, Args...> == 0u)
            ,"cmdargs: option cannot combine validator_ with extra deps and plain validator_"
        );
        static_assert(
            (details::count_converter_with_deps_v<Args...> == 0u)
            || (details::count_plain_converters_v<converter_type, Args...> == 0u)
            ,"cmdargs: option cannot combine converter_ with extra deps and plain converter_"
        );

        std::unique_ptr<details::deps_storage_base> vstor;
        std::unique_ptr<details::deps_storage_base> cstor;

        if constexpr ( !std::is_same_v<ext_validator_type, details::ext_none> ) {
            vstor = std::make_unique<details::typed_deps_storage<ext_validator_type>>(
                std::move(std::get<ext_validator_type>(as_tuple))
            );
        }
        if constexpr ( !std::is_same_v<ext_converter_type, details::ext_none> ) {
            cstor = std::make_unique<details::typed_deps_storage<ext_converter_type>>(
                std::move(std::get<ext_converter_type>(as_tuple))
            );
        }

        if ( vstor || cstor ) {
            assert(
                owner
                && "cmdargs: options with validator_/converter_ dependencies must be members of kwords_group"
            );
            m_deps_reg = details::hook_option_ext_for_kwords(
                owner
                ,static_cast<const void *>(this)
                ,std::move(vstor)
                ,std::move(cstor)
            );
        }
    }
    ~option() noexcept {
        if ( m_deps_reg ) {
            m_deps_reg->unregister_at(static_cast<const void *>(this));
        }
    }

    template<typename U>
    option operator= (U &&r) const noexcept {
        option res{*this};
        res.m_value = std::forward<U>(r);

        return res;
    }

    static constexpr std::string_view name() noexcept {
        constexpr auto n = details::type_name<ID>();
        return n.substr(0, n.length()-(sizeof(__CMDARGS__STRINGIZE(__CMDARGS__OPTION_SUFFIX))-1));
    }
    std::string_view type_name() const noexcept { return m_type_name; }
    std::string_view description() const noexcept { return m_description; }
    bool has_default() const noexcept { return m_default_value.has_value(); }
    const auto& get_default_value() const noexcept { return m_default_value.value(); }
    bool is_required() const noexcept { return m_is_required; }
    bool is_optional() const noexcept { return !is_required(); }
    bool is_set() const noexcept { return m_value.has_value(); }
    const auto& get_value() const noexcept { return m_value.value(); }
    void set_value(value_type v) { m_value = std::move(v); }
    bool is_bool() const noexcept { return std::is_same_v<value_type, bool>; }

    const auto& and_list() const noexcept { return m_relation_and; }
    const auto& or_list () const noexcept { return m_relation_or;  }
    const auto& not_list() const noexcept { return m_relation_not; }

    bool uses_custom_validator() const noexcept { return m_uses_custom_validator; }
    bool validate(std::string_view str) const noexcept { return m_validator(str); }
    bool uses_custom_converter() const noexcept { return m_uses_custom_converter; }
    bool convert(std::string_view str) {
        value_type v{};
        if ( m_converter(v, str) ) {
            m_value = std::move(v);
            return true;
        }
        return false;
    }

    template<typename OS>
    OS& dump(OS &os) const {
        const auto flags = os.flags();
        os
            << "name            : " << name() << details::endl
            << "type            : " << m_type_name << details::endl
            << "description     : " << '"' << m_description << '"' << details::endl
            << "is required     : " << (m_is_required ? "true" : "false") << details::endl
            << "value           : "
        ;
        if ( m_value.has_value() ) {
            os << m_value.value();
        } else {
            if ( m_default_value.has_value() ) {
                os
                    << m_default_value.value()
                    << " (D)"
                ;
            } else {
                os << "<UNINITIALIZED>";
            }
        }
        os
            << details::endl
            << "custom validator: " << (uses_custom_validator() ? "true" : "false") << details::endl
            << "custom converter: " << (uses_custom_converter() ? "true" : "false") << details::endl
            << "relation     AND: " << m_relation_and.size()
        ;
        if ( m_relation_and.size() )
        { os << " (" << details::cat_vector("--", m_relation_and) << ")" << details::endl; }
        else { os << details::endl; }
        os  << "relation      OR: " << m_relation_or.size();
        if ( m_relation_or.size() )
        { os << " (" << details::cat_vector("--", m_relation_or) << ")" << details::endl; }
        else { os << details::endl; }
        os  << "relation     NOT: " << m_relation_not.size();
        if ( m_relation_not.size() )
        { os << " (" << details::cat_vector("--", m_relation_not) << ")" << details::endl; }
        else { os << details::endl; }

        os.flags(flags);

        return os;
    }

    template<typename ...P>
    void rebind_to_pack(args_pack<P...> &pack, std::size_t idx) {
        if ( pack.m_validator_dep[idx] ) {
            if ( !details::rebind_validator_storage_into<P...>(
                    pack.m_validator_dep[idx].get()
                    ,m_validator
                    ,pack
                )
            ) {
                assert(false && "cmdargs: rebind validator_with_deps failed");
            }
        }
        if ( pack.m_converter_dep[idx] ) {
            if ( !details::rebind_converter_storage_into<value_type, P...>(
                    pack.m_converter_dep[idx].get()
                    ,m_converter
                    ,pack
                )
            ) {
                assert(false && "cmdargs: rebind converter_with_deps failed");
            }
        }
    }

private:
    static bool default_converter(value_type &dst, std::string_view str) {
        details::from_string_impl(&dst, str);
        return true;
    }
    static bool default_validator(std::string_view /*str*/) {
        return true;
    }

    template<typename Req, typename ...Types>
    static constexpr bool has_visitor(const std::tuple<Types...> &/*tuple*/) noexcept {
        return details::contains<std::is_same, Req, Types...>::value;
    }
    template<typename Req, typename ...Types>
    static constexpr typename std::enable_if_t<std::is_same_v<Req, validator_type>, Req>
    init_visitor(std::tuple<Types...> &tuple) noexcept {
        if constexpr ( details::contains<std::is_same, Req, Types...>::value ) {
            return Req{std::move(std::get<Req>(tuple))};
        } else {
            return Req{default_validator};
        }
    }
    template<typename Req, typename ...Types>
    static constexpr typename std::enable_if_t<std::is_same_v<Req, converter_type>, Req>
    init_visitor(std::tuple<Types...> &tuple) noexcept {
        if constexpr ( details::contains<std::is_same, Req, Types...>::value ) {
            return Req{std::move(std::get<Req>(tuple))};
        } else {
            return Req{default_converter};
        }
    }

    template<details::e_relation_type Rel, typename ...Types>
    static std::vector<std::string_view>
    init_cond_list(const std::tuple<Types...> &tuple) noexcept {
        if constexpr ( Rel == details::e_relation_type::AND
            && details::contains_and<Types...>::value )
        {
            using list_type = typename details::get_relation_list<
                details::relation_pred_and, Types...>::type;
            const auto &list = std::get<list_type>(tuple).list;
            if ( list.size() ) return {std::begin(list), std::end(list)};
        }
        if constexpr ( Rel == details::e_relation_type::OR
            && details::contains_or<Types...>::value )
        {
            using list_type = typename details::get_relation_list<
                details::relation_pred_or, Types...>::type;
            const auto &list = std::get<list_type>(tuple).list;
            if ( list.size() ) return {std::begin(list), std::end(list)};
        }
        if constexpr ( Rel == details::e_relation_type::NOT
            && details::contains_not<Types...>::value )
        {
            using list_type = typename details::get_relation_list<
                details::relation_pred_not, Types...>::type;
            const auto &list = std::get<list_type>(tuple).list;
            if ( list.size() ) return {std::begin(list), std::end(list)};
        }

        return {};
    }
    template<typename ...Types>
    static optional_type get_default_value(const std::tuple<Types...> &tuple) noexcept {
        if constexpr ( details::contains_default<Types...>::value ) {
            auto def = std::get<details::default_t<value_type>>(tuple);
            return {std::move(def.val)};
        }

        return {};
    }
};

/*************************************************************************************************/
// help and version types

namespace details {

using help_option_type = option<
     __CMDARGS_CAT(struct help, __CMDARGS__OPTION_SUFFIX)
    ,bool
>;
using version_option_type = option<
     __CMDARGS_CAT(struct version, __CMDARGS__OPTION_SUFFIX)
    ,std::string
>;

} // ns details

/*************************************************************************************************/
// predefined converters

namespace details {

template<
     template<typename, typename> class Sequence
    ,typename T
    ,typename A
>
bool convert_as_sequence(Sequence<T, A> &dst, std::string_view str, char sep) {
    std::vector<std::string_view> vec;
    split(vec, str, sep);
    for ( const auto &it: vec) {
        T v{};
        from_string_impl(&v, it);

        dst.emplace_back(std::move(v));
    }

    return true;
}

template<typename T>
bool convert_as_vector(std::vector<T> &dst, std::string_view str, char sep) {
    return convert_as_sequence(dst, str, sep);
}

template<typename T>
bool convert_as_list(std::list<T> &dst, std::string_view str, char sep) {
    return convert_as_sequence(dst, str, sep);
}

template<typename K, typename C, typename A>
bool convert_as_set(std::set<K, C, A> &dst, std::string_view str, char sep) {
    std::vector<std::string> vec;
    convert_as_sequence(vec, str, sep);
    for ( const auto &it: vec ) {
        K k{};
        from_string_impl(&k, {it.data(), it.size()});

        dst.emplace(std::move(k));
    }

    return true;
}

template<typename K, typename V, typename C, typename A>
bool convert_as_map(std::map<K, V, C, A> &dst, std::string_view str, char pair_sep, char kv_sep) {
    std::vector<std::string_view> vec;
    convert_as_sequence(vec, str, pair_sep);
    for ( const auto &it: vec ) {
        std::vector<std::string_view> pair;
        convert_as_sequence(pair, it, kv_sep);
        if ( pair.size() != 2 )
            return false;

        K k{};
        from_string_impl(&k, pair[0]);

        V v{};
        from_string_impl(&v, pair[1]);

        dst.emplace(std::move(k), std::move(v));
    }

    return true;
}

} // ns details

/*************************************************************************************************/

struct kwords_group {
    kwords_group() { register_ext_registry(this); }
    ~kwords_group() { unregister_ext_registry(this); }

    kwords_group(const kwords_group &) = delete;
    kwords_group &operator=(const kwords_group &) = delete;

    static constexpr details::optional_option_t optional{};

    template<typename T>
    static auto default_(T &&v) noexcept
    { return details::default_t<T>{std::forward<T>(v)}; }

    template<typename ...Types>
    static auto and_(const Types &...args) noexcept
    { return get_relations<details::e_relation_type::AND>(args...); }
    template<typename ...Types>
    static auto or_(const Types &...args) noexcept
    { return get_relations<details::e_relation_type::OR>(args...); }
    template<typename ...Types>
    static auto not_(const Types &...args) noexcept {
        return get_relations<details::e_relation_type::NOT>(args...);
    }

    template<typename F>
    static auto validator_(F &&f) noexcept {
        static_assert(details::is_callable<F>::value);
        using signature = typename details::callable_traits<F>::signature;
        static_assert(std::is_same_v<signature, bool(const std::string_view str)>);
        return std::function<signature>{std::forward<F>(f)};
    }

    template<typename F, typename ...Opts>
    static auto validator_(F &&f, const Opts &...opts) noexcept {
        static_assert(sizeof...(Opts) > 0u);
        static_assert(
            sizeof...(Opts) <= CMDARGS_MAX_OPTION_DEPS
            ,"cmdargs: too many validator dependencies (raise CMDARGS_MAX_OPTION_DEPS)"
        );
        static_assert(
            details::validator_with_deps_invocable_v<F, Opts...>
            ,"cmdargs: validator with deps must be bool(string_view, const std::optional<T>&...)"
        );
        return details::validator_with_deps<std::decay_t<Opts>...>{
            std::forward<F>(f)
            ,opts...
        };
    }

    template<typename F>
    static auto converter_(F &&f) noexcept {
        static_assert(details::is_callable<F>::value);
        static_assert(details::callable_traits<F>::size == 2);
        using signature = typename details::callable_traits<F>::signature;
        return std::function<signature>{std::forward<F>(f)};
    }

    template<typename F, typename ...Opts>
    static auto converter_(F &&f, const Opts &...opts) noexcept {
        static_assert(sizeof...(Opts) > 0u);
        static_assert(
            sizeof...(Opts) <= CMDARGS_MAX_OPTION_DEPS
            ,"cmdargs: too many converter dependencies (raise CMDARGS_MAX_OPTION_DEPS)"
        );
        using DT = std::decay_t<F>;
        if constexpr ( details::callable_operator_addressable_v<DT> ) {
            static_assert(details::is_callable<F>::value);
            static_assert(
                details::callable_traits<DT>::size == 2u + sizeof...(Opts)
                ,"cmdargs: converter with deps must be bool(T&, string_view, const std::optional<U>&...)"
            );
            using conv_sig = typename details::callable_traits<DT>::signature;
            using V = typename details::signature_first_arg_decay<conv_sig>::type;
            static_assert(
                details::converter_with_deps_invocable_v<F, V, Opts...>
                ,"cmdargs: converter with deps must be bool(T&, string_view, const std::optional<U>&...)"
            );
            return details::converter_with_deps<V, std::decay_t<Opts>...>{
                std::forward<F>(f)
                ,opts...
            };
        } else {
            static_assert(
                details::always_false_v<F>
                ,"cmdargs: converter with deps: value type is not deducible for a generic lambda; use converter_for<V>(f, dep_options...)"
            );
        }
    }

    template<typename V, typename F, typename ...Opts>
    static auto converter_for(F &&f, const Opts &...opts) noexcept {
        static_assert(sizeof...(Opts) > 0u);
        static_assert(
            sizeof...(Opts) <= CMDARGS_MAX_OPTION_DEPS
            ,"cmdargs: too many converter dependencies (raise CMDARGS_MAX_OPTION_DEPS)"
        );
        static_assert(
            details::converter_with_deps_invocable_v<F, V, Opts...>
            ,"cmdargs: converter with deps must be bool(T&, string_view, const std::optional<U>&...)"
        );
        return details::converter_with_deps<V, std::decay_t<Opts>...>{
            std::forward<F>(f)
            ,opts...
        };
    }

    // predefined converters
    template<typename T>
    static auto convert_as_vector(char sep = ',') noexcept {
        return converter_(
            [sep](std::vector<T> &dst, std::string_view str){
                return details::convert_as_vector(dst, str, sep);
            }
        );
    }
    template<typename T>
    static auto convert_as_list(char sep = ',') noexcept {
        return converter_(
            [sep](std::list<T> &dst, std::string_view str){
                return details::convert_as_list(dst, str, sep);
            }
        );
    }
    template<typename T>
    static auto convert_as_set(char sep = ',') noexcept {
        return converter_(
            [sep](std::set<T> &dst, std::string_view str){
                return details::convert_as_set(dst, str, sep);
            }
        );
    }
    template<typename K, typename V>
    static auto convert_as_map(char pair_sep = ',', char kv_sep = '=') noexcept {
        return converter_(
            [pair_sep, kv_sep](std::map<K, V> &dst, std::string_view str){
                return details::convert_as_map(dst, str, pair_sep, kv_sep);
            }
        );
    }

private:
    static std::vector<
        std::pair<kwords_group *, std::unique_ptr<details::option_ext_registry>>
    > &ext_registry_storage() noexcept {
        thread_local std::vector<
            std::pair<kwords_group *, std::unique_ptr<details::option_ext_registry>>
        > storage;

        return storage;
    }

    static void register_ext_registry(kwords_group *g) {
        auto &s = ext_registry_storage();
        assert(
            std::find_if(
                 s.begin()
                ,s.end()
                ,[g](const auto &p) { return p.first == g; }
            ) == s.end()
        );
        s.emplace_back(g, std::make_unique<details::option_ext_registry>());
    }

    static void unregister_ext_registry(kwords_group *g) {
        auto &s = ext_registry_storage();
        const auto it = std::find_if(
             s.begin()
            ,s.end()
            ,[g](const auto &p) { return p.first == g; }
        );
        assert(it != s.end() && "cmdargs: kwords_group ext registry not registered");
        s.erase(it);
    }

    static details::option_ext_registry *lookup_ext_registry(kwords_group *g) noexcept {
        auto &s = ext_registry_storage();
        const auto it = std::find_if(
             s.begin()
            ,s.end()
            ,[g](const auto &p) { return p.first == g; }
        );

        return it == s.end() ? nullptr : it->second.get();
    }

    friend details::option_ext_registry *details::hook_option_ext_for_kwords(
         kwords_group *g
        ,const void *opt_key
        ,std::unique_ptr<details::deps_storage_base> v
        ,std::unique_ptr<details::deps_storage_base> c
    );

    template<details::e_relation_type R, typename ...Types>
    static auto get_relations(const Types &...args) noexcept {
        static_assert(
            (... && details::is_option_ref_for_relation<std::decay_t<Types>>::value)
            ,"cmdargs: and_, or_ and not_ only accept option objects"
        );
        using tuple_type = std::tuple<typename std::decay_t<Types>...>;
        static_assert(
            std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<std::is_same, tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return details::relations_list<R, Types...>{args.name()...};
    }
};

namespace details {

inline option_ext_registry *hook_option_ext_for_kwords(
     ::cmdargs::kwords_group *g
    ,const void *opt_key
    ,std::unique_ptr<deps_storage_base> v
    ,std::unique_ptr<deps_storage_base> c
) {
    assert(g && (v || c));
    auto *const reg = kwords_group::lookup_ext_registry(g);
    assert(reg && "cmdargs: kwords_group not registered for ext storage");
    reg->register_at(opt_key, std::move(v), std::move(c));

    return reg;
}

} // namespace details

/*************************************************************************************************/

template<typename ...Args>
struct args_pack final {
private:
    using container_type = std::tuple<Args...>;
    static_assert(
         std::tuple_size<container_type>::value
            == std::tuple_size<details::without_duplicates<std::is_same, container_type>>::value
        ,"duplicates of keywords are detected!"
    );

    std::array<std::unique_ptr<details::deps_storage_base>, sizeof...(Args)> m_validator_dep{};
    std::array<std::unique_ptr<details::deps_storage_base>, sizeof...(Args)> m_converter_dep{};

    container_type m_kwords;

    template<typename ID, typename V>
    friend struct option;

    template<typename... Os>
    friend struct ::cmdargs::details::validator_with_deps;
    template<typename Vx, typename... Os>
    friend struct ::cmdargs::details::converter_with_deps;

    template<typename ...Types>
    static container_type init_kwords_and_dep_storage(
         std::array<std::unique_ptr<details::deps_storage_base>, sizeof...(Args)> &va
        ,std::array<std::unique_ptr<details::deps_storage_base>, sizeof...(Args)> &ca
        ,Types &&...types
    ) {
        constexpr std::size_t n = sizeof...(Args);
        static_assert(sizeof...(Types) == n);
        auto src_refs = std::forward_as_tuple(types...);
        details::init_pack_dep_arrays_from_registry(
             va
            ,ca
            ,src_refs
            ,std::make_index_sequence<n>{}
        );

        return container_type{std::forward<Types>(types)...};
    }

    template<std::size_t ...I>
    void rebind_slots_impl(std::index_sequence<I...>) {
        (void)(std::get<I>(m_kwords).rebind_to_pack(*this, I), ...);
    }

public:
    template<typename ...Types>
    explicit args_pack(Types && ...types)
        :m_validator_dep{}
        ,m_converter_dep{}
        ,m_kwords{init_kwords_and_dep_storage(
             m_validator_dep
            ,m_converter_dep
            ,std::forward<Types>(types)...
        )}
    {
        constexpr std::size_t n = sizeof...(Args);
        static_assert(sizeof...(Types) == n);
        rebind_slots_impl(std::make_index_sequence<n>{});
    }

    constexpr std::size_t size() const noexcept { return sizeof...(Args); }

    template<typename T>
    static constexpr bool contains(const T &) noexcept
    { return details::contains<std::is_same, T, Args...>::value; }
    template<typename T>
    static constexpr bool contains() noexcept
    { return details::contains<std::is_same, T, Args...>::value; }

    template<typename Opt>
    std::optional<typename Opt::value_type> optional_for() const {
        static_assert(contains<Opt>(), "cmdargs: dependency option is absent from this args_pack");

        const auto &o = std::get<Opt>(m_kwords);
        if ( o.is_set() ) { return o.get_value(); }
        if ( o.has_default() ) { return o.get_default_value(); }

        return std::nullopt;
    }

    auto optionals() const noexcept {
        return std::make_tuple(std::get<Args>(m_kwords).m_value...);
    }
    auto values() const {
        auto res = std::make_tuple(
            (std::get<Args>(m_kwords).m_value.has_value()
                ? std::get<Args>(m_kwords).m_value.value()
                : typename Args::value_type{})...
        );
        return res;
    }

    template<typename T>
    bool is_set() const {
        static_assert(contains<T>(), "");

        return std::get<T>(m_kwords).is_set();
    }

    template<typename T>
    bool is_set(const T &) const {
        static_assert(contains<T>(), "");

        return std::get<T>(m_kwords).is_set();
    }

    template<typename T>
    bool has_default(const T &) const {
        static_assert(contains<T>(), "");

        return std::get<T>(m_kwords).has_default();
    }

    bool is_bool_type(const std::string_view name) const {
        bool res{};

        for_each(
            [&res, &name](const auto &item) {
                if ( item.name() == name ) {
                    res = item.is_bool();
                    return false;
                }
                return true;
            }
            ,false
        );

        return res;
    }

    template<typename T>
    const auto& get() const {
        static_assert(contains<T>(), "");

        const auto &val = std::get<T>(m_kwords);
        if ( is_set<T>() ) {
            return val.get_value();
        }

        return val.get_default_value();
    }
    template<typename T>
    const auto& get(const T &k) const {
        static_assert(contains<T>(), "");

        const auto &val = std::get<T>(m_kwords);
        if ( is_set(k) ) {
            return val.get_value();
        }

        return val.get_default_value();
    }
    template<typename T, typename Default>
    typename T::value_type get(const T &k, Default &&def) const {
        if ( is_set(k) ) {
            return std::get<T>(m_kwords).get_value();
        }

        return std::forward<Default>(def);
    }

    template<typename T>
    const auto& operator[] (const T &k) const { return get(k); }

    const auto& operator() () const { return m_kwords; }

    template<typename OS>
    OS& dump(OS &os, bool inited_only = false) const {
        return to_file(os, *this, inited_only);
    }
    template<typename OS>
    friend OS& operator<< (OS &os, const args_pack &set) {
        return set.dump(os);
    }

    // for debug only
    template<typename OS>
    void show_this(OS &os) const {
        for_each(
            [&os](const auto &item)
            { os << "  " << item.name() << ": this="; item.show_this(os) << details::endl; }
        );
    }

    // F -> bool(const auto & option<...>)
    // F should return TRUE to continue, of FALSE to break.
    template<typename F>
    void for_each(F &&f, bool inited_only = false) const {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }
    template<typename F>
    void for_each(F &&f, bool inited_only = false) {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }

private:
    template<typename Iter, typename ...TArgs>
    friend void parse_kv_list(
         std::string *emsg
        ,const char *pref
        ,std::size_t pref_len
        ,Iter beg
        ,Iter end
        ,args_pack<TArgs...> &set
    );

    bool get_is_set(const std::string_view name) const {
        bool res = false;
        for_each(
             m_kwords
            ,[&res, &name](const auto &item) {
                if ( item.name() == name ) {
                    res = item.is_set();
                }
                return res == false;
            }
            ,false
        );

        return res;
    }

    std::string_view
    check_for_unexpected(const std::string_view optname) const {
        std::string_view res;
        for_each(
             m_kwords
            ,[&res, optname](const auto &item) {
                if ( item.name() == optname ) {
                    res = optname;
                }
                return res.empty();
            }
            ,false
        );

        return res.empty() ? optname : std::string_view{};
    }
    std::string_view check_for_required() const {
        std::string_view res;
        for_each(
             m_kwords
            ,[&res](const auto &item){
                if ( item.is_required() && !item.is_set() ) {
                    res = item.name();
                }
                return res.empty();
             }
            ,false
        );

        return res;
    }

    using cond_ret_type = std::pair<
         std::string_view
        ,std::vector<std::string_view>
    >;
    cond_ret_type check_for_cond_and() const {
        return check_for_conditional(
            [](const auto &item) {
                return item.is_set()
                    ? item.and_list()
                    : std::vector<std::string_view>{}
                ;
            }
            ,[this](const auto &list) {
                std::vector<std::string_view> res;
                for ( const auto &it: list ) {
                    bool is_set = get_is_set(it);
                    if ( !is_set ) {
                        res.push_back(it);
                    }
                }
                return res;
            }
        );
    }
    cond_ret_type check_for_cond_or() const {
        return check_for_conditional(
             [](const auto &item) { return item.or_list(); }
            ,[this](const auto &list) {
                auto num = 0u;
                std::vector<std::string_view> ret;
                for ( const auto &it: list ) {
                    bool is_set = get_is_set(it);
                    if ( is_set ) {
                        ++num;
                        ret.push_back(it);
                    }
                }
                if ( num == 1 ) {
                    ret.clear();
                } else {
                    ret = list;
                }
                return ret;
            }
        );
    }
    cond_ret_type check_for_cond_not() const {
        return check_for_conditional(
             [](const auto &item) {
                return item.is_set()
                    ? item.not_list()
                    : std::vector<std::string_view>{}
                ;
            }
            ,[this](const auto &list) {
                std::vector<std::string_view> ret;
                for ( const auto &it: list ) {
                    bool is_set = get_is_set(it);
                    if ( is_set ) {
                        ret.push_back(it);
                    }
                }
                return ret;
            }
        );
    }

private:
    template<typename GetF, typename CmpF>
    cond_ret_type check_for_conditional(const GetF &get, const CmpF &cmp) const {
        cond_ret_type ret;
        for_each(
             m_kwords
            ,[&get, &cmp, &ret](const auto &item) {
                ret.first = item.name();
                const auto &list = get(item);
                ret.second = cmp(list);
                return ret.second.empty();
            }
            ,false
        );

        return ret;
    }

private:
    template<typename Tuple, typename Func>
    static void for_each(const Tuple &tuple, const Func &func, bool inited_only) {
        std::apply(
            [&func, inited_only](const auto &...items) {
                auto call = [&func, inited_only](const auto &item) -> bool {
                    if (inited_only && !item.is_set()) return true;
                    return func(item);
                };
                (void)(call(items) && ...);
            }
            ,tuple
        );
    }
    template<typename Tuple, typename Func>
    static void for_each(Tuple &tuple, const Func &func, bool inited_only) {
        std::apply(
            [&func, inited_only](auto &...items) {
                auto call = [&func, inited_only](auto &item) -> bool {
                    if (inited_only && !item.is_set()) return true;
                    return func(item);
                };
                (void)(call(items) && ...);
            }
            ,tuple
        );
    }
};

/*************************************************************************************************/

namespace details {

template<typename... Opts>
struct validator_with_deps {
    using fn_type = std::function<bool(
         std::string_view
        ,const std::optional<typename Opts::value_type> &...
    )>;
    fn_type f;

    template<
         typename Fw
        ,typename = std::enable_if_t<std::is_constructible_v<fn_type, Fw>>
    >
    validator_with_deps(Fw &&fn, const Opts &...opts)
        : f{std::forward<Fw>(fn)}
    {
        (void)std::initializer_list<int>{(static_cast<void>(opts), 0)...};
    }

    template<typename ...P>
    bool call(std::string_view s, const args_pack<P...> &pack) const {
        static_assert((... && args_pack<P...>::template contains<std::decay_t<Opts>>()));

        return f(
            s
            ,std::get<std::decay_t<Opts>>(pack.m_kwords).m_value...
        );
    }
};

template<typename V, typename... Opts>
struct converter_with_deps {
    using fn_type = std::function<bool(
         V &
        ,std::string_view
        ,const std::optional<typename Opts::value_type> &...
    )>;
    fn_type f;

    template<
         typename Fw
        ,typename = std::enable_if_t<std::is_constructible_v<fn_type, Fw>>
    >
    converter_with_deps(Fw &&fn, const Opts &...opts)
        : f{std::forward<Fw>(fn)}
    {
        (void)std::initializer_list<int>{(static_cast<void>(opts), 0)...};
    }

    template<typename ...P>
    bool call(V &dst, std::string_view s, const args_pack<P...> &pack) const {
        static_assert((... && args_pack<P...>::template contains<std::decay_t<Opts>>()));

        return f(
            dst
            ,s
            ,std::get<std::decay_t<Opts>>(pack.m_kwords).m_value...
        );
    }
};

/*************************************************************************************************/

template<std::size_t I, typename ...P>
using pack_option_t = std::tuple_element_t<I, std::tuple<P...>>;

template<std::size_t I, typename ...P>
inline bool try_rebind_validator_1(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    using Ev = validator_with_deps<pack_option_t<I, P...>>;

    if ( ti != typeid(Ev) ) {
        return false;
    }

    auto &v = *static_cast<Ev *>(stor.storage_obj_void());
    slot = std::function<bool(std::string_view)>{
        [&v, &pack](std::string_view s) noexcept -> bool {
            return v.call(s, pack);
        }
    };

    return true;
}

template<typename ...P, std::size_t ...I>
inline bool rebind_validator_dispatch_1(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<I...>
) {
    return (... || try_rebind_validator_1<I, P...>(ti, stor, slot, pack));
}

template<std::size_t I, std::size_t J, typename ...P>
inline bool try_rebind_validator_2(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if constexpr ( I == J ) {
        (void)ti;
        (void)stor;
        (void)slot;
        (void)pack;

        return false;
    } else {
        using Ev = validator_with_deps<
             pack_option_t<I, P...>
            ,pack_option_t<J, P...>
        >;

        if ( ti != typeid(Ev) ) {
            return false;
        }

        auto &v = *static_cast<Ev *>(stor.storage_obj_void());
        slot = std::function<bool(std::string_view)>{
            [&v, &pack](std::string_view s) noexcept -> bool {
                return v.call(s, pack);
            }
        };

        return true;
    }
}

template<std::size_t K, typename ...P>
inline bool try_rebind_validator_2_flat(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    constexpr std::size_t n = sizeof...(P);
    constexpr std::size_t i = K / n;
    constexpr std::size_t j = K % n;

    return try_rebind_validator_2<i, j, P...>(ti, stor, slot, pack);
}

template<typename ...P, std::size_t ...K>
inline bool rebind_validator_dispatch_2(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<K...>
) {
    return (... || try_rebind_validator_2_flat<K, P...>(ti, stor, slot, pack));
}

template<std::size_t I, std::size_t J, std::size_t L, typename ...P>
inline bool try_rebind_validator_3(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if constexpr ( I == J || I == L || J == L ) {
        (void)ti;
        (void)stor;
        (void)slot;
        (void)pack;

        return false;
    } else {
        using Ev = validator_with_deps<
             pack_option_t<I, P...>
            ,pack_option_t<J, P...>
            ,pack_option_t<L, P...>
        >;

        if ( ti != typeid(Ev) ) {
            return false;
        }

        auto &v = *static_cast<Ev *>(stor.storage_obj_void());
        slot = std::function<bool(std::string_view)>{
            [&v, &pack](std::string_view s) noexcept -> bool {
                return v.call(s, pack);
            }
        };

        return true;
    }
}

template<std::size_t Flat, typename ...P>
inline bool try_rebind_validator_3_flat(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    constexpr std::size_t n = sizeof...(P);
    constexpr std::size_t i = Flat / (n * n);
    constexpr std::size_t j = (Flat / n) % n;
    constexpr std::size_t k = Flat % n;

    return try_rebind_validator_3<i, j, k, P...>(ti, stor, slot, pack);
}

template<typename ...P, std::size_t ...K>
inline bool rebind_validator_dispatch_3(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<K...>
) {
    return (... || try_rebind_validator_3_flat<K, P...>(ti, stor, slot, pack));
}

template<typename ...P>
inline bool rebind_validator_storage_into(
     deps_storage_base *stor
    ,std::function<bool(std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if ( !stor ) {
        return false;
    }

    const std::type_info &ti = stor->storage_type();
    deps_storage_base &s = *stor;
    constexpr std::size_t n = sizeof...(P);

    if ( rebind_validator_dispatch_1<P...>(ti, s, slot, pack, std::make_index_sequence<n>{}) ) {
        return true;
    }

#if CMDARGS_MAX_OPTION_DEPS >= 2
    if constexpr ( n >= 2u ) {
        if ( rebind_validator_dispatch_2<P...>(
                ti, s, slot, pack, std::make_index_sequence<n * n>{}
            ) )
        {
            return true;
        }
    }
#endif

#if CMDARGS_MAX_OPTION_DEPS >= 3
    if constexpr ( n >= 3u ) {
        if ( rebind_validator_dispatch_3<P...>(
                ti, s, slot, pack, std::make_index_sequence<n * n * n>{}
            ) )
        {
            return true;
        }
    }
#endif

    return false;
}

template<typename V, std::size_t I, typename ...P>
inline bool try_rebind_converter_1(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    using Ec = converter_with_deps<V, pack_option_t<I, P...>>;

    if ( ti != typeid(Ec) ) {
        return false;
    }

    auto &c = *static_cast<Ec *>(stor.storage_obj_void());
    slot = std::function<bool(V &, std::string_view)>{
        [&c, &pack](V &dst, std::string_view s) -> bool {
            return c.call(dst, s, pack);
        }
    };

    return true;
}

template<typename V, typename ...P, std::size_t ...I>
inline bool rebind_converter_dispatch_1(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<I...>
) {
    return (... || try_rebind_converter_1<V, I, P...>(ti, stor, slot, pack));
}

template<typename V, std::size_t I, std::size_t J, typename ...P>
inline bool try_rebind_converter_2(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if constexpr ( I == J ) {
        (void)ti;
        (void)stor;
        (void)slot;
        (void)pack;

        return false;
    } else {
        using Ec = converter_with_deps<
             V
            ,pack_option_t<I, P...>
            ,pack_option_t<J, P...>
        >;

        if ( ti != typeid(Ec) ) {
            return false;
        }

        auto &c = *static_cast<Ec *>(stor.storage_obj_void());
        slot = std::function<bool(V &, std::string_view)>{
            [&c, &pack](V &dst, std::string_view s) -> bool {
                return c.call(dst, s, pack);
            }
        };

        return true;
    }
}

template<typename V, std::size_t K, typename ...P>
inline bool try_rebind_converter_2_flat(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    constexpr std::size_t n = sizeof...(P);
    constexpr std::size_t i = K / n;
    constexpr std::size_t j = K % n;

    return try_rebind_converter_2<V, i, j, P...>(ti, stor, slot, pack);
}

template<typename V, typename ...P, std::size_t ...K>
inline bool rebind_converter_dispatch_2(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<K...>
) {
    return (... || try_rebind_converter_2_flat<V, K, P...>(ti, stor, slot, pack));
}

template<typename V, std::size_t I, std::size_t J, std::size_t L, typename ...P>
inline bool try_rebind_converter_3(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if constexpr ( I == J || I == L || J == L ) {
        (void)ti;
        (void)stor;
        (void)slot;
        (void)pack;

        return false;
    } else {
        using Ec = converter_with_deps<
             V
            ,pack_option_t<I, P...>
            ,pack_option_t<J, P...>
            ,pack_option_t<L, P...>
        >;

        if ( ti != typeid(Ec) ) {
            return false;
        }

        auto &c = *static_cast<Ec *>(stor.storage_obj_void());
        slot = std::function<bool(V &, std::string_view)>{
            [&c, &pack](V &dst, std::string_view s) -> bool {
                return c.call(dst, s, pack);
            }
        };

        return true;
    }
}

template<typename V, std::size_t Flat, typename ...P>
inline bool try_rebind_converter_3_flat(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    constexpr std::size_t n = sizeof...(P);
    constexpr std::size_t i = Flat / (n * n);
    constexpr std::size_t j = (Flat / n) % n;
    constexpr std::size_t k = Flat % n;

    return try_rebind_converter_3<V, i, j, k, P...>(ti, stor, slot, pack);
}

template<typename V, typename ...P, std::size_t ...K>
inline bool rebind_converter_dispatch_3(
     const std::type_info &ti
    ,deps_storage_base &stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
    ,std::index_sequence<K...>
) {
    return (... || try_rebind_converter_3_flat<V, K, P...>(ti, stor, slot, pack));
}

template<typename V, typename ...P>
inline bool rebind_converter_storage_into(
     deps_storage_base *stor
    ,std::function<bool(V &, std::string_view)> &slot
    ,args_pack<P...> &pack
) {
    if ( !stor ) {
        return false;
    }

    const std::type_info &ti = stor->storage_type();
    deps_storage_base &s = *stor;
    constexpr std::size_t n = sizeof...(P);

    if ( rebind_converter_dispatch_1<V, P...>(ti, s, slot, pack, std::make_index_sequence<n>{}) ) {
        return true;
    }

#if CMDARGS_MAX_OPTION_DEPS >= 2
    if constexpr ( n >= 2u ) {
        if ( rebind_converter_dispatch_2<V, P...>(
                ti, s, slot, pack, std::make_index_sequence<n * n>{}
            ) )
        {
            return true;
        }
    }
#endif

#if CMDARGS_MAX_OPTION_DEPS >= 3
    if constexpr ( n >= 3u ) {
        if ( rebind_converter_dispatch_3<V, P...>(
                ti, s, slot, pack, std::make_index_sequence<n * n * n>{}
            ) )
        {
            return true;
        }
    }
#endif

    return false;
}

} // namespace details

/*************************************************************************************************/

template<typename Iter, typename ...Args>
void parse_kv_list(
     std::string *emsg
    ,const char *pref
    ,std::size_t pref_len
    ,Iter beg
    ,Iter end
    ,args_pack<Args...> &args)
{
    for ( ; beg != end; ++beg ) {
        if ( pref ) {
            std::string_view pref_sv{pref, pref_len};
            if ( pref_sv.compare(0, pref_len, *beg, pref_len) != 0 ) {
                continue;
            }
        }
        std::string_view line = pref ? (*beg) + pref_len : (*beg);
        line = cmdargs::details::trim(line);

        auto pos = line.find('=');
        std::string_view key = (pos != std::string_view::npos)
            ? line.substr(0u, pos)
            : line
        ;

        auto unexpected = args.check_for_unexpected(key);
        if ( !unexpected.empty() ) {
            std::string msg = "there is an extra \"";
            msg += (pref ? pref : "");
            msg += unexpected;
            msg += "\" option was specified";

            if ( emsg ) {
                *emsg = std::move(msg);
            } else {
                throw invalid_argument(msg);
            }

            return;
        }

        if ( pos != std::string_view::npos ) {
            std::string msg;
            std::string_view val = line.substr(pos + 1);
            args.for_each(
                [pref, key, val, &msg](auto &item) {
                    if ( item.name() == key ) {
                        if ( !item.validate(val) ) {
                            msg = "an invalid value \"";
                            msg += val;
                            msg += "\" was received for \"";
                            msg += (pref ? pref : "");
                            msg += key;
                            msg += "\" option";

                            return false;
                        }
                        if ( !item.convert(val) ) {
                            msg = "can't convert value \"";
                            msg += val;
                            msg += "\" for \"";
                            msg += (pref ? pref : "");
                            msg += key;
                            msg += "\" option";

                            return false;
                        }
                        return false;
                    }
                    return true;
                }
                ,false
            );

            if ( !msg.empty() ) {
                if ( emsg ) {
                    *emsg = std::move(msg);
                } else {
                    throw invalid_argument(msg);
                }

                return;
            }
        } else {
            if ( key != details::version_option_type::name() ) {
                if ( !args.is_bool_type(key) ) {
                    std::string msg = "a value must be provided for \"";
                    msg += (pref ? pref : "");
                    msg += key;
                    msg += "\" option";

                    if ( emsg ) {
                        *emsg = std::move(msg);
                    } else {
                        throw invalid_argument(msg);
                    }

                    return;
                }

                args.for_each(
                    [key](auto &item) {
                        if ( item.name() == key ) {
                            static const std::string_view _true{"true"};
                            item.convert(_true);
                            return false;
                        }
                        return true;
                    }
                    ,false
                );
            } else {
                // version case
                args.for_each(
                    [](auto &item) {
                        if ( item.name() == details::version_option_type::name() ) {
                            item.set_value(item.get_default_value());
                            return false;
                        }
                        return true;
                    }
                    ,false
                );
            }

            if ( key == details::help_option_type::name()
                || key == details::version_option_type::name() )
            {
                return;
            }
        }
    }

    auto required = args.check_for_required();
    if ( !required.empty() ) {
        std::string msg = "no required \"";
        msg += (pref ? pref : "");
        msg += required;
        msg += "\" option was specified";

        if ( emsg ) {
            *emsg = std::move(msg);
        } else {
            throw invalid_argument(msg);
        }

        return;
    }

    auto cond_and = args.check_for_cond_and();
    if ( !cond_and.second.empty() ) {
        std::string names = details::cat_vector(pref, cond_and.second);
        std::string msg = "the \"";
        msg += (pref ? pref : "");
        msg += cond_and.first;
        msg += "\" option must be used together with \"";
        msg += names;
        msg += "\"";

        if ( emsg ) {
            *emsg = std::move(msg);
        } else {
            throw invalid_argument(msg);
        }

        return;
    }

    auto cond_or = args.check_for_cond_or();
    if ( !cond_or.second.empty() ) {
        std::string names = details::cat_vector(pref, cond_or.second, true);
        std::string msg = "the \"";
        msg += (pref ? pref : "");
        msg += cond_or.first;
        msg += "\" option must be used together with ";
        if ( cond_or.second.size() > 1 ) {
            msg += "one of ";
        }
        msg += names;

        if ( emsg ) {
            *emsg = std::move(msg);
        } else {
            throw invalid_argument(msg);
        }

        return;
    }

    auto cond_not = args.check_for_cond_not();
    if ( !cond_not.second.empty() ) {
        std::string names = details::cat_vector(pref, cond_not.second, true);
        std::string msg = "the \"";
        msg += (pref ? pref : "");
        msg += cond_not.first;
        msg += "\" option can't be used together with ";
        if ( cond_not.second.size() > 1 ) {
            msg += "one of ";
        }
        msg += names;

        if ( emsg ) {
            *emsg = std::move(msg);
        } else {
            throw invalid_argument(msg);
        }
    }
}

/*************************************************************************************************/

template<
     typename ...Args
    ,typename = typename std::enable_if_t<
        sizeof...(Args) != 1
            && !std::is_base_of_v<
                 kwords_group
                ,typename std::tuple_element<0, std::tuple<Args...>>::type
        >
    >
>
auto parse_args(std::string *emsg, int argc, char* const* argv, const Args & ...kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args_pack<typename std::decay_t<Args>...> set{kwords...};
    parse_kv_list(emsg, "--", 2, beg, end, set);

    return set;
}

template<typename ...Args>
auto parse_args(std::string *emsg, int argc, char* const* argv, const std::tuple<Args...> &kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args_pack<typename std::decay_t<Args>...> set{std::get<Args>(kwords)...};
    parse_kv_list(emsg, "--", 2, beg, end, set);

    return set;
}

template<
     typename KWords
    ,typename = typename std::enable_if_t<
        std::is_class_v<KWords> &&
        std::is_base_of_v<kwords_group, KWords>
    >
>
auto parse_args(std::string *emsg, int argc, char* const* argv, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    auto res = parse_args(emsg, argc, argv, tuple);

    return res;
}

/*************************************************************************************************/

template<typename ...Args>
auto make_args(Args && ...args) {
    cmdargs::args_pack<typename std::decay_t<Args>...> set{std::forward<Args>(args)...};

    return set;
}

/*************************************************************************************************/

template<typename OS, typename ...Args>
OS& to_file(OS &os, const args_pack<Args...> &args, bool inited_only = true) {
    args.for_each(
        [&os](const auto &item) {
            os << "# " << item.description() << details::endl;
            os << item.name() << "=";
            bool is_set = item.is_set();
            bool has_default = item.has_default();
            if ( is_set || has_default ) {
                const auto &value = (!is_set) ? item.get_default_value() : item.get_value();
                using decayed = typename std::decay_t<decltype(item)>;
                if constexpr ( std::is_same_v<typename decayed::value_type, bool> ) {
                    os << value ? "true" : "false";
                } else {
                    os << value;
                }
                if ( !is_set ) {
                    os << " (D)";
                }
            }
            os << details::endl;

            return true;
        }
        ,inited_only
    );

    return os;
}

template<typename IS, typename ...Args>
auto& from_file(std::string *emsg, IS &is, args_pack<Args...> &args) {
    std::vector<std::string> lines;
    for ( std::string line; std::getline(is, line); ) {
        details::trim(line);

        if ( !line.empty() && line.front() == '#' ) {
            line.clear();

            continue;
        }

        lines.push_back(std::move(line));
    }

    std::vector<const char*> linesptrs;
    linesptrs.reserve(lines.size());
    for ( const auto &it: lines ) {
        linesptrs.push_back(it.c_str());
    }

    parse_kv_list(emsg, nullptr, 0, linesptrs.begin(), linesptrs.end(), args);

    return args;
}

template<
     typename IS
    ,typename ...Args
    ,typename = typename std::enable_if_t<
        sizeof...(Args) != 1 && !std::is_base_of_v<
             kwords_group
            ,typename std::tuple_element<0, std::tuple<Args...>>::type
        >
    >
>
auto from_file(std::string *emsg, IS &is, const Args & ...kwords) {
    args_pack<typename std::decay_t<Args>...> args{kwords...};
    from_file(emsg, is, args);

    return args;
}

template<typename IS, typename ...Args>
auto from_file(std::string *emsg, IS &is, const std::tuple<Args...> &kwords) {
    args_pack<typename std::decay_t<Args>...> args{std::get<Args>(kwords)...};
    from_file(emsg, is, args);

    return args;
}

template<
     typename IS
    ,typename KWords
    ,typename = typename std::enable_if_t<
        std::is_class_v<KWords> &&
        std::is_base_of_v<kwords_group, KWords>
    >
>
auto from_file(std::string *emsg, IS &is, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    return from_file(emsg, is, tuple);
}

/*************************************************************************************************/

template<typename OS, typename ...Args>
OS& show_help(OS &os, const char *argv0, const args_pack<Args...> &args) {
    const auto pos = std::string_view{argv0}.rfind(details::path_separator);
    const char *p  = (pos != std::string_view::npos ? argv0+pos+1 : argv0);
    os << p << ":" << details::endl;

    std::size_t max_len = 0;
    args.for_each(
        [&max_len](const auto &item) {
            std::size_t len = item.name().size();
            max_len = (len > max_len) ? len : max_len;

            return true;
        }
        ,false
    );

    args.for_each(
        [&os, max_len](const auto &item) {
            static const char ident[] = "                                        ";
            std::string_view name = item.name();
            std::size_t len = name.size();
            os << "--" << name << "=*";
            os.write(ident, static_cast<std::streamsize>(max_len - len));
            os
            << ": \"" << item.description()
            << "\" ("
                << item.type_name()
                << ", "
                << (item.is_required() ? "required" : "optional")
            ;
            if ( item.has_default() ) {
                os << ", default=\"" << item.get_default_value() << "\"";
            }
            const auto &and_list = item.and_list();
            if ( !and_list.empty() ) {
                os << ", and(" << details::cat_vector("--", and_list) << ")";
            }
            const auto &or_list = item.or_list();
            if ( !or_list.empty() ) {
                os << ", or(" << details::cat_vector("--", or_list) << ")";
            }
            const auto &not_list = item.not_list();
            if ( !not_list.empty() ) {
                os << ", not(" << details::cat_vector("--", not_list) << ")";
            }
            os << ")" << details::endl;

            return true;
        }
        ,false
    );

    return os;
}

template<typename OS, typename ...Args>
OS & show_help(OS &os, const char *argv0, const std::tuple<Args...> &kwords) {
    args_pack<typename std::decay_t<Args>...> args{std::get<Args>(kwords)...};

    return show_help(os, argv0, args);
}

template<
     typename OS
    ,typename KWords
    ,typename = typename std::enable_if_t<
        std::is_class_v<KWords> &&
        std::is_base_of_v<kwords_group, KWords>
    >
>
OS& show_help(OS &os, const char *argv0, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    return show_help(os, argv0, tuple);
}

/*************************************************************************************************/
// for debug purposes

template<
     typename OS
    ,typename KWords
    ,typename = typename std::enable_if_t<
        std::is_class_v<KWords> &&
        std::is_base_of_v<kwords_group, KWords>
    >
>
OS& dump_group(OS &os, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);
    std::apply(
        [&os](auto&&... args) {
            ((args.dump(os) << "*******************************************" << details::endl), ...);
        }
        ,tuple
    );

    return os;
}

template<typename OS, typename ...Types>
OS& dump_group(OS &os, const args_pack<Types...> &args) {
    args.for_each(
        [&os](const auto &it){
            it.dump(os) << "*******************************************" << details::endl;
            return true;
        }
    );

    return os;
}

/*************************************************************************************************/

template<typename OS, typename ...Args>
bool is_help_requested(OS &os, const char *argv0, const args_pack<Args...> &args) {
    if constexpr ( args_pack<Args...>::template contains<details::help_option_type>() ) {
        if ( args.template is_set<details::help_option_type>() ) {
            show_help(os, argv0, args);

            return true;
        }
    }

    return false;
}

template<typename OS, typename ...Args>
bool is_version_requested(OS &os, const char *argv0, const args_pack<Args...> &args) {
    if constexpr ( args_pack<Args...>::template contains<details::version_option_type>() ) {
        if ( args.template is_set<details::version_option_type>() ) {
            const auto pos = std::string_view{argv0}.rfind(details::path_separator);
            const auto ptr  = (pos != std::string_view::npos ? argv0+pos+1 : argv0);
            os << ptr << ": version - " << args.template get<details::version_option_type>() << details::endl;

            return true;
        }
    }

    return false;
}

template<typename OS, typename ...Args>
bool is_help_or_version_requested(OS &os, const char *argv0, const args_pack<Args...> &args) {
    return is_help_requested(os, argv0, args) || is_version_requested(os, argv0, args);
}

/*************************************************************************************************/

#define CMDARGS_OPTION(OPTION_NAME, OPTION_TYPE, OPTION_DESCRIPTION, ...) \
    const ::cmdargs::option<\
        struct __CMDARGS_CAT(OPTION_NAME, __CMDARGS__OPTION_SUFFIX)\
        , OPTION_TYPE\
    > \
        OPTION_NAME{this, OPTION_DESCRIPTION, std::make_tuple(__VA_ARGS__)}

#define CMDARGS_OPTION_HELP() \
    const ::cmdargs::details::help_option_type help{this, "show help message" \
        ,std::make_tuple(optional)}

#define CMDARGS_OPTION_VERSION(str) \
    const ::cmdargs::details::version_option_type version{this, "show version message" \
        ,std::make_tuple(optional, ::cmdargs::details::default_t<std::string>{str})}

/*************************************************************************************************/

} // ns cmdargs
