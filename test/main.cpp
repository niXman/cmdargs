
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

#include <cmdargs/cmdargs.hpp>

#include <iostream>
#include <cassert>

#ifdef NDEBUG
#   error "This file MUST be compiled with NDEBUG undefined!"
#endif

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
        CMDARGS_OPTION_ADD(fname, std::string, "source file name");
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size");
    } const kwords;

    assert(kwords.fname.is_required() == true);
    assert(kwords.fname.is_optional() == false);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == true);
    assert(kwords.fsize.is_optional() == false);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == true);
    assert(kwords.fsize.or_list().empty() == true);
    assert(kwords.fsize.not_list().empty() == true);
}

/*************************************************************************************************/

static void test_decl_01() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size");
    } const kwords;

    assert(kwords.fname.is_required() == false);
    assert(kwords.fname.is_optional() == true);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == true);
    assert(kwords.fsize.is_optional() == false);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == true);
    assert(kwords.fsize.or_list().empty() == true);
    assert(kwords.fsize.not_list().empty() == true);
}

static void test_decl_02() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional);
    } const kwords;

    assert(kwords.fname.is_required() == false);
    assert(kwords.fname.is_optional() == true);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == false);
    assert(kwords.fsize.is_optional() == true);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == true);
    assert(kwords.fsize.or_list().empty() == true);
    assert(kwords.fsize.not_list().empty() == true);
}

static void test_decl_03() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name");
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", and_(fname));
    } const kwords;

    assert(kwords.fname.is_required() == true);
    assert(kwords.fname.is_optional() == false);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == true);
    assert(kwords.fsize.is_optional() == false);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == false);
    assert(kwords.fsize.or_list().empty() == true);
    assert(kwords.fsize.not_list().empty() == true);
}

static void test_decl_04() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name");
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", or_(fname));
    } const kwords;

    assert(kwords.fname.is_required() == true);
    assert(kwords.fname.is_optional() == false);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == true);
    assert(kwords.fsize.is_optional() == false);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == true);
    assert(kwords.fsize.or_list().empty() == false);
    assert(kwords.fsize.not_list().empty() == true);
}

static void test_decl_05() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name");
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", not_(fname));
    } const kwords;

    assert(kwords.fname.is_required() == true);
    assert(kwords.fname.is_optional() == false);
    assert(kwords.fname.is_set() == false);
    assert(kwords.fsize.is_required() == true);
    assert(kwords.fsize.is_optional() == false);
    assert(kwords.fsize.is_set() == false);

    assert(kwords.fname.and_list().empty() == true);
    assert(kwords.fname.or_list().empty() == true);
    assert(kwords.fname.not_list().empty() == true);
    assert(kwords.fsize.and_list().empty() == true);
    assert(kwords.fsize.or_list().empty() == true);
    assert(kwords.fsize.not_list().empty() == false);
}

/*************************************************************************************************/

static void test_string_trim() {
    static const std::string_view ws{" \n\r\t"};
    // left trim
    {
        std::string_view src = " string";
        auto res = cmdargs::details::ltrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\nstring";
        auto res = cmdargs::details::ltrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\rstring";
        auto res = cmdargs::details::ltrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\tstring";
        auto res = cmdargs::details::ltrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = " \n\r\tstring";
        auto res = cmdargs::details::ltrim(src, ws);
        assert(res == "string");
    }
    // right trim
    {
        std::string_view src = "string ";
        auto res = cmdargs::details::rtrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "string\n";
        auto res = cmdargs::details::rtrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "string\r";
        auto res = cmdargs::details::rtrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "string\t";
        auto res = cmdargs::details::rtrim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "string \n\r\t";
        auto res = cmdargs::details::rtrim(src, ws);
        assert(res == "string");
    }
    // left + right trim
    {
        std::string_view src = " string ";
        auto res = cmdargs::details::trim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\nstring\n";
        auto res = cmdargs::details::trim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\rstring\r";
        auto res = cmdargs::details::trim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = "\tstring\t";
        auto res = cmdargs::details::trim(src, ws);
        assert(res == "string");
    }
    {
        std::string_view src = " \n\r\tstring \n\r\t";
        auto res = cmdargs::details::trim(src, ws);
        assert(res == "string");
    }
}

