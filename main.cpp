#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    argParser parser;

    parser.addArgument<std::string>("-s, --ss", {"string"})
            .help("help message");
    parser["-s"].default_value(std::string("default string"));

    parser.parseArgs(argc, argv);

    auto x = parser.getValue<std::string>("-s");


    return 0;
}
