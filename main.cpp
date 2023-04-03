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
    CL(bool _a, int _b) : a{_a}, b{_b} {}
    bool a = false;
    int b = 0;
};

CL *createClass(const char *bl, const char *itgr = nullptr){
    //static parser helper function, converts string to  basic type
    bool b = argParser::scanValue<bool>(bl);
    int i = argParser::scanValue<int>(itgr);
    return new CL(b, i);
}

int main(int argc, char *argv[]) {

    argParser parser;

    int global = 0;
    int i_val = 0;
    const char *hh = nullptr;

    /**
     *  Args with implicit values
     *
     *  Can be called only by key without parameters: '-x', '-b', '--bool', etc.
     *
     *  Arguments can be repeatable or non-repeatable (default)
     *
     *  Short repeatable implicit arguments that start with '-' can be combined using single '-':
     *  '-jjj', '-ij'
     */

    // for implicit values, if no function provided and the type is arithmetic,
    // parser will increment their value if called,
    // starting from 0 or default_value, if provided.
    // by default, the arguments are NON-REPEATABLE,
    // i.e. a call '-x -x' or '-xx' is NOT VALID and will cast an error
    parser.addArgument<int>("-x")
            .help("int arbitrary argument with implicit value (if set, returns 1)");

    // bool is also considered arithmetic,
    // so parser will increment it, thus setting it to true.
    // again, it's NON-REPEATABLE
    parser.addArgument<bool>("-b, --bool")
            .help("bool arbitrary argument with alias and implicit value (if set, returns true)");

    // REPEATABLE argument can be specified more than once,
    // in this case, due to implicit value, it will increment each time it's called
    // i.e. a call '-j -j -j' or '-jjj' will increment counter 3 times, starting from 5 (default value),
    // and returns 8
    parser.addArgument<int>("-j")
            .default_value(5)
            .repeatable()
            .help("int arbitrary repeatable argument with implicit value and default value 5")
            .advanced_help("and with advanced help string (can be viewed with --help -j)");

    parser.addArgument<int>("-i, --int")
            .global_ptr(&i_val)
            .repeatable()
            .help("int arbitrary repeatable argument with alias, implicit value and pointer to global variable (if set, returns 1)");

    /**
     *  Arguments can be mandatory, arbitrary, required and positional
     *
     *  Arguments specified with '-' are by default arbitrary,
     *  But can be forced to be mandatory or required
     *
     *  Arguments specified without '-' are mandatory,
     *  But can be forced to be required
     *
     *  Mandatory - all such arguments should be provided
     *  Required - at least 1 such argument should be provided
     *
     */

    // arbitrary arguments can be made required
    // in this case, user should specify at least one of '--req1' and '--req2'
    parser.addArgument<int>("--req1")
            .required()
            .help("required argument 1 with implicit value");

    parser.addArgument<int>("--req2")
            .required()
            .help("required argument 2 with implicit value");

    /**
     *  Args with parameters
     *
     *  Can be called with parameters: '-s aaa', '--str=aaa', '-sFFF', etc.
     *  Or can be called without them, if parameter is arbitrary: '-p'
     *
     *  One argument can have up to 10 parameters
     *
     *  Delimiters between argument and parameter can be:
     *  '=', <space>, <empty_string>
     *
     *  I.e. '-s aaa', '--str=aaa', '-sFFF' are valid calls
     *
     *  NOTICE: Arguments that doesn't start with '-' are not allowed to be used in contiguous format.
     *  I.e. 'vFFF', 'v123' are not valid,
     *  Although 'v=FFF', 'v=123' are still valid
     *
     *  Arbitrary parameters can be omitted
     *  Mandatory parameters cannot be omitted
     *
     *  Enclose parameter name with [] to make it arbitrary,
     *  Not []-enclosed params are considered as mandatory
     */

    // arguments without '-' are mandatory
    // mandatory arguments should have at least 1 mandatory parameter
    parser.addArgument<bool>("m", {"m_param"})
            .help("mandatory bool argument with mandatory parameter");

    // str_value - mandatory value, cannot be omitted
    // i.e. calls like '-s aaa' or '--str=aaa' are VALID,
    // but '-s' or '--str' are NOT VALID and will cast an error
    parser.addArgument<const char*>("-s, --str", {"str_value"})
            .global_ptr(&hh)
            .help("string arbitrary argument with mandatory param and pointer to global variable (if set, returns specified string)");

    // [str_value] - arbitrary value, can be omitted
    // i.e. calls '-p aaaa' and '-p' are both VALID
    // function should return type of argument,
    // and its number of const char* arguments should be the same as number of parameters specified in {}
    parser.addArgument<std::string>("-p", {"[str_value]"}, test)
            .help("string arbitrary argument with arbitrary value and function test (if set, returns result of test())");

    // hidden arguments are not shown when '--help' called without '-a' specifier
    parser.addArgument<int>("--hidden", {"int_value"})
            .hidden()
            .help("hidden int argument with mandatory value (can be viewed with --help -a)");

    // function can accept not only string arguments that should be parsed,
    // but also any number of side arguments.
    // side arguments need to be placed before strings:
    // func(int a, const char* b) is VALID,
    // func(const char*b, int a) is NOT VALID
    // side arguments should be passed to addArgument after function inside a tuple
    parser.addArgument<int>("v", {"vv"}, tst, std::make_tuple(5))
            .help("mandatory arg with mandatory value and side argument 5 for function tst");

    // return type can be almost any type, all you need is a right function
    // also this one is forced to be required instead of mandatory
    parser.addArgument<CL*>("class", {"bool", "[integer]"}, createClass)
            .required()
            .help("create class from 2 strings, one is arbitrary");

    // non-capturing lambdas are considered as functions too
    // only they need to be dereferenced with *
    // NOTE! '-a' alias conflicts with help's self key '-a', but it's not an error
    // Calling '--help -a' will list advanced options, but
    // '--help --array' will return help for --array argument
    parser.addArgument<std::vector<const char*>>("-a, --array", {"a1", "[a2]"},
                                                 *([](const char* a1, const char* a2) -> auto
                                                 {
                                                     return std::vector<const char*>{a1, a2==nullptr?"null":a2};
                                                 }))
            .help("arbitrary argument with 2 string values (one arbitrary) and lambda converter");

    // positional arguments should be specified after all other arguments
    // they cannot be hidden or made arbitrary
    parser.addPositional<int>("pos")
            .global_ptr(&global)
            .help("Positional arg");

    // there's an date_t type which is an alias for std::tm struct
    parser.addArgument<date_t>("date", {"date"})
            // date_format can be specified for date_t type.
            // Format must correspond to strptime() format
            // If not specified, default format used (%Y-%m-%dT%H:%M:%S)
            // date_format takes optional argument hide_in_help.
            // If set to true, the format will not be shown in help.
            // Otherwise it'll be shown in {} brackets exactly as it's specified in the 1st argument
            .date_format("%d.%m.%Y-%H:%M")
            .help("Converts date string to date struct");

    // parseArgs accepts 3 parameters: argc, argv and optional bool 'allow_zero_options'
    // if allow_zero_options is true, it will not cast errors if required or mandatory arguments were not specified
    parser.parseArgs(argc, argv);

    // const methods of argument can be accessed via [ ]
    // here it returns if argument 'v' was set by user
    auto isArgumentSet = parser["v"].is_set();

    // to get parsed value, use getValue, explicitly setting the type,
    // or set a global_ptr beforehand
    auto b = parser.getValue<bool>("-b");
    auto s = parser.getValue<const char*>("-s");
    auto p = parser.getValue<std::string>("-p");
    auto pos = parser.getValue<int>("pos");
    auto a = parser.getValue<std::vector<const char*>>("-a");

    return 0;
}
