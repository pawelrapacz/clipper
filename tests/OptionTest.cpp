#include <gtest/gtest.h>
#include "clipper.hpp"
using namespace CLI;

class OptionTest : public testing::Test {
protected:
    OptionTest()
      : num("-n"), dbl("-d"), path("--path", "-p"), str("-s"), ch("--char", "-c"), flag("-f")
    {
        num.set("number", num_v, 11);
        dbl.set("fnumber", dbl_v, 11);
        path.set("path", path_v, "mypath.txt");
        str.set("string", str_v, "mystring");
        ch.set("char", ch_v, 'a');
        flag.set(flag_v);
    }

    void SetMatchRequirements() {
        num.match(1, 2, 11, 10.3, 20);
        dbl.allow(1, 2, 11, 10.3, 20);
        ch.match('a', 'b', 'c');
        path.allow("a.txt", "b.txt", "c.txt");
        str.match("a.txt", "b.txt", "c.txt");
    }

    void SetPredicateRequirements() {
        num.require("", ibetween<0, 10>);
        dbl.require("", ibetween<0., 10.>);
        ch.require("", [](const char& p) -> bool {return p >= 'a' && p <= 'z';});
        path.require("", [](const std::filesystem::path& p) -> bool {return p.is_relative();});
        str.require("", [](const std::string& p) -> bool {return p.length() < 5;});
    }

    option<int> num;
    int num_v;

    option<double> dbl;
    double dbl_v;

    option<std::filesystem::path> path;
    std::filesystem::path path_v;

    option<std::string> str;
    std::string str_v;

    option<char> ch;
    char ch_v;

    option<bool> flag;
    bool flag_v;
};


TEST_F(OptionTest, DefaultValueSetting) {
    EXPECT_EQ(num_v, 11);
    EXPECT_EQ(dbl_v, 11.);
    EXPECT_EQ(ch_v, 'a');
    EXPECT_EQ(flag_v, false);
    EXPECT_EQ(path_v, "mypath.txt");
    EXPECT_EQ(str_v, "mystring");
}


TEST_F(OptionTest, ValueInfo) {
    EXPECT_EQ(num.value_info(), "<number>");
    EXPECT_EQ(dbl.value_info(), "<fnumber>");
    EXPECT_EQ(ch.value_info(), "<char>");
    EXPECT_EQ(path.value_info(), "<path>");
    EXPECT_EQ(str.value_info(), "<string>");
    EXPECT_EQ(flag.value_info(), "");

    SetMatchRequirements();

    EXPECT_EQ(num.value_info(), "(1 2 11 10 20)");
    EXPECT_EQ(dbl.value_info(), "(1 2 11 10.3 20)");
    EXPECT_EQ(ch.value_info(), "(a b c)");
    EXPECT_EQ(path.value_info(), "(a.txt b.txt c.txt)");
    EXPECT_EQ(str.value_info(), "(a.txt b.txt c.txt)");
    EXPECT_EQ(flag.value_info(), "");
}


TEST_F(OptionTest, Synopsis) {
    EXPECT_EQ(num.synopsis(), "-n <number>");
    EXPECT_EQ(dbl.synopsis(), "-d <fnumber>");
    EXPECT_EQ(ch.synopsis(), "-c <char>");
    EXPECT_EQ(path.synopsis(), "-p <path>");
    EXPECT_EQ(str.synopsis(), "-s <string>");
    EXPECT_EQ(flag.synopsis(), "-f ");

    EXPECT_EQ(num.detailed_synopsis(), "-n <number>");
    EXPECT_EQ(dbl.detailed_synopsis(), "-d <fnumber>");
    EXPECT_EQ(ch.detailed_synopsis(), "-c, --char <char>");
    EXPECT_EQ(path.detailed_synopsis(), "-p, --path <path>");
    EXPECT_EQ(str.detailed_synopsis(), "-s <string>");
    EXPECT_EQ(flag.detailed_synopsis(), "-f ");

    SetMatchRequirements();

    EXPECT_EQ(num.synopsis(), "-n (1 2 11 10 20)");
    EXPECT_EQ(dbl.synopsis(), "-d (1 2 11 10.3 20)");
    EXPECT_EQ(ch.synopsis(), "-c (a b c)");
    EXPECT_EQ(path.synopsis(), "-p (a.txt b.txt c.txt)");
    EXPECT_EQ(str.synopsis(), "-s (a.txt b.txt c.txt)");
    EXPECT_EQ(flag.synopsis(), "-f ");

    EXPECT_EQ(num.detailed_synopsis(), "-n (1 2 11 10 20)");
    EXPECT_EQ(dbl.detailed_synopsis(), "-d (1 2 11 10.3 20)");
    EXPECT_EQ(ch.detailed_synopsis(), "-c, --char (a b c)");
    EXPECT_EQ(path.detailed_synopsis(), "-p, --path (a.txt b.txt c.txt)");
    EXPECT_EQ(str.detailed_synopsis(), "-s (a.txt b.txt c.txt)");
    EXPECT_EQ(flag.detailed_synopsis(), "-f ");
}


TEST_F(OptionTest, ValueAssignment) {
    using namespace std::string_view_literals;
    num = "10";
    dbl = "10.3";
    ch = "abecadło";
    path = "abecadło.txt"sv;
    str = "abecadło"sv;
    flag = "abecadło";

    EXPECT_EQ(num_v, 10);
    EXPECT_EQ(dbl_v, 10.3);
    EXPECT_EQ(ch_v, 'a');
    EXPECT_EQ(path_v, "abecadło.txt");
    EXPECT_EQ(str_v, "abecadło");
    EXPECT_EQ(flag_v, true);
}


