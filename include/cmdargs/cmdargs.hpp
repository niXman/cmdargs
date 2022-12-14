
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2022 niXman (github dot nixman at pm dot me)
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

//#include <iostream> // TODO: comment out

#include <ostream>
#include <istream>
#include <sstream>
#include <vector>
#include <tuple>
#include <array>
#include <string>
#include <type_traits>
#include <functional>
#include <stdexcept>

#include <cassert>
#include <cstring>
#include <cinttypes>

/*************************************************************************************************/

#define __CMDARGS_CAT_I(a, b) a ## b
#define __CMDARGS_CAT(a, b) __CMDARGS_CAT_I(a, b)

#define __CMDARGS_STRINGIZE_I(x) #x
#define __CMDARGS_STRINGIZE(x) __CMDARGS_STRINGIZE_I(x)

/*************************************************************************************************/

#define __CMDARGS_REPEAT_0(macro, data)
#define __CMDARGS_REPEAT_1(macro, data) \
    macro(0, data)
#define __CMDARGS_REPEAT_2(macro, data) \
    __CMDARGS_REPEAT_1(macro, data) macro(1, data)
#define __CMDARGS_REPEAT_3(macro, data) \
    __CMDARGS_REPEAT_2(macro, data) macro(2, data)
#define __CMDARGS_REPEAT_4(macro, data) \
    __CMDARGS_REPEAT_3(macro, data) macro(3, data)
#define __CMDARGS_REPEAT_5(macro, data) \
    __CMDARGS_REPEAT_4(macro, data) macro(4, data)
#define __CMDARGS_REPEAT_6(macro, data) \
    __CMDARGS_REPEAT_5(macro, data) macro(5, data)
#define __CMDARGS_REPEAT_7(macro, data) \
    __CMDARGS_REPEAT_6(macro, data) macro(6, data)
#define __CMDARGS_REPEAT_8(macro, data) \
    __CMDARGS_REPEAT_7(macro, data) macro(7, data)
#define __CMDARGS_REPEAT_9(macro, data) \
    __CMDARGS_REPEAT_8(macro, data) macro(8, data)
#define __CMDARGS_REPEAT_10(macro, data) \
    __CMDARGS_REPEAT_9(macro, data) macro(9, data)
#define __CMDARGS_REPEAT_11(macro, data) \
    __CMDARGS_REPEAT_10(macro, data) macro(10, data)
#define __CMDARGS_REPEAT_12(macro, data) \
    __CMDARGS_REPEAT_11(macro, data) macro(11, data)
#define __CMDARGS_REPEAT_13(macro, data) \
    __CMDARGS_REPEAT_12(macro, data) macro(12, data)
#define __CMDARGS_REPEAT_14(macro, data) \
    __CMDARGS_REPEAT_13(macro, data) macro(13, data)
#define __CMDARGS_REPEAT_15(macro, data) \
    __CMDARGS_REPEAT_14(macro, data) macro(14, data)
#define __CMDARGS_REPEAT_16(macro, data) \
    __CMDARGS_REPEAT_15(macro, data) macro(15, data)
#define __CMDARGS_REPEAT_17(macro, data) \
    __CMDARGS_REPEAT_16(macro, data) macro(16, data)
#define __CMDARGS_REPEAT_18(macro, data) \
    __CMDARGS_REPEAT_17(macro, data) macro(17, data)
#define __CMDARGS_REPEAT_19(macro, data) \
    __CMDARGS_REPEAT_18(macro, data) macro(18, data)
#define __CMDARGS_REPEAT_20(macro, data) \
    __CMDARGS_REPEAT_19(macro, data) macro(19, data)

#define __CMDARGS_REPEAT_IMPL(start_macro, macro, data) \
    start_macro(macro, data)
#define __CMDARGS_REPEAT(n, macro, data) \
    __CMDARGS_REPEAT_IMPL(__CMDARGS_CAT(__CMDARGS_REPEAT_, n), macro, data)

/*************************************************************************************************/

#if __cplusplus < 201703L

namespace cmdargs {

template<typename T>
struct optional_type {
    optional_type()
        :m_inited{}
    {}

    template<typename V>
    explicit optional_type(V &&v)
        :m_val{std::forward<V>(v)}
        ,m_inited{true}
    {}

