
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

struct third_party_class_real_implementation {
    third_party_class_real_implementation(std::string fname, std::size_t fsize)
        :m_fname{std::move(fname)}
        ,m_fsize{fsize}
    {}

    std::string m_fname;
    std::size_t m_fsize;
};

struct third_party_class_implementation_face {
    template<typename ...Args>
    third_party_class_implementation_face(Args && ...args)
        :m_impl{
             justargs::make_args(std::forward<Args>(args)...).get(kwords.fname)
            ,justargs::make_args(std::forward<Args>(args)...).get(kwords.fsize)
        }
    {}

    third_party_class_real_implementation m_impl;
};

bool funcargs_for_constructor_for_a_third_party_class() {
    const char *fname_0 = "1.txt";
    const std::size_t fsize_0 = 1024;
    third_party_class_implementation_face impl_0{kwords.fname = fname_0, kwords.fsize = fsize_0};

    assert(impl_0.m_impl.m_fname == fname_0);
    assert(impl_0.m_impl.m_fsize == fsize_0);

    //**********************************

    const char *fname_1 = "1.txt";
    const std::size_t fsize_1 = 1024;
    third_party_class_implementation_face impl_1{kwords.fsize = fsize_1, kwords.fname = fname_1};

    assert(impl_1.m_impl.m_fname == fname_1);
    assert(impl_1.m_impl.m_fsize == fsize_1);

    return true;
}

/*************************************************************************************************/

struct own_class_implementation {
    template<typename ...Args>
    own_class_implementation(Args && ...args)
        :m_fname{justargs::make_args(std::forward<Args>(args)...).get(kwords.fname)}
        ,m_fsize{justargs::make_args(std::forward<Args>(args)...).get(kwords.fsize)}
    {}

    std::string m_fname;
    std::size_t m_fsize;
};

bool funcargs_for_constructor_for_own_class() {
    const char *fname_0 = "1.txt";
    const std::size_t fsize_0 = 1024;
    own_class_implementation impl_0{kwords.fname = fname_0, kwords.fsize = fsize_0};

    assert(impl_0.m_fname == fname_0);
    assert(impl_0.m_fsize == fsize_0);

    //**********************************

    const char *fname_1 = "1.txt";
    const std::size_t fsize_1 = 1024;
    own_class_implementation impl_1{kwords.fsize = fsize_1, kwords.fname = fname_1};

    assert(impl_1.m_fname == fname_1);
    assert(impl_1.m_fsize == fsize_1);

    return true;
}

/*************************************************************************************************/

bool third_party_function(const std::string &fname, std::size_t fsize) {
    return fname == "1.txt" && fsize == 1024;
}

template<typename ...Args>
bool own_wrapper_for_a_third_party_function(Args && ...args) {
    const auto set = justargs::make_args(std::forward<Args>(args)...);

    const std::string &fname = set.get(kwords.fname);
    const std::size_t &fsize = set.get(kwords.fsize);

    return third_party_function(fname, fsize);
}

bool funcargs_for_a_third_party_function() {
    assert(own_wrapper_for_a_third_party_function(kwords.fname = "1.txt", kwords.fsize = 1024u) == true);
    assert(own_wrapper_for_a_third_party_function(kwords.fsize = 1024u, kwords.fname = "1.txt") == true);

    return true;
}

/*************************************************************************************************/

template<typename ...Args>
bool own_function(Args && ...args) {
    const auto set = justargs::make_args(std::forward<Args>(args)...);

    const std::string &fname = set.get(kwords.fname);
    const std::size_t &fsize = set.get(kwords.fsize);

    return fname == "1.txt" && fsize == 1024;
}

bool funcargs_for_own_function() {
    assert(own_function(kwords.fname = "1.txt", kwords.fsize = 1024u) == true);
    assert(own_function(kwords.fsize = 1024u, kwords.fname = "1.txt") == true);

    return true;
}

/*************************************************************************************************/

int main(int, char *const *) {
    if ( !funcargs_for_constructor_for_a_third_party_class() ) {
        std::cout << "funcargs_for_constructor_for_a_third_party_class() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !funcargs_for_constructor_for_own_class() ) {
        std::cout << "funcargs_for_constructor_for_own_class() test error!" << std::endl;

        return EXIT_FAILURE;
    }

    if ( !funcargs_for_a_third_party_function() ) {
        std::cout << "funcargs_for_a_third_party_function() test error!" << std::endl;

        return EXIT_FAILURE;
    }
    if ( !funcargs_for_own_function() ) {
        std::cout << "funcargs_for_own_function() test error!" << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*************************************************************************************************/
