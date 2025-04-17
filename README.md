<h1 align="center">
  <br>
  <a href="https://github.com/pawelrapacz/clipper" title="GitHub repo" style="color: white; text-decoration: none;">clipper</a>
  <br><br>
  <a href="https://github.com/pawelrapacz/clipper/releases/latest" title="Latest release"><img src="https://img.shields.io/github/v/release/pawelrapacz/clipper" alt="GitHub Release"></a>
  <a href="https://github.com/pawelrapacz/clipper/blob/master/LICENSE" title="License"><img src="https://img.shields.io/github/license/pawelrapacz/clipper" alt="GitHub License"></a>
</h1>


## Table of contents
- [About](#about)
- [Quick Start](#quick-start)
  - [Get clipper](#get-clipper)
  -  [Usage](#usage)
  -  [Example](#example)
- [Details](#details)
  - [⚠ important](#-important)
  - [clipper](#clipper-class)
  - [flags](#flag-class)
  - [options](#option-class)
  - [predicates](#predicates)
- [Upcoming changes](#upcoming-changes)
- [License](#license)

<br>


## About

Clipper is a simple, header-only library that handles commad line arguments and parsing.  
(This library requires C++20 support)

It is a learning project, that I took on to improve my C++ 📈.
<br>


## Quick Start


### Get clipper

Firstly [download](https://github.com/pawelrapacz/clipper/releases/latest) the library and include `clipper.hpp` in your `.cpp` file. Now just use it!


### Usage

To begin you need to create an instance of `CLI::clipper` class.

```cpp
CLI:clipper
```

Then you can add flags and options.<br>
`set()` gives the variable that an option is going to save the value to (for options you have to give the name of the parameter that it is setting).

```cpp
bool vrbs;
std::filesystem::path inpt;

cli.add_flag("--verbose", "-v")
    .set(vrbs)
    .doc("Displays verbose information");

cli.add_option<std::filesystem::path>("--input", "-i")
    .set("file", inpt)
    .doc("Input file")
    .req();
```

You can use the `match()` or `allow()` function to give viable values for an option. Also with the `set()` function you can set the default value of the option.

```cpp
cli.add_option<std::string>("--encoding", "-e")
    .set("charset", inpt, "utf8")  // utf8 is the default
    .match("utf8", "utf16", "cp1252", "latin1") // could be .allow("utf8", ...)
    .doc("Sets the encoding");
```

The `validate()` or `require()` function allow you to put custom restrictions on option values.
You simply have to set a [predicate](#predicates) function that will check your restrictions (or use a predefined one).
```cpp
uint32_t itrcount;
cli.add_option<uint32_t>("--iteration-count", "-c")
    .set("number", itrcount)
    .validate("[0; 100]", CLI::pred::ibetween<0u, 100u>)
    .doc("Sets the iteration count");

std::string name;
cli.add_option<std::string>("--custom-name", "-n")
    .set("string", name)
    .require("max 10 characters", [](const std::string& s) -> bool { return s.length() <= 10; })
    .doc("Sets custom output name")
    .req();
```

To parse arguments just use `parse()`.

```cpp
cli.parse(argc, argv);
```

Furthermore it is possible to add some information about the program.

```cpp
CLI::clipper cli;
cli
  .name("foo")
  .version("1.0.0")
  .author("me")
  .description("app that does things")
  .license("GPLv3")
  .web_link("https://github.com/pawelrapacz");
```

To display help or version info you can do this:

```cpp
std::cout << cli.make_help();
std::cout << cli.make_version_info();
```
 
The `wrong()` function returns a `const std::vector<std::string>&` that contains parsing errors like:
- Unkonown argument
- Missing required argument
- Missing option value
- Value is not allowed

```cpp
std::cout << cli.wrong().front();
```
<br>


### Example

```cpp
#include "clipper.hpp"
#include <iostream>

int main(CLI::arg_count argc, CLI::args argv) {
    CLI::clipper cli("app", "1.0.0", "", "LGPLv3");

    bool show_help, show_version, flag;
    std::filesystem::path input_file;
    int count;
    double myvalue;
    std::size_t length;

    cli.help_flag("--help", "-h")
        .set(show_help);

    cli.version_flag("--version", "-v")
        .set(show_version)
        .doc("Custom version documentation");

    cli.add_flag("--flag", "-f")
        .set(flag)
        .doc("Sets Flag");

    cli.add_option<std::filesystem::path>("--input", "-i")
        .set("file", input_file)
        .doc("Input file")
        .req();

    cli.add_option<int>("--count", "-c")
        .doc("Sets count")
        .set("number", count, 13)
        .match(1, 2, 3, 13, 14);
    
    cli.add_option<double>("--myvalue")
        .doc("My value")
        .set("value", myvalue, .5)
        .validate("(0; 1)", CLI::between<0., 1.>); // could be: CLI::pred::between<0., 1.>
                                                   // if you want to be specific

    cli.add_option<std::size_t>("--length", "-l")
        .doc("Output length")
        .set("number", length)
        .req()
        .require(">10", CLI::greater_than<10ull>);

    if (not cli.parse(argc, argv)) {
        for (auto& i: cli.wrong())
            std::cout << i << "\n";
        return 1;
    }

    if (show_help) {
        std::cout << cli.make_help();
    }

    if (show_version) {
        std::cout << cli.make_version_info();
    }

    return 0;
}
```
#### Output:

```
> app --help
SYNOPSIS
        app -i <file> -l <number> [...]

FLAGS
        -h, --help            displays help
        -v, --version         Custom version documentation
        -f, --flag            Sets Flag

OPTIONS
        -i, --input <file>    Input file
        -c, --count (1 2 3 13 14)
                              Sets count
        --myvalue <value>     My value (0; 1)
        -l, --length <number> Output length >10

LICENSE
        LGPLv3

> app --version
app 1.0.0


> app -c
[-c] Missing option value
Missing required argument(s) 2

> app --input file.txt -c 17 -f -j --myvalue 0
[-c] Value 17 is not allowed
        { -c, --count (1 2 3 13 14)  Sets count }
[-j] Unkonown argument
[--myvalue] Value 0 is not allowed
        { --myvalue <value>  My value (0; 1) }
Missing required argument(s) 1

>
```
<br>

## Details

For the most detailed documentation see the documentation generated by [doxygen](https://www.doxygen.nl/).

### ⚠ important

- Internally, this utility stores option and flag names as `std::string_view`, so you must ensure that the referenced name values remain valid throughout its lifetime. The most convenient way to do this is to use C-style string literals.
- If an option is repeated, the value is overritten.

### clipper class
This class is practically the only interface of the clipper library,
that is meant to be directly used.
It holds the neccessary and optional information about
the application, options and flags.

| Member                                               | Description                                                    | Return value                                     |
| ---------------------------------------------------- | -------------------------------------------------------------- | ------------------------------------------------ |
| `clipper()`                                          | constructor                                                    |                                                  |
| `clipper(app_name)`                                  | constructor                                                    |                                                  |
| `clipper(app_name, version, author, license_notice)` | constructor                                                    |                                                  |
| `~clipper()`                                         | destructor                                                     |                                                  |
| `name(name)`                                         | sets the name                                                  | `clipper&`                                       |
| `name()`                                             | gets the name                                                  | `std::string_view`                               |
| `description(description)`                           | sets the description                                           | `clipper&`                                       |
| `description()`                                      | gets the description                                           | `std::string_view`                               |
| `version(version)`                                   | sets the version                                               | `clipper&`                                       |
| `version()`                                          | gets the version                                               | `std::string_view`                               |
| `author(author)`                                     | sets the author                                                | `clipper&`                                       |
| `author()`                                           | gets the author                                                | `std::string_view`                               |
| `license(license_notice)`                            | sets the license notice                                        | `clipper&`                                       |
| `license()`                                          | gets the license notice                                        | `std::string_view`                               |
| `web_link(web_link)`                                 | sets the web link                                              | `clipper&`                                       |
| `web_link()`                                         | gets the web link                                              | `std::string_view`                               |
| `add_option<type>(name)`                             | adds a option of a given type                                  | `option&`                                        |
| `add_option<type>(name, alt_name)`                   | adds a option of a given type with an alternative name         | `option&`                                        |
| `add_flag(name)`                                     | adds a flag                                                    | `option<bool>&`                                  |
| `add_flag(name, alt_name)`                           | adds a flag with an alternative name                           | `option<bool>&`                                  |
| `help_flag(name, alt_name = "")`                     | sets the help flag name/names                                  | `option<bool>&`                                  |
| `version_flag(name, alt_name = "")`                  | sets the help flag name/name                                   | `option<bool>&`                                  |
| `make_help()`                                        | returns help page                                              | `std::string`                                    |
| `make_version_info()`                                | returns version information                                    | `std::string`                                    |
| `allow_no_args()`                                    | allows the app to be used without any arguments                | `void`                                           |
| `no_args()`                                          | checks if no arguments were given                              | `bool`                                           |
| `parse(argc, argv)`                                  | parses command line arguments                                  | `bool` (`true` if successful, `false` otherwise) |
| `wrong()`                                            | gets a list of parsing errors                                  | `const std::vector<std::string>&`                |

<br>

### flag class

To be precise it is option<bool> class.

| Member            | Description                                         | Return value         |
| ----------------- | --------------------------------------------------- | -------------------- |
| `option(nm)`      | constructs and sets the name reference              |                      |
| `option(nm, anm)` | constructs and sets the name and alt_name reference |                      |
| `~option()`       | destructor                                          |                      |
| `set(ref)`        | sets the variable to write to                       | `option<bool>&`      |
| `req()`           | sets the flag to be required                        | `option<bool>&`      |
| `doc(doc)`        | sets the flag description                           | `option<bool>&`      |
| `doc()`           | gets the flag description                           | `const std::string&` |

<br>

### option class
It is a template class that allows `integral types`, `floating point types`, `std::string` and `std::filesystem::path` (CLI::option_types concept).


| Member                               | Description                                                                              | Return value         |
| ------------------------------------ | ---------------------------------------------------------------------------------------  | -------------------- |
| `option(nm)`                         | constructs and sets the name reference                                                   |                      |
| `option(nm, anm)`                    | constructs and sets the name and alt_name reference                                      |                      |
| `~option()`                          | destructor                                                                               |                      |
| `set(value_name, ref)`               | sets the variable to write to and the value name (e.g. file, charset)                    | `option&`            |
| `set(value_name, ref, def)`          | sets the variable to write to with default value and the value name (e.g. file, charset) | `option&`            |
| `req()`                              | sets the option to be required                                                           | `option&`            |
| `match(...)` or `allow()`            | sets allowed values                                                                      | `option&`            |
| `validate(doc, pred)` or `require()` | sets a function that validates the value                                                 | `option&`            |
| `doc(doc)`                           | sets the option description                                                              | `option&`            |
| `doc()`                              | gets the option description                                                              | `const std::string&` |

<br>

### predicates
Predicate is a function that allows you to set custom restrictions on option values. These are set with the `validate()` or `require()` methods.

`CLI::pred` namespace contains a set of predefined template predicates to validate numeric options. This namespace is marked as inline so all of predicate functions can be accessed directly trough the CLI namespace.
Type of a predicate is strictly set for each option variant: `bool (*)(const Tp&)` where Tp is the type of the option (template argument).
That`s why when using these predefined predications you have to be precise with the  types of given values:
```cpp
CLI::between<1, 2>;       // int
CLI::ibetween<1ul, 2ul>;  // unsigned long
CLI::less_than<100.0l>;   // long double
CLI::igreater_than<5.0f>; // float
...
```



| Predicate           | Description                                                           |
| ------------------- | --------------------------------------------------------------------- |
| `between<V1 , V2>`  | checks whether a value is between bounds (excludes the bounds         |
| `ibetween<V1 , V2>` | checks whether a value is between bounds (includes the bounds)        |
| `greater_than<V>`   | checks whether a value is greater than a number (excludes the number) |
| `igreater_than<V>`  | checks whether a value is greater than a number (includes the number) |
| `less_than<V>`      | checks whether a value is less than a number (excludes the number))   |
| `iless_than<V>`     | checks whether a value is less than a number (includes the number)    |

<br>

## Upcoming changes
- Positional values
- Option repetition policy
- Subcommands

<br>

## License

This project is licensed under the [MIT License](https://github.com/pawelrapacz/clipper/blob/master/LICENSE).
