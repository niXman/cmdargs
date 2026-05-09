
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

#ifdef TEST_MAX_OPTIONS
#   define CMDARGS_MAX_OPTIONS_SIZE 32
#endif

#include <iostream>
#include <sstream>

#include <cmdargs/cmdargs.hpp>

#include <cstdio>
#include <cstdlib>

#define CMDARGS_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #cond); \
            std::abort(); \
        } \
    } while (0)

inline char * const *cmdargs_mutable_argv(const char * const *argv) {
    return const_cast<char * const *>(argv);
}


/*************************************************************************************************/

bool has_substring(const std::string &str, const char *substr) {
    return str.find(substr) != std::string::npos;
}

/*************************************************************************************************/

namespace test_templates_impl {

using namespace cmdargs::details;

template<typename T>
constexpr bool check_and() {
    return relation_pred_and<char, T>::value;
}

template<typename T>
constexpr bool check_or() {
    return relation_pred_or<char, T>::value;
}

template<typename T>
constexpr bool check_not() {
    return relation_pred_not<char, T>::value;
}

template<typename T>
constexpr bool is_relation() {
    return is_relation_type<T>::value;
}

template<typename ...Types>
constexpr bool contains_and_f(const std::tuple<Types...> &) {
    return contains<relation_pred_and, char, Types...>::value;
}

template<typename ...Types>
constexpr bool contains_or_f(const std::tuple<Types...> &) {
    return contains<relation_pred_or, char, Types...>::value;
}

template<typename ...Types>
constexpr bool contains_not_f(const std::tuple<Types...> &) {
    return contains<relation_pred_not, char, Types...>::value;
}

template<typename ...Types>
constexpr bool has_relation_and(const std::tuple<Types...> &) {
    using list_type = typename get_relation_list<relation_pred_and, Types...>::type;
    return std::is_same<list_type, relations_list<e_relation_type::AND>>::value;
}

template<typename ...Types>
constexpr bool has_relation_or(const std::tuple<Types...> &) {
    using list_type = typename get_relation_list<relation_pred_or, Types...>::type;
    return std::is_same<list_type, relations_list<e_relation_type::OR>>::value;
}

template<typename ...Types>
constexpr bool has_relation_not(const std::tuple<Types...> &) {
    using list_type = typename get_relation_list<relation_pred_not, Types...>::type;
    return std::is_same<list_type, relations_list<e_relation_type::NOT>>::value;
}

} // ns test_templates_impl

static void test_templates() {
    using namespace test_templates_impl;

    static_assert(true == check_and<relations_list<e_relation_type::AND>>());
    static_assert(true == check_or<relations_list<e_relation_type::OR>>());
    static_assert(true == check_not<relations_list<e_relation_type::NOT>>());
    static_assert(false == check_and<int>());

    static_assert(true == is_relation<relations_list<e_relation_type::AND>>());
    static_assert(true == is_relation<relations_list<e_relation_type::OR>>());
    static_assert(true == is_relation<relations_list<e_relation_type::NOT>>());
    static_assert(false == is_relation<int>());

    constexpr auto tuple = std::make_tuple(
         relations_list<e_relation_type::AND>{}
        ,relations_list<e_relation_type::OR>{}
        ,relations_list<e_relation_type::NOT>{}
    );
    static_assert(true == contains_and_f(tuple));
    static_assert(true == contains_or_f(tuple));
    static_assert(true == contains_not_f(tuple));

    constexpr auto tuple2 = std::make_tuple(
         int{}
        ,char{}
        ,float{}
    );
    static_assert(false == contains_and_f(tuple2));
    static_assert(false == contains_or_f(tuple2));
    static_assert(false == contains_not_f(tuple2));

    constexpr auto tuple3 = std::make_tuple(
         relations_list<e_relation_type::AND>{}
        ,char{}
        ,float{}
    );
    static_assert(true  == contains_and_f(tuple3));
    static_assert(false == contains_or_f(tuple3));
    static_assert(false == contains_not_f(tuple3));

    static_assert(true  == has_relation_and(tuple3));

    constexpr auto tuple4 = std::make_tuple(
         int{}
        ,relations_list<e_relation_type::OR>{}
        ,float{}
    );
    static_assert(false == contains_and_f(tuple4));
    static_assert(true  == contains_or_f(tuple4));
    static_assert(false == contains_not_f(tuple4));

    static_assert(true  == has_relation_or(tuple4));

    constexpr auto tuple5 = std::make_tuple(
         int{}
        ,char{}
        ,relations_list<e_relation_type::NOT>{}
    );
    static_assert(false == contains_and_f(tuple5));
    static_assert(false == contains_or_f(tuple5));
    static_assert(true  == contains_not_f(tuple5));

    static_assert(true  == has_relation_not(tuple5));
}

