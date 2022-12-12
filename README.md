# Cmdargs
Command-line and config files parsing library

# The idea
`Cmdargs` this is an attempt to implement the concept of `if the code is successfully compiled - it works correctly` and `zero boilerplate code`.

# Capabilities
- `optional` and `required` options.
- option relations: `and`, `or`, `not`.

# Command line example

```cpp
// using the whole group

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name")
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional)
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
// using part of the options

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name")
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional)
    CMDARGS_OPTION_ADD(fmode, std::string, "processing mode")
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


# Config-file example

```cpp
// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name")
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional)
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
    CMDARGS_OPTION_ADD(fname, std::string, "source file name")
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    cmdargs::show_help(std::cout, argv[0], kwords);
}
```
```cpp
// for selected keywords only

// declaring key-words
struct: cmdargs::kwords_group {
    CMDARGS_OPTION_ADD(fname, std::string, "source file name")
    CMDARGS_OPTION_ADD(fsize, std::size_t, "source file size", optional)
} const kwords;

int main(int argc, char **argv) {
    std::string emsg;
    auto args = cmdargs::parse_args(&emsg, argc, argv, kwords.fname, kwords.fsize);

    cmdargs::show_help(std::cout, argv[0], args);
}
```


# TODO
- converters