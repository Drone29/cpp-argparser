#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    auto parser = new argParser();

    parser->addArgument<char>("-i, --int")
            .help("int option");
    parser->addArgument<std::string>("-s, --ss", {"string"})
            .help("help message");

    parser->parseArgs(argc, argv);

    auto x = parser->getValue<std::string>("-s");

    delete parser;

    return 0;
}
