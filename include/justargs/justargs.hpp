
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021 niXman (github dot nixman at pm dot me)
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

// TODO: comment out
//#include <iostream>
#include <ostream>
#include <istream>
#include <sstream>
#include <vector>
#include <tuple>
#include <array>
#include <type_traits>
#include <functional>
#include <string>
#include <stdexcept>

#include <cstring>
#include <cinttypes>

/*************************************************************************************************/

namespace justargs {
namespace details {

#if __cplusplus < 201703L

#define EXPAND_ON_CPP17(...)

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

    void reset() {
        if ( m_inited ) {
            m_val.T::~T();
            m_inited = false;
        }
    }

    T m_val;
    bool m_inited;
};

#else

#include <optional>

#define EXPAND_ON_CPP17(...) \
    __VA_ARGS__

template<typename T>
using optional_type = std::optional<T>;

#endif // __cplusplus < 201703L

inline void ltrim(std::string &s, const char* t = " \t\n\r") {
    s.erase(0, s.find_first_not_of(t));
}

inline void rtrim(std::string &s, const char* t = " \t\n\r") {
    s.erase(s.find_last_not_of(t) + 1);
}

inline void trim(std::string &s, const char* t = " \t\n\r") {
    rtrim(s, t);
    ltrim(s, t);
}

// is callable
template<typename...>
using void_t = void;

template<typename F>
using has_operator_call_t = decltype(&F::operator());

template<typename F, typename = void>
struct is_callable: std::false_type
{};

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

} // ns details

enum obligatoriness {
     optional
    ,required
};

/*************************************************************************************************/

struct option_base {
    constexpr option_base(const char *name, const char *type_name, const char *description, bool is_required, option_base *next)
        :m_name{name}
        ,m_type_name{type_name}
        ,m_description{description}
        ,m_is_required{is_required}
        ,m_next{next}
    {}

    constexpr std::size_t id() const { return fnv1a(m_name); }

    const char *m_name;
    const char *m_type_name;
    const char *m_description;
    const bool  m_is_required;
    option_base *m_next;

    std::ostream& show_help(std::ostream &os, std::size_t max_len) const {
        static const char ident[] = "                                        ";
        os << m_name;
        if ( m_is_required ) {
            os << "=*";
        } else {
            os << "  ";
        }

        std::size_t name_len = std::strlen(m_name);
        os.write(ident, static_cast<std::streamsize>(max_len - name_len));
        os << ": " << m_description << " (" << m_type_name << ", " << (m_is_required ? "required" : "optional") << ")";
        os << std::endl;

        return os;
    }

    virtual bool is_set() const = 0;
    virtual bool is_bool() const = 0;
    virtual bool has_default() const = 0;

private:
    static constexpr std::uint32_t fnv1a(const char *s, std::size_t len) {
        std::uint32_t seed = 0x811c9dc5;
        for ( ; len; --len, ++s ) {
            seed = static_cast<std::uint32_t>(
                (seed ^ static_cast<std::uint32_t>(*s)) * static_cast<std::uint64_t>(0x01000193)
            );
        }

        return seed;
    }
    static constexpr std::uint32_t fnv1a(const char *s) {
        return fnv1a(s, ct_strlen(s));
    }

    static constexpr std::size_t ct_strlen(const char *s) {
        const char *p = s;
        for ( ; *p; ++p )
            ;
        return p - s;
    }

