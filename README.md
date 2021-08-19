# justargs
command line/config-files and function named arguments library

# command line example

```cpp
int main(int argc, char **argv) {
    // declaring key-words
    struct kwords: justargs::kwords_group {
        JUSTARGS_ADD_OPTION(fname, std::string, "source file name")
        JUSTARGS_ADD_OPTION(fsize, std::size_t, "source file size", optional)
    } const kwords;

    bool ok{};
    std::string emsg{};
    auto args = justargs::parse_args(&ok, &emsg, argc, argv, kwords.fname, kwords.fsize);
    if ( !ok ) {
        std::cout << "args parse error: " << emsg << std::endl;
        
        return EXIT_FAILURE;
    }
    
    assert(args.is_set(kwords.fname));
    const auto fname = args.get(kwords.fname);
    
    assert(args.is_set(kwords.fsize));
    const auto fsize = args.get(kwords.fsize);
}
```

# config-file example

```cpp
int main(int argc, char **argv) {
    // declaring key-words
    struct kwords: justargs::kwords_group {
        JUSTARGS_ADD_OPTION(fname, std::string, "source file name")
        JUSTARGS_ADD_OPTION(fsize, std::size_t, "source file size", optional)
    } const kwords;

    bool ok{};
    std::string emsg{};
    auto set = justargs::from_file(&ok, &emsg, ss, kwords.fname, kwords.fsize);
    if ( !ok ) {
        std::cout << "file parse error: " << emsg << std::endl;
        
        return EXIT_FAILURE;
    }
    
    assert(args.is_set(kwords.fname));
    const auto fname = args.get(kwords.fname);
    
    assert(args.is_set(kwords.fsize));
    const auto fsize = args.get(kwords.fsize);
}
```

#named function args example

```cpp
// declaring key-words
struct kwords: justargs::kwords_group {
    JUSTARGS_ADD_OPTION(fname, std::string, "source file name")
    JUSTARGS_ADD_OPTION(fsize, std::size_t, "source file size", optional)
} const kwords;

// the real function
int real_func(const std::string &fname, std::size_t fsize) {
    
}

// the proxy function
template<typename ...Args>
int proxy_func(Args && ...args) {
    const auto set = justargs::make_args(std::forward<Args>(args)...);
    return real_func(set.get(kwords.fname), set.get(kwords.fsize));
}

int main() {
    int res = proxy_func(kwords.fname = "file.txt", kwords.fsize = 1024);
    res = proxy_func(kwords.fsize = 1024, kwords.fname = "file.txt");
    
    return res;
}
```

# TODO
tests! tests! tests!