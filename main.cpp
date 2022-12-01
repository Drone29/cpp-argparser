#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    auto parser = new argParser();

//    parser->addArgument<std::vector<const char*>>("-v, --vector", "vector", {"1st", "[2nd]"},
//            *([](int a, const char *a3) -> std::any
//            {
//                return std::vector<const char*>({""});
//            }),
//            true);
    parser->addArgument<uint8_t>("-b, --bool", "bool option", {});

    parser->parseArgs(argc, argv);

    auto b = parser->getValue("-b");

    delete parser;

    return 0;
}
