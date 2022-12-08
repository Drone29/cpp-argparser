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

class C{
public:
    C() = default;
    template <typename T, class...Targs>
    struct Creator{
        template<typename...args>
        auto create(T(*func)(Targs..., args...), Targs...targs){
            return new DerivedOption<T, Targs...>(func, targs...);
        }
    };
};




int main(int argc, char *argv[]) {

    C ccc;
    auto x = C::Creator<int,int>();
    auto y = x.create(tst,5);
    auto z = y->action({"123"});


    argParser parser;

//    parser.addArgument<int>("-i, --int")
//            .help("integer arbitrary argument with implicit value (repeatable)");
//
//    parser.addArgument<int>("-j")
//            .default_value((int)5)
//            .help("integer arbitrary argument with implicit value and default value 5(repeatable)")
//            .advanced_help("and with advanced help string");
//
//    parser.addArgument<bool>("-b, --bool")
//            .non_repeatable()
//            .help("bool arbitrary argument that can be only set once (non-repeatable)");
//
//    parser.addArgument<const char*>("-s, --str", {"str_value"})
//            .help("string arbitrary argument with mandatory value");
//
//    parser.addArgument<std::string>("-p", {"[str_value]"}, test)
//            .help("string arbitrary argument with arbitrary value and function test");
//
//    parser.addArgument<int>("--hidden", {"int_value"})
//            .hidden()
//            .help("hidden int argument with mandatory value (can be viewed with --help -a)");
//
//    parser.addArgument<std::vector<const char*>>("-a, --array", {"a1", "[a2]"},
//            *([](const char* a1, const char* a2) -> auto{ return std::vector<const char*>{a1, a2==nullptr?"null":a2}; }))
//            .help("arbitrary argument with 2 string values (one arbitrary) and lambda converter");

//    parser.addArgument<int, int>("-v", {"vv"}, tst, 5)
//            .help("blah blah blah");

//    parser.parseArgs(argc, argv);
//
//    auto i = parser.getValue<int>("-i");
//    auto j = parser.getValue<int>("-j");
//    auto b = parser.getValue<bool>("-b");
//    auto s = parser.getValue<const char*>("-s");
//    auto p = parser.getValue<std::string>("-p");
//    auto m = parser.getValue<int>("m");
//    auto pos = parser.getValue<int>("pos");
//    auto a = parser.getValue<std::vector<const char*>>("-a");


    return 0;
}
