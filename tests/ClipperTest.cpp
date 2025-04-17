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
        cli.add_flag("--verbose", "-v").set(v_v);
        cli.add_flag("-s").set(s_v);
        cli.add_flag("-h").set(h_v);

        cli.help_flag("--help").set(help_v);
        cli.version_flag("--version").set(version_v);
    }

    std::string ParsingWrong() {
        std::string joined;
        for (auto& i : cli.wrong())
            joined += i + '\n';
        return joined;
    }
    
    bool help_v, version_v;
    bool f_v, v_v, s_v, h_v;
    std::string i_v, n_v, e_v;
    std::filesystem::path o_v;
    int c_v;
    double m_v;
    std::size_t l_v;

    clipper cli;
};


// Should evaluate to true
TEST_F(ClipperTest, ParsingRequiredOnly) {
    const char* argv[] = { 
        "app",
        "-i", "in.txt",
        "-o", "out.txt",
        "-c", "5",
        "-f", 
        nullptr
    };
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(8, argv)) << ParsingWrong();
    });
    EXPECT_EQ(i_v, "in.txt");
    EXPECT_EQ(o_v, "out.txt");
    EXPECT_EQ(c_v, 5);
    EXPECT_TRUE(f_v);
    EXPECT_FALSE(help_v);
    EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingAllOptionsSet) {
    const char* argv[] = {
        "app",
        "-i", "file.txt",
        "-o", "out.txt", 
        "-c", "42",
        "-f",
        "--name", "TestName",
        "--encoding", "utf8",
        "--myvalue", "3.14",
        "-l", "64",
        "--verbose",
        "-s",
        "-h",
        nullptr
    };
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(19, argv)) << ParsingWrong();
    });
    EXPECT_EQ(n_v, "TestName");
    EXPECT_EQ(e_v, "utf8");
    EXPECT_DOUBLE_EQ(m_v, 3.14);
    EXPECT_EQ(l_v, 64u);
    EXPECT_TRUE(v_v);
    EXPECT_TRUE(s_v);
    EXPECT_TRUE(h_v);
    EXPECT_FALSE(help_v);
    EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingRepeatedOptions) {
    const char* argv[] = { // some options repeated
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
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(13, argv)) << ParsingWrong();
    });
    EXPECT_EQ(i_v, "input.txt");
    EXPECT_EQ(o_v, "output2.txt");
    EXPECT_EQ(c_v, 145);
    EXPECT_TRUE(f_v);
    EXPECT_TRUE(h_v);
    EXPECT_FALSE(help_v);
    EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingAllOptionsSomeRepeated) {
    const char* argv[] = { // all options (and repeated)
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
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(34, argv));
    });
    EXPECT_EQ(i_v, "input2.txt");
    EXPECT_EQ(o_v, "output2.txt");
    EXPECT_EQ(c_v, 10);
    EXPECT_EQ(l_v, 134);
    EXPECT_EQ(n_v, "abc");
    EXPECT_EQ(e_v, "utf8");
    EXPECT_DOUBLE_EQ(m_v, 304.45);
    EXPECT_TRUE(f_v);
    EXPECT_TRUE(h_v);
    EXPECT_TRUE(v_v);
    EXPECT_TRUE(s_v);
    EXPECT_FALSE(help_v);
    EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingAllOptionsNoRepeats) {
    const char* argv[] = {
        "app",
        "-i", "input.txt",
        "-o", "out.txt",
        "-c", "42",
        "--flag",
        "--name", "example",
        "--encoding", "ascii",
        "--myvalue", "123.456",
        "-l", "789",
        "--verbose",
        "-s",
        "-h",
        nullptr
    };
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(19, argv)) << ParsingWrong();
    });
    EXPECT_EQ(i_v, "input.txt");
    EXPECT_EQ(o_v, "out.txt");
    EXPECT_EQ(c_v, 42);
    EXPECT_EQ(n_v, "example");
    EXPECT_EQ(e_v, "ascii");
    EXPECT_DOUBLE_EQ(m_v, 123.456);
    EXPECT_EQ(l_v, 789);
    EXPECT_TRUE(f_v);
    EXPECT_TRUE(v_v);
    EXPECT_TRUE(s_v);
    EXPECT_TRUE(h_v);
    EXPECT_FALSE(help_v);
    EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingHelp) {
    const char* argv[] = { "app", "--help", nullptr };
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(2, argv)) << ParsingWrong();
    });
        EXPECT_TRUE(help_v);
        EXPECT_FALSE(version_v);
}

TEST_F(ClipperTest, ParsingVersion) {
    const char* argv[] = { "app", "--version", nullptr };
    EXPECT_NO_THROW({
        ASSERT_TRUE(cli.parse(2, argv)) << ParsingWrong();
    });
        EXPECT_FALSE(help_v);
        EXPECT_TRUE(version_v);
}

// Should evaluate to false
TEST_F(ClipperTest, ParsingMissingRequiredOptions) {
    const char* argv[] = {
        "app",
        "--name", "missing",
        nullptr
    };
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(3, argv));
    });
}

TEST_F(ClipperTest, ParsingMissingRequiredOptionInput) {
    const char* argv[] = { // missing required (--input)
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
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(28, argv));
    });
}

TEST_F(ClipperTest, ParsingMissingOnlyRequiredOptions) {
    const char* argv[] = { // no required options
        "app",
        "-n", "aa",
        "--encoding", "utf8",
        "-v",
        "-h",
        "--myvalue", "10.3",
        "-s",
        "-l", "123",
        nullptr
    };
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(12, argv));
    });
}

TEST_F(ClipperTest, ParsingMissingOptionValues) {
    const char* argv[] = { // options missing values
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
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(31, argv));
    });
}

TEST_F(ClipperTest, ParsingInvalidOptionNames) {
    const char* argv[] = { // invalid option names (some repeated)
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
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(36, argv));
    });
}

TEST_F(ClipperTest, ParsingInvalidOptionValues) {
    const char* argv[] = { // invalid values
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
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(36, argv));
    });
}

TEST_F(ClipperTest, ParsingInvalidUseOfHelpFlag) {
    const char* argv[] = { // all options (and repeated)
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
        "--help",
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
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(35, argv));
    });
}

TEST_F(ClipperTest, ParsingInvalidUseOfVersionFlag) {
    const char* argv[] = { // all options (and repeated)
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
        "--version",
        nullptr
    };
    EXPECT_NO_THROW({
        EXPECT_FALSE(cli.parse(35, argv));
    });
}


TEST_F(ClipperTest, NoArgs) {
    const char* argv[] = { "app", nullptr };
    const char* argv2[] = { "app", "-i", "in.txt", "-o", "out.txt", "-c", "5", "-f", nullptr };
    EXPECT_NO_THROW({
        ASSERT_FALSE(cli.parse(1, argv));
        EXPECT_TRUE(cli.no_args());

        cli.allow_no_args();
        ASSERT_TRUE(cli.parse(1, argv));
        EXPECT_TRUE(cli.no_args());
        
        cli.parse(8, argv2);
        ASSERT_FALSE(cli.no_args());
    });
}