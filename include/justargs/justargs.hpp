
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2022 niXman (github dot nixman at pm dot me)
// This file is part of JustArgs(github.com/niXman/justargs) project.
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

#ifndef __JUSTARGS__JUSTARGS_HPP
#define __JUSTARGS__JUSTARGS_HPP

//#include <iostream> // TODO: comment out

#include <ostream>
#include <istream>
#include <sstream>
#include <map>
#include <vector>
#include <tuple>
#include <array>
#include <string>
#include <type_traits>
#include <typeindex>
#include <functional>
#include <stdexcept>

#include <cassert>
#include <cstring>
#include <cinttypes>

/*************************************************************************************************/

#if __cplusplus < 201703L

namespace justargs {

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

    explicit operator bool() const noexcept { return m_inited; }

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

} // ns justargs

#else

#include <optional>

namespace justargs {

template<typename T>
struct optional_type: std::optional<T> {
    using std::optional<T>::operator=;
};

template<typename T>
inline std::ostream& operator<< (std::ostream &os, const optional_type<T> &v) {
    if ( v ) {
        os << v.value();
    } else {
        os << "<UNINITIALIZED>";
    }

    return os;
}

} // ns justargs

#endif // __cplusplus < 201703L

/*************************************************************************************************/

namespace justargs {

template<typename D>
struct kwords_group;

template<typename ...>
struct args;

template<typename Iter, typename ...Args>
auto parse_kv_list(
     bool *ok
    ,std::string *emsg
    ,const char *pref
    ,std::size_t pref_len
    ,Iter beg
    ,Iter end
    ,args<Args...> &set
);

namespace details {

inline void ltrim(std::string &s, const char* t = " \t\n\r") {
    s.erase(0, s.find_first_not_of(t));
};

inline void rtrim(std::string &s, const char* t = " \t\n\r") {
    s.erase(s.find_last_not_of(t) + 1);
};

inline void trim(std::string &s, const char* t = " \t\n\r") {
    rtrim(s, t);
    ltrim(s, t);
};

constexpr std::size_t ct_strlen(const char *s) {
    const char *p = s;
    for ( ; *p; ++p )
        ;
    return p - s;
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
typename std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, bool>::value>
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

} // ns details

#define __JUSTARGS_IIF_0(t, f) f
#define __JUSTARGS_IIF_1(t, f) t
#define __JUSTARGS_IIF_I(bit, t, f) __JUSTARGS_IIF_ ## bit(t, f)
#define __JUSTARGS_IIF(bit, t, f) __JUSTARGS_IIF_I(bit, t, f)
#define __JUSTARGS_IF(cond, t, f) __JUSTARGS_IIF(cond, t, f)

#define __JUSTARGS_CAT_I(a, b) a ## b
#define __JUSTARGS_CAT(a, b) __JUSTARGS_CAT_I(a, b)

#define __JUSTARGS_STRINGIZE_I(x) #x
#define __JUSTARGS_STRINGIZE(x) __JUSTARGS_STRINGIZE_I(x)

#define __JUSTARGS_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define __JUSTARGS_HAS_COMMA(...) __JUSTARGS_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define __JUSTARGS_TRIGGER_PARENTHESIS_(...) ,
#define __JUSTARGS_PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define __JUSTARGS_IS_EMPTY_CASE_0001 ,
#define __JUSTARGS_IS_EMPTY(_0, _1, _2, _3) \
    __JUSTARGS_HAS_COMMA(__JUSTARGS_PASTE5(__JUSTARGS_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define __JUSTARGS_TUPLE_IS_EMPTY(...) \
    __JUSTARGS_IS_EMPTY( \
        __JUSTARGS_HAS_COMMA(__VA_ARGS__), \
        __JUSTARGS_HAS_COMMA(__JUSTARGS_TRIGGER_PARENTHESIS_ __VA_ARGS__), \
        __JUSTARGS_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
        __JUSTARGS_HAS_COMMA(__JUSTARGS_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)) \
    )

/*************************************************************************************************/

#define JUSTARGS_OPTION_DECLARE(OPT_NAME, OPT_TYPE, ...) \
    struct __JUSTARGS_CAT(OPT_NAME, _t): ::justargs::option_base { \
        using value_type       = OPT_TYPE; \
        using optional_type    = ::justargs::optional_type<value_type>; \
        using value_changed_cb = std::function<void(const optional_type &)>; \
        using category         = __JUSTARGS_IF( \
             __JUSTARGS_TUPLE_IS_EMPTY(__VA_ARGS__) \
            ,::justargs::required \
            ,::justargs::optional \
        ); \
        \
        __JUSTARGS_CAT(OPT_NAME, _t)(const __JUSTARGS_CAT(OPT_NAME, _t) &) = default; \
        __JUSTARGS_CAT(OPT_NAME, _t)& operator= (const __JUSTARGS_CAT(OPT_NAME, _t) &) = default; \
        __JUSTARGS_CAT(OPT_NAME, _t)(__JUSTARGS_CAT(OPT_NAME, _t) &&) = default; \
        __JUSTARGS_CAT(OPT_NAME, _t)& operator= (__JUSTARGS_CAT(OPT_NAME, _t) &&) = default; \
        \
        __JUSTARGS_CAT(OPT_NAME, _t)(kwords_group *p, const char *name, const char *type, const char *descr) \
            :option_base{name, type, descr} \
            ,m_group{p} \
            ,m_val{} \
            ,m_cb{} \
        { m_group->add(this); } \
        __JUSTARGS_CAT(OPT_NAME, _t)(kwords_group *p, const char *name, const char *type, const char *descr, optional_type v, value_changed_cb c) \
            :option_base{name, type, descr} \
            ,m_group{p} \
            ,m_val{std::move(v)} \
            ,m_cb{std::move(c)} \
        { m_group->add(this); } \
        \
        kwords_group *m_group; \
        optional_type m_val; \
        value_changed_cb m_cb; \
        \
        template<typename T> \
        __JUSTARGS_CAT(OPT_NAME, _t) operator= (T &&v) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{std::forward<T>(v)} \
                ,value_changed_cb{} \
            }; \
            \
            return res; \
        } \
        \
        bool is_required() const override { return std::is_same<category, justargs::required>::value; } \
        bool is_optional() const override { return !is_required(); } \
        bool is_set() const override { return static_cast<bool>(m_val); } \
        bool is_bool() const override { return std::is_same<value_type, bool>::value; } \
        \
        __JUSTARGS_CAT(OPT_NAME, _t) bind(value_type *var) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{} \
                ,[var](const optional_type &v){ *var = v.value(); } \
            }; \
            \
            return res; \
        } \
        template<typename Obj> \
        __JUSTARGS_CAT(OPT_NAME, _t) bind(Obj *o, void(Obj::*m)(const value_type &)) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{} \
                ,[o, m](const optional_type &v){ (o->*m)(v.value()); } \
            }; \
            \
            return res; \
        } \
        template<typename Obj> \
        __JUSTARGS_CAT(OPT_NAME, _t) bind(Obj *o, void(Obj::*m)(value_type &)) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{} \
                ,[o, m](const optional_type &v){ (o->*m)(v.value()); } \
            }; \
            \
            return res; \
        } \
        template<typename T, typename Obj> \
        __JUSTARGS_CAT(OPT_NAME, _t) bind(Obj *o, T Obj::*m) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{} \
                ,[o, m](const optional_type &v){ o->*m = v.value(); } \
            }; \
            \
            return res; \
        } \
        template<typename F, typename = typename std::enable_if_t<::justargs::details::is_callable<F>::value>> \
        __JUSTARGS_CAT(OPT_NAME, _t) bind(F &&f) const { \
            __JUSTARGS_CAT(OPT_NAME, _t) res{ \
                 m_group \
                ,name() \
                ,type() \
                ,description() \
                ,optional_type{} \
                ,[f=std::forward<F>(f)](const optional_type &v){ f(v.value()); } \
            }; \
            \
            return res; \
        } \
        \
        void set_cb(value_changed_cb c) { m_cb = std::move(c); } \
        void call_cb() const { if ( m_cb ) m_cb(m_val); } \
        \
        void from_string(const char *ptr, std::size_t len) { \
            value_type v{}; \
            ::justargs::details::from_string_impl(std::addressof(v), ptr, len); \
            m_val = std::move(v); \
        } \
        std::ostream& show_this(std::ostream &os) const { os << this; return os; } \
    };

