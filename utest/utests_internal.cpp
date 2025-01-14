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
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--inf"), "--int");
}

MYTEST(closestKeyBegin) {
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("-int"), "--int");
}

MYTEST(closestKeyOneEditExtraChar) {
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--innt"), "--int");
}

MYTEST(closestKeyOneEditMissingChar) {
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--it"), "--int");
}

MYTEST(closestKeyAlias) {
    parser.addArgument<int>("-int", "--integer").finalize();
    EXPECT_EQ(parser.closestKeyTest("-inf"), "--integer");
}

MYTEST(closestKeyExactMatchWithAlias) {
    parser.addArgument<int>("-int", "--integer").finalize();
    EXPECT_EQ(parser.closestKeyTest("--integer"), "--integer");  // Exact match on alias
}

MYTEST(closestKeySwappedLetters) {
    parser.addArgument<int>("--list").finalize();
    EXPECT_EQ(parser.closestKeyTest("--lsit"), "--list");
}

MYTEST(closestKeyNoMatch) {
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--xyz"), "");  // or a default value or `null`
}

MYTEST(closestKeyMultipleMatches) {
    parser.addArgument<int>("--int").finalize();
    parser.addArgument<int>("--in").finalize();
    parser.addArgument<int>("--inn").finalize();
    parser.addArgument<int>("--intt").finalize();
    parser.addArgument<int>("--itn").finalize();
    EXPECT_EQ(parser.closestKeyTest("--iin"), "--inn");
}

MYTEST(closestKeyExactMatch) {
    parser.addArgument<int>("--int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--int"), "--int");
}

MYTEST(closestKeyCaseSensitivity) {
    parser.addArgument<int>("--Int").finalize();
    EXPECT_EQ(parser.closestKeyTest("--int"), "--Int");
}

MYTEST(closestKeySpecialCharacters) {
    parser.addArgument<int>("--long-option").finalize();
    EXPECT_EQ(parser.closestKeyTest("--logn-option"), "--long-option");
}

MYTEST(closestKeyMultipleWords) {
    parser.addArgument<int>("--my-key").finalize();
    EXPECT_EQ(parser.closestKeyTest("--my-ke"), "--my-key");
}

MYTEST(closestKeyLexicographicalOrder) {
    parser.addArgument<int>("-ab").finalize();
    parser.addArgument<int>("-ae").finalize();
    parser.addArgument<int>("-ay").finalize();
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
            .help("help message for -i")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i : help message for -i");
}

MYTEST(helpCommonImplicitFlagWithAlias) {
    parser.addArgument<int>("-i", "--int")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i, --int : ");
}

MYTEST(helpCommonSingleParamFlag) {
    parser.addArgument<int>("-i")
            .parameters("int")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <int> : ");
}

MYTEST(helpCommonSingleParamArbitraryFlag) {
    parser.addArgument<int>("-i")
            .parameters("[int]")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [int] : ");
}

MYTEST(helpCommonTwoParamFlag) {
    parser.addArgument<int>("-i")
            .parameters("int", "[int]")
            .callable([](auto a, auto b) {
                return 0;
            })
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <int> [int] : ");
}

MYTEST(helpCommonNArgMetavarMandatoryFlag) {
    parser.addArgument<int>("-i")
            .nargs<1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <I> : ");
}

MYTEST(helpCommonNArgMetavarArbitraryFlag) {
    parser.addArgument<int>("-i")
            .nargs<0, 1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [I] : ");
}

MYTEST(helpCommonNArgParamMndFlag) {
    parser.addArgument<int>("-i")
            .parameters("meta")
            .nargs<1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i <meta> : ");
}

MYTEST(helpCommonNArgParamArbFlag) {
    parser.addArgument<int>("-i")
            .parameters("meta")
            .nargs<0, 1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [meta] : ");
}

MYTEST(helpCommonNArgPureVariadicFlag) {
    parser.addArgument<int>("-i")
            .nargs<0, -1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i [I...] : ");
}

MYTEST(helpCommonOption){
    parser.addArgument<int>("i")
            .parameters("int")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] parameters...");
    EXPECT_EQ(lines[3], "Parameters (mandatory):");
    EXPECT_EQ(lines[4], "\ti <int> : ");
}

MYTEST(helpCommonNargVariadicOption){
    parser.addArgument<int>("i")
            .nargs<1, -1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[4], "\ti <I> [I...] : ");
}

MYTEST(helpCommonPositional){
    parser.addPositional<int>("pos")
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos");
    EXPECT_EQ(lines[1], "Positional arguments:");
    EXPECT_EQ(lines[2], "\tpos : ");
}

MYTEST(helpCommonNargPositional){
    parser.addPositional<int>("pos")
            .nargs<3>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos pos pos");
    EXPECT_EQ(lines[2], "\tpos : ");
}

MYTEST(helpCommonNargVariadicPositional){
    parser.addPositional<int>("pos")
            .nargs<1, 3>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos [pos] [pos]");
    EXPECT_EQ(lines[2], "\tpos : ");
}

MYTEST(helpCommonNargPureVariadicPositional){
    parser.addPositional<int>("pos")
            .nargs<0, -1>()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] [pos...]");
    EXPECT_EQ(lines[2], "\tpos : ");
}

MYTEST(helpCommonPositionalWithChoices){
    parser.addPositional<int>("pos")
            .choices(1, 2, 3)
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] pos");
    EXPECT_EQ(lines[2], "\tpos {1,2,3} : ");
}

MYTEST(helpCommonVariadicPositionalWithChoices){
    parser.addPositional<int>("pos")
            .nargs<0,-1>()
            .choices(1, 2, 3)
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 5);
    EXPECT_EQ(lines[0], "Usage:  [flags...] [pos...]");
    EXPECT_EQ(lines[2], "\tpos {1,2,3} : ");
}

MYTEST(helpRepeatable) {
    parser.addArgument<int>("-i")
            .repeatable()
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i :  [repeatable]");
}

MYTEST(helpDefault) {
    parser.addArgument<int>("-i")
            .defaultValue(5)
            .finalize();
    parser.printHelpCommonTest(false);
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 4);
    EXPECT_EQ(lines[3], "\t-i :  (default 5)");
}

MYTEST(helpForParam) {
    parser.addArgument<int>("-i")
            .help("help message")
            .advancedHelp("advanced help message")
            .finalize();
    parser.printHelpForParamTest("-i");
    auto lines = GetOutLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_EQ(lines[0], "-i : help message");
    EXPECT_EQ(lines[1], "advanced help message");
}

