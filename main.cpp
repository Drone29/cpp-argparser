#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    auto parser = new argParser();

    parser->addArgument<char>("-i, --int", {})
            .help("int option");
    parser->addArgument<const char*>("-s, --ss", {"string"})
    .help("help message");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<char>("-i");

    delete parser;

    return 0;
}
