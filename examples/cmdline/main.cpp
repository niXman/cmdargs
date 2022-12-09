
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

enum e_mode { read, write };

struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name");
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional);
    CMDARGS_OPTION_ADD(fmode, e_mode     , "source file mode", optional);
    CMDARGS_OPTION_ADD_HELP();
    CMDARGS_OPTION_ADD_VERSION();
} const kwords;

int main(int argc, char *const *argv) {
    std::string error_message;
    const auto args = cmdargs::parse_args(
         &error_message
        ,argc
        ,argv
        ,kwords.fname
        ,kwords.fsize
        ,kwords.fmode
        ,kwords.help
        ,kwords.version
    );
    if ( !error_message.empty() ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return EXIT_FAILURE;
    }
    args.dump(std::cout, false);

    if ( args.is_set(kwords.help) ) {
        cmdargs::show_help(std::cout, argv[0], args);

        return EXIT_SUCCESS;
    }
    if ( args.is_set(kwords.version) ) {
        std::cout << "my version 0.0.1" << std::endl;

        return EXIT_SUCCESS;
    }

    const auto &fname = args.get(kwords.fname);
    std::cout << "fname=" << fname << std::endl;
    if ( args.is_set(kwords.fsize) ) {
        std::cout << "fsize=" << args.get(kwords.fsize) << std::endl;
    }

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
