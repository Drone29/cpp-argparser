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
};

// Create a test fixture
class FIXTURE : public testing::Test {
protected:
    parserFake parser;
    explicit FIXTURE()
            : parser(){}
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


