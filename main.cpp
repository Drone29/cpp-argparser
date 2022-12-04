#include <iostream>
#include "argparser.hpp"
bool o = false;

int main(int argc, char *argv[]) {

    auto parser = new argParser();

    parser->addArgument<std::vector<const char*>>("-v, --vector", {"1st", "[2nd]"},
            *([](const char* a1, const char* a2) -> std::any{
                return std::vector<const char*>({a1, a2});
            }))
            .help("Vector")
            .advanced_help("advanced help");

    parser->addArgument<bool>("-b, --bool", {})
            .help("hlp");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<bool>("-b");

    delete parser;

    return 0;
}
