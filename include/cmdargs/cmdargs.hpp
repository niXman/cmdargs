
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2024 niXman (github dot nixman at pm dot me)
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

#ifndef __CMDARGS__CMDARGS_HPP
#define __CMDARGS__CMDARGS_HPP

#define __CMDARGS__STRINGIZE_I(x) #x
#define __CMDARGS__STRINGIZE(x) __CMDARGS__STRINGIZE_I(x)

/*************************************************************************************************/

// CMDARGS_VERSION_HEX >> 24 - is the major version
// CMDARGS_VERSION_HEX >> 16 - is the minor version
// CMDARGS_VERSION_HEX >> 8  - is the bugfix level

#define CMDARGS_VERSION_MAJOR 0
#define CMDARGS_VERSION_MINOR 1
#define CMDARGS_VERSION_BUGFIX 1

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

//#include <iostream> // TODO: comment out

#include <ostream>
#include <istream>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <tuple>
#include <array>
#include <string>
#include <string_view>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <optional>
#include <utility>

#include <cinttypes>

/*************************************************************************************************/

namespace cmdargs {
namespace details {

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
    constexpr auto double_colon_pos = name.find(double_colon);
    if constexpr ( double_colon_pos != std::string_view::npos ) {
        constexpr auto name2 = name.substr(double_colon_pos + double_colon.size()
            ,end - (double_colon_pos + double_colon.size()));
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
    constexpr auto& value = type_name_holder<T>::value;
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

template<class T, std::size_t... I>
decltype(void(T{(I, std::declval<any_type>())...}), std::true_type{})
    test_is_braces_constructible_n(std::index_sequence<I...>);

#if defined(__GNUC__) && !defined(__clang__)
#   pragma GCC diagnostic pop
#elif defined(__clang__)
#   pragma clang diagnostic pop
#endif

template <class, class...>
std::false_type test_is_braces_constructible_n(...);

template <class T, std::size_t N>
using is_braces_constructible_n =
    decltype(test_is_braces_constructible_n<T>(std::make_index_sequence<N>{}));

template<class T, std::size_t L = 0u, std::size_t R = sizeof(T) + 1u>
constexpr std::size_t to_tuple_size_impl() {
    constexpr std::size_t M = (L + R) / 2u;
    return (M == 0)
        ? std::is_empty<T>{}
            ? 0u
            : throw "Unable to determine number of elements"
        : (L == M)
            ? M
            : is_braces_constructible_n<T, M>{}
                ? to_tuple_size_impl<T, M, R>()
                : to_tuple_size_impl<T, L, M>()
    ;
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
    return std::make_tuple(p0);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 2>) noexcept {
    const auto& [p0, p1] = object;
    return std::make_tuple(p0, p1);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 3>) noexcept {
    const auto& [p0, p1, p2] = object;
    return std::make_tuple(p0, p1, p2);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 4>) noexcept {
    const auto& [p0, p1, p2, p3] = object;
    return std::make_tuple(p0, p1, p2, p3);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 5>) noexcept {
    const auto& [p0, p1, p2, p3, p4] = object;
    return std::make_tuple(p0, p1, p2, p3, p4);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 6>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 7>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 8>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 9>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 10>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 11>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 12>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 13>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 14>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 15>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 16>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 17>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 18>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 19>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
}
template<typename T>
auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, 20>) noexcept {
    const auto& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = object;
    return std::make_tuple(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
}

template<
     typename T
    ,typename = struct current_value
    ,std::size_t = 21
    ,typename = struct required_value
    ,std::size_t N
>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, N>) noexcept {
    static_assert(N < 21, "Please increase the number of placeholders");
}

template<
     typename T
    ,typename = std::enable_if_t<std::is_class<T>::value>
    ,typename S = to_tuple_size<std::decay_t<T>>
>
auto to_tuple(const T &kw) noexcept {
    return to_tuple_impl(kw, S{});
}

/*************************************************************************************************/
// string ops

inline void ltrim(std::string &s, const char* t = " \t\n\r") noexcept
{ s.erase(0, s.find_first_not_of(t)); }

inline void rtrim(std::string &s, const char* t = " \t\n\r") noexcept
{ s.erase(s.find_last_not_of(t) + 1); }

inline void trim(std::string &s, const char* t = " \t\n\r") noexcept
{ rtrim(s, t); ltrim(s, t); }

template<
     template<typename, typename> typename Sequence
    ,typename T
    ,typename A
>
inline bool split(Sequence<T, A> &result, std::string_view str, char delim) {
    std::size_t start = 0;

    for ( auto found = str.find(delim); found != std::string_view::npos; found = str.find(delim, start) ) {
        result.emplace_back(str.begin() + start, str.begin() + found);
        start = found + sizeof(delim);
    }

    if ( start != str.size() )
        result.emplace_back(str.begin() + start, str.end());

    return !result.empty();
}

constexpr std::size_t ct_strlen(const char *s) noexcept {
    const char *p = s;
    for ( ; *p; ++p )
        ;

    return p - s;
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

template<std::size_t N>
constexpr auto ct_init_array(const char *s, char c0, char c1) noexcept {
    std::array<char, N> res{};
    for ( auto i = 0u; *s; ++s, ++i ) {
        res[i] = *s;
    }
    res[1] = c0;
    res[2] = c1;

    return res;
}

template<typename T>
typename std::enable_if_t<std::is_same<T, std::string>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) noexcept {
    val->assign(ptr, len);
}

template<typename T>
typename std::enable_if_t<std::is_same<T, bool>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) noexcept {
    *val = std::string_view{ptr, len} == "true";
}

template<typename T>
typename std::enable_if_t<
    std::is_integral<T>::value
        && !std::is_same<T, bool>::value
>
from_string_impl(T *val, const char *ptr, std::size_t len) noexcept {
    constexpr const char *fmt = (
        std::is_unsigned<T>::value
        ? (std::is_same<T, std::uint8_t>::value
            ? "%  " SCNu8 : std::is_same<T, std::uint16_t>::value
                ? "%  " SCNu16 : std::is_same<T, std::uint32_t>::value
                    ? "%  " SCNu32
                    : "%  " SCNu64
        )
        : (std::is_same<T, std::int8_t>::value
            ? "%  " SCNi8 : std::is_same<T, std::int16_t>::value
                ? "%  " SCNi16 : std::is_same<T, std::int32_t>::value
                    ? "%  " SCNi32
                    : "%  " SCNi64
        )
    );

    enum { S = ct_strlen(fmt)+1 };
    const auto fmtbuf = ct_init_array<S>(fmt, '0' + (len / 10), '0' + (len % 10));

    std::sscanf(ptr, fmtbuf.data(), val);
}

template<typename T>
typename std::enable_if_t<std::is_enum<T>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) noexcept {
    typename std::underlying_type<T>::type tmp{};
    from_string_impl(&tmp, ptr, len);
    *val = static_cast<T>(tmp);
}

