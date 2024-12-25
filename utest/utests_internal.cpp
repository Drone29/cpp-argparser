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

MYTEST(closestKeyMiddle) {
    parser.addArgument<int>("--int").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--ibt"), "--int");
}

MYTEST(closestKeyAlias) {
    parser.addArgument<int>("-int", "--integer").Finalize();
    EXPECT_EQ(parser.closestKeyTest("-inf"), "--integer");
}

MYTEST(closestKeySwappedLetters) {
    parser.addArgument<int>("--list").Finalize();
    EXPECT_EQ(parser.closestKeyTest("--lsit"), "--list");
}