/*************************************************************************************************/

static void test_bool_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", and_(fname));
        CMDARGS_OPTION_ADD(report, bool, "report when finished", optional, and_(fname, fsize));
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );
        assert(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        assert(args.is_set(kwords.fname) == true);
        assert(args[kwords.fname] == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args[kwords.fsize] == 1024);

        assert(!args.is_set(kwords.report));
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--report"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        assert(args.is_set(kwords.fname) == true);
        assert(args[kwords.fname] == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args[kwords.fsize] == 1024);

        assert(args.is_set(kwords.report));
        assert(args[kwords.report] == true);
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--report=false"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());

        static_assert(args.contains(kwords.fname) == true);
        static_assert(args.contains(kwords.fsize) == true);
        static_assert(args.contains(kwords.report) == true);

        assert(args.is_set(kwords.fname) == true);
        assert(args[kwords.fname] == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args[kwords.fsize] == 1024);

        assert(args.is_set(kwords.report));
        assert(args[kwords.report] == false);
    }
}

/*************************************************************************************************/

static void test_cond_and_00() {
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
            CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", and_(fname));
            CMDARGS_OPTION_ADD_HELP();
        } const kwords;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            assert(!emsg.empty());
            assert(has_substring(emsg, "together with \"--fname\""));
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(fname, std::string, "source file name");
            CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", and_(fname));
        } const kwords;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            assert(!emsg.empty());
            assert(emsg == "there is no required \"--fname\" option was specified");
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(fname, std::string, "source file name");
            CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", and_(fname));
        } const kwords;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        {
            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            if ( !emsg.empty() ) {
                std::cout << __func__ << ": " << emsg << std::endl;
            }
            assert(emsg.empty());
        }
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(fname, std::string, "source file name");
            CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size");
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", and_(fname, fsize));
        } const kwords;

        {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wwrite-strings"
            char * const margv[] = {
                 "cmdargs-test"
                ,"--fsize=1024"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);
            #pragma GCC diagnostic pop

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            assert(!emsg.empty());
            assert(emsg == "there is no required \"--fname\" option was specified");
        }

        {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wwrite-strings"
            char * const margv[] = {
                 "cmdargs-test"
                ,"--fname=1.txt"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);
            #pragma GCC diagnostic pop

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            assert(!emsg.empty());
            assert(emsg == "there is no required \"--fsize\" option was specified");
        }

        {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wwrite-strings"
            char * const margv[] = {
                 "cmdargs-test"
                ,"--fname=1.txt"
                ,"--fsize=1024"
                ,"--fmode=read"
            };
            int margc = sizeof(margv)/sizeof(margv[0]);
            #pragma GCC diagnostic pop

            std::string emsg;
            auto args = cmdargs::parse_args(
                 &emsg
                ,margc
                ,margv
                ,kwords
            );

            assert(emsg.empty());
        }
    }
}

/*************************************************************************************************/

static void test_cond_or_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
        CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional);
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(fname, fsize));
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(!emsg.empty());
        assert(has_substring(emsg, "together with one of \"--fname\", \"--fsize\""));
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fsize=1024"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fname=1.txt"
            ,"--fsize=1024"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(!emsg.empty());
        assert(has_substring(emsg, "together with one of \"--fname\", \"--fsize\""));
    }
}

/*************************************************************************************************/

static void test_cond_not_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(!emsg.empty());
        assert(emsg == "the \"--fmode\" option must be used together with one of \"--netsrc\", \"--filesrc\"");
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(!emsg.empty());
        assert(emsg == "the \"--fmode\" option must be used together with one of \"--netsrc\", \"--filesrc\"");
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());
    }
}