template<typename T>
typename std::enable_if_t<std::is_floating_point<T>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) noexcept {
    constexpr const char *fmt = (
        std::is_same<T, float>::value
            ? "%  f"
            : "%  lf"
    );

    enum { S = ct_strlen(fmt)+1 };
    const auto fmtbuf = ct_init_array<S>(fmt, '0' + (len / 10), '0' + (len % 10));

    std::sscanf(ptr, fmtbuf.data(), val);
}

template<typename T>
typename std::enable_if_t<std::is_pointer<T>::value>
from_string_impl(T *val, const char *, std::size_t) noexcept {
    *val = nullptr;
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
    using type = typename std::conditional<
         contains<Pred, InCar, Out...>::value
        ,filter<Pred, std::tuple<Out...>, std::tuple<InCdr...>>
        ,filter<Pred, std::tuple<Out..., InCar>, std::tuple<InCdr...>>
    >::type::type;
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

template<typename...>
using void_t = void;

template<typename F>
struct is_callable<
     F
    ,void_t<
        has_operator_call_t<
            typename std::decay<F>::type
        >
    >
>: std::true_type
{};

/*************************************************************************************************/
// callable traits

template<typename F>
struct callable_traits: callable_traits<decltype(&F::operator())>
{};

template<typename R, typename Arg0>
struct callable_traits<R(*)(Arg0)> {
    using signature = R(Arg0);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 1;
};

template<typename R, typename Arg0, typename Arg1>
struct callable_traits<R(*)(Arg0, Arg1)> {
    using signature = R(Arg0, Arg1);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 2;
};

template<typename R, typename Arg0>
struct callable_traits<R(&)(Arg0)> {
    using signature = R(Arg0);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 1;
};

template<typename R, typename Arg0, typename Arg1>
struct callable_traits<R(&)(Arg0, Arg1)> {
    using signature = R(Arg0, Arg1);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 2;
};

template<typename Obj, typename R, typename Arg0>
struct callable_traits<R(Obj::*)(Arg0) const> {
    using signature = R(Arg0);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 1;
};

template<typename Obj, typename R, typename Arg0, typename Arg1>
struct callable_traits<R(Obj::*)(Arg0, Arg1) const> {
    using signature = R(Arg0, Arg1);
    using function  = std::function<signature>;
    static constexpr std::size_t size = 2;
};

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
    using type = typename std::conditional<
         Pred<char, T>::value
        ,type_identity<T>
        ,get_relation_list<Pred, Rest...>
    >::type::type;
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

} // ns details