static void test_decl_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name");
        CMDARGS_OPTION(fsize, std::size_t, "source file size");
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == true);
    CMDARGS_ASSERT(kwords.fname.is_optional() == false);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == true);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == false);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == true);
}

/*************************************************************************************************/

static void test_decl_01() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name", optional);
        CMDARGS_OPTION(fsize, std::size_t, "source file size");
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == false);
    CMDARGS_ASSERT(kwords.fname.is_optional() == true);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == true);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == false);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == true);
}

static void test_decl_02() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name", optional);
        CMDARGS_OPTION(fsize, std::size_t, "source file size", optional);
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == false);
    CMDARGS_ASSERT(kwords.fname.is_optional() == true);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == false);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == true);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == true);
}

static void test_decl_03() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name");
        CMDARGS_OPTION(fsize, std::size_t, "source file size", and_(fname));
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == true);
    CMDARGS_ASSERT(kwords.fname.is_optional() == false);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == true);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == false);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == false);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == true);
}

static void test_decl_04() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name");
        CMDARGS_OPTION(fsize, std::size_t, "source file size", or_(fname));
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == true);
    CMDARGS_ASSERT(kwords.fname.is_optional() == false);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == true);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == false);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == false);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == true);
}

static void test_decl_05() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name");
        CMDARGS_OPTION(fsize, std::size_t, "source file size", not_(fname));
    } const kwords;

    CMDARGS_ASSERT(kwords.fname.is_required() == true);
    CMDARGS_ASSERT(kwords.fname.is_optional() == false);
    CMDARGS_ASSERT(kwords.fname.is_set() == false);
    CMDARGS_ASSERT(kwords.fsize.is_required() == true);
    CMDARGS_ASSERT(kwords.fsize.is_optional() == false);
    CMDARGS_ASSERT(kwords.fsize.is_set() == false);

    CMDARGS_ASSERT(kwords.fname.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fname.not_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.and_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.or_list().empty() == true);
    CMDARGS_ASSERT(kwords.fsize.not_list().empty() == false);
}

/*************************************************************************************************/

