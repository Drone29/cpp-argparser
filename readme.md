# cpp-argparser

A C++17 header-only library for parsing command-line arguments.  
It supports positional arguments, flags, and options, allowing for flexible and robust command-line interfaces.

## Table of contents

- [Features](#features)
- [Basic usage](#basic-usage)
- [Details](#details)
  * [Positional arguments](#positional-arguments)
  * [Optional arguments](#optional-arguments)
  * [Mandatory arguments](#mandatory-arguments)
  * [Required arguments](#required-arguments)
  * [Aliases](#aliases)
  * [Parameters](#parameters)
  * [nargs](#nargs)
  * [Parsing function](#parsing-function)
  * [Parsing logic](#parsing-logic)
  * [Obtaining parsed values](#obtaining-parsed-values)
  * [Public parser methods](#public-parser-methods)
  * [Modifiers](#modifiers)
  * [Exceptions](#exceptions)
  * [Parse errors](#parse-errors)
- [Environment](#environment)

## Features

- **Single Header**
- **Requires C++17**
- **Positional Arguments**: Support for positional arguments
- **Parameterized Flags**: Optional named arguments with parameters
- **Implicit Flags**: Optional switches
- **Parameters**: Named arguments that can be mandatory or optional, with support for default values
- **Variadic**: Support for variadic positionals, flags and parameters
- **Automatic Help Message Generation**: Provides usage information based on defined arguments
- **Customizable Parsing Logic**: Allows for tailored argument validation and processing

## Basic usage

1. Include argparser.hpp
   ```c++
   #include "argparser.hpp"
   ```
2. Create an ```argParser``` instance inside main() function
   ```c++
   int main(int argc, char *argv[]){
        // Without parameters
        argParser parser;
        // Optional, specify program name and description
        argParser parser("program name", "this program does some useful stuff");
   }
   ```
3. Add arguments to parse
    ```c++
   // add positional int argument
   parser.addPositional<int>("positional")
               .finalize() 
               .help("positional argument");
   // add mandatory argument with parameter
   parser.addArgument<int>("mandatory")
               .setParameters("mandatory_param")
               .finalize()
               .help("mandatory argument with mandatory param"); 
   // add implicit int flag
   parser.addArgument<int>("-optional")
               .finalize() 
               .help("int optional argument (flag) with implicit value");
   // add int flag with mandatory parameter
   parser.addArgument<int>("-optional-with-param")
               .setParameters("mandatory_param") 
               .finalize() 
               .help("flag with single mandatory param");
   // add int flag with optional parameter
   parser.addArgument<int>("-optional-with-param2")
               .setParameters("[optional_param]") 
               .finalize() 
               .help("flag with single optional param");
   // add int flag with multiple parameters and parsing function
    parser.addArgument<int>("-optional--with-multiple-params")
            .setParameters("mandatory_param", "[optional_param]")
            .setCallable([](auto mandatory_param, auto optional_param) {
                int result = argParser::scanValue<int>(mandatory_param);
                if (optional_param){
                    result += argParser::scanValue<int>(optional_param);
                }
                return result;
            })
            .finalize()
            .help("flag with multiple params and parsing function");
    ```  
4. Parse arguments from command line
    ```c++
   parser.parseArgs(argc, argv);
    ```

### Help message

argParser generates help message automatically if invoked with `--help` or `-h` flag

   ```text
   >./app -h
   this program does some useful stuff
   Usage: program name [flags...] parameters... positional
   Positional arguments:
       positional : positional argument
   Flags (optional):
       -h, --help [arg] : Show this message and exit. 'arg' to get help about certain arg
       -optional : int optional argument (flag) with implicit value
       -optional--with-multiple-params <mandatory_param> [optional_param] : flag with multiple params
       -optional-with-param <mandatory_param> : flag with single mandatory param
       -optional-with-param2 [optional_param] : flag with single optional param
   Parameters (mandatory):
       mandatory <mandatory_param> : mandatory argument with mandatory param
   ```
   
## Details

Arguments can be:

* `optional` (not required to be set by user)
* `mandatory` (all such arguments should be set by user)
* `required` (at least one such argument should be set by user)
* `positional` (determined by their position in the cli)

`optional`, `mandatory` or `required` arguments can have `parameters`

Arguments with no parameters are called `implicit`

`addArgument` method is used to add an `optional`, `mandatory` or `required` argument

`addPositional` method is used to add a `positional` argument

### Positional arguments

`positional` arguments are parsed in the end, when all other arguments are parsed

Positional arguments `cannot have aliases and parameters`     

To specify a positional argument, use `addPositional` method:
```c++
parser.addPositional<int>("pos")
              .finalize()
              .help("Positional int argument");
```

### Optional arguments

`optional` arguments are not required to be set by user

Optional arguments can be `implicit`

To specify an optional argument, start it `with -`

```c++
parser.addArgument<int>("-x")
          .finalize()
          .help("int optional argument with implicit value");
```
 
### Mandatory arguments

All `mandatory` arguments must be set by the caller, otherwise an error is generated

Mandatory argument should have `at least one mandatory parameter`, i.e. they `cannot be implicit`

To specify a mandatory argument, its name/aliases should start `without -`
    
```c++
parser.addArgument<bool>("m")
          .setParameters("m_param")
          .finalize()
          .help("mandatory bool argument with mandatory parameter");
```
     
Also, any `optional` argument can be made `mandatory` using method `mandatory()`:
    
```c++
parser.addArgument<bool>("-m")
          .setParameters("m_param")
          .finalize()
          .mandatory()
          .help("optional argument made mandatory"); 
```
     
**NOTE:** positional or hidden arguments cannot be made mandatory
     
### Required arguments

The logic behind `required` arguments is such that `at least one` required argument should be set by the caller

To specify a required argument, use method `required()`

Here we declare 2 optional arguments `--req1` and `--req2` and then make them required:

```c++
parser.addArgument<int>("--req1")
          .finalize()
          .required()
          .help("required argument 1 with implicit value");
    
parser.addArgument<int>("--req2")
          .finalize()
          .required()
          .help("required argument 2 with implicit value"); 
```

Now the caller should set `either --req1 or --req2` or `both`, otherwise an error will be thrown

Mandatory arguments can be made required too:
    
```c++
parser.addArgument<bool>("m")
          .setParameters("m_param")
          .finalize()
          .required()
          .help("mandatory bool argument made required");
```

### Aliases

Non-positional arguments can have aliases

Here is a `--bool` argument with `-b` alias:

```c++
parser.addArgument<bool>("-b", "--bool")
          .finalize()
          .help("specifying name and alias in a single string");
```

The last string in the list is considered argument's name,
and other elements are aliases

**NOTE:** Argument name and alias should be of the same type:
if the name starts with `-`, aliases should also start with `-`:

```c++
parser.addArgument<int>("i, integer") //VALID
     parser.addArgument<int>("-i, --integer") //VALID
     parser.addArgument<int>("i, -integer") //NOT VALID
     parser.addArgument<int>("-i, integer") //NOT VALID
```
     
### Parameters

Non-positional arguments can have `parameters`

Parameters can also be `mandatory` or `optional`

`optional` parameters should be enclosed in `[]` and must not precede mandatory parameters

```c++
parser.addArgument<const char*>("-s, --str")
          .setParameters("str_value")
          .finalize()
          .help("string optional argument with mandatory parameter"); 
parser.addArgument<const char*>("-p")
          .setParameters("[str_value]")
          .finalize()
          .help("string optional argument with optional parameter");  
```

If an argument has no parameters (`implicit`), and is of arithmetic type (bool, int, float,...),
its value is `incremented` each time a user specifies it.

Arguments can also be made `repeatable`, which allows them to be set more than once:

```c++
parser.addArgument<int>("-j")
          .finalize()
          .repeatable()
          .help("int optional repeatable argument (implicit)");    
                
> ./app -jjj    - Repeatable argument, increments 3 times  
```
                              
If an argument can have several or an arbitrary number of identical parameters,  
this can be achieved with `nargs`:

```c++
parser.addArgument<int>("--variadic", "-var")
                .nargs<3>() // takes 3 mandatory integers
                .finalize() 
                .help("parses 3 integers. Result is std::vector<int>");
                
> ./app -var 123 321 12     - stores 3 numbers as a vector
```

And here's an example of a `variadic` argument (can take any number of parameters):

```c++
parser.addArgument<int>("--variadic", "-var")
                .nargs<0,-1>() // takes any number of integer parameters
                .finalize() 
                .help("parses any number of integers. Result is std::vector<int>");
                
> ./app -var 123 321 12     - stores 3 numbers as a vector
> ./app -var 123            - stores 1 number as a vector
```

The return type of variadic argument changes to `std::vector<argument_type>`

**NOTE:** `nargs<1>` or `nargs<0,1>` will not change the return type to `vector`

Positional arguments can be variadic too:

```c++
parser.addPositional<int>("var_pos") 
                .nargs<0,-1>() // make argument variadic
                .finalize() 
                .help("Variadic pos argument of type int");
                
> ./app 1 2 3 4 5       - stores 5 numbers as a vector  
```

For more, refer to [nargs](#nargs) section
                        
### nargs

Apart from `setParameters` method, an `Nargs` method can be used

nargs() takes 2 template parameters:
- Number of mandatory parameters
- Total number of parameters (ignored if less than the first one)

So, for example the call `nargs<1,3>` will add 1 mandatory and 2 optional parameters to an argument

```c++
parser.addArgument<int>("-i")
                .nargs<1, 3>() // add 1 mandatory and 2 optional parameters
                .finalize()
                .help("Variadic pos argument of type int");

> ./app -i 1 2 3       - stores 3 numbers as a vector    
> ./app -i 1 2         - stores 2 numbers as a vector 
> ./app -i 1           - stores 1 numbers as a vector
> ./app -i 1 2 3 4     - ERROR: only 3 values allowed, provided 4    
```

`nargs<3,1>` will simply add 3 mandatory parameters, ignoring the value 1 as it's less than 3

```c++
parser.addArgument<int>("-i")
                .nargs<3, 1>() // add 3 mandatory parameters
                .finalize()
                .help("Variadic pos argument of type int");

> ./app -i 1 2 3       - stores 3 numbers as a vector    
> ./app -i 1 2         - ERROR: not enough parameters
> ./app -i 1           - ERROR: not enough parameters
> ./app -i 1 2 3 4     - ERROR: only 3 values allowed, provided 4
```

`setParameters()` can be called before `nargs()`, but you can only specify ONE parameter there.  
This way it will act as a metavar for parameter's name.  
Passing more than 1 parameter to `setParameters()` followed by `nargs()`, will result in a compilation error

**NOTE:** Calling `setParameters("[optional]")` along with `nargs()` will throw an exception

If setParameters() were not called before nargs(), a default name is provided for parameter (capitalized argument's key)

The second parameter in `nargs` can be -1 (or any value less than 0), thus making the argument variadic

**NOTE:** `nargs` cannot have both parameters set to 0, this will result in a compilation error
      
### Parsing function
                                                            
An argument can be parsed by built-in parser only if
it has `0 or 1 mandatory parameters` of `arithmetic` or `string` type:
* `bool`, `int`, `float`, `double`, `unsigned int`, ... (arithmetic types)
* `char*`, `const char*`, `std::string` (string types)

Otherwise, a `parsing function` should be specified
    
```c++
// Parsing function
std::string test(const char* a){
    if(a == nullptr){
        a = "null";
    }
    return std::string(a);
}

...
// add parsing function test() to argument with optional parameter
parser.addArgument<std::string>("-p")
        .setParameters("[str_value]")
        .setCallable(test)
        .finalize();  
```
                
A parsing function is called every time an argument is found in the command line,
and its result is applied to the underlying argument's variable

The parsing function `must` be specified for an argument in the following cases:

* The argument is `not of arithmetic or string` type
* The argument has `more than 1 parameter`
                
Here are some constraints for parsing functions:                
                
* A parsing function must return a value of argument's type and 
take as many `string parameters` (const char*) as there are `parameters` specified for argument
* Parsing function can also have `side parameters`. 
They need to be placed before string parameters in a function declaration,
and corresponding values should be specified in setCallable method:
    
```c++
// Function not valid, side parameters must be placed before string
int func(const char* a, int i){...}

// Valid parsing function 
int tst(int a, const char* a1){
    return a + (int)strtol(a1, nullptr, 0);
}

...

// variable will be used as side parameter
int x = 5;
parser.addArgument<int>("v", "v_int")
        .setParameters("vv")
        .setCallable(tst, x)
        .finalize()
        .help("mandatory arg with mandatory value and side argument x for function tst()");
```

Here's an example of an argument with custom type CL:                 
    
```c++
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
parser.addArgument<CL>("struct")
        .setParameters("bool", "[integer]")
        .setCallable(createStruct)
        .finalize()
        .help("mandatory arg with 2 parameters: mandatory and optional, returns result of function createStruct()");
```

Lambdas can be used as parsing functions too:

```c++
parser.addArgument<std::vector<const char*>>("-a", "--array")
            .setParameters("a1", "[a2]")
            .setCallable([](auto a1, auto a2){
                return std::vector<const char*>{a1, a2==nullptr?"null":a2};
            })
            .finalize()
            .help("optional argument with 2 string values (one optional) and lambda converter");
```

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

Parsed values can be obtained with one of the following methods:

* Using `getValue()` method after `parseArgs()` is called 
* Using `globalPtr()` modifier upon defining arguments with `addArgument` or `addPositional`
* Using function or lambda to set the variable

Examples:

Obtaining value with `getValue()` method:
    
```c++
// add int argument
parser.addArgument<int>("-x").finalize()
      .help("int optional argument with implicit value");
// parse arguments
parser.parseArgs(argc, argv);            
// retrieve int value. The type of getValue() must correspond to the type of addArgument(), 
// otherwise error is thrown 
auto x = parser.getValue<int>("-x"); 
// it's also possible to obtain value with overloaded operator[]:
int x2 = parser["-x"]; 
```
    
Obtaining value with `globalPtr()` modifier:
    
```c++
// declare int variable
int j;
// add int argument and pass the address of declared variable
parser.addArgument<int>("-j").finalize()
      .globalPtr(&i)
      .help("int optional argument with (implicit)");
// parse arguments
parser.parseArgs(argc, argv);
// after that, the parsed result will be stored in j variable
```

**NOTE:** `variadic` arguments' values can be obtained only with `getValue()` method,
with the type `std::vector<arg_type>`:
    
```c++
// add variadic int argument
parser.addArgument<int>("--variadic", "-var")
            .nargs<0,-1>() // make argument variadic
            .finalize()
            .help("parses any number of integers. Result is std::vector<int>");
// parse arguments
parser.parseArgs(argc, argv);  
// retrieve variadic arg value as a vector
auto x = parser.getValue<std::vector<int>>("-var");  
```

### Public parser methods

A list of public parser methods:

* `addPositional<T>("name")` - adds positional argument of type T
with optional parser function and side arguments
* `addArgument<T>("aliases",...)` - adds argument of type T
with aliases
* `setParameters("params",...)` - adds string parameters to an argument (not applicable to positionals)
* `setCallable(callable, size_args,...)` - adds callable (function, lambda, etc) along with its side arguments
* `nargs<FRO, TO>()` - specify nargs. FRO - number of mandatory params, TO - overall number of params (if TO > FRO)
* `finalize()` - finalizes argument definition
* `getValue<T>("name or alias")` - returns value of type T of specified argument
* `scanValue<T>(value)` - static method to parse some value from string using built-in parser.
Applicable to `arithmetic` or `string` values.
* `getLastUnparsed()` - get last unparsed argument (in case of argparser::unparsed_parameter error).
Returns a reference to the instance of unparsed argument
* `setAlias("name", "aliases")` - set aliases to argument name if wasn't set in addArgument
* `getSelfName()` - get executable self name, applicable only after arguments are parsed
* `parseArgs(argc, argv)` - parse arguments from command line
* `operator [] ("name or alias")` - provides access to const methods of argument, such as `isSet()`. Can also be used along with static cast to obtain values
    
### Modifiers

Here's the full list of argument methods (modifiers):

* `help("your help text")` - specify help text for the argument
* `advancedHelp("advanced help text")` - specify advanced help text, 
will be shown if user calls `--help your_argument_here`
* `hidden()` - make argument `hidden` from generic help message 
(can still be displayed with advanced help call `--help -h`). Only for `optional` arguments
* `repeatable()` - make argument `repeatable`. Only for non-positional arguments
* `defaultValue(defaultValue, hide_in_help=false)` - specify default 
(the one that will be assigned if not set by user) 
value for argument. Only for `optional` or `required` arguments. 
`hide_in_help` - optional parameter, hides default value from help message if set to true
* `globalPtr(pointer)` - specify pointer to 'global' variable. 
Must point to the variable of corresponding type.
Not applicable to variadic arguments
* `mandatory()` - make `optional` or `required` argument `mandatory`. 
Cannot be applied to `hidden` arguments
* `required()` - make `optional` or `mandatory` argument `required`.
Cannot be applied to `hidden` arguments
* `choices(choices,...)` - adds a list of possible valid choices for the argument. Applicable only to arithmetic types and strings

There are also some useful const methods for arguments:

* `isSet()` - returns `true` if argument was set by user
* `isOptional()` - returns `true` if argument is optional
* `isRequired()` - returns `true` if argument is required
* `isPositional()` - returns `true` if argument is positional
* `isImplicit()` - returns `true` if argument is implicit
* `isRepeatable()` - returns `true` if argument is repeatable
* `isVariadic()`   - returns `true` if argument is variadic
* `optionsSize()` - returns `size_t` size of argument parameters
* `getCliParams()` - returns an `std::vector<std::string>` of parameters passed by the caller.
Applicable only after parseArgs is called
* `getName()` - returns `std::string` argument's name without aliases
 
### Exceptions

Some methods may throw exceptions on the stage where arguments are added if some conditions are not met:

* `std::logic_error` is thrown by arg modifiers like `defaultValue()` and some others
* `std::invalid_argument` is thrown by parser methods like `addArgument()`, `addPositional()`, `getValue()`, etc...        
* `std::runtime_error` is thrown by `scanValue()` method along with some internal methods     

### Parse errors

`parseArgs()` can throw 2 types of errors:
* `argParser::unparsed_param` - if a certain argument could not be parsed.
in that case, `getLastUnparsed()` method can be called to retrieve some info about that argument
* `argParser::parse_error` - if case of unknown arguments and other errors

Here's an example:

```c++
try{
    // parse arguments
    parser.parseArgs(argc, argv);
}catch(argParser::unparsed_param &e){
    //catch argument parsing error
    std::cout << "Caught error: " + std::string(e.what()) << std::endl;
    // check unparsed argument
    auto &last_unparsed = parser.getLastUnparsed();
    std::cout << "Last unparsed arg: " << last_unparsed.getName() << std::endl;
    // get list of parameters that were provided by the caller
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


> ./app -b345 
Output:
Caught error: scan: unable to convert 345 to bool
Last unparsed arg: -b
Passed parameters: 345
```

For more details, see [example.cpp](./example.cpp) and [tests](./utest/utests.cpp)         

## Environment

* GCC ver >= 8.3.0, Ubuntu 20.04
* MSVC ver >= 16.0, Visual Studio 2019
