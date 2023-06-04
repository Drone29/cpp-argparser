//
// Created by andrey on 17.05.2023.
//

static int test_num = 0;
#define START_TEST std::cout << "Starting test " << ++test_num << " " << __func__ << std::endl;
#include "argparser.hpp"

template <size_t SIZE>
void call_parser(argParser &parser, const char * const(&arr)[SIZE]){
    const char* a[SIZE+1];
    a[0] = "binary_name"; // add dummy first argument
    std::cout << "\tPassed args: ";
    auto ptr = &a[1];
    for(auto &j : arr){
        *ptr++ = j;
        std::cout << "\"" << j << "\" ";
    }
    std::cout << std::endl;
    parser.parseArgs(SIZE+1, const_cast<char **>(a));
}
void check_negative_int(){
    START_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    call_parser(parser, {"-i", "-123"});
    if(parser.getValue<int>("-i") != -123){
        throw std::runtime_error("Should parse -123 into int");
    }
}
void check_long_long(){
    START_TEST
    argParser parser;
    parser.addArgument<unsigned long long>("-l", {"long_long_value"});
    call_parser(parser, {"-l", "4000000000"});
    if(parser.getValue<unsigned long long>("-l") != 4000000000){
        throw std::runtime_error("Should convert long long");
    }
}
void check_overflow(){
    START_TEST
    try{
        argParser parser;
        parser.addArgument<short int>("-i", {"short_value"});
        call_parser(parser, {"-i", "70000"});
    }catch (std::runtime_error &){
        return;
    }
    throw std::runtime_error("Should throw error if overflow");
}
void check_invalid_pointer(){
    START_TEST
    try{
        argParser parser;
        parser.addArgument<int *>("-i", {"int_ptr"});
    }catch(std::invalid_argument &){
        return;
    }
    throw std::runtime_error("Should throw error as no scan provided for int*");
}
void check_repeating_throw(){
    START_TEST
    try{
        argParser parser;
        parser.addArgument<int>("-i ");
        call_parser(parser, {"-i", "-i"});
    }catch(argParser::parse_error &){
        return;
    }
    throw std::runtime_error("Non-repeatable argument should throw redefinition error");
}
void check_composite(){
    START_TEST
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
    START_TEST
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
    START_TEST
    argParser parser;
    parser.addArgument<std::string>("-s", {"str_val"});
    call_parser(parser, {"-s-sss"});
    if(parser.getValue<std::string>("-s") != "-sss"){
        throw std::runtime_error("Should parse '-sss' after '-s' as value");
    }
}
void check_variadic_opt(){
    START_TEST
    argParser parser;
    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic();
    call_parser(parser, {"-var", "1", "2", "3"});
    if(parser.getValue<std::vector<int>>("-var") != std::vector<int>{1,2,3}){
        throw std::runtime_error("Should parse 3 digits to variadic argument");
    }
}

