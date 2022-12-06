#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    std::any i = (int)1;

    auto parser = new argParser();

    parser->addArgument<char>("-i, --int", {})
            .help("int option");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<char>("-i");

    delete parser;

    return 0;
}
