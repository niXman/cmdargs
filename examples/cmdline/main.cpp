
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

/*************************************************************************************************/

enum e_mode { read, write };

std::ostream& operator<< (std::ostream &os, e_mode mode) {
    os << (mode == e_mode::read ? "read" : "write");
    return os;
}

struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name");
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional);
    CMDARGS_OPTION(fmode, e_mode     , "source file mode", optional
        ,validator_([](const std::string_view str){
            return str == "read" || str == "write";
        })
        ,converter_([](e_mode &dst, const std::string_view str){
            dst = (str == "read" ? e_mode::read : e_mode::write);
            return true;
        })
    );

    CMDARGS_OPTION(dst_ip, std::string, "the destination IP address", optional);
    CMDARGS_OPTION(dst_port, std::uint16_t, "the destination port number", optional);
    CMDARGS_OPTION(retransmit, bool, "retransmit the file over network?", optional, and_(dst_ip, dst_port));

    CMDARGS_OPTION_HELP();
    CMDARGS_OPTION_VERSION("0.0.1");
} const kwords;

int main(int argc, char *const *argv) {
    std::string error_message;
    const auto [fname, fsize, fmode, dst_ip, dst_port, retransmit, help_req, version_req]
        = cmdargs::parse_args(&error_message, argc, argv, kwords).optionals();
    if ( !error_message.empty() ) {
        std::cerr << "command line parse error: " << error_message << std::endl;

        return EXIT_FAILURE;
    }
    //args.dump(std::cout, false);

    if ( help_req ) {
        cmdargs::show_help(std::cout, argv[0], kwords);
        return EXIT_SUCCESS;
    }
    if ( version_req ) {
        std::cout << "version: " << version_req.value() << std::endl;
        return EXIT_SUCCESS;
    }

    std::cout << "fname=" << fname.value() << std::endl;
    if ( fsize ) { std::cout << "fsize=" << fsize.value() << std::endl; }
    if ( fmode ) { std::cout << "fmode=" << fmode.value() << std::endl; }
    if ( dst_ip ) { std::cout << "dst_ip=" << dst_ip.value() << std::endl; }
    if ( dst_port ) { std::cout << "dst_port=" << dst_port.value() << std::endl; }
    if ( retransmit ) { std::cout << "retransmit=" << retransmit.value() << std::endl; }

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