#define JUSTARGS_OPTION(OPT_NAME, OPT_TYPE, DESCRIPTION, ...) \
    JUSTARGS_OPTION_DECLARE(OPT_NAME, OPT_TYPE, __VA_ARGS__) \
    const __JUSTARGS_CAT(OPT_NAME, _t) OPT_NAME{ \
         this \
        ,__JUSTARGS_STRINGIZE(OPT_NAME) \
        ,__JUSTARGS_STRINGIZE(OPT_TYPE) \
        ,DESCRIPTION \
    };

#define JUSTARGS_OPTION_HELP() \
    JUSTARGS_OPTION(help, bool, "show help message", optional)

#define JUSTARGS_OPTION_VERSION() \
    JUSTARGS_OPTION(version, bool, "show version message", optional)

/*************************************************************************************************/

struct required;
struct optional;

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

private:
    const char *m_name;
    const char *m_type;
    const char *m_descr;
};

struct kwords_group_base {};

template<typename D>
struct kwords_group: kwords_group_base {
    using options_map_type = std::map<
         std::string
        ,const option_base *
    >;
    using options_group_type = std::map<
         std::type_index
        ,options_map_type
    >;

    static options_map_type& options() {
        const std::type_index idx{typeid(D)};
        static options_group_type map;
        return map[idx];
    }

