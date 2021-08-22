
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021 niXman (github dot nixman at pm dot me)
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

// #include <iostream> // TODO: comment out
#include <ostream>
#include <istream>
#include <sstream>
#include <map>
#include <vector>
#include <tuple>
#include <array>
#include <type_traits>
#include <functional>
#include <string>
#include <stdexcept>

#include <cassert>
#include <cstring>
#include <cinttypes>

/*************************************************************************************************/

#if __cplusplus < 201703L

namespace justargs {

template<typename T>
struct optional_type {
    optional_type() = default;

    template<typename V>
    explicit optional_type(V &&v)
        :m_val{v}
        ,m_inited{true}
    {}

    template<typename V>
    explicit optional_type(V &v)
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

    T m_val;
    bool m_inited = false;
};

} // ns justargs

#else

#include <optional>

namespace justargs {

template<typename T>
using optional_type = std::optional<T>;

} // ns justargs

#endif // __cplusplus < 201703L

/*************************************************************************************************/

namespace justargs {

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

#define JUSTARGS_ADD_KEYWORD(SETNAME, SETTYPE, DESCRIPTION, ...) \
    struct __JUSTARGS_CAT(SETNAME, _t): ::justargs::option_base { \
        using value_type       = SETTYPE; \
        using optional_type    = ::justargs::optional_type<value_type>; \
        using value_changed_cb = std::function<void(const optional_type &)>; \
        using category         = __JUSTARGS_IF( \
             __JUSTARGS_TUPLE_IS_EMPTY(__VA_ARGS__) \
            ,::justargs::required \
            ,::justargs::optional \
        ); \
        \
        __JUSTARGS_CAT(SETNAME, _t)(const __JUSTARGS_CAT(SETNAME, _t) &) = default; \
        __JUSTARGS_CAT(SETNAME, _t)& operator= (const __JUSTARGS_CAT(SETNAME, _t) &) = default; \
        __JUSTARGS_CAT(SETNAME, _t)(__JUSTARGS_CAT(SETNAME, _t) &&) = default; \
        __JUSTARGS_CAT(SETNAME, _t)& operator= (__JUSTARGS_CAT(SETNAME, _t) &&) = default; \
        \
        __JUSTARGS_CAT(SETNAME, _t)(kwords_group *p, const char *name, const char *type, const char *descr) \
            :option_base{name, type, descr} \
            ,m_group{p} \
            ,m_val{} \
            ,m_cb{} \
        { m_group->add(this); } \
        __JUSTARGS_CAT(SETNAME, _t)(kwords_group *p, const char *name, const char *type, const char *descr, optional_type v, value_changed_cb c) \
        	:option_base{name, type, descr} \
            ,m_group{p} \
            ,m_val{std::move(v)} \
            ,m_cb{std::move(c)} \
        {} \
        \
        kwords_group *m_group; \
        optional_type m_val; \
        value_changed_cb m_cb; \
        \
        template<typename T> \
        __JUSTARGS_CAT(SETNAME, _t) operator= (T &&v) const { \
            value_type val = std::forward<T>(v); \
            optional_type opt{std::move(val)}; \
            __JUSTARGS_CAT(SETNAME, _t) res{m_group, name(), type(), description(), std::move(opt), value_changed_cb{}}; \
            return res; \
        } \
        \
        bool is_required() const override { return std::is_same<category, justargs::required>::value; } \
        bool is_optional() const override { return !is_required(); } \
        bool is_set() const override { return static_cast<bool>(m_val); } \
        \
        void set_cb(value_changed_cb c) { m_cb = std::move(c); } \
        void call_cb() const { if ( m_cb ) m_cb(m_val); } \
        \
        void from_string(const char *ptr, std::size_t len) { \
        value_type v{}; \
        from_string_impl(v, ptr, len); \
        m_val = std::move(v); \
        } \
        \
        bool __equal(const char *str) const { return std::strcmp(str, name()) == 0; } \
        \
    private: \
        template<typename T> \
        static typename std::enable_if<std::is_same<T, std::string>::value>::type \
        from_string_impl(T &val, const char *ptr, std::size_t len) { \
            val.assign(ptr, len); \
        } \
        template<typename T> \
        static typename std::enable_if<std::is_same<T, bool>::value>::type \
        from_string_impl(T &val, const char *ptr, std::size_t len) { \
            val = std::strncmp(ptr, "true", len) == 0; \
        } \
        template<typename T> \
        static typename std::enable_if<(std::is_integral<T>::value && !std::is_same<T, bool>::value)>::type \
        from_string_impl(T &val, const char *ptr, std::size_t len) { \
            constexpr const char *fmt = ( \
                std::is_unsigned<T>::value \
                    ? (std::is_same<T, std::uint8_t>::value \
                        ? "%  " SCNu8 : std::is_same<T, std::uint16_t>::value \
                            ? "%  " SCNu16 : std::is_same<T, std::uint32_t>::value \
                                ? "%  " SCNu32 \
                                : "%  " SCNu64 \
                      ) \
                    : (std::is_same<T, std::int8_t>::value \
                        ? "%  " SCNi8 : std::is_same<T, std::int16_t>::value \
                            ? "%  " SCNi16 : std::is_same<T, std::int32_t>::value \
                                ? "%  " SCNi32 \
                                : "%  " SCNi64 \
                      ) \
            ); \
            \
            enum { S = ct_strlen(fmt)+1 }; \
            auto fmtbuf = ct_init_array<S>(fmt); \
            fmtbuf[1] = '0' + (len / 10); \
            fmtbuf[2] = '0' + (len % 10); \
            \
            T tmpval{}; \
            std::sscanf(ptr, fmtbuf.data(), &tmpval); \
            \
            val = tmpval; \
        } \
        template<typename T> \
        static typename std::enable_if<std::is_floating_point<T>::value>::type \
        from_string_impl(T &val, const char *ptr, std::size_t len) { \
            constexpr const char *fmt = ( \
                std::is_same<T, float>::value \
                    ? "%  f" \
                    : "%  lf" \
            ); \
            \
            enum { S = ct_strlen(fmt)+1 }; \
            std::array<char, S> fmtbuf = ct_init_array<S>(fmt); \
            fmtbuf[1] = '0' + (len / 10); \
            fmtbuf[2] = '0' + (len % 10); \
            \
            T tmpval{}; \
            std::sscanf(ptr, fmtbuf.data(), &tmpval); \
            \
            val = tmpval; \
        } \
        template<typename T> \
        static typename std::enable_if<std::is_pointer<T>::value>::type \
        from_string_impl(T &val, const char *, std::size_t) { \
            val = nullptr; \
        } \
        \
        static constexpr std::size_t ct_strlen(const char *s) { \
            const char *p = s; \
            for ( ; *p; ++p ) \
                ; \
            return p - s; \
        } \
        template<std::size_t N> \
        static constexpr auto ct_init_array(const char *s) { \
            std::array<char, N> res{}; \
            for ( auto i = 0u; *s; ++s, ++i ) { \
                res[i] = *s; \
            } \
            return res; \
        } \
    } const SETNAME{ \
         this \
        ,__JUSTARGS_STRINGIZE(SETNAME) \
        ,__JUSTARGS_STRINGIZE(SETTYPE) \
        ,DESCRIPTION \
    };

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

private:
    const char *m_name;
    const char *m_type;
    const char *m_descr;
};

