
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
#include <cassert>

/*************************************************************************************************/

struct: justargs::options_group {
    JUSTARGS_OPTION(fname, std::string, "source file name");
    JUSTARGS_OPTION(fsize, std::size_t, "source file size", justargs::optional);
    JUSTARGS_OPTION_HELP();
    JUSTARGS_OPTION_VERSION();
} const kwords;

/*************************************************************************************************/

bool to_file() {
    const auto args = justargs::make_args(kwords.fname = "1.txt");

    std::ostringstream os;

    justargs::to_file(os, args);

    static const char *expected = ""
        "# source file name\n"
        "fname=1.txt\n"
    ;

    return os.str() == expected;
}

bool from_file() {
    static const char *file = ""
        "# source file size\n"
        "fsize=1024\n"
    ;
    std::istringstream is{file};

    bool ok{};
    std::string error_message;
    const auto args = justargs::from_file(
         &ok
        ,&error_message
        ,is
        ,kwords.fname.default_("1.txt")
        ,kwords.fsize
    );
    static_assert(args.size() == 2, "");
    assert(error_message.empty());
    assert(ok == true);

    return args.get(kwords.fname) == "1.txt" && args.get(kwords.fsize) == 1024;
}

/*************************************************************************************************/

int main(int, char *const *) {
    if ( !to_file() ) {
        std::cout << "to_file() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !from_file() ) {
        std::cout << "from_file() test error!" << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