static void test_string_trim() {
    static const std::string_view ws{" \n\r\t"};
    // left trim
    {
        std::string_view src = " string";
        auto res = cmdargs::details::ltrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\nstring";
        auto res = cmdargs::details::ltrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\rstring";
        auto res = cmdargs::details::ltrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\tstring";
        auto res = cmdargs::details::ltrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = " \n\r\tstring";
        auto res = cmdargs::details::ltrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    // right trim
    {
        std::string_view src = "string ";
        auto res = cmdargs::details::rtrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "string\n";
        auto res = cmdargs::details::rtrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "string\r";
        auto res = cmdargs::details::rtrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "string\t";
        auto res = cmdargs::details::rtrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "string \n\r\t";
        auto res = cmdargs::details::rtrim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    // left + right trim
    {
        std::string_view src = " string ";
        auto res = cmdargs::details::trim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\nstring\n";
        auto res = cmdargs::details::trim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\rstring\r";
        auto res = cmdargs::details::trim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = "\tstring\t";
        auto res = cmdargs::details::trim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
    {
        std::string_view src = " \n\r\tstring \n\r\t";
        auto res = cmdargs::details::trim(src, ws);
        CMDARGS_ASSERT(res == "string");
    }
}

/*************************************************************************************************/

static void test_bool_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name", optional);
        CMDARGS_OPTION(fsize, std::size_t, "source file size", and_(fname));
        CMDARGS_OPTION(report, bool, "report when finished", optional, and_(fname, fsize));
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );
        CMDARGS_ASSERT(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        CMDARGS_ASSERT(args.is_set(kwords.fname) == true);
        CMDARGS_ASSERT(args[kwords.fname] == "1.txt");

        CMDARGS_ASSERT(args.is_set(kwords.fsize) == true);
        CMDARGS_ASSERT(args[kwords.fsize] == 1024);

        CMDARGS_ASSERT(!args.is_set(kwords.report));
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--report"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        CMDARGS_ASSERT(args.is_set(kwords.fname) == true);
        CMDARGS_ASSERT(args[kwords.fname] == "1.txt");

        CMDARGS_ASSERT(args.is_set(kwords.fsize) == true);
        CMDARGS_ASSERT(args[kwords.fsize] == 1024);

        CMDARGS_ASSERT(args.is_set(kwords.report));
        CMDARGS_ASSERT(args[kwords.report] == true);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--report=false"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        CMDARGS_ASSERT(args.is_set(kwords.fname) == true);
        CMDARGS_ASSERT(args[kwords.fname] == "1.txt");

        CMDARGS_ASSERT(args.is_set(kwords.fsize) == true);
        CMDARGS_ASSERT(args[kwords.fsize] == 1024);

        CMDARGS_ASSERT(args.is_set(kwords.report));
        CMDARGS_ASSERT(args[kwords.report] == false);
    }
}

/*************************************************************************************************/

static void test_cond_and_00() {
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(fname, std::string, "source file name", optional);
            CMDARGS_OPTION(fsize, std::size_t, "source file size", and_(fname));
            CMDARGS_OPTION_HELP();
        } const kwords;

        const char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            CMDARGS_ASSERT(!emsg.empty());
            CMDARGS_ASSERT(has_substring(emsg, "together with \"--fname\""));
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(fname, std::string, "source file name");
            CMDARGS_OPTION(fsize, std::size_t, "source file size", and_(fname));
        } const kwords;

        const char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            CMDARGS_ASSERT(!emsg.empty());
            CMDARGS_ASSERT(emsg == "no required \"--fname\" option was specified");
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(fname, std::string, "source file name");
            CMDARGS_OPTION(fsize, std::size_t, "source file size", and_(fname));
        } const kwords;

        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            if ( !emsg.empty() ) {
                std::cout << __func__ << ": " << emsg << std::endl;
            }
            CMDARGS_ASSERT(emsg.empty());
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(fname, std::string, "source file name");
            CMDARGS_OPTION(fsize, std::size_t, "source file size");
            CMDARGS_OPTION(fmode, std::string, "processing mode", and_(fname, fsize));
        } const kwords;

        {
            const char * const margv[] = {
                 "cmdargs-test"
                ,"--fsize=1024"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            CMDARGS_ASSERT(!emsg.empty());
            CMDARGS_ASSERT(emsg == "no required \"--fname\" option was specified");
        }

        {
            const char * const margv[] = {
                 "cmdargs-test"
                ,"--fname=1.txt"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            CMDARGS_ASSERT(!emsg.empty());
            CMDARGS_ASSERT(emsg == "no required \"--fsize\" option was specified");
        }

        {
            const char * const margv[] = {
                 "cmdargs-test"
                ,"--fname=1.txt"
                ,"--fsize=1024"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,cmdargs_mutable_argv(margv)
                ,kwords
            );

            CMDARGS_ASSERT(emsg.empty());
        }
    }
}

/*************************************************************************************************/

static void test_cond_or_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fname, std::string, "source file name", optional);
        CMDARGS_OPTION(fsize, std::size_t, "source file size", optional);
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(fname, fsize));
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(has_substring(emsg, "together with one of \"--fname\", \"--fsize\""));
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(has_substring(emsg, "together with one of \"--fname\", \"--fsize\""));
    }
}

/*************************************************************************************************/

static void test_cond_not_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "the \"--fmode\" option must be used together with one of \"--netsrc\", \"--filesrc\"");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "the \"--fmode\" option must be used together with one of \"--netsrc\", \"--filesrc\"");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());
    }
}

/*************************************************************************************************/

static void test_default_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.filesrc) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        CMDARGS_ASSERT(args[kwords.filesrc] == "1.txt");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.101"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.filesrc) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args[kwords.netsrc] == "192.168.1.101");
        CMDARGS_ASSERT(args.get(kwords.filesrc, "2.txt") == "2.txt");
    }
}

/*************************************************************************************************/

static void test_validator_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(fmode, std::string, "processing mode"
            ,validator_([](std::string_view str) {
                return str == "read" || str == "write";
            })
        );
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=write"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "write");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=wrong"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "an invalid value \"wrong\" was received for \"--fmode\" option");
    }
}

/*************************************************************************************************/