void check_variadic_pos(){
    START_TEST
    argParser parser;
    //define lambda to parse int from 2 args
    auto lmb = [&](const char* mnd_, const char* arb_) -> int{
        auto res = argParser::scanValue<int>(mnd_);
        if(arb_ != nullptr){
            res += argParser::scanValue<int>(arb_);
        }
        return res;
    };

    auto pos_lmb = [](const char *a) -> int{
        auto res = argParser::scanValue<int>(a);
        return res;
    };

    parser.addArgument<int>("-i", {"mnd", "[arb]"}, lmb);

    parser.addPositional<int>("pos", pos_lmb)
            .variadic();
    call_parser(parser, {"-i", "1", "2", "3", "4", "5"});
    // 1,2 should go to -i, the res - to positional arg
    if(parser.getValue<int>("-i") != 3){
        throw std::runtime_error("Should parse first 2 options");
    }
    if(parser.getValue<std::vector<int>>("pos") != std::vector<int>{3,4,5}){
        throw std::runtime_error("Should parse 3 digits to variadic argument");
    }
}
void check_variadic_pos_throw(){
    START_TEST
    argParser parser;
    try{
        // add variadic positional arg
        parser.addPositional<int>("var_pos")
                .variadic();
        parser.addPositional<int>("pos");
    }catch(std::invalid_argument &){
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
    START_TEST
    argParser parser;
    parser.addArgument<std::string>("-p", {"[str_value]"}, test);

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
    START_TEST
    argParser parser;
    parser.addArgument<std::string>("-p", {"[str_value]"}, test);

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
    START_TEST
    argParser parser;
    parser.addArgument<char>("-c", {"char_value"});
    call_parser(parser, {"-c", "A"});
    if(parser.getValue<char>("-c") != 'A'){
        throw std::runtime_error("should parse single char");
    }
}
void check_int8_single_invalid_char(){
    START_TEST
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
    START_TEST
    argParser parser;
    parser.addArgument<int8_t>("-i", {"int8_value"});
    call_parser(parser, {"-i", "9"});
    if(parser.getValue<int8_t>("-i") != 9){
        throw std::runtime_error("Should parse 9 into int8");
    }
}
void check_char_number(){
    START_TEST
    argParser parser;
    parser.addArgument<char>("-c", {"char_value"});
    call_parser(parser, {"-c", "123"});
    if(parser.getValue<char>("-c") != 123){
        throw std::runtime_error("should parse multiple digits as char number");
    }
}
void check_format_invalid_dec(){
    START_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    try{
        call_parser(parser, {"-i", "1abc"});
    }catch(std::runtime_error &){
        return;
    }
    throw std::runtime_error("1abc should not be convertible to int");
}
void check_format_hex(){
    START_TEST
    argParser parser;
    parser.addArgument<int>("-i", {"int_value"});
    call_parser(parser, {"-i", "0x12"});
    if(parser.getValue<int>("-i") != 0x12){
        throw std::runtime_error("should parse hex numbers");
    }
}
void check_format_float(){
    START_TEST
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
    START_TEST
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
    START_TEST
    try{
        argParser parser;
        parser.addArgument<date_t>("date", {"date_str"})
                .date_format("%d.%m.%");
    }catch(std::logic_error &){
        return;
    }
    throw std::runtime_error("should throw if invalid date format");
}
void check_date_with_spaces_separate(){
    START_TEST
    argParser parser;
    parser.addArgument<date_t>("date", {"date_str"})
            .date_format("%d.%m.%Y %H:%M");
    call_parser(parser, {"date", "01.02.2022 12:34"});
    if(parser.getValue<date_t>("date").tm_min != 34){
        throw std::runtime_error("should parse date with space if defined as separate argument");
    }
}
void check_date_with_spaces_eq(){
    START_TEST
    argParser parser;
    parser.addArgument<date_t>("date", {"date_str"})
            .date_format("%d.%m.%Y %H:%M");
    call_parser(parser, {"date=01.02.2022 12:34"});
    if(parser.getValue<date_t>("date").tm_min != 34){
        throw std::runtime_error("should parse date with space if defined after equals");
    }
}
void check_side_args_pos(){
    START_TEST
    argParser parser;

    auto parse_pos = [](int g, const char* a){
        auto a_ = argParser::scanValue<int>(a);
        return g + a_;
    };

    parser.addPositional<int>("pos", parse_pos, std::make_tuple(34));
    call_parser(parser, {"6"});
    if(parser.getValue<int>("pos") != 40){
        throw std::runtime_error("result should be 40");
    }
}
void check_choices(){
    START_TEST
    argParser parser;
    parser.addArgument<int>("--choices", {"int"})
            .choices({0,1,2,3});
    try{
        call_parser(parser, {"--choices=4"});
    }catch(argParser::unparsed_param &){
        return;
    }
    throw std::runtime_error("should throw if not a valid choice");
}
void check_choices_throw(){
    START_TEST
    try{
        argParser parser;
        parser.addArgument<const char*>("--choices", {"str"})
                .choices({"1","2","3","4"});
    }catch(std::logic_error &){
        return;
    }
    throw std::runtime_error("should throw if not arithmetic or not std::string");
}
void check_choices_float(){
    START_TEST
    argParser parser;
    parser.addArgument<float>("--choices", {"int"})
            .choices({0.12f, 0.15f, 1.14f});
    call_parser(parser, {"--choices=1.14"});
}
void check_choices_var_pos(){
    START_TEST
    argParser parser;
    parser.addPositional<int>("ch_pos")
            .variadic()
            .choices({0,1,2,3});
    call_parser(parser, {"2", "3", "0", "1"});
    if(parser.getValue<std::vector<int>>("ch_pos") != std::vector<int>{2,3,0,1}){
        throw std::runtime_error("variadic choices should be allowed");
    }
}
int main(){

    std::cout << "Internal tests started" << std::endl;
    check_negative_int();
    check_long_long();
    check_overflow();
    check_invalid_pointer();
    check_repeating_throw();
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
    check_date_with_spaces_separate();
    check_date_with_spaces_eq();
    check_side_args_pos();
    check_choices();
    check_choices_float();
    check_choices_throw();
    check_choices_var_pos();
    return 0;
}
