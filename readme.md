## About

* Single header
* Requires C++17

## Basic usage

1. Include argparser.hpp
    ```.cpp
    #include "argparser.hpp"
    ```
2. Create an ```argParser``` instance inside main() function
    ```.cpp
   int main(int argc, char *argv[]){
   
        argParser parser;
   
   }
    ```
3. Add arguments to parse
    ```.cpp
   parser.addArgument<int>("-x")
               .help("int arbitrary argument with implicit value");
    ```  
4. Parse arguments from command line
    ```.cpp
   parser.parseArgs(argc, argv);
    ```
   
## Details

Arguments can be:

* `arbitrary` (not required to be set by user)
* `mandatory` (all such arguments should be set by user)
* `required` (at least one such argument should be set by user)
* `positional`

`arbitrary`, `mandatory` or `required` arguments can have up to 9 `parameters`

Arguments with no parameters are called `implicit`

`addArgument` method is used to add an `arbitrary`, `mandatory` or `required` argument

`addPositional` method is used to add a `positional` argument

#### Positional arguments

`positional` arguments are parsed in the end, when all other arguments were parsed

Positional arguments `cannot have aliases and parameters`     

To specify a positional argument, use `addPositional` method:

        parser.addPositional<int>("pos")
              .help("Positional int argument");

#### Aliases

Non-positional arguments can have aliases

A string containing commas or other non-alphanumeric characters excluding '-' and '_',
is treated as an alias list

Here is a `--bool` argument with `-b` alias:

    parser.addArgument<bool>("-b, --bool")
          .help("specifying name and alias in a single string");

Alternatively, an addArgument() overload can be used, 
which takes `std::vector<std::string>` as its first argument:  

    parser.addArgument<bool>({"-b", "--bool"})
                .help("specifying name and alias as a vector");

In either case, the longest string in a resulting list is considered argument's name,
and other element are aliases

**NOTE:** Argument name and alias should be of the same type: 
`if name starts with -, aliases should also start with -`:

     parser.addArgument<int>("i, integer"); //VALID
     parser.addArgument<int>("-i, --integer"); //VALID
     parser.addArgument<int>("i, -integer"); //NOT VALID
     parser.addArgument<int>("-i, integer"); //NOT VALID

#### Arbitrary arguments

`arbitrary` arguments are not required to be set by user

Arbitrary arguments `can be implicit`

To specify an arbitrary argument, start it `with -`

    parser.addArgument<int>("-x")
          .help("int arbitrary argument with implicit value");
 
#### Mandatory arguments

All `mandatory` arguments must be set by the caller, otherwise an error is generated

Mandatory argument should have `at least one mandatory parameter`, i.e. they `cannot be implicit`

To specify a mandatory argument, its name/aliases should start `without -`
    
    parser.addArgument<bool>("m", {"m_param"})
          .help("mandatory bool argument with mandatory parameter");
     
Also, any `arbitrary` argument can be made `mandatory` using method `mandatory()`:
    
    parser.addArgument<bool>("-m", {"m_param"})
          .mandatory()
          .help("arbitrary argument made mandatory");    
     
**NOTE:** positional or hidden arguments cannot be made mandatory
     
#### Required arguments

The logic behind `required` arguments is such that `at least one` required argument should be set by the caller

To specify a required argument, use method `required()`

Here we declare 2 arbitrary arguments `--req1` and `--req2` and then make them required:

    parser.addArgument<int>("--req1")
          .required()
          .help("required argument 1 with implicit value");
    
    parser.addArgument<int>("--req2")
          .required()
          .help("required argument 2 with implicit value");    

Now the caller should set `either --req1 or --req2` or `both`, otherwise an error will be thrown

Mandatory arguments can be made required too:
    
    parser.addArgument<bool>("m", {"m_param"})
          .required()
          .help("mandatory bool argument made required");
     
### Parameters

Non-positional arguments can have up to 9 `parameters`

