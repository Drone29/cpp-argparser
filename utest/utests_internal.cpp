#include <gtest/gtest.h>
#include "argparser.hpp"

#define FIXTURE UtestInternal
#define MYTEST(NAME) TEST_F(FIXTURE, NAME)

class parserFake : public argParser {
public:
    using argParser::argParser;
    std::string closestKeyTest(const std::string &name) {
        return argParser::closestKey(name);
    }
    void printHelpCommonTest(bool advanced) {
        argParser::printHelpCommon(advanced);
    }
    void printHelpForParamTest(const std::string &param) {
        argParser::printHelpForParameter(param);
    }
};

// Create a test fixture
class FIXTURE : public testing::Test {
protected:
    parserFake parser;
    std::ostringstream capturedOutput;
    std::streambuf* originalBuffer;
    explicit FIXTURE()
            : parser(){}

    std::vector<std::string> GetOutLines() {
        std::istringstream capturedStream(capturedOutput.str());
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(capturedStream, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    void SetUp() override {
        originalBuffer = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());
    }
    void TearDown() override {
        // Restore the original buffer
        std::cout.rdbuf(originalBuffer);
    }
};

MYTEST(closestKeyEnd) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--inf"), "--int");
}

MYTEST(closestKeyBegin) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("-int"), "--int");
}

MYTEST(closestKeyOneEditExtraChar) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--innt"), "--int");
}

MYTEST(closestKeyOneEditMissingChar) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--it"), "--int");
}

MYTEST(closestKeyAlias) {
    parser.addArgument<int>("-int", "--integer").Finalize();
    EXPECT_EQ(parser.closestKeyTest("-inf"), "--integer");
}

MYTEST(closestKeyExactMatchWithAlias) {
    parser.addArgument<int>("-int", "--integer").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--integer"), "--integer");  // Exact match on alias
}

MYTEST(closestKeySwappedLetters) {
    parser.addArgument<int>("--list").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--lsit"), "--list");
}

MYTEST(closestKeyNoMatch) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--xyz"), "");  // or a default value or `null`
}

MYTEST(closestKeyMultipleMatches) {
    parser.addArgument<int>("--int").Finalize();
    parser.addArgument<int>("--in").Finalize();
    parser.addArgument<int>("--inn").Finalize();
    parser.addArgument<int>("--intt").Finalize();
    parser.addArgument<int>("--itn").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--iin"), "--inn");
}

MYTEST(closestKeyExactMatch) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--int"), "--int");
}

MYTEST(closestKeyCaseSensitivity) {
    parser.addArgument<int>("--Int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--int"), "--Int");
}

MYTEST(closestKeySpecialCharacters) {
    parser.addArgument<int>("--long-option").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--logn-option"), "--long-option");
}

MYTEST(closestKeyMultipleWords) {
    parser.addArgument<int>("--my-key").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--my-ke"), "--my-key");
}

MYTEST(closestKeyLexicographicalOrder) {
    parser.addArgument<int>("-ab").Finalize();
    parser.addArgument<int>("-ae").Finalize();
    parser.addArgument<int>("-ay").Finalize();
    // Input: "-ad", which is equally close to "-ab" and "-ay" (Levenshtein distance = 1)
    // The function should return "-ae" because "-ae" is lexicographically closer to "ad".
    EXPECT_EQ(parser.closestKeyTest("-ad"), "-ae");
}

/// Help tests
MYTEST(helpEmpty) {
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "Usage:  [flags...]");
    EXPECT_EQ(lines[1], "Flags (optional):");
    EXPECT_EQ(lines[2], "\t-h, --help [arg] : Show this message and exit. 'arg' to get help about certain arg");
}

MYTEST(helpCommonImplicitFlagWithHelp) {
    parser.addArgument<int>("-i")
            .Finalize()
            .help("help message for -i");
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i : help message for -i");
}

MYTEST(helpCommonImplicitFlagWithAlias) {
    parser.addArgument<int>("-i", "--int")
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i, --int : ");
}

MYTEST(helpCommonSingleParamFlag) {
    parser.addArgument<int>("-i")
            .SetParameters("int")
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <int> : ");
}

MYTEST(helpCommonSingleParamArbitraryFlag) {
    parser.addArgument<int>("-i")
            .SetParameters("[int]")
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [int] : ");
}

MYTEST(helpCommonTwoParamFlag) {
    parser.addArgument<int>("-i")
            .SetParameters("int", "[int]")
            .SetCallable([](auto a, auto b){
                return 0;
            })
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <int> [int] : ");
}

MYTEST(helpCommonNArgMetavarMandatoryFlag) {
    parser.addArgument<int>("-i")
            .NArgs<1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <I> : ");
}

MYTEST(helpCommonNArgMetavarArbitraryFlag) {
    parser.addArgument<int>("-i")
            .NArgs<0,1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [I] : ");
}

MYTEST(helpCommonNArgParamMndFlag) {
    parser.addArgument<int>("-i")
            .SetParameters("meta")
            .NArgs<1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <meta> : ");
}

MYTEST(helpCommonNArgParamArbFlag) {
    parser.addArgument<int>("-i")
            .SetParameters("meta")
            .NArgs<0,1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [meta] : ");
}

MYTEST(helpCommonNArgPureVariadicFlag) {
    parser.addArgument<int>("-i")
            .NArgs<0,-1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [I...] : ");
}

MYTEST(helpCommonOption){
    parser.addArgument<int>("i")
            .SetParameters("int")
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] parameters...");
    EXPECT_EQ(lines[3], "Parameters (mandatory):");
    EXPECT_EQ(lines[4], "\ti <int> : ");
}

MYTEST(helpCommonNargVariadicOption){
    parser.addArgument<int>("i")
            .NArgs<1,-1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[4], "\ti <I> [I...] : ");
}

MYTEST(helpCommonPositional){
    parser.addPositional<int>("pos")
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos");
    EXPECT_EQ(lines[1], "Positional arguments:");
    EXPECT_EQ(lines[2], "\tpos : ");
}

MYTEST(helpCommonNargPositional){
    parser.addPositional<int>("pos")
            .NArgs<3>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos pos pos");
}

MYTEST(helpCommonNargPureVariadicPositional){
    parser.addPositional<int>("pos")
            .NArgs<0,-1>()
            .Finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] [pos...]");
}

MYTEST(helpRepeatable) {
    parser.addArgument<int>("-i")
            .Finalize()
            .repeatable();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i :  [repeatable]");
}

MYTEST(helpDefault) {
    parser.addArgument<int>("-i")
            .Finalize()
            .default_value(5);
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i :  (default 5)");
}

MYTEST(helpForParam) {
    parser.addArgument<int>("-i")
            .Finalize()
            .help("help message")
            .advanced_help("advanced help message");
    parser.printHelpForParamTest("-i");
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "-i : help message");
    EXPECT_EQ(lines[1], "advanced help message");
}