    template<std::size_t N>
    static constexpr auto ct_init_array(const char *s, char c0, char c1) {
        std::array<char, N> res{};
        for ( auto i = 0u; *s; ++s, ++i ) {
            res[i] = *s;
        }
        res[1] = c0;
        res[2] = c1;

        return res;
    }

protected:
    template<typename T>
    static typename std::enable_if<std::is_same<T, std::string>::value>::type
    from_string_impl(T *val, const char *ptr, std::size_t len) {
        val->assign(ptr, len);
    }
    EXPAND_ON_CPP17(
        template<typename T>
        static typename std::enable_if<std::is_same<T, std::string_view>::value>::type
        from_string_impl(T *val, const char *ptr, std::size_t len) {
            *val = std::string_view{ptr, len};
        }
    )
    template<typename T>
    static typename std::enable_if<std::is_same<T, bool>::value>::type
    from_string_impl(T *val, const char *ptr, std::size_t len) {
        *val = std::strncmp(ptr, "true", len) == 0;
    }
    template<typename T>
    static typename std::enable_if<(std::is_integral<T>::value && !std::is_same<T, bool>::value)>::type
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
    static typename std::enable_if<std::is_floating_point<T>::value>::type
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
    static typename std::enable_if<std::is_pointer<T>::value>::type
    from_string_impl(T &val, const char *, std::size_t) {
        val = nullptr;
    }

};

struct options_group {
    options_group()
        :m_root{}
        ,m_end{}
    {}

    void add(option_base *p) {
        if ( !p ) {
            return;
        }

        if ( !m_root ) {
            m_root = m_end = p;
        } else {
            m_end->m_next = p;
            m_end = p;
        }
    }

    std::ostream& show_help(std::ostream &os) const {
        std::size_t max_len = 0;
        for ( const auto *it = m_root; it; it = it->m_next ) {
            std::size_t len = std::strlen(it->m_name);
            max_len = (len > max_len ? len : max_len);
        }
        for ( const auto *it = m_root; it; it = it->m_next ) {
            it->show_help(os, max_len);
        }

        return os;
    }

    option_base *m_root;
    option_base *m_end;
};


template<typename T, typename Derived>
struct option: option_base {
    using value_type          = T;
    using optional_type       = details::optional_type<value_type>;
    using validator_cb_type   = std::function<bool(const char *, std::size_t)>;
    using notificator_cb_type = std::function<void(const optional_type &)>;

    option(
         options_group *group
        ,const char *name
        ,const char *type_name
        ,const char *description
        ,obligatoriness req = obligatoriness::required
        ,validator_cb_type vcb = {}
        ,notificator_cb_type scb = {}
    )
        :option_base{name, type_name, description, req == obligatoriness::required, nullptr}
        ,m_validator_cb{std::move(vcb)}
        ,m_notificator_cb{std::move(scb)}
        ,m_value{}
        ,m_default{}
    { if ( group ) group->add(this); }

    option(const option &) = default;
    option& operator= (const option &) = default;
    option(option &&) noexcept = default;
    option& operator= (option &&) noexcept = default;

    void from_string(const char *ptr, std::size_t len) {
        value_type v{};
        from_string_impl(&v, ptr, len);
        m_value = std::move(v);
    }

    void notificate() { if ( m_notificator_cb ) m_notificator_cb(m_value); }

    bool is_set() const override { return static_cast<bool>(m_value); }
    bool is_bool() const override { return std::is_same<T, bool>::value; }
    bool has_default() const override { return static_cast<bool>(m_default); }

    value_type&       get()       { return m_value.value(); }
    const value_type& get() const { return m_value.value(); }

    void reset() {
        m_validator_cb = nullptr;
        m_notificator_cb = nullptr;
        m_default.reset();
        m_value.reset();
    }

    template<typename U>
    Derived operator= (U &&u) const noexcept {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = m_notificator_cb;
        res.m_default = m_default;
        res.m_value = optional_type{std::forward<U>(u)};

        return res;
    }
    Derived bind(value_type *vptr) const {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = [vptr](const optional_type &v){ *vptr = v.value(); };
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }
    template<typename Obj>
    Derived bind(Obj *o, void(Obj::*m)(const value_type &)) const {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = [o, m](const optional_type &v){ (o->*m)(v.value()); };
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }
    template<typename Obj>
    Derived bind(Obj *o, void(Obj::*m)(value_type &)) const {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = [o, m](const optional_type &v){ (o->*m)(v.value()); };
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }
    template<typename U, typename Obj>
    Derived bind(Obj *o, U Obj::*m) const {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = [o, m](const optional_type &v){ o->*m = v.value(); };
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }
    template<typename F, typename = typename std::enable_if<details::is_callable<F>::value>::type>
    Derived bind(F &&f) const {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = [f=std::forward<F>(f)](const optional_type &v){ f(v.value()); };
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }

    template<typename U>
    Derived default_(U &&u) const noexcept {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = m_validator_cb;
        res.m_notificator_cb = m_notificator_cb;
        res.m_default = optional_type{std::forward<U>(u)};
        res.m_value = m_value;

        return res;
    }
    template<typename F, typename = typename std::enable_if<details::is_callable<F>::value>::type>
    Derived validate(F &&f) const noexcept {
        Derived res{
             nullptr
            ,m_name
            ,m_type_name
            ,m_description
            ,m_is_required ? obligatoriness::required : obligatoriness::optional
        };
        res.m_validator_cb = std::forward<F>(f);
        res.m_notificator_cb = m_notificator_cb;
        res.m_default = m_default;
        res.m_value = m_value;

        return res;
    }

    validator_cb_type m_validator_cb;
    notificator_cb_type m_notificator_cb;
    optional_type m_value;
    optional_type m_default;
};

#define JUSTARGS_OPTION(name, type, description, ...) \
    struct name##_t: public ::justargs::option<type, name##_t> { \
        using option::option; \
        template<typename T> \
        name##_t operator= (T &&v) const { \
            return option::operator= (std::forward<T>(v)); \
        } \
    }; \
    const name##_t name{this, #name, #type, description, __VA_ARGS__}

#define JUSTARGS_OPTION_HELP() \
    JUSTARGS_OPTION(help, bool, "show help message", ::justargs::optional)

#define JUSTARGS_OPTION_VERSION() \
    JUSTARGS_OPTION(version, bool, "show version message", ::justargs::optional)

/*************************************************************************************************/

namespace details {

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

} // ns details

/*************************************************************************************************/

template<typename ...Args>
struct args {
    using container_type = std::tuple<typename std::decay<Args>::type...>;
    static_assert(
         std::tuple_size<container_type>::value == std::tuple_size<details::without_duplicates<container_type>>::value
        ,"duplicates of keywords are identified!"
    );

    container_type m_kwords;

    template<typename ...Types>
    explicit args(const Types &...types)
        :m_kwords(types...)
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

    static constexpr std::size_t size() { return std::tuple_size<container_type>::value; }

    template<typename T>
    static constexpr bool has(const T &) { return has_type<T>::value; }
    template<typename T>
    static constexpr bool has() { return has_type<T>::value; }

    template<typename T>
    bool is_set(const T &) const {
        static_assert(has<T>(), "");

        return std::get<T>(m_kwords).is_set();
    }

    bool is_valid_name(const char *name) const {
        return check_for_unexpected(name) == nullptr;
    }
    bool is_bool_type(const char *name) const {
        bool res{};

        for_each(
            [name, &res](const auto &t, const auto &) {
                if ( 0 == std::strcmp(t.m_name, name) ) { res = t.is_bool(); }
            }
            ,false
        );

        return res;
    }

    template<typename T, typename VT>
    void set(const T &, VT &&v) {
        auto &item = std::get<T>(m_kwords);
        item.m_value = std::forward<VT>(v);
    }