Parameters can also be `mandatory` or `arbitrary`

`arbitrary` parameters should be enclosed in `[]` and placed at the end

    parser.addArgument<const char*>("-s, --str", {"str_value"})
          .help("string arbitrary argument with mandatory parameter"); 
    parser.addArgument<const char*>("-p", {"[str_value]"})
          .help("string arbitrary argument with arbitrary parameter");            

If an argument has no parameters (`implicit`), and it's of arithmetic type (bool, int, float,...),
its value is `incremented` each time a user specify it.

Arguments can also be made `repeatable`, which allows them to be set more than once:

    parser.addArgument<int>("-j")
          .repeatable()
          .help("int arbitrary repeatable argument (implicit)");    
                
    > ./app -jjj    - Repeatable argument, increments 3 times            
                              
If an argument can have an arbitrary number of parameters, it can be made `variadic`

Here's an example of variadic argument:

    parser.addArgument<int>("--variadic, -var", {"N"})
                .variadic() // make argument variadic
                .help("parses any number of integers. Result is std::vector<int>");
                
    > ./app -var 123 321 12     - stores 3 numbers as a vector
    > ./app -var 123            - stores 1 number as a vector
                              
Variadic arguments should be declared with exactly 1 mandatory parameter, 
otherwise an error is thrown

The return type of variadic argument changes to `std::vector<argument_type>`

Positional arguments can be variadic too:

    parser.addPositional<int>("var_pos") 
                .variadic() // make positional argument variadic
                .help("Variadic pos argument of type int");
                
    > ./app 1 2 3 4 5       - stores 5 numbers as a vector                
                              
### Parsing function
                                                            
An argument can be parsed by built-in parser only if
it has `0 or 1 mandatory parameters` of `arithmetic` or `string` type:
* `bool`, `int`, `float`, `double`, `unsigned int`, ... (arithmetic types)
* `char*`, `const char*`, `std::string` (string types)

Otherwise, a `parsing function` should be specified
    
    // Parsing function
    std::string test(const char* a){
        if(a == nullptr){
            a = "null";
        }
        return std::string(a);
    }
    
    ...
    // add parsing function test() to argument with arbitrary parameter
    parser.addArgument<std::string>("-p", {"[str_value]"}, test)
          .help("string arbitrary argument with arbitrary value and function test");  
                
A parsing function is called every time an argument is found in the command line,
and its result is applied to the underlying argument's variable

The parsing function `must` be specified for an argument in the following cases:

* The argument is `not of arithmetic or string` type
* The argument has `more than 1 parameter`
* The argument `has arbitrary parameters`
                
Here are some constraints for parsing functions:                
                
* A parsing function must return a value of argument's type and 
take as many `string parameters` (const char*) as there are `parameters` specified for argument
* Parsing function `cannot be void`
* Parsing function can also have `side parameters`. 
They need to be placed `before string parameters` in a function declaration,
and corresponding values should be specified in a tuple in addArgument method:
    
    
    // Function not valid, side parameters must be placed before string
    int func(const char* a, int i){...}
    
    // Valid parsing function 
    int tst(int a, const char* a1){
        return a + (int)strtol(a1, nullptr, 0);
    }
    
    ...
    
    // variable will be used as side parameter
    int x = 5;
    // specify parsing function and pass side parameter (x) in a tuple
    parser.addArgument<int>("v", {"str_val"}, tst, std::make_tuple(x))
          .help("mandatory arg with mandatory value and side argument for function tst");

Here's an example of an argument with custom type CL:                 
    
    // User-defined type
    struct CL{
        bool a = false;
        int b = 0;
    };
    
    // Parsing function
    CL createStruct(const char *bl, const char *itgr = nullptr){
        //static parser helper function, converts string to  basic type
        bool b = argParser::scanValue<bool>(bl);
        int i = argParser::scanValue<int>(itgr);
        return CL{b, i};
    }
    
    ...
    // specify parsing function for custom type
    parser.addArgument<CL>("struct", {"bool", "[integer]"}, createStruct)
          .help("mandatory arg with 2 parameters: mandatory and arbitrary, returns result of function createStruct()");    
                