/*************************************************************************************************/

template<typename ...Args>
struct args_pack;

template<typename ID, typename V>
struct option final {
    template<typename ...Args>
    friend struct args_pack;

    using value_type = V;
private:
    using optional_type  = std::optional<value_type>;
    using validator_type = std::function<bool(std::string_view str)>;
    using converter_type = std::function<bool(value_type &dst, std::string_view str)>;

    const std::string_view m_type_name;
    const std::string_view m_description;
    const bool m_is_required;
    const bool m_uses_custom_validator;
    const validator_type m_validator;
    const bool m_uses_custom_converter;
    const converter_type m_converter;
    const std::vector<std::string_view> m_relation_and;
    const std::vector<std::string_view> m_relation_or;
    const std::vector<std::string_view> m_relation_not;
    const optional_type m_default_value;
    optional_type m_value;

public:
    option& operator= (const option &) = delete;
    option& operator= (option &&) = delete;
    option(const option &) = default;
    option(option &&) = default;

    template<typename ...Args>
    option(const char *type, const char *descr, std::tuple<Args...> as_tuple)
        :m_type_name{type}
        ,m_description{descr}
        ,m_is_required{!details::contains<std::is_same, details::optional_option_t, Args...>::value}
        ,m_uses_custom_validator{has_visitor<validator_type>(as_tuple)}
        ,m_validator{init_visitor<validator_type>(as_tuple)}
        ,m_uses_custom_converter{has_visitor<converter_type>(as_tuple)}
        ,m_converter{init_visitor<converter_type>(as_tuple)}
        ,m_relation_and{init_cond_list<details::e_relation_type::AND>(as_tuple)}
        ,m_relation_or{init_cond_list<details::e_relation_type::OR>(as_tuple)}
        ,m_relation_not{init_cond_list<details::e_relation_type::NOT>(as_tuple)}
        ,m_default_value{get_default_value(as_tuple)}
        ,m_value{}
    {}
    ~option() noexcept = default;

    template<typename U>
    option operator= (U &&r) const noexcept {
        option res{*this};
        res.m_value = std::forward<U>(r);

        return res;
    }