    void add(const option_base *s) {
        options().emplace(s->name(), s);
    }

    template<typename T>
    bool is_required() const {
        const auto it = options().find(T::__name());
        if ( it == options().end() ) return false;
        return it->second.is_required();
    }

    std::ostream& show_help(std::ostream &os, const char *argv0) const {
        const char *p = std::strrchr(argv0, '/');
        os
        << (p ? p+1 : "program") << ":" << std::endl;

        std::size_t max_len = 0;
        for ( const auto &it: options() ) {
            std::size_t len = it.first.length();
            max_len = (len > max_len) ? len : max_len;
        }

        for ( const auto &it: options() ) {
            static const char filler[] = "                                        ";
            const auto &name = it.first;
            std::size_t len = name.length();
            os << "--" << name << "=*";
            os.write(filler, static_cast<std::streamsize>(max_len - len));
            os
                << ": " << it.second->description()
                << " (" << it.second->type() << ", "
                << (it.second->is_required() ? "required" : "optional") << ")"
            << std::endl;
        }

        return os;
    }
};

/*************************************************************************************************/

namespace details {

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

template<typename ...Fs>
struct overloaded_set;

template<typename F1, typename ...Fs>
struct overloaded_set<F1, Fs...>: F1, overloaded_set<Fs...>::type {
    using type = overloaded_set;

    overloaded_set(F1 &&head, Fs &&...tail)
        :F1{std::forward<F1>(head)}
        ,overloaded_set<Fs...>::type{std::forward<Fs>(tail)...}
    {}