struct kwords_group {
    void add(const option_base *s) { options.emplace(s->name(), s); }

    template<typename T>
    bool is_required() const {
        const auto it = options.find(T::__name());
        if ( it == options.end() ) return false;
        return it->second.is_required();
    }

    std::ostream& show_help(std::ostream &os, const char *argv0) const {
        const char *p = std::strrchr(argv0, '/');
        os
        << (p ? p+1 : "program") << ":" << std::endl;

        std::size_t max_len = 0;
        for ( const auto &it: options) {
            std::size_t len = it.first.length();
            max_len = (len > max_len) ? len : max_len;
        }

        for ( const auto &it: options ) {
            static const char filler[] = "                                        ";
            const auto &name = it.first;
            std::size_t len = name.length();
            os << "--" << name << "=*";
            os.write(filler, static_cast<std::streamsize>(max_len-len));
            os << ": " << it.second->description() << " (" << it.second->type() << ", " << (it.second->is_required() ? "required" : "optional") << ")" << std::endl;
        }

        return os;
    }

    std::map<std::string, const option_base *> options;
};

/*************************************************************************************************/

namespace details {

template<typename ...Fs>
struct overloaded_set;

template<typename F1, typename ...Fs>
struct overloaded_set<F1, Fs...> : F1, overloaded_set<Fs...>::type {
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
    return details::overloaded_set<Fs...>(std::forward<Fs>(fs)...);
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

} // ns details

/*************************************************************************************************/

template<typename ...Args>
struct args {
private:
    using container_type = std::tuple<typename std::decay<Args>::type...>;
    static_assert(
        std::tuple_size<container_type>::value == std::tuple_size<details::without_duplicates<container_type>>::value
        ,"duplicate keywords detected!"
    );

