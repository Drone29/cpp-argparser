#include <iostream>
#include "argparser.hpp"

int main(int argc, char *argv[]) {

    auto parser = new argParser();
   // parser->addArgument<int>("g", "help", {"value"});
    parser->addArgument<std::vector<const char*>>("-v, --vector", "vector", {"1st", "[2nd]"},
            *([](const char *a1, const char *a2) -> std::any
            {return std::vector<const char*>({a1, a2});}));
    parser->addArgument<bool>("-b, --bool", "bool option", {"[bool_val]"});
    parser->addPositional<int>("positional", "positional argument");

    parser->parseArgs(argc, argv);

    auto b = parser->getValue<bool>("-b");

    delete parser;

    return 0;
}
