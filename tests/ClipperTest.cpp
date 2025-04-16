#include <gtest/gtest.h>
#include "clipper.hpp"
using namespace CLI;

class ClipperTest : public testing::Test {
protected:
    ClipperTest() {
        cli.add_option<std::string>("--input", "-i").set("", i_v).req();
        cli.add_option<std::filesystem::path>("--output", "-o").set("", o_v).req();
        cli.add_option<int>("--count", "-c").set("", c_v).req();        
        cli.add_flag("--flag", "-f").set(f_v).req();

        cli.add_option<std::string>("--name", "-n").set("", n_v);
        cli.add_option<std::string>("--encoding", "-e").set("", e_v);
        cli.add_option<double>("--myvalue", "-m").set("", m_v);
        cli.add_option<std::size_t>("-l").set("", l_v);
        cli.add_flag("--verbose", "-v").set(f_v);
        cli.add_flag("-s").set(s_v);
        cli.add_flag("-h").set(h_v);
    }

    std::string ParsingWrong() {
        std::string joined;
        for (auto& i : cli.wrong())
            joined += i + '\n';
        return joined;
    }
    
    bool f_v, v_v, s_v, h_v;
    std::string i_v, n_v, e_v;
    std::filesystem::path o_v;
    int c_v;
    double m_v;
    std::size_t l_v;

    clipper cli;

    constexpr static const char* empty[] = { "app", nullptr };

    // G is good E is error
    constexpr static const char* setG1[] = { // only required options
        "app",
        "-i", "input.txt",
        "-o", "output.txt",
        "--count", "10",
        "-f",
        nullptr
    };
    constexpr static arg_count sizeG1 = 8;

    constexpr static const char* setG2[] = { // some options repeated
        "app",
        "-i", "input.txt",
        "-o", "output.txt",
        "-o", "output2.txt",
        "--count", "10",
        "--count", "145",
        "-f",
        "-h",
        nullptr
    };
    constexpr static arg_count sizeG2 = 13;

    constexpr static const char* setG3[] = { // all options (and repeated)
        "app",
        "-e", "latin1",
        "--input", "input.txt",
        "-h",
        "--flag",
        "-o", "output.txt",
        "-i", "input2.txt",
        "-n", "cba",
        "--count", "145",
        "-l", "1034",
        "-s",
        "-f",
        "-m", "304.45",
        "-o", "output2.txt",
        "--verbose",
        "--count", "10",
        "-f",
        "--name", "abc",
        "--encoding", "utf8",
        "-v",
        "-l", "134",
        nullptr
    };
    constexpr static arg_count sizeG3 = 34;

    constexpr static const char* setE1[] = { // no required options
        "app",
        "-n", "aa",
        "-h",
        "--myvalue", "10.3",
        nullptr
    };
    constexpr static arg_count sizeE1 = 6;

    constexpr static const char* setE2[] = { // no required options
        "app",
        "-n", "aa",
        "-h",
        "--myvalue", "10.3",
        "-s",
        "-l", "123",
        nullptr
    };
    constexpr static arg_count sizeE2 = 9;

    constexpr static const char* setE3[] = { // options missing values
        "app",
        "-e", "latin1",
        "--input",
        "-h",
        "--flag",
        "-o", "output.txt",
        "-i", "input2.txt",
        "-n", "cba",
        "--count",
        "-l", "1034",
        "-s",
        "-f",
        "-m",
        "-o", "output2.txt",
        "--verbose",
        "--count", "10",
        "-f",
        "--name", "abc",
        "--encoding", "utf8",
        "-v",
        "-l", "134",
        nullptr
    };
    constexpr static arg_count sizeE3 = 31;

    constexpr static const char* setE4[] = { // invalid option names (some repeated)
        "app",
        "-es", "latin1",
        "-input", "input.txt",
        "-h",
        "--flag",
        "-o", "output.txt",
        "i", "input2.txt",
        "-n", "cba",
        "--cunt", "145",
        "-i", "input2.txt",
        "-l", "1034",
        "-s",
        "f",
        "-m", "304.45",
        "-o", "output2.txt",
        "--verbose",
        "--count", "10",
        "-f",
        "--names", "abc",
        "--encoding", "utf8",
        "-v",
        "-l", "134",
        nullptr
    };
    constexpr static arg_count sizeE4 = 36;

    constexpr static const char* setE5[] = { // missing required (--input)
        "app",
        "-e", "latin1",
        "-h",
        "--flag",
        "-o", "output.txt",
        "-n", "cba",
        "--count",
        "-l", "1034",
        "-s",
        "-f",
        "-m",
        "-o", "output2.txt",
        "--verbose",
        "--count", "10",
        "-f",
        "--name", "abc",
        "--encoding", "utf8",
        "-v",
        "-l", "134",
        nullptr
    };
    constexpr static arg_count sizeE5 = 28;

    constexpr static const char* setE6[] = { // invalid values
        "app",
        "-es", "latin1",
        "--input", "input.txt",
        "-h",
        "--flag",
        "-o", "output.txt",
        "i", "input2.txt",
        "-n", "cba",
        "--count", "145",
        "-i", "input2.txt",
        "-l", "1034",
        "-s",
        "f",
        "-m", "304.45",
        "-o", "output2.txt",
        "--verbose",
        "--count", "5000000000", // larger than int
        "-f",
        "--names", "abc",
        "--encoding", "utf8",
        "-v",
        "-l", "-134",
        nullptr
    };
    constexpr static arg_count sizeE6 = 36;
};


TEST_F(ClipperTest, ParsingG1) {
    EXPECT_NO_THROW({
        EXPECT_TRUE(cli.parse(sizeG1, setG1)) << ParsingWrong();
    });
}

TEST_F(ClipperTest, ParsingG2) {
    EXPECT_NO_THROW({
        EXPECT_TRUE(cli.parse(sizeG2, setG2)) << ParsingWrong();
    });
}

TEST_F(ClipperTest, ParsingG3) {
    EXPECT_NO_THROW({
        EXPECT_TRUE(cli.parse(sizeG3, setG3)) << ParsingWrong();
    });
}

TEST_F(ClipperTest, ParsingE1) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE1, setE1));
    });
}

TEST_F(ClipperTest, ParsingE2) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE2, setE2));
    });
}

TEST_F(ClipperTest, ParsingE3) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE3, setE3));
    });
}

TEST_F(ClipperTest, ParsingE4) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE4, setE4));
    });
}

TEST_F(ClipperTest, ParsingE5) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE5, setE5));
    });
}

TEST_F(ClipperTest, ParsingE6) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(sizeE6, setE6));
    });
}


TEST_F(ClipperTest, NoArgs) {
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(1, empty));
        EXPECT_TRUE(cli.no_args());

        cli.allow_no_args();
        EXPECT_TRUE(cli.parse(1, empty));
        EXPECT_TRUE(cli.no_args());
        
        cli.parse(sizeG1, setG1);
        EXPECT_FALSE(cli.no_args());
    });
}