/*************************************************************************************************/

static void test_default_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.filesrc) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        assert(args[kwords.filesrc] == "1.txt");
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.101"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.filesrc) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args[kwords.netsrc] == "192.168.1.101");
        assert(args.get(kwords.filesrc, "2.txt") == "2.txt");
    }
}

/*************************************************************************************************/

static void test_validator_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode"
            ,validator_([](std::string_view str) {
                return str == "read" || str == "write";
            })
        );
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=write"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "write");
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=wrong"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(!emsg.empty());
        assert(emsg == "an invalid value \"wrong\" was received for \"--fmode\" option");
    }
}

/*************************************************************************************************/

static void test_converter_00() {
    struct kwords: cmdargs::kwords_group {
        enum e_mode { undefined, read, write };
        CMDARGS_OPTION_ADD(fmode, e_mode, "processing mode"
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
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == kwords::read);
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=write"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == kwords::write);
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--fmode=wrong"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.fmode) == true);

        assert(!emsg.empty());
        assert(emsg == "can't convert value \"wrong\" for \"--fmode\" option");
    }
}

/*************************************************************************************************/

static void test_default_value_v2() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name"
            ,optional
            ,not_(filesrc)
            ,default_<std::string>("127.0.0.1")
        );
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source name"
            ,optional
            ,not_(netsrc)
            ,default_<std::string>("data.txt")
        );
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        const auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());

        assert(args.is_set(kwords.netsrc) == true);
        assert(args.has_default(kwords.netsrc) == true);
        assert(args[kwords.netsrc] == "192.168.1.106");

        assert(args.is_set(kwords.filesrc) == false);
        assert(args.has_default(kwords.filesrc) == true);
        assert(args[kwords.filesrc] == "data.txt");

        assert(args.is_set(kwords.fmode) == true);
        assert(args.has_default(kwords.fmode) == false);
        assert(args[kwords.fmode] == "read");
    }

}

/*************************************************************************************************/

static void test_to_file_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());

        std::ostringstream os;
        cmdargs::to_file(os, args);

        static const char *expected =
R"(# network source name
netsrc=192.168.1.106
# processing mode
fmode=read
)";
        auto str = os.str();
        assert(os.str() == expected);
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        assert(emsg.empty());

        std::ostringstream os;
        cmdargs::to_file(os, args);

        static const char *expected =
R"(# file source name
filesrc=1.txt
# processing mode
fmode=read
)";
        auto str = os.str();
        assert(os.str() == expected);
    }
}

/*************************************************************************************************/

static void test_from_file_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
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

        assert(!emsg.empty());
        assert(emsg == "the \"fmode\" option must be used together with one of \"netsrc\", \"filesrc\"");
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

        assert(!emsg.empty());
        assert(emsg == "the \"fmode\" option must be used together with one of \"netsrc\", \"filesrc\"");
    }
    {
        static const char *expected =
R"(netsrc=192.168.1.106
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        assert(emsg.empty());
    }
    {
        static const char *expected =
R"(filesrc=1.txt
fmode=read
)";
        std::istringstream is{expected};
        std::string emsg;
        auto args = cmdargs::from_file(&emsg, is, kwords);

        assert(emsg.empty());
    }
}

/*************************************************************************************************/

static void test_show_help_and_version_00() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

    {
        std::ostringstream os;
        cmdargs::show_help(os, "/test", kwords);

        static const char *expected =
R"(test:
--netsrc=* : "network source name" (std::string, optional, not(--filesrc))
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
)";

        auto str = os.str();
        assert(str == expected);
    }
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
        #pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords.filesrc
            ,kwords.fmode
        );
        static_assert(args.contains(kwords.filesrc) == true);
        static_assert(args.contains(kwords.fmode) == true);

        std::ostringstream os;
        cmdargs::show_help(os, "/test", args);

        static const char *expected =
R"(test:
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
)";

        assert(os.str() == expected);
    }

    // help
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--help"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_help_requested(os, margv[0], args) == true);
        static const char *expected =
