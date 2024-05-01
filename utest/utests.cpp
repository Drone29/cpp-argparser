#include <gtest/gtest.h>
#include "argparser.hpp"

#define FIXTURE Utest
#define MYTEST(NAME) TEST_F(FIXTURE, NAME)

static const char * const EMPTY_ARGV[OPTS_SZ_MAGIC] = {};

// Create a test fixture
class FIXTURE : public testing::Test {
protected: 
    argParser parser;
    explicit FIXTURE(const std::string &name = "", 
        const std::string &descr = "") 
        : parser(name, descr){

    }
    template <size_t SIZE>
    void CallParser(const char * const(&arr)[SIZE]){
        constexpr size_t sz = SIZE != OPTS_SZ_MAGIC ? SIZE+1 : 1;
        const char* a[sz];
        a[0] = "binary_name"; // add dummy first argument
        if constexpr(sz > 1){
            std::cout << "\tPassed args: ";
            auto ptr = &a[1];
            for(auto &j : arr){
                *ptr++ = j;
                std::cout << "\"" << j << "\" ";
            }
            std::cout << std::endl;
        }
        parser.parseArgs(sz, const_cast<char **>(a));
    }
};

/// Common
MYTEST(EmptyKey){
    EXPECT_THROW(parser.addArgument<int>(""), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>(""), std::invalid_argument);
}

MYTEST(OnlySpaces){
    EXPECT_THROW(parser.addArgument<int>("   "), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("   "), std::invalid_argument);
}

MYTEST(InvalidKey){
    EXPECT_THROW(parser.addArgument<int>("=y"), std::invalid_argument);
    EXPECT_THROW(parser.addPositional<int>("=y"), std::invalid_argument);
}

MYTEST(InvalidKey2){
    EXPECT_THROW(parser.addArgument<int>("y,"), std::invalid_argument) << "Should treat as mandatory (removes ',')";
    EXPECT_THROW(parser.addPositional<int>("y,"), std::invalid_argument) << "Key is invalid here";
}

