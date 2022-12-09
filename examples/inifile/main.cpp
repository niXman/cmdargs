
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021-2022 niXman (github dot nixman at pm dot me)
// This file is part of JustArgs(github.com/niXman/cmdargs) project.
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

struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name");
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional);
    CMDARGS_OPTION_ADD_HELP();
    CMDARGS_OPTION_ADD_VERSION();
} const kwords;

/*************************************************************************************************/

static const char *expected = ""
    "# \"source file name\"\n"
    "fname=1.txt\n"
    "# \"source file size\"\n"
    "fsize=1024\n"
;

bool to_file() {
    const auto args = cmdargs::make_args(kwords.fname = "1.txt", kwords.fsize = 1024u);

    std::ostringstream os;

    cmdargs::to_file(os, args);
    auto str = os.str();
    return str == expected;
}

bool from_file() {
    std::istringstream is{expected};

    std::string error_message;
    const auto args = cmdargs::from_file(&error_message, is, kwords.fname, kwords.fsize);

    assert(error_message.empty());
    static_assert (args.size() == 2, "");

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
