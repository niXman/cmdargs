
// MIT License
//
// Copyright (c) 2021-2022 niXman (github dot nixman at pm dot me)
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

/*************************************************************************************************/

bool has_substring(const std::string &str, const char *substr) {
    return std::strstr(str.data(), substr) != nullptr;
}

/*************************************************************************************************/

static void test_decl_00() {
    struct kwords: cmdargs::kwords_group {
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

static void test_decl_01() {
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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

        static_assert(args.has(kwords.fname) == true);
        static_assert(args.has(kwords.fsize) == true);
        static_assert(args.has(kwords.report) == true);

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

        static_assert(args.has(kwords.fname) == true);
        static_assert(args.has(kwords.fsize) == true);
        static_assert(args.has(kwords.report) == true);

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

        static_assert(args.has(kwords.fname) == true);
        static_assert(args.has(kwords.fsize) == true);
        static_assert(args.has(kwords.report) == true);

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
        struct kwords: cmdargs::kwords_group {
            CMDARGS_OPTION_ADD(fname, std::string, "source file name", optional);
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
            assert(has_substring(emsg, "together with \"--fname\""));
        }
    }
    {
        struct kwords: cmdargs::kwords_group {
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
            assert(has_substring(emsg, "together with \"--fname\""));
        }
    }
    {
        struct kwords: cmdargs::kwords_group {
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
        struct kwords: cmdargs::kwords_group {
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
            assert(has_substring(emsg, "together with \"--fname\""));
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
            assert(has_substring(emsg, "together with \"--fsize\""));
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
    struct kwords: cmdargs::kwords_group {
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
    struct kwords: cmdargs::kwords_group {
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
        assert(has_substring(emsg, "together with one of \"--netsrc\", \"--filesrc\""));
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
        assert(has_substring(emsg, "together with one of \"--netsrc\", \"--filesrc\""));
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

static void test_to_file_00() {
    struct kwords: cmdargs::kwords_group {
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
R"(# "network source name"
netsrc=192.168.1.106
# "processing mode"
fmode=read
)";
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
R"(# "file source name"
filesrc=1.txt
# "processing mode"
fmode=read
)";
        assert(os.str() == expected);
    }
}

/*************************************************************************************************/

static void test_from_file_00() {
    struct kwords: cmdargs::kwords_group {
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

        static_assert(args.has(kwords.netsrc) == true, "");
        static_assert(args.has(kwords.filesrc) == true, "");
        static_assert(args.has(kwords.fmode) == true, "");

        assert(!emsg.empty());
        assert(has_substring(emsg, "together with one of \"netsrc\", \"filesrc\""));
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
        assert(has_substring(emsg, "together with one of \"netsrc\", \"filesrc\""));
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
    struct kwords: cmdargs::kwords_group {
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
        auto args = cmdargs::parse_args(&emsg, margc, margv, kwords.filesrc, kwords.fmode);
        static_assert(args.has(kwords.filesrc) == true);
        static_assert(args.has(kwords.fmode) == true);

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

    test_to_file_00();
    test_from_file_00();

    test_show_help_00();

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