    template<typename V>
    explicit optional_type(const V &v)
        :m_val{v}
        ,m_inited{true}
    {}

    template<typename V>
    optional_type& operator= (V &&v) noexcept {
        m_val = std::forward<V>(v);
        m_inited = true;

        return *this;
    }

    explicit operator bool() const noexcept { return has_value(); }

    bool has_value() const noexcept { return m_inited; }

    T& value() noexcept { return m_val; }
    const T& value() const noexcept { return m_val; }

    friend std::ostream& operator<< (std::ostream &os, const optional_type &v) {
        if ( v ) {
            os << v.value();
        } else {
            os << "<UNINITIALIZED>";
        }

        return os;
    }

    T m_val;
    bool m_inited;
};

} // ns cmdargs

#else

#include <optional>

namespace cmdargs {

template<typename T>
struct optional_type: std::optional<T> {
    using std::optional<T>::operator=;
    using std::optional<T>::operator bool;

    friend std::ostream& operator<< (std::ostream &os, const optional_type &v) {
        if ( v ) {
            os << v.value();
        } else {
            os << "<UNINITIALIZED>";
        }

        return os;
    }
};

} // ns cmdargs

#endif // __cplusplus < 201703L

/*************************************************************************************************/

namespace cmdargs {
namespace details {

inline void ltrim(std::string &s, const char* t = " \t\n\r")
{ s.erase(0, s.find_first_not_of(t)); }

inline void rtrim(std::string &s, const char* t = " \t\n\r")
{ s.erase(s.find_last_not_of(t) + 1); }

inline void trim(std::string &s, const char* t = " \t\n\r")
{ rtrim(s, t); ltrim(s, t); }

constexpr std::size_t ct_strlen(const char *s) {
    const char *p = s;
    for ( ; *p; ++p )
        ;
    return p - s;
}

inline std::string cat_vector(
     const char *pref
    ,const std::vector<const char *> &names
    ,bool double_quoted = false)
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
constexpr auto ct_init_array(const char *s, char c0, char c1) {
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
from_string_impl(T *val, const char *ptr, std::size_t len) {
    val->assign(ptr, len);
}

template<typename T>
typename std::enable_if_t<std::is_same<T, bool>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) {
    *val = std::strncmp(ptr, "true", len) == 0;
}

template<typename T>
typename std::enable_if_t<
    std::is_integral<T>::value
        && !std::is_same<T, bool>::value
>
from_string_impl(T *val, const char *ptr, std::size_t len) {
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
from_string_impl(T *val, const char *ptr, std::size_t len) {
    typename std::underlying_type<T>::type tmp{};
    from_string_impl(&tmp, ptr, len);
    *val = static_cast<T>(tmp);
}

template<typename T>
typename std::enable_if_t<std::is_floating_point<T>::value>
from_string_impl(T *val, const char *ptr, std::size_t len) {
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
from_string_impl(T *val, const char *, std::size_t) {
    *val = nullptr;
}

/*************************************************************************************************/
// check a sequence for existence of a type
// based on https://stackoverflow.com/questions/55941964

template<typename, typename>
struct contains;

template<typename Car, typename... Cdr, typename Needle>
struct contains<std::tuple<Car, Cdr...>, Needle>: contains<std::tuple<Cdr...>, Needle>
{};

template<typename... Cdr, typename Needle>
struct contains<std::tuple<Needle, Cdr...>, Needle>: std::true_type
{};

template<typename Needle>
struct contains<std::tuple<>, Needle>: std::false_type
{};

template<typename Needle, typename ...Types>
constexpr bool contains_f(const Types &...) {
    return contains<std::tuple<Types...>, Needle>::value;
}

template<typename Needle>
constexpr bool contains_f() {
    return false;
}

template <typename Out, typename In>
struct filter;

template <typename... Out, typename InCar, typename... InCdr>
struct filter<std::tuple<Out...>, std::tuple<InCar, InCdr...>> {
    using type = typename std::conditional<
         contains<std::tuple<Out...>, InCar>::value
        ,filter<std::tuple<Out...>, std::tuple<InCdr...>>
        ,filter<std::tuple<Out..., InCar>, std::tuple<InCdr...>>
    >::type::type;
};

template <typename Out>
struct filter<Out, std::tuple<>> {
    using type = Out;
};

template <typename T>
using without_duplicates = typename filter<std::tuple<>, T>::type;

/*************************************************************************************************/
// is callable

template<typename F>
using has_operator_call_t = decltype(&F::operator());

template<typename F, typename = void>
struct is_callable : std::false_type
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
> : std::true_type
{};

struct optional_bool_printer {
    static void print(std::ostream &os, const cmdargs::optional_type<bool> &v)
    { os << (v.value() ? "true" : "false"); }
    template<typename T>
    static void print(std::ostream &os, const cmdargs::optional_type<T> &v)
    { os << v.value(); }
};

/*************************************************************************************************/
// compound type to tuple
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
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, 0>) {
    return std::make_tuple();
}

#define __CMDARGS_TO_TUPLE_P(idx, data) , p##idx
#define __CMDARGS_TO_TUPLE_SPECIALIZATION(idx, data) \
    template<typename T> \
    auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, idx + 1>) { \
        const auto& [p __CMDARGS_REPEAT_##idx(__CMDARGS_TO_TUPLE_P, data)] = object; \
        return std::make_tuple(p __CMDARGS_REPEAT_##idx(__CMDARGS_TO_TUPLE_P, data)); \
    }

#ifndef __CMDARGS_TO_TUPLE_MAX
#define __CMDARGS_TO_TUPLE_MAX 20
#endif

__CMDARGS_REPEAT(__CMDARGS_TO_TUPLE_MAX, __CMDARGS_TO_TUPLE_SPECIALIZATION, ~)

template<
     typename T
    ,typename = struct current_value
    ,std::size_t = __CMDARGS_TO_TUPLE_MAX
    ,typename = struct required_value
    ,std::size_t N
>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, N>) {
    static_assert(N <= __CMDARGS_TO_TUPLE_MAX, "Please increase __CMDARGS_TO_TUPLE_MAX");
}

template<
     typename T
    ,typename = std::enable_if_t<std::is_class<T>::value>
>
auto to_tuple(const T &kw) {
    return to_tuple_impl(kw, to_tuple_size<std::decay_t<T>>{});
}

} // ns details

/*************************************************************************************************/

#ifdef CMDARGS_OPTION_ASSIGN_ENABLED
#   define __CMDARGS_EXPAND_EXPR(...) __VA_ARGS__
#else
#   define __CMDARGS_EXPAND_EXPR(...)
#endif

/*************************************************************************************************/

#define CMDARGS_OPTION_DECLARE(OPTION_NAME, OPTION_TYPE_NAME, OPTION_TYPE, DESCRIPTION) \
    struct OPTION_TYPE_NAME: ::cmdargs::option_base { \
        static constexpr const char* m_opt_name() { return __CMDARGS_STRINGIZE(OPTION_NAME); } \
        static constexpr const char* m_opt_type() { return __CMDARGS_STRINGIZE(OPTION_TYPE); } \
        static constexpr const char *m_opt_descr() { return __CMDARGS_STRINGIZE(DESCRIPTION); } \
        \
        using value_type      = OPTION_TYPE; \
        using optional_type   = ::cmdargs::optional_type<value_type>; \
        using expression_list = ::cmdargs::expression_list; \
        using validator_type  = std::function<bool(const char *str, std::size_t len)>; \
        using converter_type  = std::function<bool(void *dst, const char *str, std::size_t len)>; \
        \
        /* data members */ \
        bool m_required; \
        optional_type m_val; \
        validator_type m_validator; \
        converter_type m_converter; \
        std::vector<const char *> m_and_list{}; \
        std::vector<const char *> m_or_list{}; \
        std::vector<const char *> m_not_list{}; \
        \
        OPTION_TYPE_NAME& operator= (const OPTION_TYPE_NAME &) = delete; \
        OPTION_TYPE_NAME& operator= (OPTION_TYPE_NAME &&) = delete; \
        \
        OPTION_TYPE_NAME(const OPTION_TYPE_NAME &r) = default; \
        OPTION_TYPE_NAME(OPTION_TYPE_NAME &&r) = default; \
        /* when the following 'operator=(T &&)' is enabled then a kwords-group \
         * can't be declared inside function. \
         */ \
        __CMDARGS_EXPAND_EXPR( \
            template<typename T> \
            OPTION_TYPE_NAME operator= (T &&v) const { \
                OPTION_TYPE_NAME res; \
                res.m_required = m_required; \
                res.m_val      = std::forward<T>(v); \
                res.m_validator= m_validator; \
                res.m_converter= m_converter; \
                res.m_and_list = m_and_list; \
                res.m_or_list  = m_or_list; \
                res.m_not_list = m_not_list; \
                return res; \
            } \
        ) \
        /* the following set of the constructors can't be replaced \
         * with a single templated variadic-list constructor because \
         * then this struct become not-braces-constructible, which is required \
         * for the 'to_tuple()' function. */ \
        /* default */ \
        OPTION_TYPE_NAME() \
            :option_base{m_opt_name(), m_opt_type(), m_opt_descr()} \
            ,m_required{true} \
            ,m_val{} \
            ,m_validator{} \
            ,m_converter{} \
        {} \
        /* default - optional */ \
        OPTION_TYPE_NAME(const ::cmdargs::optional_t &) \
            :option_base{m_opt_name(), m_opt_type(), m_opt_descr()} \
            ,m_required{false} \
            ,m_val{} \
            ,m_validator{} \
            ,m_converter{} \
        {} \
        /* default with cond list */ \
        OPTION_TYPE_NAME( \
             expression_list expr0 \
            ,expression_list expr1 = {} \
            ,expression_list expr2 = {} \
            ,expression_list expr3 = {} \
            ,expression_list expr4 = {} \
        ) \
            :option_base{m_opt_name(), m_opt_type(), m_opt_descr()} \
            ,m_required{true} \
            ,m_val{} \
            ,m_validator{} \
            ,m_converter{} \
        { get_conditions(std::move(expr0), std::move(expr1), std::move(expr2) \
            , std::move(expr3), std::move(expr4)); } \
        /* default with cond list and category */ \
        OPTION_TYPE_NAME( \
             const ::cmdargs::optional_t & \
            ,expression_list expr0 \
            ,expression_list expr1 = {} \
            ,expression_list expr2 = {} \
            ,expression_list expr3 = {} \
            ,expression_list expr4 = {} \
        ) \
            :option_base{m_opt_name(), m_opt_type(), m_opt_descr()} \
            ,m_required{false} \
            ,m_val{} \
            ,m_validator{} \
            ,m_converter{} \
        { get_conditions(std::move(expr0), std::move(expr1), std::move(expr2) \
            , std::move(expr3), std::move(expr4)); } \
        \
        bool is_required() const override { return m_required; } \
        bool is_optional() const override { return !is_required(); } \
        bool is_set() const override { return m_val.has_value(); } \
        bool is_bool() const override { return std::is_same<value_type, bool>::value; } \
        \
        const std::vector<const char *>& and_list() const override { return m_and_list; } \
        const std::vector<const char *>& or_list() const override { return m_or_list; } \
        const std::vector<const char *>& not_list() const override { return m_not_list; } \
        \
        bool has_validator() const override { return static_cast<bool>(m_validator); } \
        bool validate(const char *str, std::size_t len) const override { \
            return m_validator(str, len); \
        } \
        \
        bool has_converter() const override { return static_cast<bool>(m_converter); } \
        bool convert(const char *str, std::size_t len) override { \
            value_type v{}; \
            if ( m_converter(std::addressof(v), str, len) ) { \
                m_val = std::move(v); \
                return true; \
            } \
            return false; \
        } \
        \
        void from_string(const char *ptr, std::size_t len) { \
            value_type v{}; \
            ::cmdargs::details::from_string_impl(&v, ptr, len); \
            m_val = std::move(v); \
        } \
        std::ostream& dump(std::ostream &os) const override { \
            os \
                << "this=" << this << ": name=" << name() << "(" << type() << "): req=" << is_required() \
                << ", set=" << is_set() << ", and=" << m_and_list.size() \
                << ", or=" << m_or_list.size() << ", not=" << m_not_list.size() \
                << ", val=" << m_val \
            ; \
            return os; \
        } \
        void get_conditions(expression_list expr0, expression_list expr1 \
            , expression_list expr2, expression_list expr3, expression_list expr4) \
        { \
            for ( auto &&it: {std::move(expr0), std::move(expr1), std::move(expr2) \
                , std::move(expr3), std::move(expr4)} ) \
            { \
                if ( it.is_empty() ) { continue; } \
                switch ( it.type() ) { \
                    case ::cmdargs::expression_list::AND: { m_and_list = it.list(); break; } \
                    case ::cmdargs::expression_list::OR: { m_or_list = it.list(); break; } \
                    case ::cmdargs::expression_list::NOT: { m_not_list = it.list(); break; } \
                    case ::cmdargs::expression_list::VALIDATOR: { m_validator = it.validator(); break; } \
                    case ::cmdargs::expression_list::CONVERTER: { m_converter = it.converter(); break; } \
                    case ::cmdargs::expression_list::UNDEFINED: { \
                        assert("unexpected UNDEFINED expression_list!" == nullptr); break; } \
                } \
            } \
        } \
    }

#define CMDARGS_OPTION_INIT(OPTION_NAME, ...) \
    const OPTION_NAME##_t OPTION_NAME{ __VA_ARGS__ }

#define CMDARGS_OPTION_ADD(OPTION_NAME, OPTION_TYPE, DESCRIPTION, ...) \
    /* type */ CMDARGS_OPTION_DECLARE(OPTION_NAME, OPTION_NAME##_t, OPTION_TYPE, DESCRIPTION); \
    /* var  */ CMDARGS_OPTION_INIT(OPTION_NAME, __VA_ARGS__)

#define CMDARGS_OPTION_ADD_HELP() \
    CMDARGS_OPTION_ADD(help, bool, "show help message", optional)

#define CMDARGS_OPTION_ADD_VERSION() \
    CMDARGS_OPTION_ADD(version, bool, "show version message", optional)

/*************************************************************************************************/

struct option_base {
    explicit option_base(const char *name, const char *type, const char *descr)
        :m_name{name}
        ,m_type{type}
        ,m_descr{descr}
    {}
    const char* name() const { return m_name; }
    const char* type() const { return m_type; }
    const char* description() const { return m_descr; }

    virtual bool is_required() const = 0;
    virtual bool is_optional() const = 0;
    virtual bool is_set() const = 0;
    virtual bool is_bool() const = 0;

    virtual std::ostream& dump(std::ostream &os) const = 0;

    virtual const std::vector<const char *>&
    and_list() const = 0;
    virtual const std::vector<const char *>&
    or_list() const = 0;
    virtual const std::vector<const char *>&
    not_list() const = 0;

    virtual bool has_validator() const = 0;
    virtual bool validate(const char *str, std::size_t len) const = 0;

    virtual bool has_converter() const = 0;
    virtual bool convert(const char *str, std::size_t len) = 0;

private:
    const char *m_name;
    const char *m_type;
    const char *m_descr;
};

/*************************************************************************************************/

struct optional_t {};

struct expression_list {
    enum e_type {
         UNDEFINED // 0
        ,AND       // 1
        ,OR        // 2
        ,NOT       // 3
        ,VALIDATOR // 4
        ,CONVERTER // 5
    };

    expression_list(const expression_list &) = default;
    expression_list(expression_list &&) = default;

    expression_list() = default;
    expression_list(e_type t, const std::initializer_list<const char *> list)
        :m_type{t}
        ,m_list{list}
        ,m_validator{}
        ,m_converter{}
    {}

    using validator_func = std::function<bool(const char *ptr, std::size_t len)>;
    expression_list(e_type t, validator_func func)
        :m_type{t}
        ,m_list{}
        ,m_validator{std::move(func)}
        ,m_converter{}
    { assert(t == VALIDATOR); }

    using converter_func = std::function<bool(void *dst, const char *ptr, std::size_t len)>;
    expression_list(e_type t, converter_func func)
        :m_type{t}
        ,m_list{}
        ,m_validator{}
        ,m_converter{std::move(func)}
    { assert(t == CONVERTER); }

    e_type type() const { return m_type; }
    const char* type_name() const {
        static const char *arr[] = {
            "UNDEFINED" // 0
           ,"AND"       // 1
           ,"OR"        // 2
           ,"NOT"       // 3
           ,"VALIDATOR" // 4
           ,"CONVERTER" // 5
        };

        return arr[static_cast<std::size_t>(m_type)];
    }
    auto list() const { return m_list; }
    auto validator() const { return m_validator; }
    auto converter() const { return m_converter; }

    bool is_empty() const {
        bool empty = list().empty()
            && false == static_cast<bool>(m_validator)
            && false == static_cast<bool>(m_converter)
        ;
        return empty;
    }

private:
    e_type m_type;
    std::vector<const char *> m_list;
    validator_func m_validator;
    converter_func m_converter;
};

/*************************************************************************************************/

struct kwords_group {
    static constexpr optional_t optional{};

    template<typename ...Types>
    static auto and_(const Types &...args) {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
             std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return expression_list{expression_list::e_type::AND, {args.m_opt_name()...}};
    }
    template<typename ...Types>
    static auto or_(const Types &...args) {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
             std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return expression_list{expression_list::e_type::OR, {args.m_opt_name()...}};
    }
    template<typename ...Types>
    static auto not_(const Types &...args) {
        using tuple_type = std::tuple<typename std::decay<Types>::type...>;
        static_assert(
             std::tuple_size<tuple_type>::value
                == std::tuple_size<details::without_duplicates<tuple_type>>::value
            ,"duplicates of keywords is detected!"
        );
        return expression_list{expression_list::e_type::NOT, {args.m_opt_name()...}};
    }
    template<typename F>
    static auto validator_(F &&f) {
        return expression_list(expression_list::e_type::VALIDATOR, std::forward<F>(f));
    }
    template<typename F>
    static auto converter_(F &&f) {
        return expression_list(expression_list::e_type::CONVERTER, std::forward<F>(f));
    }
};

/*************************************************************************************************/

template<typename ...Args>
struct args {
    using container_type = std::tuple<typename std::decay<Args>::type...>;
    static_assert(
         std::tuple_size<container_type>::value
            == std::tuple_size<details::without_duplicates<container_type>>::value
        ,"duplicates of keywords are detected!"
    );

    container_type m_kwords;

    template<typename ...Types>
    explicit args(const Types &...types)
        :m_kwords{types...}
    {}

    container_type& kwords() { return m_kwords; }
    const container_type& kwords() const { return m_kwords; }

    template<typename T>
    struct has_type {
        static constexpr bool value = !std::is_same<
             std::integer_sequence<bool, false, std::is_same<T, typename std::decay<Args>::type>::value...>
            ,std::integer_sequence<bool, std::is_same<T, typename std::decay<Args>::type>::value..., false>
        >::value;
    };

    constexpr std::size_t size() const { return std::tuple_size<container_type>::value; }

    template<typename T>
    constexpr bool has(const T &) const { return has_type<T>::value; }
    template<typename T>
    constexpr bool has() const { return has_type<T>::value; }

    template<typename T>
    bool is_set(const T &) const {
        static_assert(has_type<T>::value, "");

        return std::get<T>(m_kwords).is_set();
    }

    bool is_valid_name(const char *name) const {
        return check_for_unexpected(name) == nullptr;
    }
    bool is_bool_type(const char *name) const {
        bool res{};

        for_each(
            [name, &res](const auto &item) {
                if ( 0 == std::strcmp(item.name(), name) ) {
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
    const typename T::value_type& get(const T &) const {
        static_assert(has_type<T>::value, "");

        return std::get<T>(m_kwords).m_val.value();
    }
    template<typename T>
    typename T::value_type& get(const T &) {
        static_assert(has_type<T>::value, "");

        return std::get<T>(m_kwords).m_val.value();
    }
    template<typename T, typename U>
    typename T::value_type get(const T &k, U &&v) const {
        if ( is_set(k) ) {
            return std::get<T>(m_kwords).m_val.value();
        }

        return std::forward<U>(v);
    }

    void reset() {
        reset(m_kwords);
    }
    template<typename ...Types>
    void reset(const Types & ...t) {
        reset_impl(t...);
    }
    template<typename ...Types>
    void reset(const std::tuple<Types...> &t) {
        reset_impl(std::get<Types>(t)...);
    }

    template<typename F>
    void for_each(F &&f, bool inited_only = false) const {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }
    template<typename F>
    void for_each(F &&f, bool inited_only = false) {
        for_each(m_kwords, std::forward<F>(f), inited_only);
    }

    std::ostream& dump(std::ostream &os, bool inited_only = false) const {
        to_file(os, *this, inited_only);
        return os;
    }
    friend std::ostream& operator<< (std::ostream &os, const args &set) {
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
    template<typename Iter, typename ...TArgs>
    friend void parse_kv_list(
         std::string *emsg
        ,const char *pref
        ,std::size_t pref_len
        ,Iter beg
        ,Iter end
        ,args<TArgs...> &set
    );

    const option_base* get_by_name(const char *name) const {
        const option_base *opt = nullptr;
        for_each(
             m_kwords
            ,[&opt, name](const auto &item) {
                if ( std::strcmp(item.name(), name) == 0 ) {
                    opt = std::addressof(item);
                }
                return opt == nullptr;
            }
            ,false
        );

        return opt;
    }

    const char* check_for_unexpected(const char *optname) const {
        const char *ptr = nullptr;
        for_each(
             m_kwords
            ,[&ptr, optname](const auto &item) {
                if ( std::strcmp(item.name(), optname) == 0 ) {
                    ptr = item.name();
                }
                return ptr == nullptr;
            }
            ,false
        );

        return ptr ? nullptr : optname;
    }
    const char* check_for_required() const {
        const char *name = nullptr;
        for_each(
             m_kwords
            ,[&name](const auto &item){
                if ( item.is_required() && !item.is_set() ) {
                    name = item.name();
                }
                return name == nullptr;
             }
            ,false
        );

        return name;
    }

    using cond_ret_type = std::pair<
         const char *
        ,std::vector<const char *>
    >;
    cond_ret_type check_for_cond_and() const {
        return check_for_conditional(
            [](const auto &item) {
                return item.is_set()
                    ? item.and_list()
                    : std::vector<const char *>{}
                ;
            }
            ,[this](const auto &list) -> std::vector<const char *> {
                std::vector<const char *> ret;
                for ( const char *it: list ) {
                    const auto *opt = get_by_name(it);
                    if ( opt && !opt->is_set() ) {
                        ret.push_back(it);
                    }
                }
                return ret;
            }
        );
    }
    cond_ret_type check_for_cond_or() const {
        return check_for_conditional(
             [](const auto &item) { return item.or_list(); }
            ,[this](const auto &list) -> std::vector<const char *> {
                auto num = 0u;
                std::vector<const char *> ret;
                for ( const char *it: list ) {
                    const auto *opt = get_by_name(it);
                    if ( opt && opt->is_set() ) {
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
                    : std::vector<const char *>{}
                ;
            }
            ,[this](const auto &list) -> std::vector<const char *> {
                std::vector<const char *> ret;
                for ( const char *it: list ) {
                    const auto *opt = get_by_name(it);
                    if ( opt && opt->is_set() ) {
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
    void reset_impl() {}
    template<typename T0, typename ...Types>
    void reset_impl(const T0 &, const Types & ...types) {
        std::get<T0>(m_kwords).m_val = typename T0::value_type{};
        reset_impl(types...);
    }

    static const char* check_for_required_impl() { return nullptr; }
    template<typename T0, typename ...Types>
    static const char* check_for_required_impl(const T0 &, const Types & ...types) {
        if ( !has_type<T0>::value ) {
            return T0::__name();
        }

        return check_for_required_impl(types...);
    }

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
    ,args<Args...> &set)
{
    for ( ; beg != end; ++beg ) {
        if ( pref ) {
            if ( std::strncmp(*beg, pref, pref_len) != 0 ) {
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

        const char *key = line.c_str();
        if ( const char *unexpected = set.check_for_unexpected(key) ) {
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
            set.for_each(
                [pref, key, val, len, &msg](auto &item) {
                    if ( std::strcmp(item.name(), key) == 0 ) {
                        bool has_validator = item.has_validator();
                        bool has_converter = item.has_converter();
                        if ( has_validator ) {
                            if ( !item.validate(val, len) ) {
                                msg = "an invalid value \"";
                                msg += val;
                                msg += "\" was received for \"";
                                msg += (pref ? pref : "");
                                msg += key;
                                msg += "\" option";

                                return false;
                            }
                        }
                        if ( has_converter ) {
                            if ( !item.convert(val, len) ) {
                                msg = "can't convert value \"";
                                msg += val;
                                msg += "\" for \"";
                                msg += (pref ? pref : "");
                                msg += key;
                                msg += "\" option";

                                return false;
                            }
                        } else {
                            item.from_string(val, len);
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
            if ( !set.is_bool_type(key) ) {
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

            set.for_each(
                [key](auto &item) {
                    if ( std::strcmp(item.name(), key) == 0 ) {
                        static const char _true[] = "true";
                        item.from_string(_true, sizeof(_true)-1);
                        return false;
                    }
                    return true;
                }
                ,false
            );

            if ( std::strcmp(key, "help") == 0 || std::strcmp(key, "version") == 0 ) {
                return;
            }
        }
    }

    if ( const char *required = set.check_for_required() ) {
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

    auto cond_and = set.check_for_cond_and();
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

    auto cond_or = set.check_for_cond_or();
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

    auto cond_not = set.check_for_cond_not();
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

template<typename ...Args>
auto make_args(Args && ...args) {
    cmdargs::args<typename std::decay<Args>::type...> set{std::forward<Args>(args)...};

    return set;
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
auto parse_args(std::string *emsg, int argc, char* const* argv, Args && ...kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args<typename std::decay<Args>::type...> set{std::forward<Args>(kwords)...};
    parse_kv_list(emsg, "--", 2, beg, end, set);

    return set;
}

template<typename ...Args>
auto parse_args(std::string *emsg, int argc, char* const* argv, const std::tuple<Args...> &kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args<typename std::decay<Args>::type...> set{std::get<Args>(kwords)...};
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
std::ostream& to_file(std::ostream &os, const args<Args...> &set, bool inited_only = true) {
    set.for_each(
        [&os](auto &item) {
            os << "# " << item.description() << std::endl;
            os << item.name() << "=";
            if ( item.m_val ) {
                details::optional_bool_printer::print(os, item.m_val);
            }
            os << std::endl;

            return true;
        }
        ,inited_only
    );

    return os;
}

template<typename ...Args>
auto from_file(std::string *emsg, std::istream &is, args<Args...> &set) {
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

    parse_kv_list(emsg, nullptr, 0, linesptrs.begin(), linesptrs.end(), set);

    return set;
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
auto from_file(std::string *emsg, std::istream &is, Args && ...kwords) {
    args<typename std::decay<Args>::type...> set{std::forward<Args>(kwords)...};

    return from_file(emsg, is, set);
}

template<typename ...Args>
auto from_file(std::string *emsg, std::istream &is, const std::tuple<Args...> &kwords) {
    args<typename std::decay<Args>::type...> set{std::get<Args>(kwords)...};

    return from_file(emsg, is, set);
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
std::string to_string(const args<Args...> &set, bool inited_only = true) {
    std::ostringstream os;

    to_file(os, set, inited_only);

    return os.str();
}

/*************************************************************************************************/

template<typename ...Args>
std::ostream& show_help(std::ostream &os, const char *argv0, const args<Args...> &set) {
    const char *p = std::strrchr(argv0, '/');
    os
    << (p ? p+1 : "program") << ":" << std::endl;

    std::size_t max_len = 0;
    set.for_each(
        [&max_len](const auto &item) {
            std::size_t len = std::strlen(item.name());
            max_len = (len > max_len) ? len : max_len;

            return true;
        }
        ,false
    );

    set.for_each(
        [&os, max_len](const auto &item) {
            static const char ident[] = "                                        ";
            const char *name = item.name();
            std::size_t len = std::strlen(name);
            os << "--" << name << "=*";
            os.write(ident, static_cast<std::streamsize>(max_len - len));
            os << ": " << item.description()
            << " ("
                << item.type()
                << ", "
                << (item.is_required() ? "required" : "optional")
            ;
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
    args<typename std::decay<Args>::type...> set{std::get<Args>(kwords)...};

    return show_help(os, argv0, set);
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

} // ns cmdargs

/*************************************************************************************************/

#endif // __CMDARGS__CMDARGS_HPP