R"(cmdargs-test:
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
--help=*   : "show help message" (bool, optional)
)";
        assert(os.str() == expected);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
             "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_help_requested(os, margv[0], args) == false);
        assert(os.str().empty() == true);
    }
    // version
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
            ,kwords.version
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_version_requested(os, margv[0], args) == false);
        assert(os.str().empty() == true);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--version"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords.filesrc
            ,kwords.fmode
            ,kwords.help
            ,kwords.version
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test: version - 0.0.1
)";
        assert(os.str() == expected);
    }

    // help or version
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_help_or_version_requested(os, margv[0], args) == false);
        assert(os.str().empty() == true);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--help"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_help_or_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test:
--netsrc=* : "network source name" (std::string, optional, not(--filesrc))
--filesrc=*: "file source size" (std::string, optional, not(--netsrc))
--fmode=*  : "processing mode" (std::string, required, or(--netsrc, --filesrc))
--help=*   : "show help message" (bool, optional)
--version=*: "show version message" (std::string, optional, default="0.0.1")
)";
        assert(os.str() == expected);
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source size", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
            CMDARGS_OPTION_ADD_HELP();
            CMDARGS_OPTION_ADD_VERSION("0.0.1");
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "/cmdargs-test"
            ,"--filesrc=1.txt"
            ,"--fmode=read"
            ,"--version"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );
        assert(emsg.empty() == true);

        std::ostringstream os;
        assert(cmdargs::is_help_or_version_requested(os, margv[0], args) == true);

        static const char *expected =
R"(cmdargs-test: version - 0.0.1
)";
        assert(os.str() == expected);
    }
}

/*************************************************************************************************/

static void test_predefined_converters() {
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION_ADD(fileslist, std::vector<std::string>, "source files list", optional, not_(netsrc)
                ,convert_as_vector<std::string>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::vector<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION_ADD(fileslist, std::vector<std::size_t>, "source files list", optional, not_(netsrc)
                ,convert_as_vector<std::size_t>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1,2,3"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::vector<std::size_t>{1, 2, 3}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION_ADD(fileslist, std::list<std::string>, "source files list", optional, not_(netsrc)
                ,convert_as_list<std::string>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::list<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            CMDARGS_OPTION_ADD(fileslist, std::set<std::string>, "source files list", optional, not_(netsrc)
                               ,convert_as_set<std::string>()
                               );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1.txt,2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::set<std::string>{"1.txt", "2.txt"}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::string, std::string>;
            CMDARGS_OPTION_ADD(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1.txt,2=2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::map<std::string, std::string>{{"1", "1.txt"}, {"2", "2.txt"}}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::size_t, std::string>;
            CMDARGS_OPTION_ADD(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1.txt,2=2.txt"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::map<std::size_t, std::string>{{1, "1.txt"}, {2, "2.txt"}}));
    }
    {
        struct: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(fileslist));
            using map_type = std::map<std::size_t, std::size_t>;
            CMDARGS_OPTION_ADD(fileslist, map_type, "source files list", optional, not_(netsrc)
                ,convert_as_map<map_type::key_type, map_type::mapped_type>()
            );
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, fileslist));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--fileslist=1=1,2=2"
            ,"--fmode=read"
        };
        int margc = sizeof(margv)/sizeof(margv[0]);
#pragma GCC diagnostic pop

        std::string emsg;
        auto args = cmdargs::parse_args(
             &emsg
            ,margc
            ,margv
            ,kwords
        );

        static_assert(args.contains(kwords.netsrc) == true);
        static_assert(args.contains(kwords.fileslist) == true);
        static_assert(args.contains(kwords.fmode) == true);

        assert(emsg.empty());

        assert(args[kwords.fmode] == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        const auto &fileslist = args[kwords.fileslist];
        assert(fileslist == (std::map<std::size_t, std::size_t>{{1, 1}, {2, 2}}));
    }
}