static void test_converter_00() {
    struct kwords: cmdargs::kwords_group {
        enum e_mode { undefined, read, write };
        CMDARGS_OPTION(fmode, e_mode, "processing mode"
            ,converter_([](e_mode &mode, std::string_view str) {
                if ( str == "read" ) {
                    mode = e_mode::read;
                } else if ( str == "write" ) {
                    mode = e_mode::write;
                } else {
                    return false;
                }

                return true;
            })
        );
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == kwords::read);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=write"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == kwords::write);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=wrong"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "can't convert value \"wrong\" for \"--fmode\" option");
    }
}

/*************************************************************************************************/

static void test_validator_with_deps_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(flag, bool, "flag", optional);
        CMDARGS_OPTION(s, std::string, "s", optional
            ,validator_([](std::string_view sv, const auto &f) {
                if ( f.value_or(false) && sv.empty() ) {
                    return false;
                }
                return !sv.empty();
            }, flag)
        );
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--flag=true"
            ,"--s=hi"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(&emsg, margc, cmdargs_mutable_argv(margv), kwords);
        CMDARGS_ASSERT(emsg.empty());
        CMDARGS_ASSERT(args[kwords.s] == "hi");
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--flag=true"
            ,"--s="
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(&emsg, margc, cmdargs_mutable_argv(margv), kwords);
        (void)args;
        CMDARGS_ASSERT(!emsg.empty());
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--s=ok"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(&emsg, margc, cmdargs_mutable_argv(margv), kwords);
        CMDARGS_ASSERT(emsg.empty());
        CMDARGS_ASSERT(args[kwords.s] == "ok");
    }
}

/*************************************************************************************************/

static void test_converter_with_deps_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(scale, std::string, "scale", optional, default_(std::string("1")));
        CMDARGS_OPTION(n, std::int32_t, "n", optional
            ,converter_for<std::int32_t>([](std::int32_t &v, std::string_view sv, const auto &sc) {
                cmdargs::details::from_string_impl(&v, sv);
                if ( sc.value_or("1") == "2" ) {
                    v *= 2;
                }
                return true;
            }, scale)
        );
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--n=21"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(&emsg, margc, cmdargs_mutable_argv(margv), kwords);
        CMDARGS_ASSERT(emsg.empty());
        CMDARGS_ASSERT(args[kwords.n] == 21);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--scale=2"
            ,"--n=21"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(&emsg, margc, cmdargs_mutable_argv(margv), kwords);
        CMDARGS_ASSERT(emsg.empty());
        CMDARGS_ASSERT(args[kwords.n] == 42);
    }
}

/*************************************************************************************************/

static void test_default_value_v2() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name"
            ,optional
            ,not_(filesrc)
            ,default_<std::string>("127.0.0.1")
        );
        CMDARGS_OPTION(filesrc, std::string, "file source name"
            ,optional
            ,not_(netsrc)
            ,default_<std::string>("data.txt")
        );
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        const char * const margv[] = {
            "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        const auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args.is_set(kwords.netsrc) == true);
        CMDARGS_ASSERT(args.has_default(kwords.netsrc) == true);
        CMDARGS_ASSERT(args[kwords.netsrc] == "192.168.1.106");

        CMDARGS_ASSERT(args.is_set(kwords.filesrc) == false);
        CMDARGS_ASSERT(args.has_default(kwords.filesrc) == true);
        CMDARGS_ASSERT(args[kwords.filesrc] == "data.txt");

        CMDARGS_ASSERT(args.is_set(kwords.fmode) == true);
        CMDARGS_ASSERT(args.has_default(kwords.fmode) == false);
        CMDARGS_ASSERT(args[kwords.fmode] == "read");
    }

}

/*************************************************************************************************/

static void test_to_file_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());

        std::ostringstream os;
        cmdargs::to_file(os, args);

        static const char *expected =
R"(# network source name
netsrc=192.168.1.106
# processing mode
fmode=read
)";
        auto str = os.str();
        CMDARGS_ASSERT(os.str() == expected);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        CMDARGS_ASSERT(emsg.empty());

        std::ostringstream os;
        cmdargs::to_file(os, args);

        static const char *expected =
R"(# file source name
filesrc=1.txt
# processing mode
fmode=read
)";
        auto str = os.str();
        CMDARGS_ASSERT(os.str() == expected);
    }
}

/*************************************************************************************************/