    container_type m_kwords;

public:

    template<typename ...Types>
    explicit args(const Types &...types)
        :m_kwords(types...)
    {}

    template<typename T>
    constexpr bool has(const T &) const { return has_type<T>::value; }
    template<typename T>
    constexpr bool has() const { return has_type<T>::value; }

    container_type& kwords() { return m_kwords; }
    const container_type& kwords() const { return m_kwords; }

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

    template<typename T, typename VT>
    void set(const T &, VT &&v) {
        auto &item = std::get<T>(m_kwords);
        item.m_val = std::forward<VT>(v);
        item.call_cb();
    }

    template<typename T>
    bool is_set(const T &) const {
        static_assert (has_type<T>::value, "");

        const auto &item = std::get<T>(m_kwords);
        return item.is_set();
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
    void for_each(bool inited_only, Fs && ...fs) const {
        auto overloaded = details::make_overloaded(std::forward<Fs>(fs)...);
        for_each(m_kwords, std::move(overloaded), inited_only);
    }
    template<typename ...Fs>
    void for_each(bool inited_only, Fs && ...fs) {
        auto overloaded = details::make_overloaded(std::forward<Fs>(fs)...);
        for_each(m_kwords, std::move(overloaded), inited_only);
    }

    const char* check_for_unexpected(const char *optname) const {
        const char *ptr = nullptr;
        for_each(
             m_kwords
            ,[&ptr, optname](const auto &t, const auto &) {
                if ( t.__equal(optname) ) { ptr = t.name(); }
            }
            ,false
        );

        return ptr ? nullptr : optname;
    }
    const char* check_for_required() const {
        for ( const auto &it: std::get<0>(m_kwords).m_group->options ) {
            if ( !it.second->is_required() ) continue;

            const char *name = it.second->name();
            const char *ptr = nullptr;
            for_each(
                 m_kwords
                ,[&ptr, name](const auto &t, const auto &){
                    if ( t.__equal(name) ) { ptr = t.name(); }
                }
                ,true
            );

            if ( !ptr ) {
                return name;
            }
        }

        return nullptr;
    }

    template<typename T, typename R, typename Obj>
    args& bind(Obj *obj, R(Obj::*m)(const typename T::value_type &)) {
        auto f = [obj, m](const typename T::optional_type &v) {
            (obj->*m)(v.value());
        };
        std::get<T>(m_kwords).set_cb(std::move(f));

        return *this;
    }
    template<typename T>
    args& bind(typename T::value_type &r) {
        auto f = [&r](const typename T::optional_type &v) {
            r = v.value();
        };
        std::get<T>(m_kwords).set_cb(std::move(f));

        return *this;
    }
    template<typename T>
    args& bind(const T &, typename T::value_type &r) {
        return bind<T>(r);
    }

    std::ostream& dump(std::ostream &os) const {
        to_file(os, *this);
        return os;
    }
    friend std::ostream& operator<< (std::ostream &os, const args &set) {
        return set.dump(os);
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

    template<typename T>
    struct has_type {
        static constexpr bool value = !std::is_same<
             std::integer_sequence<bool, false, std::is_same<T, typename std::decay<Args>::type>::value...>
            ,std::integer_sequence<bool, std::is_same<T, typename std::decay<Args>::type>::value..., false>
        >::value;
    };

    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if<I != std::tuple_size<Tuple>::value>::type
    for_each(const Tuple &tuple, Func &&func, bool inited_only) {
        const auto &val = std::get<I>(tuple);
        if ( inited_only ) {
            if ( val.m_val ) {
                func(val, val.m_val.value());
            }
        } else {
            func(val, val.m_val.value());
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
            if ( val.m_val ) {
                func(val, val.m_val.value());
            }
        } else {
            func(val, val.m_val.value());
        }

        for_each<I + 1>(tuple, std::forward<Func>(func), inited_only);
    }
    template<std::size_t I = 0, typename Tuple, typename Func>
    static typename std::enable_if<I == std::tuple_size<Tuple>::value>::type
    for_each(Tuple &, Func &&, bool) {}
};

/*************************************************************************************************/

template<typename Iter, typename ...Args>
auto parse_kv_list(bool *ok, std::string *emsg, const char *pref, std::size_t pref_len, Iter beg, Iter end, Args && ...vals) {
    args<typename std::decay<Args>::type...> set{std::forward<Args>(vals)...};
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
        auto pos = line.find('=');
        if ( pos == std::string::npos ) continue;

        line[pos] = '\0';
        const char *key = line.c_str();
        const char *val = line.c_str() + pos + 1;
        std::size_t len = (line.length() - pos) - 1;

        const char *unexpected = set.check_for_unexpected(key);
        if ( unexpected ) {
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

        set.for_each(
            [key, val, len](auto &t, const auto &) {
                if ( !t.__equal(key)) return;
                t.from_string(val, len);
            }, false
        );
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

template<typename ...Args>
auto parse_args(bool *ok, std::string *emsg, int argc, char* const* argv, Args && ...args) {
    char *const *beg = argv+1;
    char *const *end = argv+argc;
    return parse_kv_list(ok, emsg, "--", 2, beg, end, std::forward<Args>(args)...);
}

/*************************************************************************************************/

template<typename ...Args>
std::ostream& to_file(std::ostream &os, const args<Args...> &set, bool inited_only = true) {
    set.for_each(
        details::make_overloaded(
             [&os](const auto &t, const bool &v) {
                os << t.name() << "=" << (v ? "true" : "false") << std::endl;
             }
            ,[&os](const auto &t, const auto &v) {
                os << t.name() << "=" << v << std::endl;
            }
        )
        ,inited_only
    );

    return os;
}

template<typename ...Args>
std::string to_string(const args<Args...> &set, bool inited_only = true) {
    std::ostringstream os;

    to_file(os, set, inited_only);

    return os.str();
}

template<typename ...Args>
auto from_file(bool *ok, std::string *emsg, std::istream &is, Args && ...args) {
    std::vector<std::string> lines;
    for ( std::string line; std::getline(is, line); ) {
        lines.push_back(std::move(line));
    }

    std::vector<const char*> linesptrs;
    linesptrs.reserve(lines.size());
    for ( const auto &it: lines ) {
        linesptrs.push_back(it.c_str());
    }

    return parse_kv_list(ok, emsg, nullptr, 0, linesptrs.begin(), linesptrs.end(), std::forward<Args>(args)...);
}

template<typename ...Args>
auto from_file(bool *ok, std::string *emsg, std::istream &is, args<Args...> &set) {
    return from_file(ok, emsg, is, std::get<typename std::decay<Args>::type>(set.kwords())...);
}

/*************************************************************************************************/

template<typename ...Args>
std::ostream& show_help(std::ostream &os, const char *argv0, const args<Args...> &set) {
    const char *p = std::strrchr(argv0, '/');
    os
    << (p ? p+1 : "program") << ":" << std::endl;

    std::size_t max_len = 0;
    set.template for_each(
        [&max_len](const auto &t, const auto &) {
            std::size_t len = std::strlen(t.name());
            max_len = (len > max_len) ? len : max_len;
        }
        ,false
    );

    set.template for_each(
        [&os, max_len](const auto &t, const auto &) {
            static const char filler[] = "                                        ";
            const char *name = t.name();
            std::size_t len = std::strlen(name);
            os << "--" << name << "=*";
            os.write(filler, static_cast<std::streamsize>(max_len-len));
            os << ": " << t.description() << " (" << t.type() << ", " << (t.is_required() ? "required" : "optional") << ")" << std::endl;
        }
        ,false
    );

    return os;
}

/*************************************************************************************************/

} // ns justargs

/*************************************************************************************************/

#endif // __JUSTARGS__JUSTARGS_HPP