    using F1::operator();
    using overloaded_set<Fs...>::type::operator();
};

template<typename F>
struct overloaded_set<F> : F {
    using type = F;
    using F::operator();
};

template<typename ...Fs>
auto make_overloaded(Fs &&...fs) {
    return overloaded_set<Fs...>{std::forward<Fs>(fs)...};
}

// based on https://stackoverflow.com/questions/55941964
template <typename, typename>
struct contains;

template <typename Car, typename... Cdr, typename Needle>
struct contains<std::tuple<Car, Cdr...>, Needle>: contains<std::tuple<Cdr...>, Needle>
{};

template <typename... Cdr, typename Needle>
struct contains<std::tuple<Needle, Cdr...>, Needle>: std::true_type
{};

template <typename Needle>
struct contains<std::tuple<>, Needle>: std::false_type
{};

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

#ifndef __JUSTARGS_TO_TUPLE_MAX
#define __JUSTARGS_TO_TUPLE_MAX 20
#endif

template<typename T>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, 0>) {
    return std::make_tuple();
}

#define __JUSTARGS_REPEAT_0(macro, data)
#define __JUSTARGS_REPEAT_1(macro, data) \
    macro(0, data)
#define __JUSTARGS_REPEAT_2(macro, data) \
    __JUSTARGS_REPEAT_1(macro, data) macro(1, data)
#define __JUSTARGS_REPEAT_3(macro, data) \
    __JUSTARGS_REPEAT_2(macro, data) macro(2, data)
#define __JUSTARGS_REPEAT_4(macro, data) \
    __JUSTARGS_REPEAT_3(macro, data) macro(3, data)
#define __JUSTARGS_REPEAT_5(macro, data) \
    __JUSTARGS_REPEAT_4(macro, data) macro(4, data)
#define __JUSTARGS_REPEAT_6(macro, data) \
    __JUSTARGS_REPEAT_5(macro, data) macro(5, data)
#define __JUSTARGS_REPEAT_7(macro, data) \
    __JUSTARGS_REPEAT_6(macro, data) macro(6, data)
#define __JUSTARGS_REPEAT_8(macro, data) \
    __JUSTARGS_REPEAT_7(macro, data) macro(7, data)
#define __JUSTARGS_REPEAT_9(macro, data) \
    __JUSTARGS_REPEAT_8(macro, data) macro(8, data)
#define __JUSTARGS_REPEAT_10(macro, data) \
    __JUSTARGS_REPEAT_9(macro, data) macro(9, data)
#define __JUSTARGS_REPEAT_11(macro, data) \
    __JUSTARGS_REPEAT_10(macro, data) macro(10, data)
#define __JUSTARGS_REPEAT_12(macro, data) \
    __JUSTARGS_REPEAT_11(macro, data) macro(11, data)
#define __JUSTARGS_REPEAT_13(macro, data) \
    __JUSTARGS_REPEAT_12(macro, data) macro(12, data)
#define __JUSTARGS_REPEAT_14(macro, data) \
    __JUSTARGS_REPEAT_13(macro, data) macro(13, data)
#define __JUSTARGS_REPEAT_15(macro, data) \
    __JUSTARGS_REPEAT_14(macro, data) macro(14, data)
#define __JUSTARGS_REPEAT_16(macro, data) \
    __JUSTARGS_REPEAT_15(macro, data) macro(15, data)
#define __JUSTARGS_REPEAT_17(macro, data) \
    __JUSTARGS_REPEAT_16(macro, data) macro(16, data)
#define __JUSTARGS_REPEAT_18(macro, data) \
    __JUSTARGS_REPEAT_17(macro, data) macro(17, data)
#define __JUSTARGS_REPEAT_19(macro, data) \
    __JUSTARGS_REPEAT_18(macro, data) macro(18, data)
#define __JUSTARGS_REPEAT_20(macro, data) \
    __JUSTARGS_REPEAT_19(macro, data) macro(19, data)

#define __JUSTARGS_REPEAT_IMPL(start_macro, macro, data) \
    start_macro(macro, data)
#define __JUSTARGS_REPEAT(n, macro, data) \
    __JUSTARGS_REPEAT_IMPL(__JUSTARGS_CAT(__JUSTARGS_REPEAT_, n), macro, data)

