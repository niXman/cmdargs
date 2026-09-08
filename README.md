[![CI](https://github.com/niXman/cmdargs/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/niXman/cmdargs/actions/workflows/ci.yml)

# Cmdargs
Command-line and config files parsing single header-file library for C++17.

Long option names (`--name=value`), short option names (`-y`, `-h`), and positional arguments are supported.

# The idea
`Cmdargs` this is an attempt to implement the concept of `if the code is successfully compiled - it works correctly` and `no room for error` and `zero boilerplate code`.

# Capabilities
- default values for options
- `optional` and `required` options.
- positional arguments (filled in declaration order).
- short option names via `short_('y')` (`-y`, clustered `-yh`, `-c=6`).
- option relations: `and`, `or`, `not`.
- custom validators and converters.
- predefined converters for `std::vector`/`std::list`/`std::set`/`std::map`

# Command line example

```cpp
// using the whole group

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    auto args = cmdargs::parse_args(&emsg, argc, argv, kwords);
    if ( !emsg.empty() ) {
        std::cout << "cmdline parse error: " << emsg << std::endl;

        return EXIT_FAILURE;
    }

    const auto fname = args.get(kwords.fname);

    assert(args.is_set(kwords.fsize));
    const auto fsize = args.get(kwords.fsize);
}
```
```cpp
// using structure bindings

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    
    // 'fname' - std::optional<std::string>
    // 'fsize' - std::optional<std::size_t>
    const auto [fname, fsize] = cmdargs::parse_args(&emsg, argc, argv, kwords).optionals(); // or `.values()` can be used
    if ( !emsg.empty() ) {
        std::cout << "cmdline parse error: " << emsg << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "fname: is set=" << static_cast<bool>(fname) << ", v=" << fname.value() << std::endl;
    if ( fsize ) {
        std::cout << "fsize: is set=true, v=" << fsize.value() << std::endl;
    } else {
        std::cout << "fsize: is set=false, v=<UNINITIALIZED>" << std::endl;
    }
}
```

```cpp
// using part of the options

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
    CMDARGS_OPTION(fmode, std::string, "processing mode")
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    auto args = cmdargs::parse_args(&emsg, argc, argv, kwords.fname, kwords.fmode);
    if ( !emsg.empty() ) {
        std::cout << "cmdline parse error: " << emsg << std::endl;

        return EXIT_FAILURE;
    }

    const auto fname = args.get(kwords.fname);
    const auto fmode = args.get(kwords.fmode);
}
```

```cpp
// positional arguments and an enum with converter/validator

enum class command {
    install
    ,uninstall
    ,upgrade
    ,plan
    ,update
    ,list
};

struct: cmdargs::kwords_group {
    CMDARGS_OPTION(cmd, command, "command", positional
        ,validator_([](std::string_view str) {
            return str == "install"
                || str == "uninstall"
                || str == "upgrade"
                || str == "plan"
                || str == "update"
                || str == "list"
            ;
        })
        ,converter_([](command &dst, std::string_view str) {
            if ( str == "install" ) { dst = command::install; }
            else if ( str == "uninstall" ) { dst = command::uninstall; }
            else if ( str == "upgrade" ) { dst = command::upgrade; }
            else if ( str == "plan" ) { dst = command::plan; }
            else if ( str == "update" ) { dst = command::update; }
            else if ( str == "list" ) { dst = command::list; }
            else { return false; }

            return true;
        })
    )
    CMDARGS_OPTION(package, std::string, "package name", optional, positional)
    CMDARGS_OPTION(repo, std::string, "repository URL", optional)
    CMDARGS_OPTION(yes, bool, "do not prompt", optional, short_('y'))
    CMDARGS_OPTION_HELP()
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    auto args = cmdargs::parse_args(&emsg, argc, argv, kwords);
    if ( !emsg.empty() ) {
        std::cout << "cmdline parse error: " << emsg << std::endl;

        return EXIT_FAILURE;
    }

    // carbide-install install gdb --repo=https://cdn.example
    const auto cmd = args.get(kwords.cmd);
    if ( args.is_set(kwords.package) ) {
        const auto package = args.get(kwords.package);
    }
}
```


# Config-file example

```cpp
// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    std::ifstream is;
    std::string emsg;
    auto set = cmdargs::from_file(&emsg, is, kwords.fname, kwords.fsize);
    if ( !emsg.empty() ) {
        std::cout << "file parse error: " << emsg << std::endl;

        return EXIT_FAILURE;
    }

    const auto fname = args.get(kwords.fname);
    const auto fsize = args.get(kwords.fsize);
}
```

# Show help message

```cpp
// for all the keywords

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    cmdargs::show_help(std::cout, argv[0], kwords);
}
```
```cpp
// for selected keywords only

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION(fname, std::string, "source file name")
    CMDARGS_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    auto args = cmdargs::parse_args(&emsg, argc, argv, kwords.fname, kwords.fsize);

    cmdargs::show_help(std::cout, argv[0], args);
}
```

# TODO
- to implement check for child options