MYTEST(Redefinition){
    parser.addArgument<int>("--int");
    EXPECT_THROW(parser.addArgument<int>("--int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionPos){
    parser.addPositional<int>("int");
    EXPECT_THROW(parser.addPositional<int>("int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionArgPos){
    parser.addArgument<int>("int", {"int"});
    EXPECT_THROW(parser.addPositional<int>("int"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(AliasDetection){
    parser.addArgument<int>("-i, --int");
    EXPECT_NO_THROW(CallParser({"-i"})) << "Should recognise alias separated with comma";
}

MYTEST(AliasDetection2){
    parser.addArgument<int>("-i; --int");
    EXPECT_NO_THROW(CallParser({"-i"})) << "Should recognise alias separated with ;";
}

MYTEST(AliasDetection3){
    parser.addArgument<int>("   -i   --int   ");
    EXPECT_NO_THROW(CallParser({"-i"})) << "Should recognize alias separated with space";
}

MYTEST(AliasInvalid){
    EXPECT_THROW(parser.addArgument<int>("i, --int"), std::invalid_argument) << "Should detect the type of alias is different from the key";
}

MYTEST(AliasInvalidPos){
    EXPECT_THROW(parser.addPositional<int>("i, int"), std::invalid_argument) << "Should detect tinvalid alias for positional";
}

MYTEST(RedefinitionByAlias){
    parser.addArgument<int>("-i, --int");
    EXPECT_THROW(parser.addArgument<int>("-i"), std::invalid_argument) << "Should throw error in case of redefinition";
}

MYTEST(RedefinitionPosByAlias){
    parser.addArgument<int>("i, int", {"int"});
    EXPECT_THROW(parser.addPositional<int>("i"), std::invalid_argument) << "Should detect the pos name is same as other arg's alias";
}

MYTEST(MandatoryArgAtLeastOneParam){
    EXPECT_THROW(parser.addArgument<int>("i, int"), std::invalid_argument) << "Mandatory arg's param list cannot be empty";
}

MYTEST(EmptyParam){
    EXPECT_THROW(parser.addArgument<int>("-i", {""}), std::invalid_argument) << "Should detect empty params";
}

MYTEST(UnknownArg){
    parser.addArgument<int>("--int", {"int"});
    EXPECT_THROW(CallParser({"--inf", "23"}), argParser::parse_error) << "Should throw error if found unknown arg";
}

MYTEST(NegativeInt){
    parser.addArgument<int>("-i", {"int_value"});
    CallParser({"-i", "-123"});
    ASSERT_EQ(parser.getValue<int>("-i"), -123) << "Should be able to parse negative int correctly";
}

MYTEST(UnsignedLongLong){
    parser.addArgument<unsigned long long>("-l", {"long_long_value"});
    CallParser({"-l", "4000000000"});
    ASSERT_EQ(parser.getValue<unsigned long long>("-l"), 4000000000) << "Should be able to parse large number correctly";
}

MYTEST(Overflow){
    parser.addArgument<short>("-i", {"short_value"});
    EXPECT_THROW(CallParser({"-i", "70000"}), argParser::unparsed_param) << "Should throw in case of overflow";
}

MYTEST(InvalidPtr){
    EXPECT_THROW(parser.addArgument<int *>("-i", {"int_ptr"}), 
        std::invalid_argument) << "Should throw as no scan provided for int*";
}

MYTEST(NotEnoughPos){
    parser.addPositional<int>("pos"); 
    EXPECT_THROW(CallParser(EMPTY_ARGV), argParser::parse_error) << "Should throw as not enough positionals";
}

MYTEST(RepeatingThrow){
    parser.addArgument<int>("-i");
    EXPECT_THROW(CallParser({"-i", "-i"}), argParser::parse_error) << "Should throw in case of redefinition for non-repeatable argument";
}

MYTEST(GlobalPtr){
    int i;
    parser.addArgument<int>("-i", {"int"})
        .global_ptr(&i);
    CallParser({"-i", "345"});
    auto res = parser.getValue<int>("-i");
    ASSERT_EQ(res, 345);
    ASSERT_EQ(res, i) << "Global ref should work";
}

MYTEST(EqSign){
    parser.addArgument<int>("-i", {"int"});
    CallParser({"-i=123"});
    ASSERT_EQ(parser.getValue<int>("-i"), 123);
}

MYTEST(Composite){
    parser.addArgument<int>("-i", {"int"});
    CallParser({"-i123"});
    ASSERT_EQ(parser.getValue<int>("-i"), 123);
}

MYTEST(Composite2){
    // add repeatable arg with global ptr, starting at 3
    int i_val;
    parser.addArgument<int>("-i, --int")
                .default_value(3)
                .global_ptr(&i_val)
                .repeatable();
    CallParser({"-i", "-i", "-iii"});
    ASSERT_EQ(i_val, 8) << "i repeats 5 times starting at 3, so result should be 8";            
}

MYTEST(SimilarKeyAsValueEq){
    std::string res;
    parser.addArgument<std::string>("--str", {"str_val"})
                .global_ptr(&res);
    CallParser({"--str=--str"});
    ASSERT_EQ(res, "--str") << "Should parse '--str' after '=' as value";            
}

MYTEST(SimilarKeyAsValueComp){
    std::string res;
    parser.addArgument<std::string>("-s", {"str_val"})
                .global_ptr(&res);
    CallParser({"-s-sss"});
    ASSERT_EQ(res, "-sss") << "Should parse '-sss' after '-s' as value";
}

MYTEST(VariadicOpt){
    parser.addArgument<int>("--variadic, -var")
                .nargs(1, -1);
    CallParser({"-var", "1", "2", "3"});
    auto vec = parser.getValue<std::vector<int>>("-var");
    bool check = vec == std::vector<int>{1,2,3};
    ASSERT_TRUE(check) << "Should parse 3 digits to variadic argument";      
}

MYTEST(VariadicPos){
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

    parser.addArgument<int>("-i", {"mnd", "[arb]"}, lmb);
    parser.addPositional<int>("pos", pos_lmb)
                .nargs(1, -1);
    CallParser({"-i", "67", "7", "3", "4", "5"});
    // 1,2 should go to -i, the rest - to positional arg
    ASSERT_EQ(parser.getValue<int>("-i"), 74) << "Should parse first 2 options";
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{5,6,7};
    ASSERT_TRUE(check) << "Should parse 3 digits to variadic argument";
}

MYTEST(VarPosThrow){
    // add variadic positional arg
    parser.addPositional<int>("var_pos")
                .nargs(1, -1);
    EXPECT_THROW(parser.addPositional<int>("pos"), std::invalid_argument) << "Show throw invalid arg error if variadic positional arg is followed by another one";            
}

MYTEST(Complex){
    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    parser.addArgument<std::string>("-p", {"[str_value]"}, test);
    parser.addArgument<int>("--variadic, -var")
                .nargs(1, -1);
    CallParser({"-p", "-var", "1", "2", "3"});        
    auto p = parser.getValue<std::string>("-p");
    auto var = parser.getValue<std::vector<int>>("-var");    
    ASSERT_EQ(p, "null");
    ASSERT_EQ(var.size(), 3);
}

MYTEST(Complex2){
    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    parser.addArgument<std::string>("-p", {"[str_value]"}, test);
    parser.addArgument<int>("--variadic, -var")
                .nargs(1, -1);
    CallParser({"-p=-var", "-var", "1", "2", "3"});
    auto p = parser.getValue<std::string>("-p");
    auto var = parser.getValue<std::vector<int>>("-var");
    ASSERT_EQ(p, "-var");
    ASSERT_EQ(var.size(), 3);
}

MYTEST(SingleChar){
    parser.addArgument<char>("-c", {"char_value"});
    CallParser({"-c", "A"});
    ASSERT_EQ(parser.getValue<char>("-c"), 'A') << "Should parse single char";
}

MYTEST(Int8SingleInvalidChar){
    parser.addArgument<int8_t>("-i", {"int8_value"});
    EXPECT_THROW(CallParser({"-i", "A"}), std::runtime_error) << "Should throw error for non-char single digit";
}

MYTEST(Int8SingleChar){
    parser.addArgument<int8_t>("-i", {"int8_value"});
    CallParser({"-i", "9"});
    ASSERT_EQ(parser.getValue<int8_t>("-i"), 9) << "Should parse 9 into int8";
}

MYTEST(CharNumber){
    parser.addArgument<char>("-c", {"char_value"});
    CallParser({"-c", "123"});
    ASSERT_EQ(parser.getValue<char>("-c"), 123) << "Should parse multiple digits as char number";
}

MYTEST(FormatInvalidDec){
    parser.addArgument<int>("-i", {"int_value"});
    EXPECT_THROW(CallParser({"-i", "1abc"}), std::runtime_error) << "1abc should not be convertible to int";
}

MYTEST(FormatHex){
    parser.addArgument<int>("-i", {"int_value"});
    CallParser({"-i", "0x12"});
    ASSERT_EQ(parser.getValue<int>("-i"), 0x12);
}

MYTEST(FormatFloat){
    parser.addArgument<float>("-f", {"float_value"});
    CallParser({"-f", "0.123"});
    ASSERT_EQ(parser.getValue<float>("-f"), float(0.123));
}

MYTEST(FormatScientific){
    parser.addArgument<float>("-f", {"float_value"});
    CallParser({"-f", "1.000000e-05"});
    ASSERT_EQ(parser.getValue<float>("-f"), float(0.00001));
}

MYTEST(InvalidDateFormat){
    EXPECT_THROW(parser.addArgument<date_t>("date", {"date_str"})
                    .date_format("%d.%m.%"),
                    std::logic_error) << "Should throw if invalid date format";
}

MYTEST(DateWithSpacesSeparate){
    parser.addArgument<date_t>("date", {"date_str"})
                .date_format("%d.%m.%Y %H:%M");
    CallParser({"date", "01.02.2022 12:34"});
    auto date = parser.getValue<date_t>("date");
    ASSERT_EQ(date.tm_sec, 0);
    ASSERT_EQ(date.tm_min, 34);
    ASSERT_EQ(date.tm_hour, 12);
    ASSERT_EQ(date.tm_year, 2022-1900);
    ASSERT_EQ(date.tm_mon, 2-1);
    ASSERT_EQ(date.tm_mday, 1);
}

MYTEST(DateWithSpacesEq){
    parser.addArgument<date_t>("date", {"date_str"})
                .date_format("%d.%m.%Y %H:%M");
    CallParser({"date=01.02.2022 12:34"});
    auto date = parser.getValue<date_t>("date");
    ASSERT_EQ(date.tm_sec, 0);
    ASSERT_EQ(date.tm_min, 34);
    ASSERT_EQ(date.tm_hour, 12);
    ASSERT_EQ(date.tm_year, 2022-1900);
    ASSERT_EQ(date.tm_mon, 2-1);
    ASSERT_EQ(date.tm_mday, 1);
}

MYTEST(SideArgsPos){
    auto parse_pos = [](int g, const char* a){
        auto a_ = argParser::scanValue<int>(a);
        return g + a_;
    };
    parser.addPositional<int>("pos", parse_pos, std::make_tuple(34));
    CallParser({"6"});
    ASSERT_EQ(parser.getValue<int>("pos"), 40);
}

// Choices
MYTEST(Choices){
    parser.addArgument<int>("--choices", {"int"})
                .choices({0,1,2,3});
    EXPECT_THROW(CallParser({"--choices=4"}), argParser::unparsed_param) << "Should throw if not a valid choice";            
}

MYTEST(ChoicesThrow){
    EXPECT_THROW(parser.addArgument<const char*>("--choices", {"str"})
                    .choices({"1","2","3","4"}),
                    std::logic_error) << "Should throw if not arithmetic or not std::string";
}

MYTEST(ChoicesFloat){
    parser.addArgument<float>("--choices", {"float"})
                .choices({0.12f, 0.15f, 1.14f});
    EXPECT_NO_THROW(CallParser({"--choices=0.15"}));            
}

MYTEST(ChoicesChar){
    parser.addArgument<char>("--choices", {"char"})
                .choices({'a', 'b', 'c'});
    EXPECT_NO_THROW(CallParser({"--choices=b"}));              
}

MYTEST(ChoicesString){
    using namespace std::string_literals;
    parser.addArgument<std::string>("--choices", {"int"})
                .choices({"abc"s, "def"s, "ghi"s});
    EXPECT_NO_THROW(CallParser({"--choices=def"}));             
}

MYTEST(ChoicesVarPos){
    parser.addPositional<int>("ch_pos")
                .nargs(1, -1)
                .choices({0,1,2,3});
    CallParser({"2", "3", "0", "1"});
    bool check = parser.getValue<std::vector<int>>("ch_pos") == std::vector<int>{2,3,0,1};
    ASSERT_TRUE(check) << "Variadic choices should be allowed"; 
}

MYTEST(ChoicesSingleNArgs){
    parser.addPositional<int>("ch_pos")
                .nargs(1)
                .choices({0,1,2,3});
    CallParser({"3"});
    ASSERT_EQ(parser.getValue<int>("ch_pos"), 3) << "Should parse single narg with allowed choice";           
}

MYTEST(ChoicesVarPosThrow){
    parser.addPositional<int>("ch_pos")
                    .nargs(1, -1)
                    .choices({0,1,2,3});
    EXPECT_THROW(CallParser({"2", "3", "4", "1"}), argParser::unparsed_param) << "Should throw error if unknown value is found";                
}

/// Nargs
MYTEST(Nargs){
    parser.addArgument<int>("--arg")
                .nargs(3);
    CallParser({"--arg", "1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);            
}

MYTEST(NargsArbitrary){
    parser.addArgument<int>("--arg")
                .nargs(1, 3);
    CallParser({"--arg", "1", "2"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{1,2};   
    ASSERT_TRUE(check);         
}

MYTEST(NargsArbitrary2){
    parser.addArgument<int>("--arg")
                .nargs(0, 3);
    parser.addArgument<int>("--arg2")
                .nargs(0, 3);
    CallParser({"--arg", "--arg2", "1", "2"});
    bool check = parser.getValue<std::vector<int>>("--arg2") == std::vector<int>{1,2};
    ASSERT_TRUE(check);
}

MYTEST(NargsChoices){
    parser.addArgument<int>("--arg")
                    .nargs(3)
                    .choices({2,3,4});
    EXPECT_THROW(CallParser({"--arg", "1", "2", "3"}), argParser::unparsed_param) << "Should check choices for nargs as well";             
}

MYTEST(NargsPos){
    parser.addPositional<int>("pos")
                .nargs(1, 3);
    CallParser({"1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);            
}

MYTEST(NargsPosArb){
    parser.addPositional<int>("pos")
                .nargs(1, 3);
    CallParser({"1", "2", "3"});    
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);         
}

MYTEST(NargsPosArbZero){
    parser.addPositional<int>("pos")
                .nargs(0, 3);
    EXPECT_NO_THROW(CallParser(EMPTY_ARGV)) << "Should parse empty list";            
}

MYTEST(NargsPosNotEnough){
    parser.addPositional<int>("pos")
                    .nargs(3);
    EXPECT_THROW(CallParser({"1", "2"}), argParser::parse_error) << "Should throw if not enough args were provided";                
}

MYTEST(NargsPosWithRegularPos){
    parser.addPositional<int>("pos")
                .nargs(3);
    parser.addPositional<int>("pos2");
    CallParser({"1", "2", "3", "4"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("pos2"), 4);
}

MYTEST(NargsPureVariadicEmpty){
    parser.addArgument<int>("--var")
                .nargs(0, -1); //pure variadic arg
    CallParser({"--var"});
    ASSERT_TRUE(parser["--var"].is_set());  
    EXPECT_NO_THROW(parser.getValue<std::vector<int>>("--var"));          
    ASSERT_TRUE(parser.getValue<std::vector<int>>("--var").empty())  << "Should be empty vector";
}

MYTEST(NargsPureVariadicArbitraryNum){
    parser.addArgument<int>("--var")
                .nargs(0, -1); //pure variadic arg
    CallParser({"--var", "1", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
}

MYTEST(NargsPureVariadicPos){
    parser.addPositional<int>("pos")
                .nargs(0, -1); //pure variadic arg
    CallParser({"2", "3", "4"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{2,3,4};   
    ASSERT_TRUE(check);         
}

MYTEST(NargsPureVariadicPosWithZero){
    parser.addPositional<int>("pos")
                .nargs(0, -1); //pure variadic arg
    EXPECT_NO_THROW(CallParser(EMPTY_ARGV));            
}

MYTEST(NargsZero){
    parser.addArgument<int>("-z")
                .default_value(3)
                .nargs(0);
    CallParser({"-z"});
    ASSERT_EQ(parser.getValue<int>("-z"), 4) << "Should treat as implicit";
}

MYTEST(NargsSingle){
    parser.addArgument<int>("-z")
                .nargs(1);
    CallParser({"-z", "123"});
    ASSERT_EQ(parser.getValue<int>("-z"), 123);            
}

MYTEST(NargsSinglePos){
    parser.addPositional<int>("pos")
                .nargs(1);
    CallParser({"123"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);            
}

MYTEST(NargsPureVariadicChoices){
    parser.addArgument<int>("--arg")
                .choices({2,3,4})
                .nargs(0, -1);
    CallParser({"--arg", "2", "2", "3"});
    bool check = parser.getValue<std::vector<int>>("--arg") == std::vector<int>{2,2,3};            
    ASSERT_TRUE(check);
}

MYTEST(NargsPureVariadicChoicesInvalid){
    parser.addArgument<int>("--arg")
                    .choices({2,3,4})
                    .nargs(0, -1);
    EXPECT_THROW(CallParser({"--arg", "1", "2", "3"}), argParser::unparsed_param);                
}

MYTEST(NargsPureVariadicWithFollowingPureVariadicPos){
    parser.addArgument<int>("--arg")
                .nargs(0, -1);
    parser.addPositional<int>("pos")
                .nargs(0, -1);
    CallParser({"--arg", "1", "2", "3"});
    auto arg = parser.getValue<std::vector<int>>("--arg");
    auto pos = parser.getValue<std::vector<int>>("pos");
    bool check = arg == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_TRUE(pos.empty());
}

MYTEST(NargsPosWithFunction){
    auto func = [](const char* arg){
        return std::strtod(arg, nullptr) + 12;
    };
    parser.addPositional<int>("n", func)
                .nargs(1);
    CallParser({"543"});
    ASSERT_EQ(parser.getValue<int>("n"), 555) << "Should parse pos narg using function";
}

MYTEST(NargsWithFunction){
    auto func = [](const char* arg){
        return std::strtod(arg, nullptr) + 12;
    };
    parser.addArgument<int>("-n", SINGLE_ARG, func)
               .nargs(1);
    CallParser({"-n", "543"});
    ASSERT_EQ(parser.getValue<int>("-n"), 555) << "Should parse narg using function";           
}

MYTEST(NargsVarWithFunction){
    auto func = [](const char* arg){
        return std::strtod(arg, nullptr) + 12;
    };
    parser.addArgument<int>("-n", SINGLE_ARG, func)
               .nargs(0, -1);
    CallParser({"-n", "543", "12", "345"});
    bool check = parser.getValue<std::vector<int>>("-n") == std::vector<int>{555, 24, 357};
    ASSERT_TRUE(check);
}

MYTEST(NargsWithFuncSideParams){
    int side_par = 1;
    auto func = [](int &s_par, const char* arg){
        return std::strtod(arg, nullptr) + s_par++;
    };
    parser.addArgument<int>("-n", SINGLE_ARG, func, std::make_tuple(std::ref(side_par)))
                .nargs(0, -1);
    CallParser({"-n", "543", "12", "345"});
    bool check = parser.getValue<std::vector<int>>("-n") == std::vector<int>{544, 14, 348};
    ASSERT_TRUE(check);
}

MYTEST(ChildParser){
    auto &child = parser.addCommand("child", "child parser");
    child.addArgument<int>("--int")
        .default_value(36);
    CallParser({"child", "--int"});
    ASSERT_EQ(child.getValue<int>("--int"), 37);
}

MYTEST(PosWithChild){
    parser.addPositional<int>("pos");
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    CallParser({"123", "child", "--int=54"});
    ASSERT_EQ(parser.getValue<int>("pos"), 123);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VariadicArgWithChild){
    parser.addArgument<int>("--var")
                .nargs(1, -1);
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    CallParser({"--var", "1", "2", "3", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VariadicArgWithPosAndChild){
    parser.addArgument<int>("--var")
                .nargs(1, -1);
    parser.addPositional<int>("pos");
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    CallParser({"--var", "1", "2", "3", "4", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("--var") == std::vector<int>{1,2,3};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("pos"), 4);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(ArgWithVariadicPosAndChild){
    parser.addArgument<int>("--var", {"int"});
    parser.addPositional<int>("pos")
                .nargs(1, -1);
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    CallParser({"--var", "1", "2", "3", "4", "child", "--int", "54"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{2,3,4};
    ASSERT_TRUE(check);
    ASSERT_EQ(parser.getValue<int>("--var"), 1);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(VarPosWithChild){
    parser.addPositional<int>("pos")
                .nargs(1, -1);
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    CallParser({"123", "345", "child", "--int=54"});
    bool check = parser.getValue<std::vector<int>>("pos") == std::vector<int>{123, 345};
    ASSERT_TRUE(check);
    ASSERT_EQ(child.getValue<int>("--int"), 54);
}

MYTEST(NotProvidedPosWithChild){
    parser.addPositional<int>("pos");
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    EXPECT_THROW(CallParser({"child", "--int=54"}), argParser::parse_error) << "Should throw error is positional not provided before child";
}

MYTEST(NotProvidedMandatoryWithChild){
    parser.addArgument<int>("mnd", {"int"});
    auto &child = parser.addCommand("child", "child descr");
    child.addArgument<int>("--int", {"int_val"});
    EXPECT_THROW(CallParser({"child", "--int=54"}), argParser::parse_error) << "Should throw error if mandatory arg not provided before child";
}

MYTEST(TrailingArgsAfterPosThrow){
    parser.addPositional<int>("pos");
    EXPECT_THROW(CallParser({"123", "--int"}), argParser::parse_error) << "Should throw error if found trailing args after positional";
}