    static constexpr std::string_view name() noexcept {
        constexpr auto n = details::type_name<ID>();
        return n ;
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
    bool is_bool() const noexcept { return std::is_same<value_type, bool>::value; }

    const auto& and_list() const noexcept { return m_relation_and; }
    const auto& or_list () const noexcept { return m_relation_or;  }
    const auto& not_list() const noexcept { return m_relation_not; }

    bool uses_custom_validator() const noexcept { return m_uses_custom_validator; }
    bool validate(const char *str, std::size_t len) const noexcept { return m_validator({str, len}); }
    bool uses_custom_converter() const noexcept { return m_uses_custom_converter; }
    bool convert(const char *str, std::size_t len) {
        value_type v{};
        if ( m_converter(v, {str, len}) ) {
            m_value = std::move(v);
            return true;
        }
        return false;
    }

    std::ostream& dump(std::ostream &os) const {
        os
            << "name            : " << name() << std::endl
            << "type            : " << m_type_name << std::endl
            << "description     : " << '"' << m_description << '"' << std::endl
            << "is required     : " << (m_is_required ? "true" : "false") << std::endl
            << "value           : "
        ;
        if ( m_value.has_value() ) {
            os << m_value.value();
        } else {
            os << "<UNINITIALIZED>";
        }
        os  << std::endl
            << "custom validator: " << (uses_custom_validator() ? "true" : "false") << std::endl
            << "custom converter: " << (uses_custom_converter() ? "true" : "false") << std::endl
            << "relation     AND: " << m_relation_and.size();
        if ( m_relation_and.size() )
        { os << " (" << details::cat_vector("--", m_relation_and) << ")" << std::endl; }
        else { os << std::endl; }
        os  << "relation      OR: " << m_relation_or.size();
        if ( m_relation_or.size() )
        { os << " (" << details::cat_vector("--", m_relation_or) << ")" << std::endl; }
        else { os << std::endl; }
        os  << "relation     NOT: " << m_relation_not.size();
        if ( m_relation_not.size() )
        { os << " (" << details::cat_vector("--", m_relation_not) << ")" << std::endl; }
        else { os << std::endl; }

        return os;
    }

private:
    static bool default_converter(value_type &dst, std::string_view str) {
        details::from_string_impl(&dst, str.data(), str.length());
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

using help_option_type = option<struct help, bool>;
using version_option_type = option<struct version, std::string>;

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
    std::vector<std::string> vec;
    split(vec, str, sep);
    for ( const auto &it: vec) {
        T v{};
        from_string_impl(&v, it.data(), it.size());

        dst.emplace_back(std::move(v));
    }

    return true;
}

// map
template<
     template<typename, typename, typename> class Assoc
    ,typename K
    ,typename V
    ,typename A
>
bool convert_as_assoc(Assoc<K, V, A> &dst, std::string_view str, char pair_sep, char kv_sep) {
    std::vector<std::string> vec;
    convert_as_sequence(vec, str, pair_sep);
    for ( const auto &it: vec ) {
        std::vector<std::string> pair;
        convert_as_sequence(pair, it, kv_sep);
        if ( pair.size() != 2 )
            return false;

        K k{};
        from_string_impl(&k, pair[0].data(), pair[0].size());

        V v{};
        from_string_impl(&v, pair[1].data(), pair[1].size());

        dst.emplace(std::move(k), std::move(v));
    }

    return true;
}

// set
template<
     template<typename, typename> class Assoc
    ,typename K
    ,typename A
>
bool convert_as_assoc(Assoc<K, A> &dst, std::string_view str, char sep) {
    std::vector<std::string> vec;
    convert_as_sequence(vec, str, sep);
    for ( const auto &it: vec ) {
        K k{};
        from_string_impl(&k, it.data(), it.size());

        dst.emplace(std::move(k));
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

template<typename T>
bool convert_as_set(std::set<T> &dst, std::string_view str, char sep) {
    return convert_as_assoc(dst, str, sep);
}

template<typename K, typename V>
bool convert_as_map(std::map<K, V> &dst, std::string_view str, char pair_sep, char kv_sep) {
    return convert_as_assoc(dst, str, pair_sep, kv_sep);
}

} // ns details

/*************************************************************************************************/

struct kwords_group {
    static constexpr details::optional_option_t optional{};

    template<typename T>
    static auto default_(T &&v) noexcept {
        return details::default_t<T>{std::forward<T>(v)};
    }

    template<typename ...Types>
    static auto and_(const Types &...args) noexcept {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
            std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<std::is_same, tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return details::relations_list<details::e_relation_type::AND, Types...>{args.name()...};
    }
    template<typename ...Types>
    static auto or_(const Types &...args) noexcept {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
            std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<std::is_same, tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return details::relations_list<details::e_relation_type::OR, Types...>{args.name()...};
    }
    template<typename ...Types>
    static auto not_(const Types &...args) noexcept {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
            std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<std::is_same, tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return details::relations_list<details::e_relation_type::NOT, Types...>{args.name()...};
    }

    template<typename F>
    static auto validator_(F &&f) noexcept {
        static_assert(details::is_callable<F>::value);
        using signature = typename details::callable_traits<F>::signature;
        static_assert(std::is_same<signature, bool(const std::string_view str)>::value);
        return std::function<signature>{std::forward<F>(f)};
    }
    template<typename F>
    static auto converter_(F &&f) noexcept {
        static_assert(details::is_callable<F>::value);
        static_assert(details::callable_traits<F>::size == 2);
        using signature = typename details::callable_traits<F>::signature;
        return std::function<signature>{std::forward<F>(f)};
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
};

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

    container_type m_kwords;

public:
    template<typename ...Types>
    explicit args_pack(const Types &...types)
        :m_kwords{types...}
    {}

    auto values() const {
        auto res = std::make_tuple(std::get<Args>(m_kwords).m_value...);
        return res;
    }

    constexpr std::size_t size() const { return sizeof...(Args); }

    template<typename T>
    static constexpr bool contains(const T &)
    { return details::contains<std::is_same, T, Args...>::value; }
    template<typename T>
    static constexpr bool contains()
    { return details::contains<std::is_same, T, Args...>::value; }

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

    bool is_valid_name(const char *name) const {
        return check_for_unexpected(name) == nullptr;
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

    std::ostream& dump(std::ostream &os, bool inited_only = false) const {
        return to_file(os, *this, inited_only);
    }
    friend std::ostream& operator<< (std::ostream &os, const args_pack &set) {
        return set.dump(os);
    }

    // for debug only
    void show_this(std::ostream &os) const {
        for_each(
            [&os](const auto &item)
            { os << "  " << item.name() << ": this="; item.show_this(os) << std::endl; }
        );
    }

private:
    template<typename F>
    void for_each(F &&f, bool inited_only = false) const {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }
    template<typename F>
    void for_each(F &&f, bool inited_only = false) {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }

    template<typename Iter, typename ...TArgs>
    friend void parse_kv_list(
         std::string *emsg
        ,const char *pref
        ,std::size_t pref_len
        ,Iter beg
        ,Iter end
        ,args_pack<TArgs...> &set
    );

    template<typename ...TArgs>
    friend std::ostream& to_file(
         std::ostream &os
        ,const args_pack<TArgs...> &set
        ,bool inited_only
    );

    template<typename ...TArgs>
    friend std::ostream& show_help(
         std::ostream &os
        ,const char *argv0
        ,const args_pack<TArgs...> &set
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
    // const
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I != std::tuple_size<Tuple>::value>
    for_each(const Tuple &tuple, const Func &func, bool inited_only) {
        const auto &item = std::get<I>(tuple);
        if ( inited_only ) {
            if ( item.is_set() ) {
                if ( !func(item) ) {
                    return;
                }
            }
        } else {
            if ( !func(item) ) {
                return;
            }
        }

        for_each<I + 1>(tuple, func, inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I == std::tuple_size<Tuple>::value>
    for_each(const Tuple &, const Func &, bool) {}

    // non-const
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I != std::tuple_size<Tuple>::value>
    for_each(Tuple &tuple, const Func &func, bool inited_only) {
        auto &item = std::get<I>(tuple);
        if ( inited_only ) {
            if ( item.is_set() ) {
                if ( !func(item) ) {
                    return;
                }
            }
        } else {
            if ( !func(item) ) {
                return;
            }
        }

        for_each<I + 1>(tuple, func, inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I == std::tuple_size<Tuple>::value>
    for_each(Tuple &, const Func &, bool) {}
};

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

        std::string line = pref
            ? (*beg) + pref_len
            : (*beg)
        ;

        cmdargs::details::trim(line);

        auto pos = line.find('=');
        if ( pos != std::string::npos ) {
            line[pos] = '\0';
        }

        std::string_view key = line.c_str();
        auto unexpected = args.check_for_unexpected(key);
        if ( !unexpected.empty() ) {
            std::string msg = "there is an extra \"";
            msg += (pref ? pref : "");
            msg += unexpected;
            msg += "\" option was specified";

            if ( emsg ) {
                *emsg = std::move(msg);
            } else {
                throw std::invalid_argument(msg);
            }

            return;
        }

        if ( pos != std::string::npos ) {
            std::string msg;
            const char *val = line.c_str() + pos + 1;
            std::size_t len = (line.length() - pos) - 1;
            args.for_each(
                [pref, key, val, len, &msg](auto &item) {
                    if ( item.name().compare(key) == 0 ) {
                        if ( !item.validate(val, len) ) {
                            msg = "an invalid value \"";
                            msg += val;
                            msg += "\" was received for \"";
                            msg += (pref ? pref : "");
                            msg += key;
                            msg += "\" option";

                            return false;
                        }
                        if ( !item.convert(val, len) ) {
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
                    throw std::invalid_argument(msg);
                }

                return;
            }
        } else {
            static const std::string_view help_key{"help"};
            static const std::string_view version_key{"version"};

            if ( key != version_key ) {
                if ( !args.is_bool_type(key) ) {
                    std::string msg = "a value must be provided for \"";
                    msg += (pref ? pref : "");
                    msg += key;
                    msg += "\" option";

                    if ( emsg ) {
                        *emsg = std::move(msg);
                    } else {
                        throw std::invalid_argument(msg);
                    }

                    return;
                }

                args.for_each(
                    [key](auto &item) {
                        if ( item.name().compare(key) == 0 ) {
                            static const char _true[] = "true";
                            item.convert(_true, sizeof(_true)-1);
                            return false;
                        }
                        return true;
                    }
                    ,false
                );
            } else {
                // version case
                args.for_each(
                    [key](auto &item) {
                        if ( item.name() == version_key ) {
                            item.set_value(item.get_default_value());
                            return false;
                        }
                        return true;
                    }
                    ,false
                );
            }

            if ( key == help_key || key == version_key ) {
                return;
            }
        }
    }

    auto required = args.check_for_required();
    if ( !required.empty() ) {
        std::string msg = "there is no required \"";
        msg += (pref ? pref : "");
        msg += required;
        msg += "\" option was specified";

        if ( emsg ) {
            *emsg = std::move(msg);
        } else {
            throw std::invalid_argument(msg);
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
            throw std::invalid_argument(msg);
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
            throw std::invalid_argument(msg);
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
            throw std::invalid_argument(msg);
        }
    }
}

/*************************************************************************************************/

template<
     typename ...Args
    ,typename = typename std::enable_if<
        sizeof...(Args) != 1
            && !std::is_base_of<
                 kwords_group
                ,typename std::tuple_element<0, std::tuple<Args...>>::type
        >::value
    >::type
>
auto parse_args(std::string *emsg, int argc, char* const* argv, const Args & ...kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args_pack<typename std::decay<Args>::type...> set{kwords...};
    parse_kv_list(emsg, "--", 2, beg, end, set);

    return set;
}

template<typename ...Args>
auto parse_args(std::string *emsg, int argc, char* const* argv, const std::tuple<Args...> &kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args_pack<typename std::decay<Args>::type...> set{std::get<Args>(kwords)...};
    parse_kv_list(emsg, "--", 2, beg, end, set);

    return set;
}

template<
     typename KWords
    ,typename = typename std::enable_if<
        std::is_class<KWords>::value &&
        std::is_base_of<kwords_group, KWords>::value
    >::type
>
auto parse_args(std::string *emsg, int argc, char* const* argv, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    auto res = parse_args(emsg, argc, argv, tuple);

    return res;
}

/*************************************************************************************************/

template<typename ...Args>
std::ostream& to_file(std::ostream &os, const args_pack<Args...> &args, bool inited_only = true) {
    args.for_each(
        [&os](const auto &item) {
            os << "# " << item.description() << std::endl;
            os << item.name() << "=";
            if ( item.is_set() ) {
                using decayed = typename std::decay<decltype(item)>::type;
                if constexpr ( std::is_same_v<typename decayed::value_type, bool> ) {
                    os << (item.get_value() ? "true" : "false");
                } else {
                    os << item.get_value();
                }
            }
            os << std::endl;

            return true;
        }
        ,inited_only
    );

    return os;
}

template<typename ...Args>
const auto& from_file(std::string *emsg, std::istream &is, args_pack<Args...> &args) {
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
     typename ...Args
    ,typename = typename std::enable_if<
        sizeof...(Args) != 1
            && !std::is_base_of<
                kwords_group
                ,typename std::tuple_element<0, std::tuple<Args...>>::type
        >::value
    >::type
>
auto from_file(std::string *emsg, std::istream &is, const Args & ...kwords) {
    args_pack<typename std::decay<Args>::type...> args{kwords...};
    from_file(emsg, is, args);

    return args;
}

template<typename ...Args>
auto from_file(std::string *emsg, std::istream &is, const std::tuple<Args...> &kwords) {
    args_pack<typename std::decay<Args>::type...> args{std::get<Args>(kwords)...};
    from_file(emsg, is, args);

    return args;
}

template<
     typename KWords
    ,typename = typename std::enable_if<
        std::is_class<KWords>::value &&
        std::is_base_of<kwords_group, KWords>::value
    >::type
>
auto from_file(std::string *emsg, std::istream &is, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    return from_file(emsg, is, tuple);
}

template<typename ...Args>
std::string to_string(const args_pack<Args...> &args, bool inited_only = true) {
    std::ostringstream os;

    to_file(os, args, inited_only);

    return os.str();
}

/*************************************************************************************************/

#ifdef _WIN32
constexpr char path_separator = '\\';
#else
constexpr char path_separator = '/';
#endif // _WIN32

template<typename ...Args>
std::ostream& show_help(std::ostream &os, const char *argv0, const args_pack<Args...> &args) {
    const auto pos = std::string_view{argv0}.rfind(path_separator);
    const char *p  = (pos != std::string_view::npos ? argv0+pos+1 : argv0);
    os << p << ":" << std::endl;

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
            os << ")" << std::endl;

            return true;
        }
        ,false
    );

    return os;
}

template<typename ...Args>
std::ostream& show_help(std::ostream &os, const char *argv0, const std::tuple<Args...> &kwords) {
    args_pack<typename std::decay<Args>::type...> args{std::get<Args>(kwords)...};

    return show_help(os, argv0, args);
}

template<
     typename KWords
    ,typename = typename std::enable_if<
        std::is_class<KWords>::value &&
        std::is_base_of<kwords_group, KWords>::value
    >::type
>
std::ostream& show_help(std::ostream &os, const char *argv0, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    return show_help(os, argv0, tuple);
}

/*************************************************************************************************/

#define CMDARGS_OPTION_ADD(OPTION_NAME, OPTION_TYPE, OPTION_DESCRIPTION, ...) \
    const ::cmdargs::option<struct OPTION_NAME, OPTION_TYPE> \
        OPTION_NAME{#OPTION_TYPE, OPTION_DESCRIPTION, std::make_tuple(__VA_ARGS__)}

#define CMDARGS_OPTION_ADD_HELP() \
    const ::cmdargs::details::help_option_type help{"bool", "show help message" \
        , std::make_tuple(optional)};

#define CMDARGS_OPTION_ADD_VERSION(str) \
    const ::cmdargs::details::version_option_type version{"std::string", "show version message" \
        , std::make_tuple(optional, ::cmdargs::details::default_t<std::string>{str})};

/*************************************************************************************************/

template<typename ...Args>
bool is_help_requested(std::ostream &os, const char *argv0, const args_pack<Args...> &args) {
    if constexpr ( args.template contains<details::help_option_type>() ) {
        if ( args.template is_set<details::help_option_type>() ) {
            show_help(os, argv0, args);

            return true;
        }
    }

    return false;
}

template<typename ...Args>
bool is_version_requested(std::ostream &os, const char *argv0, const args_pack<Args...> &args) {
    if constexpr ( args.template contains<details::version_option_type>() ) {
        if ( args.template is_set<details::version_option_type>() ) {
            const auto pos = std::string_view{argv0}.rfind(path_separator);
            const auto ptr  = (pos != std::string_view::npos ? argv0+pos+1 : argv0);
            os << ptr << ": version - " << args.template get<details::version_option_type>() << std::endl;

            return true;
        }
    }

    return false;
}

template<typename ...Args>
bool is_help_or_version_requested(std::ostream &os, const char *argv0, const args_pack<Args...> &args) {
    return is_help_requested(os, argv0, args) || is_version_requested(os, argv0, args);
}

/*************************************************************************************************/

} // ns cmdargs

#endif // __CMDARGS__CMDARGS_HPP
