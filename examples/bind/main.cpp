
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

#include <justargs/justargs.hpp>

#include <iostream>

/*************************************************************************************************/

struct {
    JUSTARGS_OPTION(fname, std::string, "source file name")
    JUSTARGS_OPTION(fsize, std::size_t, "source file size", optional)
    JUSTARGS_OPTION_HELP()
    JUSTARGS_OPTION_VERSION()
} const kwords;

/*************************************************************************************************/

bool bind_to_vars() {
    std::string fname;
    std::size_t fsize{};

    std::stringstream ss;
    ss
    << "fname=1.txt" << std::endl
    << "fsize=1024" << std::endl;

    bool ok{};
    std::string error_message;
    const auto args = justargs::from_file(
         &ok
        ,&error_message
        ,ss
        ,kwords.fname.bind(&fname)
        ,kwords.fsize.bind(&fsize)
    );
    if ( !ok ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return false;
    }

    assert(args.has(kwords.fname));
    assert(args.is_set(kwords.fname));
    assert(args.get(kwords.fname) == "1.txt");

    assert(args.has(kwords.fsize));
    assert(args.is_set(kwords.fsize));
    assert(args.get(kwords.fsize) == 1024);

    assert(fname == "1.txt");
    assert(fsize == 1024);

    assert(!args.has(kwords.help));
    assert(!args.has(kwords.version));

    return true;
}

/*************************************************************************************************/

bool bind_to_member_functions() {
    struct options_user {
        void on_fname(const std::string &v) { fname = v; }
        void on_fsize(const std::size_t &v) { fsize = v; }
        std::string fname;
        std::size_t fsize{};
    } options_user;

    std::stringstream ss;
    ss
    << "fname=1.txt" << std::endl
    << "fsize=1024" << std::endl;

    bool ok{};
    std::string error_message;
    const auto args = justargs::from_file(
         &ok
        ,&error_message
        ,ss
        ,kwords.fname.bind(&options_user, &options_user::on_fname)
        ,kwords.fsize.bind(&options_user, &options_user::on_fsize)
    );
    if ( !ok ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return false;
    }

    assert(args.has(kwords.fname));
    assert(args.is_set(kwords.fname));
    assert(args.get(kwords.fname) == "1.txt");

    assert(args.has(kwords.fsize));
    assert(args.is_set(kwords.fsize));
    assert(args.get(kwords.fsize) == 1024);

    assert(options_user.fname == "1.txt");
    assert(options_user.fsize == 1024);

    return true;
}

/*************************************************************************************************/

bool bind_to_member_vars() {
    struct options_user {
        std::string fname;
        std::size_t fsize{};
    } options_user;

    std::stringstream ss;
    ss
    << "fname=1.txt" << std::endl
    << "fsize=1024" << std::endl;

    bool ok{};
    std::string error_message;
    const auto args = justargs::from_file(
         &ok
        ,&error_message
        ,ss
        ,kwords.fname.bind(&options_user, &options_user::fname)
        ,kwords.fsize.bind(&options_user, &options_user::fsize)
    );
    if ( !ok ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return false;
    }

    assert(args.has(kwords.fname));
    assert(args.is_set(kwords.fname));
    assert(args.get(kwords.fname) == "1.txt");

    assert(args.has(kwords.fsize));
    assert(args.is_set(kwords.fsize));
    assert(args.get(kwords.fsize) == 1024);

    assert(options_user.fname == "1.txt");
    assert(options_user.fsize == 1024);

    return true;
}

/*************************************************************************************************/

bool bind_to_functional_object() {
    std::string fname;
    std::size_t fsize{};

    std::stringstream ss;
    ss
    << "fname=1.txt" << std::endl
    << "fsize=1024" << std::endl;

    bool ok{};
    std::string error_message;
    const auto args = justargs::from_file(
         &ok
        ,&error_message
        ,ss
        ,kwords.fname.bind([&fname](const std::string &v){ fname = v; })
        ,kwords.fsize.bind([&fsize](const std::size_t &v){ fsize = v; })
    );
    if ( !ok ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return false;
    }

    assert(args.has(kwords.fname));
    assert(args.is_set(kwords.fname));
    assert(args.get(kwords.fname) == "1.txt");

    assert(args.has(kwords.fsize));
    assert(args.is_set(kwords.fsize));
    assert(args.get(kwords.fsize) == 1024);

    assert(fname == "1.txt");
    assert(fsize == 1024);

    return true;
}

/*************************************************************************************************/

int main(int, char *const *) {
    if ( !bind_to_vars() ) {
        std::cout << "bind_to_vars() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !bind_to_member_functions() ) {
        std::cout << "bind_to_member_functions() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !bind_to_member_vars() ) {
        std::cout << "bind_to_member_vars() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !bind_to_functional_object() ) {
        std::cout << "bind_to_functional_object() test error!" << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
