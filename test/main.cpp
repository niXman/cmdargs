
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2023 niXman (github dot nixman at pm dot me)
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

template<typename T>
void foo() { std::cout << __PRETTY_FUNCTION__ << std::endl; }

/*************************************************************************************************/

bool has_substring(const std::string &str, const char *substr) {
    return str.find(substr) != std::string::npos;
}

/*************************************************************************************************/

namespace test_templates {

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
constexpr bool contains_and(const std::tuple<Types...> &) {
    return contains<relation_pred_and, char, Types...>::value;
}

template<typename ...Types>
constexpr bool contains_or(const std::tuple<Types...> &) {
    return contains<relation_pred_or, char, Types...>::value;
}

template<typename ...Types>
constexpr bool contains_not(const std::tuple<Types...> &) {
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

static void test_templates() {
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
    static_assert(true == contains_and(tuple));
    static_assert(true == contains_or(tuple));
    static_assert(true == contains_not(tuple));

    constexpr auto tuple2 = std::make_tuple(
         int{}
        ,char{}
        ,float{}
    );
    static_assert(false == contains_and(tuple2));
    static_assert(false == contains_or(tuple2));
    static_assert(false == contains_not(tuple2));

    constexpr auto tuple3 = std::make_tuple(
         relations_list<e_relation_type::AND>{}
        ,char{}
        ,float{}
    );
    static_assert(true  == contains_and(tuple3));
    static_assert(false == contains_or(tuple3));
    static_assert(false == contains_not(tuple3));

    static_assert(true  == has_relation_and(tuple3));

    constexpr auto tuple4 = std::make_tuple(
         int{}
        ,relations_list<e_relation_type::OR>{}
        ,float{}
    );
    static_assert(false == contains_and(tuple4));
    static_assert(true  == contains_or(tuple4));
    static_assert(false == contains_not(tuple4));

    static_assert(true  == has_relation_or(tuple4));

    constexpr auto tuple5 = std::make_tuple(
         int{}
        ,char{}
        ,relations_list<e_relation_type::NOT>{}
    );
    static_assert(false == contains_and(tuple5));
    static_assert(false == contains_or(tuple5));
    static_assert(true  == contains_not(tuple5));

    static_assert(true  == has_relation_not(tuple5));
}

} // ns test_templates

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
        assert(args.get(kwords.fname) == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args.get(kwords.fsize) == 1024);

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
        assert(args.get(kwords.fname) == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args.get(kwords.fsize) == 1024);

        assert(args.is_set(kwords.report));
        assert(args.get(kwords.report) == true);
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
        assert(args.get(kwords.fname) == "1.txt");

        assert(args.is_set(kwords.fsize) == true);
        assert(args.get(kwords.fsize) == 1024);

        assert(args.is_set(kwords.report));
        assert(args.get(kwords.report) == false);
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

        assert(args.get(kwords.fmode) == "read");
        assert(args.get(kwords.netsrc, std::string{"192.168.1.101"}) == "192.168.1.101");
        assert(args.get(kwords.filesrc) == "1.txt");
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

        assert(args.get(kwords.fmode) == "read");
        assert(args.get(kwords.netsrc) == "192.168.1.101");
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

        assert(args.get(kwords.fmode) == "read");
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

        assert(args.get(kwords.fmode) == "write");
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

        assert(args.get(kwords.fmode) == kwords::read);
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

        assert(args.get(kwords.fmode) == kwords::write);
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

static void test_show_help_00() {
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
}

/*************************************************************************************************/

int main(int, char **) {
    test_templates::test_templates();

    test_decl_00();
    test_decl_01();
    test_decl_02();
    test_decl_03();
    test_decl_04();
    test_decl_05();

    test_bool_00();

    test_cond_and_00();

    test_cond_or_00();

    test_cond_not_00();

    test_default_00();

    test_validator_00();

    test_converter_00();

    test_to_file_00();
    test_from_file_00();

    test_show_help_00();

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
