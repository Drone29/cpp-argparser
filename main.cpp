#include <iostream>
#include "argparser.hpp"

std::string test(const char* a){
    if(a == nullptr){
        a = "34";
    }
    return std::string(a);
}

int tst(int a, const char* a1){
    return a + (int)strtol(a1, nullptr, 0);
}

class CL{
public:
    bool a = false;
};

bool ttt(CL *c, const char* a){
    return c->a;
}

int main(int argc, char *argv[]) {

    argParser parser;

    int global = 0;
    const char *hh = nullptr;

    CL *c = new CL();

    parser.addArgument<int>("-i, --int")
            .global_ptr(&global)
            .repeatable()
            .help("integer arbitrary argument with implicit value (repeatable)");

    parser.addArgument<int>("-j")
            .default_value((int)5)
            .repeatable()
            .help("integer arbitrary argument with implicit value and default value 5(repeatable)")
            .advanced_help("and with advanced help string");

    parser.addArgument<bool>("-b, --bool")
            .help("bool arbitrary argument that can be only set once (non-repeatable)");

    parser.addArgument<const char*>("-s, --str", {"str_value"})
            .global_ptr(&hh)
            .help("string arbitrary argument with mandatory value");

    parser.addArgument<std::string>("-p", {"[str_value]"}, test)
            .help("string arbitrary argument with arbitrary value and function test");

    parser.addArgument<int>("--hidden", {"int_value"})
            .hidden()
            .help("hidden int argument with mandatory value (can be viewed with --help -a)");

    parser.addArgument<std::vector<const char*>>("-a, --array", {"a1", "[a2]"},
            *([](const char* a1, const char* a2) -> auto{ return std::vector<const char*>{a1, a2==nullptr?"null":a2}; }))
            .help("arbitrary argument with 2 string values (one arbitrary) and lambda converter");

    parser.addArgument<int>("v", {"vv"}, tst, std::make_tuple(5))
            .help("mandatory arg with mandatory value and side argument 5 for function tst");

    parser.addArgument<bool>("--version");

    parser.addArgument<int>("--ggg")
            .required()
            .help("Required option 1");
    parser.addArgument<int>("--ccc")
            .required()
            .help("Required option 2");

    parser.addPositional<int>("pos")
            .global_ptr(&global)
            .help("Positional arg");

    parser.parseArgs(argc, argv);


    //int k = parser<int>["-v"];

    auto b = parser.getValue<bool>("-b");
    auto s = parser.getValue<const char*>("-s");
    auto p = parser.getValue<std::string>("-p");
    auto pos = parser.getValue<int>("pos");
    auto a = parser.getValue<std::vector<const char*>>("-a");


    return 0;
}
