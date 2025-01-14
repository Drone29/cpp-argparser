#include <iostream>
#include "argparser.hpp"

std::string test(const char* a){
    if(a == nullptr){
        a = "null";
    }
    return a;
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
            .help("int optional argument with implicit value (if set, returns 1)")
            .finalize();

    // bool is also considered arithmetic,
    // so parser will increment it, thus setting it to true.
    // again, it's NON-REPEATABLE
    // also, it has an alias -b
    parser.addArgument<bool>("-b", "--bool")
            .help("bool optional argument '--bool' with alias '-b' and implicit value (if set, returns true)")
            .finalize();

    // REPEATABLE argument can be specified more than once,
    // in this case, due to implicit value, it will increment each time it's called
    // i.e. a call '-j -j -j' or '-jjj' will increment counter 3 times, starting from 5 (default value),
    // and returns 8
    parser.addArgument<int>("-j")
            .defaultValue(5)   // specify default value for argument
            .repeatable()   // make argument repeatable
            .help("int optional repeatable argument with implicit value and default value 5")
                    // advanced help message can be viewed if '--help' is called with argument's name
            .advancedHelp("and with advanced help string (can be viewed with --help -j)")
            .finalize();

    // to automatically set value to 'global' variable (visible in current scope)
    // global_ptr() method is used.
    // It takes a pointer to a variable of the type which was specified for the argument
    // after such argument is parsed, that variable's value is automatically set
    // NOTE: not applicable for variadic arguments
    parser.addArgument<int>("-i", "--int")
            .globalPtr(&i_val) // specify global variable for argument
            .repeatable()   // make argument repeatable
            .help("int optional repeatable argument with alias, implicit value and pointer to global variable (if set, returns 1)")
            .finalize();

    /**
     *  Arguments can be mandatory, optional, required and positional
     *
     *  Arguments specified with '-' are by default optional,
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

    // optional arguments can be made required
    // in the following case, user should specify either '--req1' or '--req2', or both
    parser.addArgument<int>("--req1")
            .required() // make optional argument required
            .help("required argument 1 with implicit value")
            .finalize();

    parser.addArgument<int>("--req2")
            .required() // make optional argument required
            .help("required argument 2 with implicit value")
            .finalize();

    /**
     *  Args with parameters
     *
     *  Can be called with parameters: '-s aaa', '--str=aaa', '-sFFF', etc.
     *  Or can be called without them, if parameter is optional
     *
     *  An argument can have as many parameters as you like
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
     *  Enclose parameter name with [] to make it optional,
     *  Not []-enclosed params are considered mandatory
     */

    // arguments without '-' are mandatory
    // mandatory arguments should have at least 1 mandatory parameter
    parser.addArgument<bool>("m")
            .parameters("m_param")
            .help("mandatory bool argument with mandatory parameter")
            .finalize();

    // str_value - mandatory parameter, cannot be omitted
    // i.e. calls like '-s aaa' or '--str=aaa' are VALID,
    // but '-s' or '--str' are NOT VALID and will cast an error
    parser.addArgument<const char *>("--str", "-s")
            .parameters("str_value")
            .globalPtr(&s_val) // pointer to global variable
            .help("string optional argument with mandatory parameter")
            .finalize();

    // an optional argument can be made hidden,
    // so it's not shown in help message unless it's called with '-a' specifier
    // NOTE: mandatory or required arguments cannot be hidden
    parser.addArgument<int>("--hidden")
            .parameters("int_value")
            .hidden() // specify argument as hidden
            .help("hidden int argument with mandatory value (can be viewed with --help -a)")
            .finalize();

    // an argument can be made variadic
    // in this case, an optional number of options (but not less than 1) can be specified by the caller
    // for example, the following will add a variadic argument '--variadic'
    // so '-var 123', '-var 123 321' or '-var 123 321 231' etc... are VALID calls,
    // after argument is made variadic, its return type changes to std::vector<TYPE>()
    // NOTE: global_ptr() is not applicable for variadic arguments, so the only way to obtain its value is by calling parser.getValue() method
    parser.addArgument<int>("--variadic", "-var")
            .nargs<1, -1>() // make argument variadic
            .help("nargs(1, -1) parses any number of integers. Result is std::vector<int>")
            .finalize();

    // positional arguments can be specified by the separate method addPositional()
    // positional arguments cannot be made hidden, optional or required
    // all other properties apply
    parser.addPositional<int>("pos")
            .globalPtr(&pos_val) // pointer to global variable
            .help("Positional int argument with global variable")
            .finalize();

    // positional arguments can be variadic too
    // in that case, ONLY ONE such argument can be present
    // also, variadic positional argument should be added AFTER all other positional arguments
    parser.addPositional<int>("var_pos")
            .nargs<1, -1>() // make positional argument variadic
            .help("Variadic pos argument of type int")
            .finalize();

    /**
     *  Custom parsing functions
     *
     *  By default, arithmetic, string or date_t arguments (implicit or having a single mandatory parameter)
     *  can be parsed by built-in logic, so no custom functions required
     *
     *  Although, if an argument has more than 1 parameter, has optional parameters, or is of custom type,
     *  a custom parsing function is required
     *
     *  Parsing function is any function, function pointer, lambda, etc...
     *  which takes as many string (const char*) arguments that there are specified by argument's parameter list (parameters),
     *  and returns a value of argument's type
     *
     *  Also, such function can take an optional number of so-called side arguments,
     *  which are arguments of any type, specified by the function's signature
     *
     *  Side arguments (if present) in function's signature should go BEFORE string arguments which are to be parsed by that function
     *  Side arguments are specified in callable method after the function itself
     *
     *  NOTE: a function must have a return type corresponding to argument's type, and CANNOT BE VOID
     *
     */

    // here's an argument with custom parsing function 'test'
    // it returns a value of argument's type (std::string), and takes 1 string argument
    // [str_value] - optional value, can be omitted
    // i.e. calls '-p aaaa' and '-p' are both VALID
    // function should return the argument's type value,
    // and its number of const char* arguments should be the same as number of parameters specified in parameter list
    // (otherwise it won't compile anyway)
    parser.addArgument<std::string>("-p")
            .parameters("[str_value]")
            .callable(test)
            .help("string optional argument with optional value and function test (if set, returns result of test())")
            .finalize();

    // function can accept not only string arguments that should be parsed,
    // but also any number of side arguments.
    // side arguments need to be placed before strings in function declaration:
    // func(int a, const char* b) is VALID,
    // func(const char*b, int a) is NOT VALID
    // side arguments should be passed to callable after function as a parameter pack
    parser.addArgument<int>("v", "v_int")
            .parameters("vv")
            .callable(tst, 5)
            .help("mandatory arg with mandatory value and side argument 5 for function tst()")
            .finalize();

    // with custom parsing function, return type can be almost any type
    // here, we create an argument with custom type CL and 2 parameters, one of which is optional,
    // and parse it with createStruct function
    parser.addArgument<CL>("struct")
            .parameters("bool", "[integer]")
            .callable(createStruct)
            .required() // make argument required instead of mandatory
            .help("mandatory arg with 2 parameters: mandatory and optional, returns result of function createStruct()")
            .finalize();

    // lambdas can be used as parsing functions too
    // here, we create an argument with std::vector type and 2 parameters, one of which is optional
    // and parse it with lambda expression
    parser.addArgument<std::vector<const char *>>("-a", "--array")
            .parameters("a1", "[a2]")
            .callable([](auto a1, auto a2) {
                return std::vector<const char *>{a1, a2 == nullptr ? "null" : a2};
            })
            .help("optional argument with 2 string values (one optional) and lambda converter")
            .finalize();

    // for arguments of integral types or std::string with single parameter,
    // a list of choices can be specified inside an initializer_list:
    parser.addArgument<int>("--choices")
            .parameters("int")
            .choices(0,1,2,3) // create list of possible valid choices for that argument
            .help("list with choices")
            .finalize();
    // the same, but with nargs
    parser.addArgument<int>("--choices2")
            .nargs<1, 3>()
            .choices(0,1,2,3) // create list of possible valid choices for that argument
            .help("list with choices")
            .finalize();

    /**
     * nargs
     *
     * Apart from specifying argument's parameters with parameters(), we can also specify them with nargs
     * nargs() takes 2 template parameters:
     * - Number of mandatory parameters
     * - Total number of parameters
     *
     * So, for example this call nargs<1,3> will add 1 mandatory and 2 optional parameters to an argument
     *
     * parameters() can be called before nargs(), but you can only specify ONE parameter there.
     * This way it will act as a metavar for parameter's name
     *
     * Passing more than 1 parameter to parameters() followed by nargs(), will result in a compilation error
     *
     * If parameters() were not called before nargs(), a default name is provided for parameter (capitalized argument's key)
     *
     * The second parameter in nargs can be -1 (or any value less than 0), thus making the argument variadic
     * (i.e. it can take as many parameters as supplied by the caller)
     *
     * nargs cannot have both parameters set to 0, this will result in a compilation error
     *
     */

    ///nargs
    parser.addArgument<int>("--narg")
            .nargs<1, 3>()
            .help("nargs(1, 3) arg with 3 values: 1 mandatory, 2 - optional")
            .finalize();

    /// pure variadic
    parser.addArgument<int>("--vararg")
            .nargs<0, -1>() //pure variadic arg
            .help("nargs(0, -1) pure variadic narg")
            .finalize();

    /// add child parser
    argParser &child = parser.addCommand("child_command", "some child command");

    child.addArgument<int>("--int")
            .help("int value")
            .finalize();

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
        std::cout << "Last unparsed arg: " << last_unparsed.getName() << std::endl;
        // get list of parameters that were provided along with that argument by the caller
        std::cout << "Passed parameters:";
        auto raw_params = last_unparsed.getCliParams();
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

    //-b date=01.02.2000-13:56 m=true v=4 --req1 1 23
    // const methods of argument struct can be accessed via [ ]
    // here it returns true if argument 'v' was set by user
    auto isArgumentSet = parser["v"].isSet();

    // to get parsed value, use getValue, explicitly setting the type
    // if wrong type is specified, an std::invalid_argument exception is thrown
    auto b = parser.getValue<bool>("-b");
    auto s = parser.getValue<const char*>("-s");
    auto p = parser.getValue<std::string>("-p");
    // get variadic
    auto a = parser.getValue<std::vector<const char*>>("-a");

    // get value of variadic argument
    auto N = parser.getValue<std::vector<int>>("-var");
    // get variadic positional value
    auto var_pos = parser.getValue<std::vector<int>>("var_pos");

    //get value with conversion operator
    bool b2 = parser["-b"];
    bool m = parser["m"];
    int v = parser["v"];
    int req = parser["--req1"];
    int ppos = parser["pos"];

    return 0;
}