static void test_from_file_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        static const char *expected =
R"(netsrc=192.168.1.106
filesrc=1.txt
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        static_assert(args.contains(kwords.netsrc) == true, "");
        static_assert(args.contains(kwords.filesrc) == true, "");
        static_assert(args.contains(kwords.fmode) == true, "");

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "the \"fmode\" option must be used together with one of \"netsrc\", \"filesrc\"");
    }
    {
        static const char *expected =
R"(filesrc=1.txt
netsrc=192.168.1.106
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        CMDARGS_ASSERT(!emsg.empty());
        CMDARGS_ASSERT(emsg == "the \"fmode\" option must be used together with one of \"netsrc\", \"filesrc\"");
    }
    {
        static const char *expected =
R"(netsrc=192.168.1.106
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        CMDARGS_ASSERT(emsg.empty());
    }
    {
        static const char *expected =
R"(filesrc=1.txt
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        CMDARGS_ASSERT(emsg.empty());
    }
}

/*************************************************************************************************/

static void test_show_help_and_version_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kw_top;

    {
        std::ostringstream os;
        cmdargs::show_help(os, "/test", kw_top);

        static const char *expected =
R"(test:
--netsrc=* : "network source name" (std::string, optional, not(--filesrc))
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
)";

        auto str = os.str();
        CMDARGS_ASSERT(str == expected);
    }
    {
        const char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kw_top.filesrc
            ,kw_top.fmode
        );
        static_assert(args.contains(kw_top.filesrc) == true);
        static_assert(args.contains(kw_top.fmode) == true);

        std::ostringstream os;
        cmdargs::show_help(os, "/test", args);

        static const char *expected =
R"(test:
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
)";

        CMDARGS_ASSERT(os.str() == expected);
    }

    // help
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--help"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_help_requested(os, margv[0], args) == true);
        static const char *expected =
R"(cmdargs-test:
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
--help=*   : "show help message" (bool, optional)
)";
        CMDARGS_ASSERT(os.str() == expected);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
             "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_help_requested(os, margv[0], args) == false);
        CMDARGS_ASSERT(os.str().empty() == true);
    }
    // version
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
            ,kwords.version
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_version_requested(os, margv[0], args) == false);
        CMDARGS_ASSERT(os.str().empty() == true);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--version"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
            ,kwords.version
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test: version - 0.0.1
)";
        CMDARGS_ASSERT(os.str() == expected);
    }

    // help or version
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_help_or_version_requested(os, margv[0], args) == false);
        CMDARGS_ASSERT(os.str().empty() == true);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--help"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_help_or_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test:
--netsrc=* : "network source name" (std::string, optional, not(--filesrc))
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
--help=*   : "show help message" (bool, optional)
--version=*: "show version message" (std::string, optional, default="0.0.1")
)";
        CMDARGS_ASSERT(os.str() == expected);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_HELP();
            CMDARGS_OPTION_VERSION("0.0.1");
        } const kwords;

        const char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--version"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );
        CMDARGS_ASSERT(emsg.empty() == true);

        std::ostringstream os;
        CMDARGS_ASSERT(cmdargs::is_help_or_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test: version - 0.0.1
)";
        CMDARGS_ASSERT(os.str() == expected);
    }
}

/*************************************************************************************************/

