
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

struct kw: justargs::options_group {
    JUSTARGS_OPTION(fname, std::string, "source file name");
    JUSTARGS_OPTION(fsize, std::size_t, "source file size", justargs::optional);
    JUSTARGS_OPTION_HELP();
    JUSTARGS_OPTION_VERSION();
} const kwords;

/*************************************************************************************************/

bool test_assign() {
    auto k = kwords.fname = "12.txt";

    return std::addressof(k) != std::addressof(kwords.fname);
}

/*************************************************************************************************/

bool test_reset() {
    auto k = kwords.fname = "12.txt";

    assert(k.is_set());
    assert(k.get() == "12.txt");

    k.reset();

    return !k.is_set();
}

/*************************************************************************************************/

bool test_reset2() {
    auto k = justargs::make_args(kwords.fname = "1.txt", kwords.fsize = 1024u);

    assert(k.is_set(kwords.fname));
    assert(k.get(kwords.fname) == "1.txt");

    assert(k.is_set(kwords.fsize));
    assert(k.get(kwords.fsize) == 1024);

    k.reset();

    assert(!k.is_set(kwords.fname));
    assert(!k.is_set(kwords.fsize));

    return true;
}

/*************************************************************************************************/

bool test_set() {
    auto k = justargs::make_args(kwords.fname = "1.txt", kwords.fsize = 1024u);

    k.reset();

    k.set(kwords.fname, "1.txt");
    k.set(kwords.fsize, 1024);

    return k.get(kwords.fname) == "1.txt" && k.get(kwords.fsize) == 1024;
}

/*************************************************************************************************/

#define TEST(testname) \
    if ( !testname() ) { \
        std::cout << "\"" #testname "\" test error!" << std::endl; \
        return EXIT_FAILURE; \
    } else { \
        std::cout << "\"" #testname "\" test successfully finished! " << std::endl; \
    }

int main(int, char *const *) {
    TEST(test_assign);
    TEST(test_reset);
    TEST(test_reset2);
    TEST(test_set);

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