Lambdas can be used as parsing functions too:

    parser.addArgument<std::vector<const char*>>("-a, --array", {"a1", "[a2]"},
                                                     [](const char* a1, const char* a2) -> auto{
                                                         return std::vector<const char*>{a1, a2==nullptr?"null":a2};
                                                     })
                .help("arbitrary argument with 2 string values (one arbitrary) and lambda converter");

### Parsing logic

Arguments and parameters are parsed according to the following logic:

* Arguments are separated by `space`
* Delimiters between argument and parameters can be: `space`, `=`, or `empty string` (if argument has a short name/alias)
* Short implicit arguments can be combined

Here are some examples:
    
    > ./app -s aaa          - classic way
    > ./app -string=aaa     - using equals as delimiter between arg name and parameter
    > ./app -p123           - using empty string delimiter for short alias
    > ./app -jjj            - combining the same repeating short implicit arg
    > ./app -ij             - combining different short implicit args                       
    
### Obtaining parsed values

Parsed values can be obtained via one of the following methods:

* Using `getValue()` method after `parseArgs()` is called 
* Using `global_ptr()` modifier upon defining arguments with `addArgument` or `addPositional`

Examples:

Obtaining value with `getValue()` method:
    
    // add int argument
    parser.addArgument<int>("-x")
          .help("int arbitrary argument with implicit value");
    // parse arguments
    parser.parseArgs(argc, argv);            
    // retrieve int value. The type of getValue() must correspond to the type of addArgument(), 
    // otherwise error is thrown 
    auto x = parser.getValue<int>("-x"); 
    
Obtaining value with `global_ptr()` modifier:
    
    // declare int variable
    int j;
    // add int argument and pass the address of declared variable
    parser.addArgument<int>("-j")
          .global_ptr(&i)
          .help("int arbitrary argument with (implicit)");
    // parse arguments
    parser.parseArgs(argc, argv);
    // after that, the parsed result will be stored in j variable


**NOTE:** `variadic` arguments' values can be obtained only with `getValue()` method,
with the type `std::vector<arg_type>`:
    
    // add variadic int argument
    parser.addArgument<int>("--variadic, -var", {"N"})
                .variadic() // make argument variadic
                .help("parses any number of integers. Result is std::vector<int>");
    // parse arguments
    parser.parseArgs(argc, argv);  
    // retrieve variadic arg value as a vector
    auto x = parser.getValue<std::vector<int>>("-var");           

### Public parser methods

A list of public parser methods:

* `addPositional<T>("name", parser_func, side_args)` - adds positional argument of type T
with optional parser function and side arguments
* `addArgument<T>("name,aliases", {parameter_list}, parser_func, side_args)` - adds argument of type T
with optional aliases, parameters, parser function and side arguments
* `getValue<T>("name or alias")` - returns value of type T of specified argument
* `scanValue<T>(value, date_format)` - static method to parse some value from string using built-in parser.
Applicable to `arithmetic` or `string` values and `date_t` type. 
`date_format` - optional, applicable to `date_t` type only
* `getLastUnparsed()` - get last unparsed argument (in case of argparser::unparsed_parameter error).
Returns a reference to the instance of unparsed argument
* `setAlias("name", "aliases")` - set aliases to argument name if wasn't set in addArgument
* `getSelfName()` - get executable self name, applicable only after arguments are parsed
* `parseArgs(argc, argv, allow_zero_options)` - parse arguments from command line
* `operator [] ("name or alias")` - provides access to const methods of argument, such as `is_set()`
    
### addArgument and addPositional modifiers

Here's the full list of `addArgument()` and `addPositional()` modifiers:

