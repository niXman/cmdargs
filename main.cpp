
// ----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2021 niXman (github dot nixman at pm dot me)
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

#include <sstream>
#include <iostream>

/*************************************************************************************************/

struct kwords: justargs::kwords_group {
    JUSTARGS_ADD_KEYWORD(fname, std::string, "source file name")
    JUSTARGS_ADD_KEYWORD(fsize, std::size_t, "source file size")
    JUSTARGS_ADD_KEYWORD(radius, double, "radius as double", optional)
    JUSTARGS_ADD_KEYWORD(radius2, float, "radius as float", optional)
    JUSTARGS_ADD_KEYWORD(poss, bool, "positive values only?", optional)
    JUSTARGS_ADD_KEYWORD(ptr , int*, "the result of jod", optional)
} const kwords;

/*************************************************************************************************/

void myfunc_impl(const std::string &fname, std::size_t fsize, int *ptr) {
    std::cout << "myfunc_impl(fname=" << fname << ", fsize=" << fsize << ", *ptr=" << *ptr << ")" << std::endl;
    *ptr = 44;
}

template<typename ...Args>
void myfunc(Args && ...args) {
    const auto set = justargs::make_args(std::forward<Args>(args)...);
    return myfunc_impl(set.get(kwords.fname), set.get(kwords.fsize), set.get(kwords.ptr));
}

/*************************************************************************************************/

int main(int argc, char **argv) {
    auto set = justargs::make_args(
         kwords.fname = "file.txt"
        ,kwords.fsize = 1024
        ,kwords.radius = 3.14156
        ,kwords.radius2 = 3.14156f
        ,kwords.poss = true
        ,kwords.ptr
    );

    assert(set.has(kwords.fname));
    assert(set.has(kwords.fsize));
    assert(set.has(kwords.radius));
    assert(set.has(kwords.radius2));
    assert(set.has(kwords.poss));
    assert(set.has(kwords.ptr));

    assert(set.is_set(kwords.fname));
    assert(set.is_set(kwords.fsize));
    assert(set.is_set(kwords.radius));
    assert(set.is_set(kwords.radius2));
    assert(set.is_set(kwords.poss));
    assert(!set.is_set(kwords.ptr));

    assert(set.get(kwords.fname) == "file.txt");
    assert(set.get(kwords.fname, "file.txt") == "file.txt");
    assert(set.get(kwords.fname, std::string("file.txt")) == "file.txt");

    std::string fname;
    set.bind(kwords.fname, fname);

    set.set(kwords.fname, std::string{"file1.txt"});
    assert(fname == "file1.txt");
    assert(set.get(kwords.fname) == "file1.txt");

    std::cout << "settings::for_each():" << std::endl;
    set.for_each(
        [](const auto &t, const auto &v){ std::cout << "0: " << t.name() << ": " << v << std::endl; }
    );

    set.for_each(
         false
        ,[](decltype(kwords.fname)  &t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
        ,[](decltype(kwords.fsize)  &t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
        ,[](decltype(kwords.radius) &t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
        ,[](decltype(kwords.radius2)&t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
        ,[](decltype(kwords.poss)   &t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
        ,[](decltype(kwords.ptr)    &t, const auto &v){ std::cout << "1: " << t.name() << ": " << v << std::endl; }
    );

    std::cout << std::endl << std::endl;

    std::stringstream ss;
    std::cout << "to file:" << std::endl;
    to_file(ss, set);
    std::cout << ss.str() << std::endl;
    std::cout << std::endl << std::endl;

    set.reset(kwords.fname, kwords.fsize, kwords.radius, kwords.radius2, kwords.poss);
    set.reset();

    std::cout << "to file2:" << std::endl;
    to_file(std::cout, set);
    std::cout << std::endl << std::endl;

    {
        std::cout << "from file:" << std::endl;
        bool ok = false;
        std::string errmsg;
        from_file(&ok, &errmsg, ss, set);
        assert(ok);
        assert(errmsg.empty());
        std::cout << std::endl << std::endl;
    }

    std::cout << "to file3:" << std::endl;
    to_file(std::cout, set);
    std::cout << std::endl << std::endl;

    std::cout << "myfunc():" << std::endl;
    int vv = 33;
    myfunc(kwords.fname = "fname-1", kwords.fsize = 1024, kwords.ptr = &vv);
    myfunc(kwords.ptr = &vv, kwords.fsize = 1024*2, kwords.fname = "fname-2");
    std::cout << std::endl << std::endl;

    {
        bool ok = false;
        std::string emsg;
        auto args = justargs::parse_args(&ok, &emsg, argc, argv, kwords.fname, kwords.fsize, kwords.poss, kwords.ptr);
        if ( !ok ) {
            std::cout << "parse args test 1 error: " << emsg << std::endl;
        } else {
            const std::string fname = args.get(kwords.fname);
            const std::size_t fsize = args.get(kwords.fsize);
            const bool fpos = args.get(kwords.poss);

            justargs::to_file(std::cout, args);
        }
    }
    {
        justargs::args<kwords::fname_t, kwords::fsize_t, kwords::poss_t> set{
             kwords.fname = "file1.txt"
            ,kwords.fsize = 1032
            ,kwords.poss = true
        };

        std::stringstream ss;
        justargs::to_file(ss, set);
        std::cout << "file:\n" << ss.str() << std::endl;
        set.reset();

        std::cout << "after reset:\n" << set << std::endl;

        bool ok = false;
        std::string emsg;
        auto set2 = justargs::from_file(&ok, &emsg, ss, kwords.fname, kwords.fsize, kwords.poss);

        std::cout << "after from_file:\n";
        std::cout << set2 << std::endl;

        justargs::show_help(std::cout, argv[0], set2);
    }
}

/*************************************************************************************************/