TEST_F(OptionTest, ValueMatchValidation) {
    SetMatchRequirements();
    EXPECT_NO_THROW(num = "1";);  EXPECT_EQ(num_v, 1);
    EXPECT_NO_THROW(num = "2";);  EXPECT_EQ(num_v, 2);
    EXPECT_NO_THROW(num = "11";); EXPECT_EQ(num_v, 11);
    EXPECT_NO_THROW(num = "10";); EXPECT_EQ(num_v, 10);
    EXPECT_NO_THROW(num = "20";); EXPECT_EQ(num_v, 20);
    EXPECT_ANY_THROW(num = "-1";);
    EXPECT_ANY_THROW(num = "111";);
    EXPECT_ANY_THROW(num = "abc";);
    EXPECT_ANY_THROW(num = "AbC";);
    EXPECT_ANY_THROW(num = "-AbC";);

    EXPECT_NO_THROW(dbl = "1";);    EXPECT_EQ(dbl_v, 1);
    EXPECT_NO_THROW(dbl = "2";);    EXPECT_EQ(dbl_v, 2);
    EXPECT_NO_THROW(dbl = "11";);   EXPECT_EQ(dbl_v, 11);
    EXPECT_NO_THROW(dbl = "10.3";); EXPECT_EQ(dbl_v, 10.3);
    EXPECT_NO_THROW(dbl = "20";);   EXPECT_EQ(dbl_v, 20);
    EXPECT_ANY_THROW(dbl = "-1";);
    EXPECT_ANY_THROW(dbl = "10";);
    EXPECT_ANY_THROW(dbl = "abc";);
    EXPECT_ANY_THROW(dbl = "AbC";);
    EXPECT_ANY_THROW(dbl = "-AbC";);

    EXPECT_NO_THROW(ch = "a";);   EXPECT_EQ(ch_v, 'a');
    EXPECT_NO_THROW(ch = 'b';);    EXPECT_EQ(ch_v, 'b');
    EXPECT_NO_THROW(ch = "cstring";);   EXPECT_EQ(ch_v, 'c');
    EXPECT_ANY_THROW(ch = "11";);
    EXPECT_ANY_THROW(ch = 'd';);
    EXPECT_ANY_THROW(ch = "AbC";);
    EXPECT_ANY_THROW(ch = "-AbC";);

    using namespace std::string_literals;
    using namespace std::string_view_literals;

    EXPECT_NO_THROW(path = "a.txt"sv;); EXPECT_EQ(path_v, "a.txt");
    EXPECT_NO_THROW(path = "b.txt"sv;); EXPECT_EQ(path_v, "b.txt");
    EXPECT_NO_THROW(path = "c.txt"sv;); EXPECT_EQ(path_v, "c.txt");
    EXPECT_ANY_THROW(path = "mypath.txt"sv;);
    EXPECT_ANY_THROW(path = "aa"sv;);
    EXPECT_ANY_THROW(path = "abecadło"sv;);

    EXPECT_NO_THROW(str = "a.txt"s;); EXPECT_EQ(str_v, "a.txt");
    EXPECT_NO_THROW(str = "b.txt"sv;); EXPECT_EQ(str_v, "b.txt");
    EXPECT_NO_THROW(str = "c.txt"sv;); EXPECT_EQ(str_v, "c.txt");
    EXPECT_ANY_THROW(str = "mystring"s;);
    EXPECT_ANY_THROW(str = "aa"s;);
    EXPECT_ANY_THROW(str = "abecadło"sv;);
}


TEST_F(OptionTest, ValuePredicateValidation) {
    SetPredicateRequirements();

    EXPECT_NO_THROW(num = "1";);  EXPECT_EQ(num_v, 1);
    EXPECT_NO_THROW(num = "5";);  EXPECT_EQ(num_v, 5);
    EXPECT_NO_THROW(num = "10";); EXPECT_EQ(num_v, 10);
    EXPECT_ANY_THROW(num = "-1";);
    EXPECT_ANY_THROW(num = "11";);

    EXPECT_NO_THROW(dbl = "1";);  EXPECT_EQ(dbl_v, 1.);
    EXPECT_NO_THROW(dbl = "5";);  EXPECT_EQ(dbl_v, 5);
    EXPECT_NO_THROW(dbl = "10";); EXPECT_EQ(dbl_v, 10.);
    EXPECT_ANY_THROW(dbl = "-1";);
    EXPECT_ANY_THROW(dbl = "11";);

    EXPECT_NO_THROW(ch = "a";);   EXPECT_EQ(ch_v, 'a');
    EXPECT_NO_THROW(ch = 'b';);    EXPECT_EQ(ch_v, 'b');
    EXPECT_NO_THROW(ch = "string";);   EXPECT_EQ(ch_v, 's');
    EXPECT_ANY_THROW(ch = "Abc";);
    EXPECT_ANY_THROW(ch = 'Z';);
    EXPECT_ANY_THROW(ch = "100";);

    using namespace std::string_literals;
    using namespace std::string_view_literals;

    EXPECT_NO_THROW(path = "a.txt"sv;); EXPECT_EQ(path_v, "a.txt");
    EXPECT_NO_THROW(path = "b.txt"sv;); EXPECT_EQ(path_v, "b.txt");
    EXPECT_ANY_THROW(path = "C:/mypath.txt"sv;);
    EXPECT_ANY_THROW(path = "C:/Users/user/abecadło"sv;);

    EXPECT_NO_THROW(str = "aa"s;); EXPECT_EQ(str_v, "aa");
    EXPECT_NO_THROW(str = "abc"sv;); EXPECT_EQ(str_v, "abc");
    EXPECT_ANY_THROW(str = "mystring"s;);
    EXPECT_ANY_THROW(str = "abecadło"sv;);
}