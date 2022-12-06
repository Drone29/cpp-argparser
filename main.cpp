#include <iostream>
#include "argparser.hpp"

std::string test(const char* a){
    if(a == nullptr){
        a = "34";
    }
    return std::string(a);
}

int main(int argc, char *argv[]) {

    argParser parser;

    parser.addArgument<std::string>("-s, --ss", {"[string]"}, test)
            .help("help message");

    parser.parseArgs(argc, argv);

    auto x = parser.getValue<std::string>("-s");


    return 0;
}
