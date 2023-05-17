//
// Created by andrey on 17.05.2023.
//
#include "argparser.hpp"

std::string test(const char* a){
    if(a == nullptr){
        a = "null";
    }
    return std::string(a);
}

int main(int argc, char *argv[]){

    std::cout << "CLI tests started" << std::endl;
    std::cout << "Arguments: ";
    for(int i=1; i < argc; i++){
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    int i_val;

    argParser parser;

    parser.addArgument<int>("-i, --int")
            .global_ptr(&i_val)
            .repeatable();

    parser.addArgument<int>("-j")
            .default_value(5)
            .repeatable();

    parser.addArgument<std::string>("-p", {"[str_value]"}, test);

    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic();

    parser.parseArgs(argc, argv);

    if(parser["-i"].is_set()){
        std::cout << "-i value: " << i_val << std::endl;
        if(i_val <= 0)
            throw std::runtime_error("Implicit argument -i cannot be < 0");
    }
    if(parser["-j"].is_set()){
        auto j = parser.getValue<int>("-j");
        std::cout << "-j value: " << j << std::endl;
        if(j <= 5)
            throw std::runtime_error("Implicit argument -j cannot be <= 5");
    }
    if(parser["-p"].is_set()){
        auto p = parser.getValue<std::string>("-p");
        std::cout << "-p value: " << p << std::endl;
        if(p.empty())
            throw std::runtime_error("Argument -p cannot be empty");
    }
    if(parser["-var"].is_set()){
        auto var = parser.getValue<std::vector<int>>("-var");
        std::cout << "-var value: ";
        for(auto i : var){
            std::cout << i << " ";
        }
        std::cout << std::endl;
        if(var.empty())
            throw std::runtime_error("Argument -var cannot be empty");
    }

    return 0;

}
