#include <iostream>
#include "argparser.hpp"
int test(const char* a2 = nullptr){
    if(a2 == nullptr){
        a2 = "12";
    }
    return (int)strtol(a2, nullptr, 0) + 1;
}

int main(int argc, char *argv[]) {

    auto parser = new argParser();

    parser->addArgument<int>("-i, --int", {"[arbitrary_int]"}, test)
            .help("int option");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<int>("-i");

    delete parser;

    return 0;
}
