#include <iostream>
#include "argparser.hpp"
int test(){
    return 156;
}

int main(int argc, char *argv[]) {

    std::any i = (int)1;

    auto parser = new argParser();

    parser->addArgument<int>("-i, --int", {})
            .help("int option");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<int>("-i");

    delete parser;

    return 0;
}
