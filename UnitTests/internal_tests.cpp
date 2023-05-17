//
// Created by andrey on 17.05.2023.
//

#define HIGHLIGHT_TEST std::cout << "Starting test " << __func__ << std::endl;

#include "argparser.hpp"
template <size_t SIZE>
void call_parser(argParser &parser, const char * const(&arr)[SIZE]){
    const char* a[SIZE+1];
    a[0] = "binary_name"; // add dummy first argument
    std::cout << "Passed args: ";
    auto ptr = &a[1];
    for(auto &j : arr){
        *ptr++ = j;
        std::cout << j << " ";
    }
    std::cout << std::endl;
    parser.parseArgs(SIZE+1, const_cast<char **>(a));
}

void check_repeating(){
    HIGHLIGHT_TEST
    try{
        argParser parser;
        parser.addArgument<int>("-i ");
        call_parser(parser, {"-i", "-i"});
    }catch(argParser::parse_error &e){
        return;
    }
    throw std::runtime_error("Non-repeatable argument should throw redefinition error");
}
void check_composite(){
    HIGHLIGHT_TEST
    argParser parser;
    int i_val;
    parser.addArgument<int>("-i, --int")
            .default_value(3)
            .global_ptr(&i_val)
            .repeatable();
    call_parser(parser, {"-i", "-i", "-iii"});
    if(i_val != 8){
        throw std::runtime_error("Value should be 8");
    }
}
void check_key_similar_eq(){
    HIGHLIGHT_TEST
    argParser parser;
    std::string res;
    parser.addArgument<std::string>("--str", {"str_val"})
        .global_ptr(&res);
    call_parser(parser, {"--str=--str"});
    if(res != "--str"){
        throw std::runtime_error("Should parse '--str' after '=' as value");
    }
}
void check_key_similar_composite(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<std::string>("-s", {"str_val"});
    call_parser(parser, {"-s-sss"});
    if(parser.getValue<std::string>("-s") != "-sss"){
        throw std::runtime_error("Should parse '-sss' after '-s' as value");
    }
}
void check_variadic_opt(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic();
    call_parser(parser, {"-var", "1", "2", "3"});
    if(parser.getValue<std::vector<int>>("-var").size() != 3){
        throw std::runtime_error("Should parse 3 digits to variadic argument");
    }
}
void check_variadic_pos(){
    HIGHLIGHT_TEST
    argParser parser;
    //define lambda to parse int from 2 args
    auto lmb = [](const char* mnd_, const char* arb_ = nullptr)->int{
        auto res = argParser::scanValue<int>(mnd_);
        if(arb_ != nullptr){
            res += argParser::scanValue<int>(arb_);
        }
        return res;
    };
    parser.addArgument<int>("-i", {"mnd", "[arb]"}, *lmb);
    parser.addPositional<int>("pos")
            .variadic();
    call_parser(parser, {"-i", "1", "2", "3", "4", "5"});
    // 1,2 should go to -i, the res - to positional arg
    if(parser.getValue<int>("-i") != 3){
        throw std::runtime_error("Should parse first 2 options");
    }
    if(parser.getValue<std::vector<int>>("pos").size() != 3){
        throw std::runtime_error("Should parse 3 digits to variadic argument");
    }
}
void check_variadic_pos_throw(){
    HIGHLIGHT_TEST
    argParser parser;
    try{
        // add variadic positional arg
        parser.addPositional<int>("var_pos")
                .variadic();
        parser.addPositional<int>("pos");
    }catch(std::invalid_argument &e){
        return;
    }
    throw std::runtime_error("Show throw invalid arg error if variadic positional arg is followed by another one");
}

void check_complex(){

    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<std::string>("-p", {"[str_value]"}, *test);

    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic();

    call_parser(parser, {"-p", "-var", "1", "2", "3"});

    auto p = parser.getValue<std::string>("-p");
    auto var = parser.getValue<std::vector<int>>("-var");
    if(p != "null"){
        throw std::runtime_error("-p should be = 'null'");
    }
    if(var.size() < 3){
        throw std::runtime_error("-var size should be 3");
    }
}

void check_complex_2(){

    auto test = [](const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    };
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<std::string>("-p", {"[str_value]"}, *test);

    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic();

    call_parser(parser, {"-p=-var", "-var", "1", "2", "3"});
    auto p = parser.getValue<std::string>("-p");
    auto var = parser.getValue<std::vector<int>>("-var");
    if(p != "-var"){
        throw std::runtime_error("-p should be = '-var'");
    }
    if(var.size() < 3){
        throw std::runtime_error("-var size should be 3");
    }
}

int main(){
    std::cout << "Internal tests started" << std::endl;
    check_repeating();
    check_composite();
    check_key_similar_eq();
    check_key_similar_composite();
    check_variadic_opt();
    check_variadic_pos();
    check_variadic_pos_throw();
    check_complex();
    check_complex_2();
    return 0;
}