* `help("your help text")` - specify help text for the argument
* `advanced_help("advanced help text")` - specify advanced help text, 
will be shown if user calls `--help your_argument_here`
* `hidden()` - make argument `hidden` from generic help message 
(can still be displayed with advanced help call `--help -a`). Only for `arbitrary` arguments
* `repeatable()` - make argument `repeatable`. Only for non-positional arguments
* `default_value(default_value, hide_in_help=false)` - specify default 
(the one that will be assigned if not set by user) 
value for argument. Only for `arbitrary` or `required` arguments. 
`hide_in_help` - optional parameter, hides default value from help message if set to true
* `global_ptr(pointer)` - specify pointer to 'global' variable. 
Must point to the variable of corresponding type.
Not applicable to variadic arguments
* `mandatory()` - make `arbitrary` or `required` argument `mandatory`. 
Cannot be applied to `hidden` arguments
* `required()` - make `arbitrary` or `mandatory` argument `required`.
Cannot be applied to `hidden` arguments
In that case, an arbitrary number (not less than 1) of parameters can be passed by the caller
* `date_format("format", hide_in_help=false)` - special modifier, 
applicable only to `positional` or `single-parameter` arguments whose type is `date_t` (alias for std::tm).
`format` must be valid strptime() format `without spaces`, for example `%d.%m.%YT%H:%M`.
If not set, `default format` is used: `%Y-%m-%dT%H:%M:%S`.
`hide_in_help` - optional, set to true to hide format from help message
* `variadic()` - make argument variadic

There are also some useful const methods for arguments:

* `is_set()` - returns `true` if argument was set by user
* `is_arbitrary()` - returns `true` if argument is arbitrary
* `is_required()` - returns `true` if argument is required
* `is_positional()` - returns `true` if argument is positional
* `is_implicit()` - returns `true` if argument is implicit
* `is_repeatable()` - returns `true` if argument is repeatable
* `is_variadic()`   - returns `true` if argument is variadic
* `get_date_format()` - returns `const char*` date format. 
applicable only for `date_t` type, otherwise `nullptr` returned
* `options_size()` - returns `size_t` size of argument parameters
* `get_cli_params()` - returns an `std::vector<std::string>` of parameters passed by the caller.
Applicable only after parseArgs is called
* `get_name()` - returns `std::string` argument's name without aliases

These methods can be called using [] overload:

    bool isArgumentSet = parser["v"].is_set();
    bool isArbitrary = parser["v"].is_arbitrary();
    ...
            
### Exceptions

Some methods may throw exceptions on the stage where arguments are added if some conditions are not met:

* `std::logic_error` is thrown by arg modifiers like `default_value()`, `date_format()` and some others
* `std::invalid_argument` is thrown by parser methods like `addArgument()`, `addPositional()`, `getValue()`, etc...        
* `std::runtime_error` is thrown by `scanValue()` method along with some internal methods     

### Parse errors    

`parseArgs()` can throw 2 types of errors:
* `argParser::unparsed_param` - if a certain argument could not be parsed.
in that case, `getLastUnparsed()` method can be called to retrieve some info about that argument
* `argParser::parse_error` - if case of unknown arguments and other errors

Here's an example:

        try{
            // parse arguments
            parser.parseArgs(argc, argv);
        }catch(argParser::unparsed_param &e){
            //catch argument parsing error
            std::cout << "Caught error: " + std::string(e.what()) << std::endl;
            // check unparsed argument
            auto &last_unparsed = parser.getLastUnparsed();
            std::cout << "Last unparsed arg: " << last_unparsed.get_name() << std::endl;
            // get list of parameters that were provided by the caller
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
        
        
        > ./app date=01.34.22 
        Output:
        Caught error: scan: unable to convert 01.34.22 to date
        Last unparsed arg: date
        Passed parameters: 01.34.22
        

For more details, see example.cpp         

## Environment

* GCC ver >= 8.3.0, Ubuntu 20.04
* MSVC ver >= 16.0, Visual Studio 2019
