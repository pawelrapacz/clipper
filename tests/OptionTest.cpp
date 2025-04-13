#include <gtest/gtest.h>
#include "clipper.hpp"
using namespace CLI;

class OptionTest : public testing::Test {
protected:
    OptionTest()
      : num(""), dbl(""), path(""), str(""), ch(""), flag("")
    {
        num.set("number", num_v, 11);
        dbl.set("fnumber", dbl_v, 11);
        path.set("path", path_v, "mypath.txt");
        str.set("string", str_v, "mystring");
        ch.set("char", ch_v, 'a');
        flag.set(flag_v);
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
    // EXPECT_EQ(flag.value_info(), "");
    EXPECT_EQ(path.value_info(), "<path>");
    EXPECT_EQ(str.value_info(), "<string>");

    num.match(1, 2, 11, 10.3, 20);
    dbl.allow(1, 2, 11, 10.3, 20);
    ch.match('a', 'b', 'c');
    path.allow("a.txt", "b.txt", "c.txt");
    str.match("a.txt", "b.txt", "c.txt");

    EXPECT_EQ(num.value_info(), "(1 2 11 10 20)");
    EXPECT_EQ(dbl.value_info(), "(1 2 11 10.3 20)");
    EXPECT_EQ(ch.value_info(), "(a b c)");
    // EXPECT_EQ(flag.value_info(), "")
    EXPECT_EQ(path.value_info(), "(a.txt b.txt c.txt)");
    EXPECT_EQ(str.value_info(), "(a.txt b.txt c.txt)");
}