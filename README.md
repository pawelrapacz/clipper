<h1 align="center">
  <br>
  clipper
  <br><br>
  <a href="https://github.com/pawelrapacz/clipper/releases/latest"><img src="https://img.shields.io/github/v/release/pawelrapacz/clipper" alt="GitHub Release"></a>
  <a href="https://github.com/pawelrapacz/clipper/blob/master/LICENSE"><img src="https://img.shields.io/github/license/pawelrapacz/clipper" alt="GitHub License"></a>
</h1>


## Table of contents
- [About](#about)
- [Quick Start](#quick-start)
  - [Get clipper](#get-clipper)
  -  [Usage](#usage)
  -  [Example](#example)
- [Details](#details)
  - [clipper](#clipper-class)
  - [flags](#flag-class)
  - [options](#option-class)
- [License](#license)

<br>


## About

Clipper is a simple, header-only library that handles commad line arguments and parsing.

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
cli.add_flag("--verbose", "-v")
    .set(f)
    .doc("Displays verbose information");

cli.add_option<std::string>("--input", "-i")
    .set("file", i)
    .doc("Input file")
    .req();
```

You use the `match()` function to give viable values for an option. Alse with the `set()` function you can set the default value of the option.

```cpp
cli.add_option<std::string>("--encoding", "-e")
    .set("charset", i, "utf8")  // utf8 is the default
    .match("utf8", "utf16", "cp1252", "latin1")
    .doc("Sets the encoding");
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

`clipper` class gives access to the `wrong` member which is a `const std::vector<std::string>&` and contains any parsing errors like:
- Unkonown argument
- Missing required argument
- Missing option value
- Value is not allowed

```cpp
std::cout << cli.wrong.front();
```
<br>


### Example

```cpp
#include "clipper.hpp"
#include <iostream>

int main(int argc, char** argv) {
    CLI::clipper cli("app", "1.0.0", "", "LGPLv3");

    bool show_help, show_version, flag;
    std::string input_file;
    int count;

    cli.help_flag("--help", "-h").set(show_help);
    cli.version_flag("--version", "-v").set(show_version);

    cli.add_flag("--flag", "-f")
        .set(flag)
        .doc("Sets Flag");

    cli.add_option<std::string>("--input", "-i")
        .set("file", input_file)
        .doc("Input file")
        .req();

    cli.add_option<int>("--count")
        .doc("Sets count")
        .set("number", count, 13)
        .match(1, 2, 3, 13, 14);
        

    if (not cli.parse(argc, argv) or not cli.wrong.empty()) {
        for (auto& i: cli.wrong)
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




## Details


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
| `name()`                                             | gets the name                                                  | `const std::string&`                             |
| `description(description)`                           | sets the description                                           | `clipper&`                                       |
| `description()`                                      | gets the description                                           | `const std::string&`                             |
| `version(version)`                                   | sets the version                                               | `clipper&`                                       |
| `version()`                                          | gets the version                                               | `const std::string&`                             |
| `author(author)`                                     | sets the author                                                | `clipper&`                                       |
| `author()`                                           | gets the author                                                | `const std::string&`                             |
| `license(license_notice)`                            | sets the license notice                                        | `clipper&`                                       |
| `license()`                                          | gets the license notice                                        | `const std::string&`                             |
| `web_link(web_link)`                                 | sets the web link                                              | `clipper&`                                       |
| `web_link()`                                         | gets the web link                                              | `const std::string&`                             |
| `add_option<type>(name)`                             | adds a option of a given type                                  | `option&`                                        |
| `add_option<type>(name, alt_name)`                   | adds a option of a given type with an alternative name         | `option&`                                        |
| `add_flag(name)`                                     | adds a flag                                                    | `flag&`                                          |
| `add_flag(name, alt_name)`                           | adds a flag with an alternative name                           | `flag&`                                          |
| `help_flag(name, alt_name = "")`                     | sets the help flag name/names                                  | `flag&`                                          |
| `version_flag(name, alt_name = "")`                  | sets the help flag name/name                                   | `flag&`                                          |
| `make_help()`                                        | returns help page                                              | `std::string`                                    |
| `make_version_info()`                                | returns version information                                    | `std::string`                                    |
| `parse(argc, argv)`                                  | parses command line arguments                                  | `bool` (`true` if successful, `false` otherwise) |
| `wrong`                                              | `const std::vector<std::string>&` that contains parsing errors |                                                  |

<br>

### flag class

| Member     | Description                   | Return value         |
| ---------- | ----------------------------- | -------------------- |
| `flag()`   | constructor                   |                      |
| `~flag()`  | destructor                    |                      |
| `set(ref)` | sets the variable to write to | `flag&`              |
| `req()`    | sets the flag to be required  | `flag&`              |
| `doc(doc)` | sets the flag description     | `flag&`              |
| `doc()`    | gets the flag description     | `const std::string&` |

<br>

### option class
It is a template class that allows only: `int`, `float`, `std::string` and `char`.


| Member                      | Description                                                                              | Return value         |
| --------------------------- | ---------------------------------------------------------------------------------------  | -------------------- |
| `option()`                  | constructor                                                                              |                      |
| `~option()`                 | destructor                                                                               |                      |
| `set(value_name, ref)`      | sets the variable to write to and the value name (e.g. file, charset)                    | `option&`            |
| `set(value_name, ref, def)` | sets the variable to write to with default value and the value name (e.g. file, charset) | `option&`            |
| `req()`                     | sets the option to be required                                                           | `option&`            |
| `match(...)`                | sets allowed values                                                                      | `option&`            |
| `doc(doc)`                  | sets the option description                                                              | `option&`            |
| `doc()`                     | gets the option description                                                              | `const std::string&` |

<br>


## License

This project is licensed under the [MIT License](https://github.com/pawelrapacz/clipper/blob/master/LICENSE).
