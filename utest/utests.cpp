#include <gtest/gtest.h>
#include "argparser.hpp"

#define FIXTURE Utest
#define MYTEST(NAME) TEST_F(FIXTURE, NAME)

// Create a test fixture
class FIXTURE : public testing::Test {
protected: 
    argParser parser;
    explicit FIXTURE()
        : parser("", ""){}

    void CallParser(std::vector<const char*> args){
        args.insert(args.begin(), "binary_name");
        parser.parseArgs(int(args.size()), const_cast<char **>(&args[0]));
    }
};

/// Common
MYTEST(EmptyKey){
    EXPECT_THROW(parser.addArgument<int>(""), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>(""), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("", ""), std::invalid_argument);
}

MYTEST(OnlySpaces){
    EXPECT_THROW(parser.addArgument<int>("   "), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("   "), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("   ", ""), std::invalid_argument);
}

MYTEST(OnlyMinuses){
    EXPECT_THROW(parser.addArgument<int>("--"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("--"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("--", ""), std::invalid_argument);
}

MYTEST(OnlyUnderscores){
    EXPECT_THROW(parser.addArgument<int>("__"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("__"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("__", ""), std::invalid_argument);
}

MYTEST(OnlyNumbers){
    EXPECT_THROW(parser.addArgument<int>("0"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("0"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("0", ""), std::invalid_argument);
}

MYTEST(OnlyNegativeNumbers){
    EXPECT_THROW(parser.addArgument<int>("-123"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("-123"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("-123",""), std::invalid_argument);
}

MYTEST(OnlyFloatingNumbers){
    EXPECT_THROW(parser.addArgument<int>("123.6"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("123.6"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("123.6",""), std::invalid_argument);
}

MYTEST(OnlyFloatingNegativeNumbers){
    EXPECT_THROW(parser.addArgument<int>("-123.6"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("-123.6"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("-123.6",""), std::invalid_argument);
}

MYTEST(OnlyPunctuations){
    EXPECT_THROW(parser.addArgument<int>("..."), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("..."), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("...",""), std::invalid_argument);
}

MYTEST(InvalidKey){
    EXPECT_THROW(parser.addArgument<int>("=y"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("=y"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("=y",""), std::invalid_argument);
}

MYTEST(InvalidKey2){
    EXPECT_THROW(parser.addArgument<int>("y,"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("y,"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("y,",""), std::invalid_argument);
}

MYTEST(InvalidKey3){
    EXPECT_THROW(parser.addArgument<int>(" y"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>(" y"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand(" y",""), std::invalid_argument);
}

MYTEST(InvalidKey4){
    EXPECT_THROW(parser.addArgument<int>("[int]"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("[int]"), std::invalid_argument);
    EXPECT_THROW(parser.addCommand("[int]",""), std::invalid_argument);
}

MYTEST(InvalidPosKey){
    EXPECT_THROW(parser.addPositional<int>("-i"), std::invalid_argument);
}

MYTEST(InvalidPosKey2){
    EXPECT_THROW(parser.addPositional<int>("[int]"), std::invalid_argument);
}

MYTEST(ValidPosKey0X){
    EXPECT_NO_THROW(parser.addPositional<int>("0xINT"));
}

MYTEST(AllowSpacesAndSpecialCharsForParams) {
    EXPECT_NO_THROW(parser.addArgument<int>("-i").setParameters("[int | float]"));
}

MYTEST(AllowSpacesAndSpecialCharsForParams2) {
    EXPECT_NO_THROW(parser.addArgument<int>("-i").setParameters("0x???"));
}

MYTEST(Redefinition){
    parser.addArgument<int>("--int").finalize();
    EXPECT_THROW(parser.addArgument<int>("--int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionPos){
    parser.addPositional<int>("int").finalize();
    EXPECT_THROW(parser.addPositional<int>("int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionArgPos){
    parser.addArgument<int>("int").setParameters("int").finalize();
    EXPECT_THROW(parser.addPositional<int>("int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(AliasDetection){
    parser.addArgument<int>("-i", "--int").finalize();
    EXPECT_NO_THROW(CallParser({"-i"})) << "Should recognise alias separated with comma";
}

MYTEST(AliasInvalid){
    EXPECT_THROW(parser.addArgument<int>("i", "--int"), std::invalid_argument) << "Should detect the type of alias is different from the key";
}

MYTEST(RedefinitionByAlias){
    parser.addArgument<int>("-i", "--int").finalize();
    EXPECT_THROW(parser.addArgument<int>("-i").finalize(), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionPosByAlias){
    parser.addArgument<int>("i", "int").setParameters("int").finalize();
    EXPECT_THROW(parser.addPositional<int>("i"), std::invalid_argument) << "Should detect the pos name is same as other arg's alias";
}

MYTEST(MandatoryArgAtLeastOneParam){
    EXPECT_THROW(parser.addArgument<int>("i", "int").finalize(), std::invalid_argument) << "Mandatory arg's param list cannot be empty";
}

MYTEST(NullParam){
    const char *s = nullptr;
    EXPECT_THROW(parser.addArgument<int>("-i")
            .setParameters(s),
            std::invalid_argument) << "Should detect null param";
}

MYTEST(EmptyParam){
    EXPECT_THROW(parser.addArgument<int>("-i")
            .setParameters(""),
            std::invalid_argument) << "Should detect empty params";
}

MYTEST(NumberParam){
    EXPECT_THROW(parser.addArgument<int>("-i")
                         .setParameters("0"),
                 std::invalid_argument);
}

MYTEST(UnknownArg){
    parser.addArgument<int>("--int").setParameters("int").finalize();
    EXPECT_THROW(CallParser({"--inf", "23"}), argParser::parse_error) << "Should throw error if found unknown arg";
}

MYTEST(NegativeInt){
    parser.addArgument<int>("-i").setParameters("int_value").finalize();
    CallParser({"-i", "-123"});
    ASSERT_EQ(parser.getValue<int>("-i"), -123) << "Should be able to parse negative int correctly";
}

MYTEST(UnsignedLongLong){
    parser.addArgument<unsigned long long>("-l").setParameters("long_long_value").finalize();
    CallParser({"-l", "4000000000"});
    ASSERT_EQ(parser.getValue<unsigned long long>("-l"), 4000000000) << "Should be able to parse large number correctly";
}

MYTEST(Overflow){
    parser.addArgument<short>("-i").setParameters("short_value").finalize();
    EXPECT_THROW(CallParser({"-i", "70000"}), argParser::unparsed_param) << "Should throw in case of overflow";
}

MYTEST(InvalidPtr){
    EXPECT_THROW(parser.addArgument<int *>("-i").setParameters("int_ptr").finalize(),
        std::invalid_argument) << "Should throw as no scan provided for int*";
}

MYTEST(NotEnoughPos){
    parser.addPositional<int>("pos").finalize();
    EXPECT_THROW(CallParser({}), argParser::parse_error) << "Should throw as not enough positionals";
}

MYTEST(RepeatingThrow){
    parser.addArgument<int>("-i").finalize();
    EXPECT_THROW(CallParser({"-i", "-i"}), argParser::parse_error) << "Should throw in case of redefinition for non-repeatable argument";
}

MYTEST(GlobalPtr){
    int i;
    parser.addArgument<int>("-i")
            .setParameters("int")
            .finalize()
            .global_ptr(&i);
    CallParser({"-i", "345"});
    auto res = parser.getValue<int>("-i");
    ASSERT_EQ(res, 345);
    ASSERT_EQ(res, i) << "Global ref should work";
}

MYTEST(EqSign){
    parser.addArgument<int>("-i")
            .setParameters("int")
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i=123"}));
    ASSERT_EQ(parser.getValue<int>("-i"), 123);
}

MYTEST(AdjacentKeyValue){
    parser.addArgument<int>("-i")
            .setParameters("int")
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i123"}));
    ASSERT_EQ(parser.getValue<int>("-i"), 123);
}

MYTEST(IncrementImplicit) {
    parser.addArgument<int>("-i")
            .finalize()
            .default_value(5);
    CallParser({"-i"});
    ASSERT_EQ(parser.getValue<int>("-i"), 6) << "Should increment implicit arithmetic type";
}

MYTEST(IncrementImplicitBool) {
    parser.addArgument<bool>("-b")
            .finalize()
            .default_value(false);
    CallParser({"-b"});
    ASSERT_TRUE(parser.getValue<bool>("-b")) << "Should set implicit bool to true";
}

MYTEST(ImplicitBoolRepeated) {
    parser.addArgument<bool>("-b")
            .finalize()
            .default_value(false)
            .repeatable();
    CallParser({"-b", "-b"});
    ASSERT_FALSE(parser.getValue<bool>("-b")) << "Should 'increment' boolean twice, thus setting it to false";
}

MYTEST(ImplicitFloat) {
    parser.addArgument<float>("-f")
            .finalize()
            .default_value(1.35f);
    CallParser({"-f"});
    ASSERT_EQ(parser.getValue<float>("-f"), 2.35f) << "Should 'increment' float, increasing it by 1";
}

MYTEST(RepeatableShortArgIncrement){
    // add repeatable arg with global ptr, starting at 3
    int i_val;
    parser.addArgument<int>("-i", "--int")
            .finalize()
            .default_value(3)
            .global_ptr(&i_val)
            .repeatable();
    EXPECT_NO_THROW(CallParser({"-i", "-i", "-iii"}));
    ASSERT_EQ(i_val, 8) << "i repeats 5 times starting at 3, so result should be 8";            
}

MYTEST(SimilarKeyAsValueEq){
    std::string res;
    parser.addArgument<std::string>("--str")
            .setParameters("str_val")
            .finalize()
            .global_ptr(&res);
    EXPECT_NO_THROW(CallParser({"--str=--str"}));
    ASSERT_EQ(res, "--str") << "Should parse '--str' after '=' as value";            
}

MYTEST(SimilarKeyAsValueAdjacent){
    std::string res;
    parser.addArgument<std::string>("-s")
            .setParameters("str_val")
            .finalize()
            .global_ptr(&res);
    EXPECT_NO_THROW(CallParser({"-s-sss"}));
    ASSERT_EQ(res, "-sss") << "Should parse '-sss' after '-s' as value";
}

MYTEST(VariadicOpt){
    parser.addArgument<int>("--variadic", "-var")
            .nargs<1, -1>()
            .finalize();
    CallParser({"-var", "1", "2", "3"});
    auto vec = parser.getValue<std::vector<int>>("-var");
    bool check = vec == std::vector<int>{1,2,3};
    ASSERT_TRUE(check) << "Should parse 3 digits to variadic argument";      
}

MYTEST(ArgImplicitWithFunction){
    int val = 0;
    parser.addArgument<int>("-i")
            .setCallable([&val]() {
                return ++val;
            })
            .finalize();
    CallParser({"-i"});
    ASSERT_EQ(val, 1);
    ASSERT_EQ(parser.getValue<int>("-i"), 1);
}

MYTEST(ArgSingleMandatoryParamWithFunction) {
    parser.addArgument<int>("-i")
            .setParameters("int")
            .setCallable([](auto i) {
                return int(std::strtol(i, nullptr, 0));
            })
            .finalize();
    CallParser({"-i", "123"});
    ASSERT_EQ(parser.getValue<int>("-i"), 123);
}

MYTEST(ArgSingleOptionalParamWithFunction) {
    parser.addArgument<int>("-i")
            .setParameters("[int]")
            .setCallable([](auto i) {
                if (i == nullptr) {
                    return 13;
                }
                return int(std::strtol(i, nullptr, 0));
            })
            .finalize();
    CallParser({"-i"});
    ASSERT_EQ(parser.getValue<int>("-i"), 13);
}

MYTEST(ArgSingleOptionalParamSpecialWithFunction) {
    parser.addArgument<int>("-i")
            .setParameters("[int | float]")
            .setCallable([](auto i) {
                if (i == nullptr) {
                    return 13;
                }
                return int(std::strtod(i, nullptr));
            })
            .finalize();
    CallParser({"-i", "45.6"});
    ASSERT_EQ(parser.getValue<int>("-i"), 45);
}

MYTEST(PosWithFunction) {
    parser.addPositional<int>("i")
            .setCallable([](auto i) {
                return int(std::strtol(i, nullptr, 0));
            })
            .finalize();
    CallParser({"456"});
    ASSERT_EQ(parser.getValue<int>("i"), 456);
}

MYTEST(VoidFunction){
    int i = 0;
    parser.addArgument<int>("-i")
            .setParameters("int")
            .setCallable([&i](auto i_str) {
                i = std::strtol(i_str, nullptr, 0);
            })
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i", "456"}));
    ASSERT_EQ(i, 456);
}

MYTEST(PureVariadicPos) {
    parser.addPositional<int>("i")
            .nargs<0, -1>()
            .finalize();
    CallParser({"1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("i") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
}

MYTEST(ArgWithSingleOptionalParamNoFunctionNotProvided) {
    ASSERT_NO_THROW(parser.addArgument<int>("-i")
                            .setParameters("[int]")
                            .finalize()
            .default_value(5));
    CallParser({"-i"});
    ASSERT_TRUE(parser["-i"].is_set());
    ASSERT_EQ(parser.getValue<int>("-i"), 6) << "Should treat as implicit if optional param not provided";
}

MYTEST(ArgWithSingleOptionalNArgNoFunctionNotProvided) {
    parser.addArgument<int>("-i")
            .nargs<0, 1>()
            .finalize()
            .default_value(5);
    CallParser({"-i"});
    ASSERT_TRUE(parser["-i"].is_set());
    ASSERT_EQ(parser.getValue<int>("-i"), 6) << "Should treat as implicit if optional param not provided";
}

MYTEST(ArgWithSingleOptionalParamWithFunctionNotProvided) {
    int val = 5;
    ASSERT_NO_THROW(parser.addArgument<int>("-i")
                            .setParameters("[int]")
                            .setCallable([&val](auto c) {
                                if (c == nullptr) {
                                    return ++val;
                                }
                                val = int(std::strtol(c, nullptr, 0));
                                return val;
                            })
                            .finalize());
    CallParser({"-i"});
    ASSERT_TRUE(parser["-i"].is_set());
    ASSERT_EQ(parser.getValue<int>("-i"), 6) << "Should treat as implicit if optional param not provided";
}

MYTEST(ArgWithSingleOptionalNArgWithFunctionNotProvided) {
    int val = 5;
    parser.addArgument<int>("-i")
            .nargs<0, 1>()
            .setCallable([&val](auto c) {
                if (c == nullptr) {
                    return ++val;
                }
                val = int(std::strtol(c, nullptr, 0));
                return val;
            })
            .finalize();
    CallParser({"-i"});
    ASSERT_TRUE(parser["-i"].is_set());
    ASSERT_EQ(parser.getValue<int>("-i"), 6) << "Should treat as implicit if optional param not provided";
}

MYTEST(ArgWithParamsAndVariadicPos){
    //define lambda to parse int from 2 args
    auto lmb = [&](const char* mnd_, const char* arb_) -> int{
        auto res = argParser::scanValue<int>(mnd_);
        if(arb_ != nullptr){
            res += argParser::scanValue<int>(arb_);
        }
        return res;
    };
    // lambda for positionals
    auto pos_lmb = [](const char *a) -> int{
        auto res = argParser::scanValue<int>(a);
        return res + 2;
    };

    parser.addArgument<int>("-i")
            .setParameters("mnd", "[arb]")
            .setCallable(lmb)
            .finalize();
    parser.addPositional<int>("pos")
            .setCallable(pos_lmb)
            .nargs<1, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i", "67", "7", "3", "4", "5"}));
    // 1,2 should go to -i, the rest - to positional arg
    ASSERT_EQ(parser.getValue<int>("-i"), 74) << "Should parse first 2 options";
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{5,6,7};
    ASSERT_TRUE(check) << "Should parse 3 digits to variadic argument";
}

MYTEST(VarPosThrow){
    // add variadic positional arg
    parser.addPositional<int>("var_pos")
            .nargs<1, -1>()
            .finalize();
    EXPECT_THROW(parser.addPositional<int>("pos"), std::invalid_argument) << "Show throw invalid arg error if variadic positional arg is followed by another one";            
}

MYTEST(OptionalArgWithVariadicArg){
    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    parser.addArgument<std::string>("-p")
            .setParameters("[str_value]")
            .setCallable(test).finalize();
    parser.addArgument<int>("--variadic", "-var")
            .nargs<1, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"-p", "-var", "1", "2", "3"}));
    auto p = parser.getValue<std::string>("-p");
    bool check = parser.getValue<std::vector<int>>("-var") == std::vector<int>{1,2,3};
    ASSERT_EQ(p, "null");
    ASSERT_TRUE(check);
}

MYTEST(MeshedArgNameAndValue){
    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    parser.addArgument<std::string>("-p")
            .setParameters("[str_value]")
            .setCallable(test).finalize();
    parser.addArgument<int>("--variadic", "-var")
            .nargs<1, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"-p=-var", "-var", "1", "2", "3"}));
    auto p = parser.getValue<std::string>("-p");
    bool check = parser.getValue<std::vector<int>>("-var") == std::vector<int>{1,2,3};
    ASSERT_EQ(p, "-var");
    ASSERT_TRUE(check);
}

MYTEST(SingleChar){
    parser.addArgument<char>("-c").setParameters("char_value").finalize();
    CallParser({"-c", "A"});
    ASSERT_EQ(parser.getValue<char>("-c"), 'A') << "Should parse single char";
}

MYTEST(CharNumber){
    parser.addArgument<char>("-c").setParameters("char_value").finalize();
    CallParser({"-c", "123"});
    ASSERT_EQ(parser.getValue<char>("-c"), 123) << "Should parse multiple digits as char number";
}

MYTEST(CharNegativeNumber){
    parser.addArgument<char>("-c").setParameters("char_value").finalize();
    CallParser({"-c", "-123"});
    ASSERT_EQ(parser.getValue<char>("-c"), -123) << "Should parse multiple digits as char number";
}

MYTEST(Int8SingleInvalidChar){
    parser.addArgument<int8_t>("-i").setParameters("int8_value").finalize();
    EXPECT_THROW(CallParser({"-i", "A"}), std::runtime_error) << "Should throw error for non-char single digit";
}

MYTEST(Int8SingleChar){
    parser.addArgument<int8_t>("-i").setParameters("int8_value").finalize();
    CallParser({"-i", "9"});
    ASSERT_EQ(parser.getValue<int8_t>("-i"), 9) << "Should parse 9 into int8";
}

MYTEST(FormatInvalidDec){
    parser.addArgument<int>("-i").setParameters("int_value").finalize();
    EXPECT_THROW(CallParser({"-i", "1abc"}), std::runtime_error) << "1abc should not be convertible to int";
}

MYTEST(FormatHex){
    parser.addArgument<int>("-i").setParameters("int_value").finalize();
    CallParser({"-i", "0x12"});
    ASSERT_EQ(parser.getValue<int>("-i"), 0x12);
}

MYTEST(FormatFloat){
    parser.addArgument<float>("-f").setParameters("float_value").finalize();
    CallParser({"-f", "0.123"});
    ASSERT_EQ(parser.getValue<float>("-f"), float(0.123));
}

MYTEST(FormatScientific){
    parser.addArgument<float>("-f").setParameters("float_value").finalize();
    CallParser({"-f", "1.000000e-05"});
    ASSERT_EQ(parser.getValue<float>("-f"), float(0.00001));
}

MYTEST(PosCallableWithSideArgs){
    auto parse_pos = [](int g, const char* a){
        auto a_ = argParser::scanValue<int>(a);
        return g + a_;
    };
    parser.addPositional<int>("pos")
            .setCallable(parse_pos, 34)
            .finalize();
    CallParser({"6"});
    ASSERT_EQ(parser.getValue<int>("pos"), 40);
}

// Choices
MYTEST(ChoicesInvalid){
    parser.addArgument<int>("--choices").setParameters("int").finalize()
                .choices(0,1,2,3);
    EXPECT_THROW(CallParser({"--choices=4"}), argParser::unparsed_param) << "Should throw if not a valid choice";            
}

MYTEST(ChoicesThrow){
    EXPECT_THROW(parser.addArgument<const char *>("--choices").setParameters("str").finalize()
                    .choices("1","2","3","4"),
                    std::logic_error) << "Should throw if not arithmetic or not std::string";
}

MYTEST(ChoicesInt){
    parser.addArgument<int>("--choices").setParameters("int").finalize()
            .choices(0,1,2,3);
    EXPECT_NO_THROW(CallParser({"--choices=3"})) << "Should not throw if valid choice";
    EXPECT_EQ(parser.getValue<int>("--choices"), 3);
}

MYTEST(ChoicesFloat){
    parser.addArgument<float>("--choices").setParameters("float").finalize()
                .choices(0.12f, 0.15f, 1.14f);
    EXPECT_NO_THROW(CallParser({"--choices=0.15"}));
    EXPECT_EQ(parser.getValue<float>("--choices"), 0.15f);
}

MYTEST(ChoicesChar){
    parser.addArgument<char>("--choices").setParameters("char").finalize()
                .choices('a', 'b', 'c');
    EXPECT_NO_THROW(CallParser({"--choices=b"}));
    EXPECT_EQ(parser.getValue<char>("--choices"), 'b');
}

MYTEST(ChoicesString){
    using namespace std::string_literals;
    parser.addArgument<std::string>("--choices").setParameters("int").finalize()
                .choices("abc"s, "def"s, "ghi"s);
    EXPECT_NO_THROW(CallParser({"--choices=def"}));
    EXPECT_EQ(parser.getValue<std::string>("--choices"), "def"s);
}

MYTEST(ChoicesStringConversion){
    using namespace std::string_literals;
    EXPECT_NO_THROW(parser.addArgument<std::string>("--choices").setParameters("int").finalize()
            .choices("abc", "def", "ghi")) << "Should convert const char* to std::string";
    EXPECT_NO_THROW(CallParser({"--choices=def"}));
    EXPECT_EQ(parser.getValue<std::string>("--choices"), "def"s);
}

MYTEST(ChoicesNull){
    EXPECT_THROW(parser.addArgument<std::string>("--choices").setParameters("char").finalize()
            .choices(nullptr, nullptr, nullptr), std::logic_error);
}

MYTEST(ChoicesVarPos){
    parser.addPositional<int>("ch_pos")
            .nargs<1, -1>()
            .finalize()
            .choices(0,1,2,3);
    CallParser({"2", "3", "0", "1"});
    bool check = parser.getValue<std::vector<int>>("ch_pos") == std::vector<int>{2,3,0,1};
    ASSERT_TRUE(check) << "Variadic choices should be allowed"; 
}

MYTEST(ChoicesSingleNArgs){
    parser.addPositional<int>("ch_pos")
            .nargs<1>()
            .finalize()
            .choices(0,1,2,3);
    CallParser({"3"});
    ASSERT_EQ(parser.getValue<int>("ch_pos"), 3) << "Should parse single narg with allowed choice";           
}

MYTEST(ChoicesVarPosThrow){
    parser.addPositional<int>("ch_pos")
            .nargs<1, -1>()
            .finalize()
            .choices(0,1,2,3);
    EXPECT_THROW(CallParser({"2", "3", "4", "1"}), argParser::unparsed_param) << "Should throw error if unknown value is found";                
}

/// Nargs

MYTEST(NargsSingle){
    parser.addArgument<int>("-z")
            .nargs<1>()
            .finalize();
    CallParser({"-z", "123"});
    ASSERT_EQ(parser.getValue<int>("-z"), 123);
}

MYTEST(NargsSingleOptional){
    parser.addArgument<int>("-z")
            .nargs<0, 1>()
            .finalize();
    CallParser({"-z", "123"});
    ASSERT_EQ(parser.getValue<int>("-z"), 123);
}

MYTEST(NargsSingleOptionalNotProvided){
    parser.addArgument<int>("-z")
            .nargs<0, 1>()
            .finalize();
    ASSERT_NO_THROW(CallParser({"-z"}));
}

MYTEST(NargsSingleMandatoryWithParameter){
    parser.addArgument<int>("-z")
            .setParameters("int")
            .nargs<1>()
            .finalize();
    CallParser({"-z", "123"});
    ASSERT_EQ(parser.getValue<int>("-z"), 123);
}

MYTEST(NargsSingleOptionalWithParameter){
    parser.addArgument<int>("-z")
            .setParameters("int")
            .nargs<0, 1>()
            .finalize();
    CallParser({"-z", "123"});
    ASSERT_EQ(parser.getValue<int>("-z"), 123);
}

MYTEST(NargsTwoOptional){
    parser.addArgument<int>("-z")
            .nargs<0, 2>()
            .finalize();
    CallParser({"-z", "123"});
    bool check = parser.getValue<std::vector<int>>("-z") == std::vector<int>{123};
    ASSERT_TRUE(check);
}

MYTEST(NargsTwoOptionalNotProvided){
    parser.addArgument<int>("-z")
            .nargs<0, 2>()
            .finalize();
    ASSERT_NO_THROW(CallParser({"-z"}));
    ASSERT_TRUE(parser.getValue<std::vector<int>>("-z").empty());
}

MYTEST(NargsMandatoryAndVariadic){
    parser.addArgument<int>("-i")
            .nargs<1, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i", "1", "2", "3"}));
    bool check = parser.getValue<std::vector<int>>("-i") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
}

MYTEST(NargsMandatoryAndVariadicPartial){
    parser.addArgument<int>("-i")
            .nargs<2, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"-i", "1", "2"}));
    bool check = parser.getValue<std::vector<int>>("-i") == std::vector<int>{1,2};
    ASSERT_TRUE(check);
}

MYTEST(NargsTotalLessThanFromButGreaterThanZero){
    parser.addArgument<int>("-i")
            .nargs<3, 1>()
            .finalize();
    EXPECT_THROW(CallParser({"-i", "1", "2", "3", "4"}), argParser::parse_error)
        << "Should ignore second narg param if less than first but > 0";
}

MYTEST(NargsSinglePosOptional){
    parser.addPositional<int>("pos")
            .nargs<0, 1>()
            .finalize();
    CallParser({"123"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);
}

MYTEST(NargsSinglePosOptionalNotProvided){
    parser.addPositional<int>("pos")
            .nargs<0, 1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({}));
}

MYTEST(NargsSinglePosOptionalWithRegularPos){
    parser.addPositional<int>("pos")
            .nargs<0, 1>()
            .finalize();
    EXPECT_THROW(parser.addPositional<int>("pos2"), std::invalid_argument)
        << "Should not allow adding regular positional after positional with optional narg";
    CallParser({"123"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);
}

MYTEST(NargsPosTotalLessThanFromButGreaterThanZero){
    parser.addPositional<int>("pos")
            .nargs<3, 1>()
            .finalize();
    EXPECT_THROW(CallParser({"1", "2", "3", "4"}), argParser::parse_error)
                        << "Should ignore second narg param if less than first but > 0";
}

MYTEST(NargsFixedMandatory){
    parser.addArgument<int>("--arg")
            .nargs<3>()
            .finalize();
    CallParser({"--arg", "1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);            
}

MYTEST(NargsMandatoryAndOptional){
    parser.addArgument<int>("--arg")
            .nargs<1, 3>()
            .finalize();
    CallParser({"--arg", "1", "2"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{1,2};   
    ASSERT_TRUE(check);         
}

MYTEST(NargsOptionalOnly){
    parser.addArgument<int>("--arg")
            .nargs<0, 3>()
            .finalize();
    parser.addArgument<int>("--arg2")
            .nargs<0, 3>()
            .finalize();
    CallParser({"--arg", "--arg2", "1", "2"});
    bool check = parser.getValue<std::vector<int>>("--arg2") == std::vector<int>{1,2};
    ASSERT_TRUE(check);
}

MYTEST(NargsChoices){
    parser.addArgument<int>("--arg")
            .nargs<3>()
            .finalize()
            .choices(2,3,4);
    EXPECT_THROW(CallParser({"--arg", "1", "2", "3"}), argParser::unparsed_param) << "Should check choices for nargs as well";             
}

MYTEST(NargsPosMandatoryAndOptional){
    parser.addPositional<int>("pos")
            .nargs<1, 3>()
            .finalize();
    CallParser({"1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);            
}

MYTEST(NargsPosMandatoryAndOptionalPartial){
    parser.addPositional<int>("pos")
            .nargs<1, 3>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"1"}));
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1};
    ASSERT_TRUE(check);
}

MYTEST(NargsPosMandatoryAndVariadic){
    parser.addPositional<int>("pos")
            .nargs<1, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({"1", "2", "3"}));
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
}

MYTEST(NargsSinglePos){
    parser.addPositional<int>("pos")
            .nargs<1>()
            .finalize();
    CallParser({"123"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);
}

MYTEST(NargsPosOptionalZero){
    parser.addPositional<int>("pos")
            .nargs<0, 3>()
            .finalize();
    EXPECT_NO_THROW(CallParser({})) << "Should parse empty list";
}

MYTEST(NargsPosNotEnough){
    parser.addPositional<int>("pos")
            .nargs<3>()
            .finalize();
    EXPECT_THROW(CallParser({"1", "2"}), argParser::parse_error) << "Should throw if not enough args were provided";                
}

MYTEST(NargsPosWithRegularPos){
    parser.addPositional<int>("pos")
            .nargs<3>()
            .finalize();
    parser.addPositional<int>("pos2").finalize();
    CallParser({"1", "2", "3", "4"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("pos2"), 4);
}

MYTEST(NargsPureVariadicEmpty){
    parser.addArgument<int>("--var")
            .nargs<0, -1>() //pure variadic arg
            .finalize();
    CallParser({"--var"});
    ASSERT_TRUE(parser["--var"].is_set());  
    EXPECT_NO_THROW(parser.getValue<std::vector<int>>("--var"));          
    ASSERT_TRUE(parser.getValue<std::vector<int>>("--var").empty())  << "Should be empty vector";
}

MYTEST(NargsPureVariadicOptionalNum){
    parser.addArgument<int>("--var")
            .nargs<0, -1>() //pure variadic arg
            .finalize();
    CallParser({"--var", "1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
}

MYTEST(NargsPureVariadicPos){
    parser.addPositional<int>("pos")
            .nargs<0, -1>() //pure variadic arg
            .finalize();
    CallParser({"2", "3", "4"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{2,3,4};   
    ASSERT_TRUE(check);         
}

MYTEST(NargsPureVariadicPosWithZero){
    parser.addPositional<int>("pos")
            .nargs<0, -1>()
            .finalize();
    EXPECT_NO_THROW(CallParser({}));
}

MYTEST(NargsPureVariadicChoices){
    parser.addArgument<int>("--arg")
            .nargs<0, -1>()
            .finalize()
            .choices(2,3,4);
    CallParser({"--arg", "2", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{2,2,3};            
    ASSERT_TRUE(check);
}

MYTEST(NargsPureVariadicChoicesInvalid){
    parser.addArgument<int>("--arg")
            .nargs<0, -1>()
            .finalize()
            .choices(2,3,4);
    EXPECT_THROW(CallParser({"--arg", "1", "2", "3"}), argParser::unparsed_param);                
}

MYTEST(NargsPureVariadicWithFollowingPureVariadicPos){
    parser.addArgument<int>("--arg")
            .nargs<0, -1>()
            .finalize();
    parser.addPositional<int>("pos")
            .nargs<0, -1>()
            .finalize();
    CallParser({"--arg", "1", "2", "3"});
    auto arg = parser.getValue<std::vector<int>>("--arg");
    auto pos = parser.getValue<std::vector<int>>("pos");
    bool check = arg == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_TRUE(pos.empty());
}

MYTEST(NargsPosWithFunction){
    auto func = [](const char* arg){
        return int(std::strtol(arg, nullptr, 0)) + 12;
    };
    parser.addPositional<int>("n")
            .setCallable(func)
            .nargs<1>()
            .finalize();
    CallParser({"543"});
    ASSERT_EQ(parser.getValue<int>("n"), 555) << "Should parse pos narg using function";
}

MYTEST(NargsWithFunction){
    auto func = [](const char* arg){
        return int(std::strtol(arg, nullptr, 0)) + 12;
    };
    parser.addArgument<int>("-n")
            .setCallable(func)
            .nargs<1>()
            .finalize();
    CallParser({"-n", "543"});
    ASSERT_EQ(parser.getValue<int>("-n"), 555) << "Should parse narg using function";
}

MYTEST(NargsVarWithFunction){
    auto func = [](const char* arg){
        return int(std::strtol(arg, nullptr, 0)) + 12;
    };
    parser.addArgument<int>("-n")
            .setCallable(func)
            .nargs<0, -1>()
            .finalize();
    CallParser({"-n", "543", "12", "345"});
    bool check = parser.getValue<std::vector<int>>("-n") == std::vector<int>{555, 24, 357};
    ASSERT_TRUE(check);
}

MYTEST(NargsWithFuncSideParams){
    int side_par = 1;
    auto func = [](int &s_par, const char* arg){
        return int(std::strtol(arg, nullptr, 0)) + s_par++;
    };
    parser.addArgument<int>("-n")
            .setCallable(func, std::ref(side_par))
            .nargs<0, -1>()
            .finalize();
    CallParser({"-n", "543", "12", "345"});
    bool check = parser.getValue<std::vector<int>>("-n") == std::vector<int>{544, 14, 348};
    ASSERT_TRUE(check);
}

MYTEST(NargsForMandatoryArg){
    parser.addArgument<int>("i")
            .nargs<1>()
            .finalize();
    CallParser({"i", "543"});
    ASSERT_EQ(parser.getValue<int>("i"), 543);
}

MYTEST(NargsForMandatoryArgWithParam){
    parser.addArgument<int>("i")
            .setParameters("int")
            .nargs<1>()
            .finalize();
    CallParser({"i", "543"});
    ASSERT_EQ(parser.getValue<int>("i"), 543);
}

MYTEST(ChildParser){
    auto &child = parser.addCommand("child", "child parser");
    child.addArgument<int>("--int").finalize()
        .default_value(36);
    CallParser({"child", "--int"});
    ASSERT_EQ(child.getValue<int>("--int"), 37);
}

MYTEST(PosWithChild){
    parser.addPositional<int>("pos").finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    CallParser({"123", "child", "--int=54"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VariadicArgWithChild){
    parser.addArgument<int>("--var")
            .nargs<1, -1>()
            .finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    CallParser({"--var", "1", "2", "3", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VariadicArgWithPosAndChild){
    parser.addArgument<int>("--var")
            .nargs<1, -1>()
            .finalize();
    parser.addPositional<int>("pos").finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    CallParser({"--var", "1", "2", "3", "4", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("pos"), 4);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(ArgWithVariadicPosAndChild){
    parser.addArgument<int>("--var").setParameters("int").finalize();
    parser.addPositional<int>("pos")
            .nargs<1, -1>()
            .finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    CallParser({"--var", "1", "2", "3", "4", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{2,3,4};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("--var"), 1);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VarPosWithChild){
    parser.addPositional<int>("pos")
            .nargs<1, -1>()
            .finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    CallParser({"123", "345", "child", "--int=54"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{123, 345};
    ASSERT_TRUE(check);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(NotProvidedPosWithChild){
    parser.addPositional<int>("pos").finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    EXPECT_THROW(CallParser({"child", "--int=54"}), argParser::parse_error) << "Should throw error is positional not provided before child";
}

MYTEST(NotProvidedMandatoryWithChild){
    parser.addArgument<int>("mnd").setParameters("int").finalize();
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int").setParameters("int_val").finalize();
    EXPECT_THROW(CallParser({"child", "--int=54"}), argParser::parse_error) << "Should throw error if mandatory arg not provided before child";
}

MYTEST(TrailingArgsAfterPosThrow){
    parser.addPositional<int>("pos").finalize();
    EXPECT_THROW(CallParser({"123", "--int"}), argParser::parse_error) << "Should throw error if found trailing args after positional";
}