/*************************************************************************************************/

static void test_as_optionals() {
    struct kwords: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char * const margv[] = {
        "cmdargs-test"
        ,"--netsrc=192.168.1.106"
        ,"--fmode=read"
    };
#pragma GCC diagnostic pop

    std::string emsg;
    const auto [netsrc, filesrc, fmode] = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,margv
        ,kwords
    ).optionals();

    static_assert(std::is_same_v<decltype(netsrc), const std::optional<std::string>>);
    static_assert(std::is_same_v<decltype(filesrc), const std::optional<std::string>>);
    static_assert(std::is_same_v<decltype(fmode), const std::optional<std::string>>);

    assert(emsg.empty());

    assert(!filesrc);

    assert(netsrc);
    assert(netsrc.value() == "192.168.1.106");

    assert(fmode);
    assert(fmode.value() == "read");
}

/*************************************************************************************************/

static void test_as_values() {
    struct kwords: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
        CMDARGS_OPTION_ADD(filesrc, std::string, "file source name", optional, not_(netsrc));
        CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
    } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char * const margv[] = {
        "cmdargs-test"
        ,"--netsrc=192.168.1.106"
        ,"--fmode=read"
    };
#pragma GCC diagnostic pop

    std::string emsg;
    const auto [netsrc, filesrc, fmode] = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,margv
        ,kwords
    ).values();

    static_assert(std::is_same_v<decltype(netsrc), const std::string>);
    static_assert(std::is_same_v<decltype(filesrc), const std::string>);
    static_assert(std::is_same_v<decltype(fmode), const std::string>);

    assert(emsg.empty());

    assert(filesrc.empty());

    assert(!netsrc.empty());
    assert(netsrc == "192.168.1.106");

    assert(!fmode.empty());
    assert(fmode == "read");
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
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source name", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
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

        assert(ss.str() == expected);
    }
    {
        struct kwords: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(netsrc, std::string, "network source name", optional, not_(filesrc));
            CMDARGS_OPTION_ADD(filesrc, std::string, "file source name", optional, not_(netsrc));
            CMDARGS_OPTION_ADD(fmode, std::string, "processing mode", or_(netsrc, filesrc));
        } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
        char * const margv[] = {
            "cmdargs-test"
            ,"--netsrc=192.168.1.106"
            ,"--fmode=read"
        };
#pragma GCC diagnostic pop

        std::string emsg;
        const auto args = cmdargs::parse_args(
             &emsg
            ,std::size(margv)
            ,margv
            ,kwords
        );
        assert(emsg.empty());

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
        assert(ss.str() == expected);
    }
}

/*************************************************************************************************/

#ifdef TEST_MAX_OPTIONS
static void test_max_options() {
    struct: cmdargs::kwords_group {
        CMDARGS_OPTION_ADD(opt0 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt1 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt2 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt3 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt4 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt5 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt6 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt7 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt8 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt9 , std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt10, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt11, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt12, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt13, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt14, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt15, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt16, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt17, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt18, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt19, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt20, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt21, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt22, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt23, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt24, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt25, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt26, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt27, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt28, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt29, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt30, std::size_t, "test option", optional);
        CMDARGS_OPTION_ADD(opt31, std::size_t, "test option", optional);
    } const kwords;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char * const margv[] = {
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
#pragma GCC diagnostic pop

    std::string emsg;
    const auto args = cmdargs::parse_args(
         &emsg
        ,std::size(margv)
        ,margv
        ,kwords
    );
    assert(emsg.empty());
    static_assert(args.size() == 32);

    std::size_t cnt = 0;
    args.for_each([&cnt](const auto &it){
        assert(it.get_value() == cnt);
        ++cnt;
        return true;
    });
    assert(cnt == 32);
}
#endif // TEST_MAX_OPTIONS

/*************************************************************************************************/

#define TEST(func) \
    { std::cout << "test for " #func "..." << std::flush; func(); std::cout << "passed!" << std::endl; }

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
