#include <iostream>
#include "argparser.hpp"

std::any f(const char *a1, const char *a2){
    return std::vector<const char*>({a1, a2});
}

std::any f2(const char *a1){
    return (int)strtol(a1, nullptr, 0);
}

int main(int argc, char *argv[]) {

    auto parser = new argParser();
    parser->addArgument<double>("-g", "help", {"value"});
    parser->addArgument<std::vector<const char*>>("-v", "vector", {"1st", "2nd"}, f);

    parser->parseArgs(argc, argv);

    auto y = parser->getValue<int>("-g");
    auto v = parser->getValue<std::vector<const char*>>("-v");

    delete parser;

    /**
     * ,
                            [](const char* arg) -> int{return strtol(arg, nullptr, 0);}
     */

    return 0;
}
