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

void check_negative_int(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    call_parser(parser, {"-i", "-123"});
    if(parser.getValue<int>("-i") != -123){
        throw std::runtime_error("Should parse -123 into int");
    }
}
void check_long_long(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<unsigned long long>("-l", {"long_long_value"});
    call_parser(parser, {"-l", "4000000000"});
    if(parser.getValue<unsigned long long>("-l") != 4000000000){
        throw std::runtime_error("Should convert long long");
    }
}
void check_invalid_pointer(){
    HIGHLIGHT_TEST
    try{
        argParser parser;
        parser.addArgument<int *>("-i", {"int_ptr"});
    }catch(std::invalid_argument &e){
        return;
    }
    throw std::runtime_error("Should throw error as no scan provided for int*");
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
//todo: error here
void check_variadic_pos(){
    HIGHLIGHT_TEST
    argParser parser;
    //define lambda to parse int from 2 args
    auto lmb = [&](const char* mnd_, const char* arb_) -> int{
        auto res = argParser::scanValue<int>(mnd_);
        if(arb_ != nullptr){
            res += argParser::scanValue<int>(arb_);
        }
        return res;
    };

    parser.addArgument<int>("-i", {"mnd", "[arb]"}, lmb);

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
void check_single_char(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<char>("-c", {"char_value"});
    call_parser(parser, {"-c", "A"});
    if(parser.getValue<char>("-c") != 'A'){
        throw std::runtime_error("should parse single char");
    }
}
void check_int8_single_invalid_char(){
    HIGHLIGHT_TEST
    try{
        argParser parser;
        parser.addArgument<int8_t>("-i", {"int8_value"});
        call_parser(parser, {"-i", "A"});
    }catch(std::runtime_error &){
        return;
    }
    throw std::runtime_error("Should throw error for non-char single digit");
}
void check_int8_single_char(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<int8_t>("-i", {"int8_value"});
    call_parser(parser, {"-i", "9"});
    if(parser.getValue<int8_t>("-i") != 9){
        throw std::runtime_error("Should parse 9 into int8");
    }
}
void check_char_number(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<char>("-c", {"char_value"});
    call_parser(parser, {"-c", "123"});
    if(parser.getValue<char>("-c") != 123){
        throw std::runtime_error("should parse multiple digits as char number");
    }
}
void check_format_invalid_dec(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    try{
        call_parser(parser, {"-i", "1abc"});
    }catch(std::runtime_error &e){
        return;
    }
    throw std::runtime_error("1abc should not be convertible to int");
}
void check_format_hex(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    call_parser(parser, {"-i", "0x12"});
    if(parser.getValue<int>("-i") != 0x12){
        throw std::runtime_error("should parse hex numbers");
    }
}
void check_format_float(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<float>("-f", {"float_value"});
    call_parser(parser, {"-f", "0.123"});
    auto f = parser.getValue<float>("-f");
    char tmp[32];
    sprintf(tmp, "%.3f", f);
    auto f_str = std::string(tmp);
    if(f_str != "0.123"){
        throw std::runtime_error("should parse float numbers");
    }
}
void check_format_scientific(){
    HIGHLIGHT_TEST
    argParser parser;
    parser.addArgument<float>("-f", {"float_value"});
    call_parser(parser, {"-f", "1.000000e-05"});
    auto f = parser.getValue<float>("-f");
    char tmp[32];
    sprintf(tmp, "%.5f", f);
    auto f_str = std::string(tmp);
    if(f_str != "0.00001"){
        throw std::runtime_error("should parse scientific float numbers");
    }
}
void check_invalid_date_format(){
    HIGHLIGHT_TEST
    try{
        argParser parser;
        parser.addArgument<date_t>("date", {"date_str"})
                .date_format("%d.%m.%");
    }catch(std::logic_error &e){
        return;
    }
    throw std::runtime_error("should throw if invalid date format");
}
void check_invalid_date_format_2(){
    HIGHLIGHT_TEST
    try{
        argParser parser;
        parser.addArgument<date_t>("date", {"date_str"})
                .date_format("%d.%m.%Y %H:%M");
    }catch(std::logic_error &e){
        return;
    }
    throw std::runtime_error("should throw if invalid date format (spaces)");
}

int main(){

    std::cout << "Internal tests started" << std::endl;
    check_negative_int();
    check_long_long();
    check_invalid_pointer();
    check_repeating();
    check_composite();
    check_key_similar_eq();
    check_key_similar_composite();
    check_variadic_opt();
    check_variadic_pos();
    check_variadic_pos_throw();
    check_complex();
    check_complex_2();
    check_format_invalid_dec();
    check_format_hex();
    check_format_float();
    check_format_scientific();
    check_int8_single_invalid_char();
    check_int8_single_char();
    check_single_char();
    check_char_number();
    check_invalid_date_format();
    check_invalid_date_format_2();
    return 0;
}