#define __JUSTARGS_TO_TUPLE_P(idx, data) , p##idx
#define __JUSTARGS_TO_TUPLE_SPECIALIZATION(idx, data) \
    template<typename T> \
    auto to_tuple_impl(const T &object, std::integral_constant<std::size_t, idx + 1>) { \
        const auto& [p __JUSTARGS_REPEAT_##idx(__JUSTARGS_TO_TUPLE_P, data)] = object; \
        return std::make_tuple(p __JUSTARGS_REPEAT_##idx(__JUSTARGS_TO_TUPLE_P, data)); \
    }

__JUSTARGS_REPEAT(__JUSTARGS_TO_TUPLE_MAX, __JUSTARGS_TO_TUPLE_SPECIALIZATION, ~)

#undef __JUSTARGS_TO_TUPLE_SPECIALIZATION
#undef __JUSTARGS_TO_TUPLE_P

template<
     typename T
    ,typename = struct current_value
    ,std::size_t = __JUSTARGS_TO_TUPLE_MAX
    ,typename = struct required_value
    ,std::size_t N
>
auto to_tuple_impl(const T &, std::integral_constant<std::size_t, N>) {
    static_assert(N <= __JUSTARGS_TO_TUPLE_MAX, "Please increase __JUSTARGS_TO_TUPLE_MAX");
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
        static_assert (has_type<T>::value, "");

        return std::get<T>(m_kwords).is_set();
    }

    bool is_valid_name(const char *name) const {
        return check_for_unexpected(name) == nullptr;
    }
    bool is_bool_type(const char *name) const {
        bool res{};

        for_each(
            [name, &res](const auto &t, const auto &) {
                if ( 0 == std::strcmp(t.name(), name) ) {
                    res = t.is_bool();
                }
            }
            ,false
        );

        return res;
    }

    template<typename T, typename R, typename Obj>
    args& bind(Obj *obj, R(Obj::*m)(const typename T::value_type &)) {
        std::get<T>(m_kwords).set_cb(
            [obj, m](const typename T::optional_type &v) {
                (obj->*m)(v.value());
            }
        );

        return *this;
    }
    template<typename T>
    args& bind(typename T::value_type *r) {
        std::get<T>(m_kwords).set_cb(
            [r](const typename T::optional_type &v) {
                *r = v.value();
            }
        );

        return *this;
    }
    template<typename T>
    args& bind(const T &, typename T::value_type *r) {
        return bind<T>(r);
    }

    template<typename T, typename VT>
    void set(const T &, VT &&v) {
        auto &item = std::get<T>(m_kwords);
        item.m_val = std::forward<VT>(v);
        item.call_cb();
    }

    template<typename T>
    const typename T::value_type& get(const T &) const {
        static_assert (has_type<T>::value, "");

        return std::get<T>(m_kwords).m_val.value();
    }
    template<typename T>
    typename T::value_type& get(const T &) {
        static_assert (has_type<T>::value, "");

        return std::get<T>(m_kwords).m_val.value();
    }
    template<typename T, typename D>
    typename T::value_type get(const T &v, D &&def) const {
        if ( has(v) ) {
            return std::get<T>(m_kwords).m_val.value();
        }

        return def;
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
    template<typename ...Fs>
    void for_each(bool inited_only, Fs &&...fs) const {
        auto overloaded = details::make_overloaded(std::forward<Fs>(fs)...);
        for_each(m_kwords, overloaded, inited_only);
    }
    template<typename ...Fs>
    void for_each(bool inited_only, Fs &&...fs) {
        auto overloaded = details::make_overloaded(std::forward<Fs>(fs)...);
        for_each(m_kwords, overloaded, inited_only);
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
            [&os](const auto &t, const auto &){ os << "  " << t.name() << ": this="; t.show_this(os) << std::endl; }
        );
    }

private:
    template<typename Iter, typename ...TArgs>
    friend auto parse_kv_list(
         bool *ok
        ,std::string *emsg
        ,const char *pref
        ,std::size_t pref_len
        ,Iter beg
        ,Iter end
        ,args<TArgs...> &set
    );

    const char* check_for_unexpected(const char *optname) const {
        const char *ptr = nullptr;
        for_each(
             m_kwords
            ,[&ptr, optname](const auto &t, const auto &) {
                if ( std::strcmp(t.name(), optname) == 0 ) {
                    ptr = t.name();
                }
            }
            ,false
        );

        return ptr ? nullptr : optname;
    }
    const char* check_for_required() const {
        const char *name = nullptr;
        for_each(
             m_kwords
            ,[&name](const auto &t, const auto &){
                if ( t.is_required() && !t.is_set() ) {
                    name = t.name();
                }
             }
            ,false
        );

        return name;
    }

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

    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I != std::tuple_size<Tuple>::value>
    for_each(const Tuple &tuple, Func &&func, bool inited_only) {
        const auto &val = std::get<I>(tuple);
        if ( inited_only ) {
            if ( val.m_val ) {
                func(val, val.m_val);
            }
        } else {
            func(val, val.m_val);
        }

        for_each<I + 1>(tuple, std::forward<Func>(func), inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I == std::tuple_size<Tuple>::value>
    for_each(const Tuple &, Func &&, bool) {}

    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I != std::tuple_size<Tuple>::value>
    for_each(Tuple &tuple, Func &&func, bool inited_only) {
        auto &val = std::get<I>(tuple);
        if ( inited_only ) {
            if ( val.m_val ) {
                func(val, val.m_val);
            }
        } else {
            func(val, val.m_val);
        }

        for_each<I + 1>(tuple, std::forward<Func>(func), inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if_t<I == std::tuple_size<Tuple>::value>
    for_each(Tuple &, Func &&, bool) {}
};

/*************************************************************************************************/

template<typename Iter, typename ...Args>
auto parse_kv_list(
     bool *ok
    ,std::string *emsg
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

        details::trim(line);

        auto pos = line.find('=');
        if ( pos != std::string::npos ) {
            line[pos] = '\0';
        }

        const char *key = line.c_str();
        if ( const char *unexpected = set.check_for_unexpected(key) ) {
            std::string msg = "there is extra \"--";
            msg += unexpected;
            msg += "\" option was specified";

            if ( ok ) {
                *ok = false;
                if ( emsg ) {
                    *emsg = std::move(msg);
                }
            } else {
                throw std::invalid_argument(msg);
            }

            return set;
        }

        if ( pos != std::string::npos ) {
            const char *val = line.c_str() + pos + 1;
            std::size_t len = (line.length() - pos) - 1;
            set.for_each(
                [key, val, len](auto &t, const auto &) {
                    if ( std::strcmp(t.name(), key) == 0 ) {
                        t.from_string(val, len);
                        t.call_cb();
                    }
                }
                ,false
            );
        } else {
            if ( !set.is_bool_type(key) ) {
                std::string msg = "a value must be provided for \"--";
                msg += key;
                msg += "\" option";

                if ( ok ) {
                    *ok = false;
                    if ( emsg ) {
                        *emsg = std::move(msg);
                    }
                } else {
                    throw std::invalid_argument(msg);
                }

                return set;
            }

            set.for_each(
                [key](auto &t, const auto &) {
                    if ( std::strcmp(t.name(), key) == 0 ) {
                        static const char _true[] = "true";
                        t.from_string(_true, sizeof(_true)-1);
                        t.call_cb();
                    }
                }
                ,false
            );

            if ( std::strcmp(key, "help") == 0 || std::strcmp(key, "version") == 0 ) {
                if ( ok ) {
                    *ok = true;
                }

                return set;
            }
        }
    }

    const char *required = set.check_for_required();
    if ( required ) {
        std::string msg = "there is no required \"--";
        msg += required;
        msg += "\" option was specified";

        if ( ok ) {
            *ok = false;
            if ( emsg ) {
                *emsg = std::move(msg);
            }
        } else {
            throw std::invalid_argument(msg);
        }
    } else {
        if ( ok ) {
            *ok = true;
        }
    }

    return set;
}

/*************************************************************************************************/

template<typename ...Args>
auto make_args(Args && ...args) {
    justargs::args<typename std::decay<Args>::type...> set{std::forward<Args>(args)...};

    return set;
}

/*************************************************************************************************/

template<
     typename ...Args
    ,typename = typename std::enable_if<
        sizeof...(Args) != 1 && !std::is_base_of<
             kwords_group_base
            ,typename std::tuple_element<0, std::tuple<Args...>>::type
        >::value
    >::type
>
auto parse_args(bool *ok, std::string *emsg, int argc, char* const* argv, Args && ...kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args<typename std::decay<Args>::type...> set{std::forward<Args>(kwords)...};

    return parse_kv_list(ok, emsg, "--", 2, beg, end, set);
}

template<typename ...Args>
auto parse_args(bool *ok, std::string *emsg, int argc, char* const* argv, const std::tuple<Args...> &kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args<typename std::decay<Args>::type...> set{std::get<Args>(kwords)...};

    return parse_kv_list(ok, emsg, "--", 2, beg, end, set);
}

template<
     typename KWords
    ,typename = typename std::enable_if<
        std::is_class<KWords>::value &&
        std::is_base_of<kwords_group_base, KWords>::value
    >::type
>
auto parse_args(bool *ok, std::string *emsg, int argc, char* const* argv, const KWords &kw) {
    const auto &tuple = details::to_tuple(kw);

    return parse_args(ok, emsg, argc, argv, tuple);
}

namespace {

struct proxy_printer {
    static void print(std::ostream &os, const optional_type<bool> &v) { os << (v.value() ? "true" : "false"); }
    template<typename T>
    static void print(std::ostream &os, const optional_type<T> &v) { os << v.value(); }
};

} // anon ns

template<typename ...Args>
std::ostream& to_file(std::ostream &os, const args<Args...> &set, bool inited_only = true) {
    set.for_each(
        [&os](auto &t, const auto &opt) {
            os << "# " << t.description() << std::endl;
            os << t.name() << "=";
            if ( opt ) {
                proxy_printer::print(os, opt);
            }
            os << std::endl;
        }
        ,inited_only
    );

    return os;
}

template<typename ...Args>
auto from_file(bool *ok, std::string *emsg, std::istream &is, args<Args...> &set) {
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

    return parse_kv_list(ok, emsg, nullptr, 0, linesptrs.begin(), linesptrs.end(), set);
}

template<typename ...Args>
auto from_file(bool *ok, std::string *emsg, std::istream &is, Args && ...kwords) {
    args<typename std::decay<Args>::type...> set{std::forward<Args>(kwords)...};

    return from_file(ok, emsg, is, set);
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
        [&max_len](const auto &t, const auto &) {
            std::size_t len = std::strlen(t.name());
            max_len = (len > max_len) ? len : max_len;
        }
        ,false
    );

    set.for_each(
        [&os, max_len](const auto &t, const auto &) {
            static const char ident[] = "                                        ";
            const char *name = t.name();
            std::size_t len = std::strlen(name);
            os << "--" << name << "=*";
            os.write(ident, static_cast<std::streamsize>(max_len-len));
            os
            << ": " << t.description()
            << " ("
                << t.type()
                << ", "
                << (t.is_required() ? "required" : "optional")
            << ")"
            << std::endl;
        }
        ,false
    );

    return os;
}

/*************************************************************************************************/

} // ns justargs

/*************************************************************************************************/

#endif // __JUSTARGS__JUSTARGS_HPP