static void test_predefined_converters() {
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION(fileslist, std::vector<std::string>, "source files list", optional, not_(netsrc)
                ,convert_as_vector<std::string>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::vector<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION(fileslist, std::vector<std::size_t>, "source files list", optional, not_(netsrc)
                ,convert_as_vector<std::size_t>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1,2,3"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::vector<std::size_t>{1, 2, 3}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION(fileslist, std::list<std::string>, "source files list", optional, not_(netsrc)
                ,convert_as_list<std::string>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::list<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION(fileslist, std::set<std::string>, "source files list", optional, not_(netsrc)
                               ,convert_as_set<std::string>()
                               );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::set<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::string, std::string>;
            CMDARGS_OPTION(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1.txt,2=2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::map<std::string, std::string>{{"1", "1.txt"}, {"2", "2.txt"}}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::size_t, std::string>;
            CMDARGS_OPTION(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1.txt,2=2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::map<std::size_t, std::string>{{1, "1.txt"}, {2, "2.txt"}}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::size_t, std::size_t>;
            CMDARGS_OPTION(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1,2=2"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        CMDARGS_ASSERT(emsg.empty());

        CMDARGS_ASSERT(args[kwords.fmode] == "read");
        CMDARGS_ASSERT(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        CMDARGS_ASSERT(fileslist == (std::map<std::size_t, std::size_t>{{1, 1}, {2, 2}}));
    }
}

/*************************************************************************************************/

static void test_as_optionals() {
    struct kwords: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    const char * const margv[] = {
        "cmdargs-test"
        ,"--netsrc=192.168.1.106"
        ,"--fmode=read"
    };

    std::string emsg;
    const auto [netsrc, filesrc, fmode] = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,cmdargs_mutable_argv(margv)
        ,kwords
    ).optionals();

    static_assert(std::is_same_v<decltype(netsrc), const std::optional<std::string>>);
    static_assert(std::is_same_v<decltype(filesrc), const std::optional<std::string>>);
    static_assert(std::is_same_v<decltype(fmode), const std::optional<std::string>>);

    CMDARGS_ASSERT(emsg.empty());

    CMDARGS_ASSERT(!filesrc);

    CMDARGS_ASSERT(netsrc);
    CMDARGS_ASSERT(netsrc.value() == "192.168.1.106");

    CMDARGS_ASSERT(fmode);
    CMDARGS_ASSERT(fmode.value() == "read");
}

/*************************************************************************************************/

static void test_as_values() {
    struct kwords: cmdargs::kwords_group {
        CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    const char * const margv[] = {
        "cmdargs-test"
        ,"--netsrc=192.168.1.106"
        ,"--fmode=read"
    };

    std::string emsg;
    const auto [netsrc, filesrc, fmode] = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,cmdargs_mutable_argv(margv)
        ,kwords
    ).values();

    static_assert(std::is_same_v<decltype(netsrc), const std::string>);
    static_assert(std::is_same_v<decltype(filesrc), const std::string>);
    static_assert(std::is_same_v<decltype(fmode), const std::string>);

    CMDARGS_ASSERT(emsg.empty());

    CMDARGS_ASSERT(filesrc.empty());

    CMDARGS_ASSERT(!netsrc.empty());
    CMDARGS_ASSERT(netsrc == "192.168.1.106");

    CMDARGS_ASSERT(!fmode.empty());
    CMDARGS_ASSERT(fmode == "read");
}

/*************************************************************************************************/

static void test_version() {
    constexpr auto version = CMDARGS_VERSION_HEX;
    constexpr auto major = CMDARGS_VERSION_GET_MAJOR(version);
    constexpr auto minor = CMDARGS_VERSION_GET_MINOR(version);
    constexpr auto bugfix= CMDARGS_VERSION_GET_BUGFIX(version);

    static_assert(major == CMDARGS_VERSION_MAJOR);
    static_assert(minor == CMDARGS_VERSION_MINOR);
    static_assert(bugfix == CMDARGS_VERSION_BUGFIX);

    constexpr char str[] = {
        __CMDARGS__STRINGIZE(CMDARGS_VERSION_MAJOR) "."
        __CMDARGS__STRINGIZE(CMDARGS_VERSION_MINOR) "."
        __CMDARGS__STRINGIZE(CMDARGS_VERSION_BUGFIX)
    };
    static_assert(std::string_view{CMDARGS_VERSION_STRING} == str);
}

/*************************************************************************************************/

static void test_dump() {
    {
        struct kwords: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source name", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
        } const kwords;

        std::stringstream ss;
        cmdargs::dump_group(ss, kwords);

        static const char *expected =
R"(name            : netsrc
type            : std::string
description     : "network source name"
is required     : false
value           : <UNINITIALIZED>
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 0
relation     NOT: 1 (--filesrc)
*******************************************
name            : filesrc
type            : std::string
description     : "file source name"
is required     : false
value           : <UNINITIALIZED>
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 0
relation     NOT: 1 (--netsrc)
*******************************************
name            : fmode
type            : std::string
description     : "processing mode"
is required     : true
value           : <UNINITIALIZED>
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 2 (--netsrc, --filesrc)
relation     NOT: 0
*******************************************
)";

        CMDARGS_ASSERT(ss.str() == expected);
    }
    {
        struct kwords: cmdargs::kwords_group {
            CMDARGS_OPTION(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION(filesrc, std::string, "file source name", optional, not_(netsrc));
            CMDARGS_OPTION(fmode, std::string, "processing mode", or_(netsrc, filesrc));
        } const kwords;

        const char * const margv[] = {
            "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };

        std::string emsg;
        const auto args = cmdargs::parse_args(
             &emsg
            ,std::size(margv)
            ,cmdargs_mutable_argv(margv)
            ,kwords
        );
        CMDARGS_ASSERT(emsg.empty());

        std::stringstream ss;
        cmdargs::dump_group(ss, args);

        static const char *expected =
R"(name            : netsrc
type            : std::string
description     : "network source name"
is required     : false
value           : 192.168.1.106
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 0
relation     NOT: 1 (--filesrc)
*******************************************
name            : filesrc
type            : std::string
description     : "file source name"
is required     : false
value           : <UNINITIALIZED>
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 0
relation     NOT: 1 (--netsrc)
*******************************************
name            : fmode
type            : std::string
description     : "processing mode"
is required     : true
value           : read
custom validator: false
custom converter: false
relation     AND: 0
relation      OR: 2 (--netsrc, --filesrc)
relation     NOT: 0
*******************************************
)";
        CMDARGS_ASSERT(ss.str() == expected);
    }
}

/*************************************************************************************************/

#ifdef TEST_MAX_OPTIONS
static void test_max_options() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION(opt0 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt1 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt2 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt3 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt4 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt5 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt6 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt7 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt8 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt9 , std::size_t, "test option", optional);
        CMDARGS_OPTION(opt10, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt11, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt12, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt13, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt14, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt15, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt16, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt17, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt18, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt19, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt20, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt21, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt22, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt23, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt24, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt25, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt26, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt27, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt28, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt29, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt30, std::size_t, "test option", optional);
        CMDARGS_OPTION(opt31, std::size_t, "test option", optional);
    } const kwords;

    const char * const margv[] = {
        "cmdargs-test"
        ,"--opt0=0"
        ,"--opt1=1"
        ,"--opt2=2"
        ,"--opt3=3"
        ,"--opt4=4"
        ,"--opt5=5"
        ,"--opt6=6"
        ,"--opt7=7"
        ,"--opt8=8"
        ,"--opt9=9"
        ,"--opt10=10"
        ,"--opt11=11"
        ,"--opt12=12"
        ,"--opt13=13"
        ,"--opt14=14"
        ,"--opt15=15"
        ,"--opt16=16"
        ,"--opt17=17"
        ,"--opt18=18"
        ,"--opt19=19"
        ,"--opt20=20"
        ,"--opt21=21"
        ,"--opt22=22"
        ,"--opt23=23"
        ,"--opt24=24"
        ,"--opt25=25"
        ,"--opt26=26"
        ,"--opt27=27"
        ,"--opt28=28"
        ,"--opt29=29"
        ,"--opt30=30"
        ,"--opt31=31"
    };

    std::string emsg;
    const auto args = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,cmdargs_mutable_argv(margv)
        ,kwords
    );
    CMDARGS_ASSERT(emsg.empty());
    static_assert(args.size() == 32);

    std::size_t cnt = 0;
    args.for_each([&cnt](const auto &it){
        CMDARGS_ASSERT(it.get_value() == cnt);
        ++cnt;
        return true;
    });
    CMDARGS_ASSERT(cnt == 32);
}
#endif // TEST_MAX_OPTIONS

/*************************************************************************************************/

#define TEST(func) \
    do { \
        std::cout << "[" << __COUNTER__ << "] test for " #func "..." << std::flush; \
        (func)(); \
        std::cout << "passed!" << std::endl; \
    } while (0)

int main(int, char **) {
    TEST(test_templates);

    TEST(test_decl_00);
    TEST(test_decl_01);
    TEST(test_decl_02);
    TEST(test_decl_03);
    TEST(test_decl_04);
    TEST(test_decl_05);

    TEST(test_string_trim);

    TEST(test_bool_00);

    TEST(test_cond_and_00);

    TEST(test_cond_or_00);

    TEST(test_cond_not_00);

    TEST(test_default_00);

    TEST(test_validator_00);

    TEST(test_converter_00);

    TEST(test_validator_with_deps_00);

    TEST(test_converter_with_deps_00);

    TEST(test_default_value_v2);

    TEST(test_to_file_00);
    TEST(test_from_file_00);

    TEST(test_show_help_and_version_00);

    TEST(test_predefined_converters);

    TEST(test_as_optionals);
    TEST(test_as_values);

    TEST(test_version);

    TEST(test_dump);

#ifdef TEST_MAX_OPTIONS
    TEST(test_max_options);
#endif

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