    template<typename T>
    const typename T::value_type& get(const T &) const {
        static_assert(has_type<T>::value, "");

        return std::get<T>(m_kwords).m_value.value();
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

    const char* check_for_unexpected(const char *optname) const {
        const char *ptr = nullptr;
        for_each(
             m_kwords
            ,[&ptr, optname](const auto &t, const auto &) {
                if ( std::strcmp(t.m_name, optname) == 0 ) { ptr = t.m_name; }
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
                if ( t.m_is_required && !t.is_set() ) {
                    name = t.m_name;
                }
             }
            ,false
        );

        return name;
    }

private:
    // for debug only
    void show_this(std::ostream &os) const {
        for_each(
            [&os](const auto &t, const auto &){ os << "  " << t.name() << ": this="; t.show_this(os) << std::endl; }
        );
    }

    void reset_impl() {}
    template<typename T0, typename ...Types>
    void reset_impl(const T0 &, const Types & ...types) {
        std::get<T0>(m_kwords).reset();
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
    static typename std::enable_if<I != std::tuple_size<Tuple>::value>::type
    for_each(const Tuple &tuple, Func &&func, bool inited_only) {
        const auto &val = std::get<I>(tuple);
        if ( inited_only ) {
            if ( val.m_value ) {
                func(val, val.m_value);
            }
        } else {
            func(val, val.m_value);
        }

        for_each<I + 1>(tuple, std::forward<Func>(func), inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if<I == std::tuple_size<Tuple>::value>::type
    for_each(const Tuple &, Func &&, bool) {}

    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if<I != std::tuple_size<Tuple>::value>::type
    for_each(Tuple &tuple, Func &&func, bool inited_only) {
        auto &val = std::get<I>(tuple);
        if ( inited_only ) {
            if ( val.m_value ) {
                func(val, val.m_value);
            }
        } else {
            func(val, val.m_value);
        }

        for_each<I + 1>(tuple, std::forward<Func>(func), inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if<I == std::tuple_size<Tuple>::value>::type
    for_each(Tuple &, Func &&, bool) {}
};

/*************************************************************************************************/

template<typename Iter, typename ...Args>
auto parse_kv_list(bool *ok, std::string *emsg, const char *pref, std::size_t pref_len, Iter beg, Iter end, args<Args...> &set) {
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
            std::size_t len = line.length() - pos - 1;
            std::string msg;
            set.for_each(
                 [key, val, len, &msg](auto &t, const auto &) {
                    if ( std::strcmp(t.m_name, key) == 0 ) {
                        if ( t.m_validator_cb ) {
                            if ( !t.m_validator_cb(val, len) ) {
                                msg = "wrong validation for \"";
                                msg += t.m_name;
                                msg += "\" option";
                            } else {
                                t.from_string(val, len);
                                t.notificate();
                            }
                        }
                    }
                 }
                ,false
            );
            if ( !msg.empty() ) {
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
                    if ( std::strcmp(t.m_name, key) == 0 ) {
                        static const char _true[] = "true";
                        t.from_string(_true, sizeof(_true)-1);
                        t.notificate();
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

    // use default value if not inited
    set.for_each(
         [](const auto &t, auto &v) {
            if ( !t.is_set() && t.has_default() ) {
                v = t.m_default;
            }
         }
        ,false
    );

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

template<typename ...Args>
auto parse_args(bool *ok, std::string *emsg, int argc, char* const* argv, Args && ...kwords) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    args<typename std::decay<Args>::type...> set{std::forward<Args>(kwords)...};

    return parse_kv_list(ok, emsg, "--", 2, beg, end, set);
}

namespace {

struct proxy_printer {
    static void print(std::ostream &os, const details::optional_type<bool> &v) { os << (v.value() ? "true" : "false"); }
    template<typename T>
    static void print(std::ostream &os, const details::optional_type<T> &v) { os << v.value(); }
};

} // anon ns

template<typename ...Args>
std::ostream& to_file(std::ostream &os, const args<Args...> &set, bool inited_only = true) {
    set.for_each(
         [&os](auto &t, const auto &opt) {
            os << "# " << t.m_description << std::endl;
            os << t.m_name << "=";
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
            std::size_t len = std::strlen(t.m_name);
            max_len = (len > max_len) ? len : max_len;
         }
        ,false
    );

    set.for_each(
         [&os, max_len](const auto &t, const auto &) {
            os << "  ";
            t.show_help(os, max_len);
         }
        ,false
    );
    os << std::endl;

    return os;
}

/*************************************************************************************************/

} // ns justargs

/*************************************************************************************************/

#endif // __JUSTARGS__JUSTARGS_HPP
