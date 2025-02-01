# cpp-argparser

A C++17 header-only library for parsing command-line arguments.  
It supports positional arguments, flags, and options, allowing for flexible and robust command-line interfaces.

## Table of contents

- [Features](#features)
- [Basic usage](#basic-usage)
  * [Help message](#help-message) 
- [Details](#details)
  * [Positional arguments](#positional-arguments)
  * [Optional arguments](#optional-arguments)
  * [Mandatory arguments](#mandatory-arguments)
  * [Required arguments](#required-arguments)
  * [Aliases](#aliases)
  * [Parameters](#parameters)
  * [Implicit arguments](#implicit-arguments)
  * [nargs](#nargs)
  * [Choices](#choices)
  * [Parsing function](#parsing-function)
  * [Parsing logic](#parsing-logic)
  * [Obtaining parsed values](#obtaining-parsed-values)
  * [Child parsers (commands)](#child-parsers-commands)
  * [Typo detection](#typo-detection)
  * [Public parser methods](#public-parser-methods)
  * [Modifiers](#modifiers)
  * [Exceptions](#exceptions)
  * [Parse errors](#parse-errors)
- [Environment](#environment)

## Features

- **Single Header**
- **Requires C++17**
- **Positional Arguments**: Support for positional arguments
- **Parameterized Arguments**: Named arguments with parameters
- **Implicit Arguments**: Optional switches (flags) with no parameters
- **Variadic Arguments**: Support for variadic positionals, flags and parameters
- **Repeatable Arguments**: Allows for multiple instances of the same argument
- **Arguments With Choices**: Specify a list of valid choices for an argument
- **Default Values**: Set default values for arguments
- **Typo Detection**: Detects single-character typos in argument names
- **Automatic Help Message Generation**: Provides usage information based on defined arguments
- **Customizable Parsing Logic**: Allows for tailored argument validation and processing
- And more...

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
        argParser parser("program_name", "this program does some useful stuff");
   }
   ```
3. Add arguments to parse
    ```c++
   // add positional int argument
   parser.addPositional<int>("positional")
               .help("positional argument")
               .finalize();
   // add mandatory argument with parameter
   parser.addArgument<int>("mandatory")
               .parameters("mandatory_param")
               .help("mandatory argument with mandatory param")
               .finalize();
   // add implicit int flag
   parser.addArgument<int>("-optional")
               .help("int optional argument (flag) with implicit value")
               .finalize();
   // add int flag with mandatory parameter
   parser.addArgument<int>("-optional-with-param")
               .parameters("mandatory_param")
               .help("flag with single mandatory param")
               .finalize();
   // add int flag with optional parameter
   parser.addArgument<int>("-optional-with-param2")
               .parameters("[optional_param]")
               .help("flag with single optional param")
               .finalize()
   // add int flag with multiple parameters and parsing function
   parser.addArgument<int>("-optional-with-multiple-params")
            .parameters("mandatory_param", "[optional_param]")
            .callable([](auto mandatory_param, auto optional_param) {
                int result = argParser::scanValue<int>(mandatory_param);
                if (optional_param){
                    result += argParser::scanValue<int>(optional_param);
                }
                return result;
            })
            .help("flag with multiple params and parsing function")
            .finalize();
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
   Usage: program_name [flags]... parameters... positional
   Positional arguments:
       positional : positional argument
   Flags (optional):
       -h,--help [arg] : Show this message and exit. 'arg' to get help about certain arg
       -optional : int optional argument (flag) with implicit value
       -optional-with-multiple-params <mandatory_param> [optional_param] : flag with multiple params
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
              .help("Positional int argument")
              .finalize();
```

### Optional arguments

`optional` arguments are not required to be set by user

Optional arguments can be `implicit`

To specify an optional argument, start it `with -`

```c++
parser.addArgument<int>("-x")
          .help("int optional argument with implicit value")
          .finalize();
```
 
### Mandatory arguments

All `mandatory` arguments must be set by the caller, otherwise an error is generated

Mandatory argument should have `at least one mandatory parameter`, i.e. they `cannot be implicit`

To specify a mandatory argument, its name/aliases should start `without -`
    
```c++
parser.addArgument<bool>("m")
          .parameters("m_param")
          .help("mandatory bool argument with mandatory parameter")
          .finalize();
```
     
Also, any `optional` argument can be made `mandatory` using method `mandatory()`:
    
```c++
parser.addArgument<bool>("-m")
          .parameters("m_param")
          .mandatory()
          .help("optional argument made mandatory")
          .finalize();
```
     
**NOTE:** positional or hidden arguments cannot be made mandatory
     
### Required arguments

The logic behind `required` arguments is such that `at least one` required argument should be set by the caller

To specify a required argument, use method `required()`

Here we declare 2 optional arguments `--req1` and `--req2` and then make them required:

```c++
parser.addArgument<int>("--req1")
          .required()
          .help("required argument 1 with implicit value")
          .finalize();
    
parser.addArgument<int>("--req2")
          .required()
          .help("required argument 2 with implicit value")
          .finalize();
```

Now the caller should set `either --req1 or --req2` or `both`, otherwise an error will be thrown

Mandatory arguments can be made required too:
    
```c++
parser.addArgument<bool>("m")
          .parameters("m_param")
          .required()
          .help("mandatory bool argument made required")
          .finalize();
```

### Aliases

Non-positional arguments can have aliases

Here is a `--bool` argument with `-b` alias:

```c++
parser.addArgument<bool>("-b", "--bool")
          .help("implicit bool flag with alias")
          .finalize();
```

The last string in the list is considered argument's name,
and other elements are aliases

**NOTE:** Argument name and alias should be of the same type:
if the name starts with `-`, aliases should also start with `-`:

```c++
parser.addArgument<int>("i", "integer") //VALID
     parser.addArgument<int>("-i", "--integer") //VALID
     parser.addArgument<int>("i", "-integer") //NOT VALID
     parser.addArgument<int>("-i", "integer") //NOT VALID
```
     
### Parameters

Non-positional arguments can have `parameters`

Parameters can also be `mandatory` or `optional`

`optional` parameters should be enclosed in `[]` and must not precede mandatory parameters

```c++
parser.addArgument<const char*>("-s")
          .parameters("str_value")
          .help("string optional argument with mandatory parameter str_value")
          .finalize();
parser.addArgument<const char*>("-p")
          .parameters("[str_value]")
          .help("string optional argument with optional parameter str_value")
          .finalize();
```

### Implicit arguments

If an argument has no parameters, it's called `implicit`.

If an `implicit` argument is of arithmetic type (bool, int, float,...), its value is `incremented` each time a user specifies it:

```c++
parser.addArgument<bool>("-b")
          .repeatable()
          .help("implicit bool flag")
          .finalize();
parser.addArgument<int>("-i")
          .defaultValue(4) //set default value to 4
          .repeatable()
          .help("implicit bool flag")
          .finalize();
                
> ./app -b    - Will set the -b flag to true
> ./app -i    - Will increment the -i flag, resulting in 5
```

**NOTE:** arguments with single optional parameter are also considered `implicit` in case the parameter is not set:
```c++
parser.addArgument<int>("-i")
          .parameters("[int]") //single optional parameter
          .help("int argument with single optional parameter")
          .finalize();

> ./app -i 10    - Will explicitly set the -i flag to 10
> ./app -i       - Will increment the -i flag, resulting in 1
```

Arguments can also be made `repeatable`, which allows them to be set more than once  
(This is applicable not only to implicit arguments):

```c++
parser.addArgument<int>("-j")
          .repeatable()
          .help("int optional repeatable argument (implicit)")
          .finalize();
                
> ./app -jjj    - Repeatable implicit argument, increments 3 times, resulting in 3 
```
                        
### nargs

Apart from `parameters()` method, an `nargs()` method can be used to specify the number of parameters an argument can have

It's useful if an argument can have several or an arbitrary number of identical parameters:
```c++
parser.addArgument<int>("-var")
                .nargs<3>() // takes 3 mandatory integers
                .help("parses 3 integers. Result is std::vector<int>")
                .finalize();

> ./app -var 1 2 3       - stores 3 numbers as a vector
> ./app -var 1 2         - ERROR: not enough parameters
```

`nargs()` takes 2 template parameters:
- Number of mandatory parameters
- Total number of parameters (ignored if less than the first one)

So, for example the call `nargs<1,3>` will add 1 mandatory and 2 optional parameters to an argument
```c++
parser.addArgument<int>("-i")
                .nargs<1, 3>() // add 1 mandatory and 2 optional parameters
                .help("Variadic pos argument of type int")
                .finalize();

> ./app -i 1 2 3       - stores 3 numbers as a vector    
> ./app -i 1 2         - stores 2 numbers as a vector 
> ./app -i 1           - stores 1 numbers as a vector
> ./app -i 1 2 3 4     - ERROR: only 3 values allowed, provided 4
```

`nargs<3,1>` will simply add 3 mandatory parameters, ignoring the value 1 as it's less than 3
```c++
parser.addArgument<int>("-i")
                .nargs<3, 1>() // add 3 mandatory parameters
                .help("Variadic pos argument of type int")
                .finalize();

> ./app -i 1 2 3       - stores 3 numbers as a vector    
> ./app -i 1 2         - ERROR: not enough parameters
> ./app -i 1 2 3 4     - ERROR: only 3 values allowed, provided 4
```

The second parameter in `nargs` can be -1 (or any value less than 0), thus making the argument `variadic`

A `variadic` argument can take any number of parameters:
```c++
parser.addArgument<int>("-var")
                .nargs<0,-1>() // takes any number of integer parameters
                .help("parses any number of integers. Result is std::vector<int>")
                .finalize();
                
> ./app -var 123 321 12     - stores 3 numbers as a vector
> ./app -var 123            - stores 1 number in a vector
```

Positional arguments can be variadic too:
```c++
parser.addPositional<int>("var_pos") 
                .nargs<0,-1>() // make argument variadic
                .help("Variadic pos argument of type int")
                .finalize();
                
> ./app 1 2 3 4 5       - stores 5 numbers as a vector  
```

**NOTE:** `nargs()` cannot have both parameters set to 0, this will result in a compilation error

The return type of the argument with nargs changes to `std::vector<argument_type>`:
```c++
parser.addArgument<int>("-var")
                .nargs<3>() ...

> ./app -var 123 321 12     - stores 3 numbers as a vector

// obtain parsed value as a vector
auto var = parser.getValue<std::vector<int>>("-var");
```

**NOTE:** `nargs<1>` or `nargs<0,1>` will not change the return type to `vector`, but will act as a single parameter:
```c++
parser.addArgument<int>("-var")
                .nargs<1>() ...
// argument's type is not changed to vector                        
auto var = parser.getValue<int>("-var");               
```

`parameters()` can be called before `nargs()`, but you can only specify ONE parameter there.  
This way it will act as a metavar for parameter's name.  
Passing more than 1 parameter to `parameters()` followed by `nargs()`, will result in a compilation error

**NOTE:** Calling `parameters("[optional]")` along with `nargs()` will throw an exception

If `parameters()` was not called before `nargs()`, a default name is provided for parameter (capitalized argument's key)
      
### Choices

Arguments of `arithmetic` or `string` types can have a list of possible valid choices which can be set with the `choices()` method

If a user specifies an argument that is not on the list, an error is thrown

```c++
parser.addArgument<int>("-c")
          .choices(1, 2, 3)
          .help("int argument with choices 1, 2, 3")
          .finalize();

> ./app -c 1    - valid
> ./app -c 4    - ERROR: invalid choice
```

**NOTE:** Choices are not applicable to `const char *` or similar types due to the nature of `==` operator:
```c++
// The following results in a compilation error
parser.addArgument<const char*>("-c")
          .choices("a", "b", "c")
```

### Parsing function
                                                            
An argument can be parsed by built-in parser only if
it has `0 or 1 parameters` of `arithmetic` or `string` type:
* `bool`, `int`, `float`, `double`, `unsigned int`, ... (arithmetic types)
* `char*`, `const char*`, `std::string` (string types)

Otherwise, a `parsing function` should be provided with `callable` method

**NOTE:** `nargs` does not add multiple parameters to an argument, it still counts as 1 parameter
    
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
        .parameters("[str_value]")
        .callable(test)
        .finalize();  
```
                
A parsing function is called every time an argument is found in the command line,
and its result is applied to the underlying argument's variable

The parsing function `must` be specified for an argument in the following cases:

* The argument is `not of arithmetic or string` type
* The argument has `more than 1 parameter`
                
Here are some constraints for parsing functions:                

* Parsing function can be `void` (no return value) or have a return type           
* Function's return type must be convertible to the argument's type (if not `void`) 
* Parsing function should take as many `string parameters` (const char*) as there are `parameters` specified for argument (unless `nargs` are used, in that case it should have 1 string param)
* Parsing function can also have `side parameters`. 
They need to be placed before string parameters in the function declaration,
and corresponding values should be fed to `callable` method:
    
```c++
// Function not valid, side parameters must be placed before string params
int invalid_func(const char* a, int i){...}

// Valid parsing function, int goes before string
int valid_func(int a, const char* a1){
    return a + (int)strtol(a1, nullptr, 0);
}

...

// variable will be used as side parameter
int x = 5;
parser.addArgument<int>("v", "v_int")
        .parameters("vv")
        .callable(valid_func, x)
        .help("mandatory arg with mandatory value and side argument x for function tst()")
        .finalize();
```

Custom parsing function allows for custom types to be used as arguments.  
Here's an example of an argument with custom type CL:                 
    
```c++
// User-defined type
struct CL{
    bool a = false;
    int b = 0;
};

// Parsing function
CL createStruct(const char *bl, const char *itgr = nullptr){
    //static parser helper function, converts string to basic type
    bool b = argParser::scanValue<bool>(bl);
    int i = argParser::scanValue<int>(itgr);
    return CL{b, i};
}

...
// specify parsing function for custom type
parser.addArgument<CL>("struct")
        .parameters("bool", "[integer]")
        .callable(createStruct)
        .help("mandatory arg with 2 parameters: mandatory and optional, returns result of function createStruct()")
        .finalize();
```

Lambdas can be used as parsing functions as well:

```c++
parser.addArgument<std::vector<const char*>>("-a", "--array")
            .parameters("a1", "[a2]")
            .callable([](auto a1, auto a2){
                return std::vector<const char*>{a1, a2==nullptr?"null":a2};
            })
            .help("optional argument with 2 string values (one optional) and lambda converter")
            .finalize();
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
parser.addArgument<int>("-x")
      .help("int optional argument with implicit value")
      .finalize();
// parse arguments
parser.parseArgs(argc, argv);            
// retrieve int value. The type of getValue() must correspond to the type of addArgument(), 
// otherwise error is thrown 
auto x = parser.getValue<int>("-x"); 
// it's also possible to obtain value with overloaded operators [] and T:
int x2 = parser["-x"]; 
```
    
Obtaining value with `globalPtr()` modifier:
    
```c++
// declare int variable
int j;
// add int argument and pass the address of declared variable
parser.addArgument<int>("-j")
      .globalPtr(&i)
      .help("int optional argument with implicit value")
      .finalize();
// parse arguments
parser.parseArgs(argc, argv);
// after that, the parsed result will be stored in j variable
```

Obtaining value with capturing lambda:

```c++
// declare int variable
int j = 0;
// add int argument with capturing lambda
parser.addArgument<int>("-j")
            .callable([&j](){j++;})
            .help("int optional argument with implicit value")
            .finalize();
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
            .help("parses any number of integers. Result is std::vector<int>")
            .finalize();
// parse arguments
parser.parseArgs(argc, argv);  
// retrieve variadic arg value as a vector
auto x = parser.getValue<std::vector<int>>("-var");  
```

### Child parsers (commands)

Child parsers can be added to the main parser with `addCommand` method

They can have their own arguments and help messages

Here's how a child parser can be added:

```c++
// create main parser
argParser parser;
// add child parser
auto &child_parser = parser.addCommand("child", "child parser description");
// add an argument to child parser
child.addArgument<int>("--int")
    .help("int value")
    .finalize();
// parse arguments
parser.parseArgs(argc, argv);

> ./app child --int 5 // child parser is called with int argument
// now you can obtain parsed value from child parser:
auto x = child_parser.getValue<int>("--int");
```

### Typo detection

argParser is capable of detecting single-character typos in arguments' names

If a user specifies an unknown argument that is similar to a valid argument, a message is displayed:

```c++
parser.addArgument<int>("--list")
            .nargs<0,-1>()
            .help("list of integers")
            .finalize();

> ./app --lis 1 2 3
Output:
Unknown argument: --lis. Did you mean --list?
> ./app --lsit 1 2 3
Output:
Unknown argument: --lsit. Did you mean --list?
```

**NOTE:** Typo detection is only applicable to arguments starting with a `-` or commands

### Public parser methods

A list of public parser methods:

* `addPositional<T>("name")` - adds positional argument of type T
* `addArgument<T>("aliases",...)` - adds argument of type T with aliases
* `addCommand("name", "description")` - adds a child parser with a name and description.  
Returns a reference to the child parser
* `setCallback(callback)` - sets a callback function to be called after parsing.  
The callback should be a `void` function or lambda with no parameters
* `hiddenSecret("secret")` - static method, sets a secret that reveals hidden arguments in help message if specified
* `getValue<T>("name or alias")` - returns parsed value of type T of the argument
* `scanValue<T>("string value")` - static method to parse some value from string using built-in parser.
Applicable to `arithmetic` or `string` values.
* `getSelfName()` - get executable self name. 
Returns program name if it was specified upon argParser creation, otherwise parses it from argv[0]
* `parseArgs(argc, argv)` - parse arguments from command line
* `parsed()` - returns `true` if arguments were parsed. 
Useful for checking if a command was called 
* `operator [] ("name or alias")` - provides access to const methods of argument, such as `isSet()`. 
Can also be used along with cast operator to obtain values
    
### Modifiers

Here's the full list of argument methods (modifiers):

* `parameters("params",...)` - adds string parameters to an argument (not applicable to positionals)
* `callable(callable, side_args,...)` - adds callable (function, lambda, etc) along with its side arguments
* `nargs<FRO, TO>()` - specify nargs. FRO - number of mandatory params, TO - overall number of params (if TO > FRO)
* `help("your help text")` - specify help text for the argument
* `advancedHelp("advanced help text")` - specify advanced help text, 
will be shown if user calls `--help your_argument_here`
* `hidden()` - make argument `hidden` from generic help message.  
If the secret was set with `hiddenSecret()`, such arguments can be displayed with call to `--help secret`.  
Only for `optional` arguments
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
* `choices(choices,...)` - adds a list of possible valid choices for the argument. 
Applicable only to arithmetic types and strings
* `finalize()` - finalizes argument definition. 
An argument is not considered defined until this method is called

There are also some useful const methods for arguments:

* `isSet()` - returns `true` if argument was set by user
* `isOptional()` - returns `true` if argument is optional
* `isRequired()` - returns `true` if argument is required
* `isPositional()` - returns `true` if argument is positional
* `isImplicit()` - returns `true` if argument is implicit
* `isRepeatable()` - returns `true` if argument is repeatable
* `isVariadic()`   - returns `true` if argument is variadic
* `getName()` - returns `std::string` argument's name without aliases
 
### Exceptions

Some methods may throw exceptions on the stage where arguments are added if some conditions are not met:

* `std::logic_error` is thrown by arg modifiers like `defaultValue()` and some others
* `std::invalid_argument` is thrown by parser methods like `addArgument()`, `addPositional()`, `getValue()`, etc...        
* `std::runtime_error` is thrown by `scanValue()` method along with some internal methods     

### Parse errors

`parseArgs()` can throw 2 types of errors:
* `argParser::unparsed_param` - if a certain argument could not be parsed.
in that case, `name()`, `cli()`, and `what()` methods can be called to retrieve some info about that argument
* `argParser::parse_error` - in case of unknown arguments and other errors

Here's an example:

```c++
try{
    // parse arguments
    parser.parseArgs(argc, argv);
}catch(argParser::unparsed_param &e){
    //catch argument parsing error
    std::cout << "Caught error: " << e.what() << std::endl;
    // check unparsed argument
    std::cout << "Last unparsed arg: " << e.name() << std::endl;
    // get list of parameters that were provided along with that argument by the caller
    std::cout << "Passed parameters:";
    for(auto &el : e.cli()){
        std::cout << " " + el;
    }
    std::cout << std::endl;
    return -1;
}catch(argParser::parse_error &e){
    // catch other exceptions
    std::cout << "Caught error: " << e.what() << std::endl;
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
