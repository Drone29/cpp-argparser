#include <iostream>
#include "argparser.hpp"
int test(const char* a2){
    return (int)strtol(a2, nullptr, 0) + 1;
}

int main(int argc, char *argv[]) {

    auto parser = new argParser();

    parser->addArgument<int>("-b, --bool", {"int"}, test)
            .help("hlp");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<int>("-b");

    delete parser;

    return 0;
}
