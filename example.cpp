#include <iostream>
#include "argparser.hpp"

std::string test(const char* a){
    if(a == nullptr){
        a = "null";
    }
    return std::string(a);
}

int tst(int a, const char* a1){
    return a + (int)strtol(a1, nullptr, 0);
}

struct CL{
    bool a = false;
    int b = 0;
};

CL createStruct(const char *bl, const char *itgr = nullptr){
    //static parser helper function, converts string to  basic type
    bool b = argParser::scanValue<bool>(bl);
    int i = argParser::scanValue<int>(itgr);
    return CL{b, i};
}

int main(int argc, char *argv[]) {

    argParser parser("program_name", "this program does some pretty useful stuff");

    int pos_val = 0;
    int i_val;
    const char *s_val = nullptr;

    /**
     *  Args with implicit values
     *
     *  Can be called only by key without parameters: '-x', '-b', '--bool', etc.
     *
     *  Arguments can be repeatable or non-repeatable (default)
     *
     *  Short repeatable implicit arguments combined:
     *  '-jjj', '-ij', 'vvv', etc...
     */

    // for implicit values, if no parsing function provided and the type is arithmetic,
    // parser will increment their value if called,
    // starting from 0 or default_value, if provided.
    // by default, the arguments are NON-REPEATABLE,
    // i.e. a call '-x -x' or '-xx' is NOT VALID and will cast an error in the following case
    parser.addArgument<int>("-x")
            .help("int arbitrary argument with implicit value (if set, returns 1)");

    // bool is also considered arithmetic,
    // so parser will increment it, thus setting it to true.
    // again, it's NON-REPEATABLE
    // also, it has an alias -b
    parser.addArgument<bool>("-b, --bool")
            .help("bool arbitrary argument '--bool' with alias '-b' and implicit value (if set, returns true)");

    // REPEATABLE argument can be specified more than once,
    // in this case, due to implicit value, it will increment each time it's called
    // i.e. a call '-j -j -j' or '-jjj' will increment counter 3 times, starting from 5 (default value),
    // and returns 8
    parser.addArgument<int>("-j")
            .default_value(5)   // specify default value for argument
            .repeatable()   // make argument repeatable
            .help("int arbitrary repeatable argument with implicit value and default value 5")
            // advanced help message can be viewed if '--help' is called with argument's name
            .advanced_help("and with advanced help string (can be viewed with --help -j)");

    // to automatically set value to 'global' variable (visible in current scope)
    // global_ptr() method is used.
    // It takes a pointer to a variable of the type which was specified for the argument
    // after such argument is parsed, that variable's value is automatically set
    // NOTE: not applicable for variadic arguments
    parser.addArgument<int>("-i, --int")
            .global_ptr(&i_val) // specify global variable for argument
            .repeatable()   // make argument repeatable
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
     *  Arbitrary arguments can be made required or mandatory
     *
     *  Mandatory arguments can be made required
     *
     */

    // arbitrary arguments can be made required
    // in the following case, user should specify either '--req1' or '--req2', or both
    parser.addArgument<int>("--req1")
            .required() // make arbitrary argument required
            .help("required argument 1 with implicit value");

    parser.addArgument<int>("--req2")
            .required() // make arbitrary argument required
            .help("required argument 2 with implicit value");

    /**
     *  Args with parameters
     *
     *  Can be called with parameters: '-s aaa', '--str=aaa', '-sFFF', etc.
     *  Or can be called without them, if parameter is arbitrary
     *
     *  One argument can have up to 9 parameters (to increase, change MAX_ARGS variable in argparser.hpp)
     *
     *  Parameters are specified inside a parameter list {} in addArgument() method
     *
     *  Delimiters between argument and parameter can be:
     *  '='
     *  <space>
     *  <empty_string> (if argument has a short name/alias)
     *
     *  I.e. '-s aaa', '--str=aaa', '-sFFF' are valid calls
     *  'vFFF', 'v123' and 'v=FFF' are also valid
     *
     *  Arbitrary parameters can be omitted
     *  Mandatory parameters cannot be omitted
     *
     *  Enclose parameter name with [] to make it arbitrary,
     *  Not []-enclosed params are considered mandatory
     */

    // arguments without '-' are mandatory
    // mandatory arguments should have at least 1 mandatory parameter
    parser.addArgument<bool>("m", {"m_param"})
            .help("mandatory bool argument with mandatory parameter");

    // str_value - mandatory parameter, cannot be omitted
    // i.e. calls like '-s aaa' or '--str=aaa' are VALID,
    // but '-s' or '--str' are NOT VALID and will cast an error
    parser.addArgument<const char*>("-s, --str", {"str_value"})
            .global_ptr(&s_val) // pointer to global variable
            .help("string arbitrary argument with mandatory parameter");

    // an arbitrary argument can be made hidden,
    // so it's not shown in help message unless called without '-a' specifier
    // NOTE: mandatory or required arguments cannot be hidden
    parser.addArgument<int>("--hidden", {"int_value"})
            .hidden() // specify argument as hidden
            .help("hidden int argument with mandatory value (can be viewed with --help -a)");

    // there's a date_t type which is an alias for std::tm struct (specified in argparser.hpp)
    // such parameter can be parsed according to the strptime() format
    // format can be specified with date_format() method
    // NOTE: not applicable for types other than date_t (std::tm)
    parser.addArgument<date_t>("date", {"date_str"})
                    // date_format can be specified for date_t type.
                    // Format shouldn't contain any spaces and must correspond to strptime() format, otherwise std::logic_error is thrown
                    // If format not specified or specified as nullptr, default format used (defined as DEFAULT_DATE_FORMAT in argparser.hpp)
                    // date_format takes optional bool argument hide_in_help.
                    // If set to true, the format will not be shown in help.
                    // Otherwise it'll be shown in {} brackets exactly as it's specified in the 1st argument
            .date_format("%d.%m.%Y-%H:%M")
            .help("Converts date string to date struct");

    // an argument can be made variadic
    // in this case, an arbitrary number of options (but not less than 1) can be specified by the caller
    // for example, the following will add a variadic argument '--variadic'
    // so '-var 123', '-var 123 321' or '-var 123 321 231' etc... are VALID calls,
    // but '-var' is INVALID, because variadic argument must have AT LEAST ONE parameter
    // after argument is made variadic, its return type changes to std::vector<TYPE>()
    // NOTE: global_ptr() is not applicable for variadic arguments, so the only way to obtain its value is by calling parser.getValue() method
    parser.addArgument<int>("--variadic, -var", {"N"})
            .variadic() // make argument variadic
            .help("parses any number of integers. Result is std::vector<int>");

    // positional arguments can be specified by the separate method addPositional()
    // positional arguments cannot be made hidden, arbitrary or required
    // all other properties apply
    parser.addPositional<int>("pos")
            .global_ptr(&pos_val) // pointer to global variable
            .help("Positional int argument with global variable");

    // positional arguments can be variadic too
    // in that case, ONLY ONE such argument can be present
    // also, variadic positional argument should be added AFTER all other positional arguments
    parser.addPositional<int>("var_pos")
            .variadic() // make positional argument variadic
            .help("Variadic pos argument of type int");

    /**
     *  Custom parsing functions
     *
     *  By default, arithmetic, string or date_t arguments (implicit or having a single mandatory parameter)
     *  can be parsed by built-in logic, so no custom functions required
     *
     *  Although, if an argument has more than 1 parameter, has arbitrary parameters, or is of custom type,
     *  a custom parsing function is required
     *
     *  Parsing function is any function, function pointer, lambda, etc...
     *  which takes as many string (const char*) arguments that there are specified by argument's parameter list {},
     *  and returns a value of argument's type
     *
     *  Also, such function can take an arbitrary number of so-called side arguments,
     *  which are arguments of any type, specified by the function's signature
     *
     *  Side arguments (if present) in function's signature should go BEFORE string arguments which are to be parsed by that function
     *  Side arguments are specified in addArgument/addPositional method in a tuple after function itself
     *
     *  NOTE: max number of 'parsable' string arguments is specified as MAX_ARGS in argparser.hpp
     *  also, a function must have a return type corresponding to argument's type, and CANNOT BE VOID
     *
     */

    // here's an argument with custom parsing function 'test'
    // it returns a value of argument's type (std::string), and takes 1 string argument
    // [str_value] - arbitrary value, can be omitted
    // i.e. calls '-p aaaa' and '-p' are both VALID
    // function should return type of argument,
    // and its number of const char* arguments should be the same as number of parameters specified in parameter list
    parser.addArgument<std::string>("-p", {"[str_value]"}, test)
            .help("string arbitrary argument with arbitrary value and function test (if set, returns result of test())");

    // function can accept not only string arguments that should be parsed,
    // but also any number of side arguments.
    // side arguments need to be placed before strings in function declaration:
    // func(int a, const char* b) is VALID,
    // func(const char*b, int a) is NOT VALID
    // side arguments should be passed to addArgument after function inside a tuple
    parser.addArgument<int>("v, v_int", {"vv"}, tst, std::make_tuple(5))
            .help("mandatory arg with mandatory value and side argument 5 for function tst()");

    // with custom parsing function, return type can be almost any type
    // here, we create an argument with custom type CL and 2 parameters, one of which is arbitrary,
    // and parse it with createStruct function
    parser.addArgument<CL>("struct", {"bool", "[integer]"}, createStruct)
            .required() // make argument required instead of mandatory
            .help("mandatory arg with 2 parameters: mandatory and arbitrary, returns result of function createStruct()");

    // lambdas can be used as parsing functions too
    // here, we create an argument with std::vector type and 2 parameters, one of which is arbitrary
    // and parse it with lambda expression
    parser.addArgument<std::vector<const char*>>("-a, --array", {"a1", "[a2]"},
                                                 [](const char* a1, const char* a2) -> auto{
                                                     return std::vector<const char*>{a1, a2==nullptr?"null":a2};
                                                 })
            .help("arbitrary argument with 2 string values (one arbitrary) and lambda converter");

    // for arguments of integral types or std::string with single parameter,
    // a list of choices can be specified inside an initializer_list:
    parser.addArgument<int>("--choices", {"int"})
            .choices({0,1,2,3}) // create list of possible valid choices for that argument
            .help("list with choices");

    ///nargs
    parser.addArgument<int>("--narg", {"ggg"})
            .nargs(3)
            .help("arg with 3 values");

    /// add child parser
    argParser &child = parser.addCommand("some_command", "some child command");

    child.addArgument<int>("--int")
            .help("int value");

    try{
        // parse arguments
        parser.parseArgs(argc, argv);

        // parseArgs() can throw 2 types of errors:
        // argParser::unparsed_param - if a certain argument could not be parsed.
        //  in that case, getLastUnparsed() method can be called to retrieve some info about that argument
        // argParser::parse_error - if case of unknown arguments and other errors

    }catch(argParser::unparsed_param &e){
        //catch argument parsing error
        std::cout << "Caught error: " + std::string(e.what()) << std::endl;
        // check unparsed argument
        auto &last_unparsed = parser.getLastUnparsed();
        std::cout << "Last unparsed arg: " << last_unparsed.get_name() << std::endl;
        // get list of parameters that were provided along with that argument by the caller
        std::cout << "Passed parameters:";
        auto raw_params = last_unparsed.get_cli_params();
        for(auto &el : raw_params){
            std::cout << " " + el;
        }
        std::cout << std::endl;
        return -1;

    }catch(argParser::parse_error &e){
        // catch other exceptions
        std::cout << "Caught error: " + std::string(e.what()) << std::endl;
        return -1;
    }


    // const methods of argument struct can be accessed via [ ]
    // here it returns true if argument 'v' was set by user
    auto isArgumentSet = parser["v"].is_set();

    // to get parsed value, use getValue, explicitly setting the type
    // if wrong type is specified, an std::invalid_argument exception is thrown
    auto b = parser.getValue<bool>("-b");
    auto s = parser.getValue<const char*>("-s");
    auto p = parser.getValue<std::string>("-p");
    auto pos = parser.getValue<int>("pos");
    auto a = parser.getValue<std::vector<const char*>>("-a");

    // get value of variadic argument
    auto N = parser.getValue<std::vector<int>>("-var");
    // get variadic positional value
    auto var_pos = parser.getValue<std::vector<int>>("var_pos");

    return 0;
}
