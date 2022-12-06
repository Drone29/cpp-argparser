//
// Created by andrey on 17.12.2021.
//

#ifndef VALLAB_PRINTER_ARGPARSER_H
#define VALLAB_PRINTER_ARGPARSER_H

#include <map>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <cstring>
#include <any>
#include <memory>
#include <typeindex>
#include <string_view>

/// help key
#define HELP_NAME "--help"
#define HELP_ALIAS "-h"
/// advanced help option (show hidden)
#define HELP_HIDDEN_OPT "-a"
/// braced help option
#define HELP_ADVANCED_OPT_BRACED "[" HELP_HIDDEN_OPT " | param]"
/// help self-explanation
#define HELP_GENERIC_MESSAGE \
    "Show this message and exit. " HELP_HIDDEN_OPT " to list advanced options"
/// delimiter for key aliases
#define KEY_ALIAS_DELIMITER ","
/// bool parsable strings
#define BOOL_POSITIVES "true", "1", "yes", "on"
#define BOOL_NEGATIVES "false", "0", "no", "off"

#define FUNC_ARG_SIGNATURE "const char*"

/// list of arguments to unpack starting from x index (ONLY TRIVIAL TYPES)
#define UNPACK_ARGUMENTS(arg,x) \
(arg)[(x)],                                 \
(arg)[(x)+1],                               \
(arg)[(x)+2],                               \
(arg)[(x)+3],                               \
(arg)[(x)+4],                               \
(arg)[(x)+5],                               \
(arg)[(x)+6],                               \
(arg)[(x)+7],                               \
(arg)[(x)+8],                               \
(arg)[(x)+9]
/// short version, with index=0
#define UNPACK_ARGS(arg) UNPACK_ARGUMENTS(arg,0)

//Count certain chars in string at compile-time
constexpr int countChars( const char* s, char c ){
    return *s == '\0' ? 0
                      : countChars( s + 1, c) + (*s == c);
}

constexpr bool strings_equal(char const * a, char const * b) {
    return std::string_view(a)==b;
}

//Need 2 macros to first evaluate expression and then stringify
#define STRINGIFY_IMPL(z) #z
#define STRINGIFY(z) STRINGIFY_IMPL(z)
//temporary value to store arguments
#define ARGS_LIST (UNPACK_ARGUMENTS(arg,x))
//Turn arguments list into string
#define ARG_STRING STRINGIFY(ARGS_LIST)
//count max function arguments provided by UNPACK_ARGUMENTS
#define MAX_ARGS countChars(ARG_STRING, ']')

/**
 *  To add a new type:
 *  1. Add it between ARG_TYPE_STRING and ARG_TYPE_BOOL
 *  2. Add corresponding type to argParser.OPTION struct
 *  3. Modify argParser.optionSetToGlobal() function to set value to global var if present
 *  4. Add your type parser to argParser.parseOption()
 */
/// non-enum help type
#define ARG_TYPE_HELP "HELP"

template <typename ...>
struct are_same : std::true_type {};

template <typename S, typename T, typename ... Ts>
struct are_same <S, T, Ts...> : std::false_type {};

template <typename T, typename ... Ts>
struct are_same <T, T, Ts...> : are_same<T, Ts...> {};

template <typename ... Ts>
inline constexpr bool are_same_v = are_same<Ts...>::value;

class BaseOption{
public:
    virtual ~BaseOption() {};
    virtual std::any action (const std::vector<const char*> &args) = 0;
    virtual std::any increment() = 0;
    virtual void set() = 0;
    virtual std::string get_str_val() = 0;
    std::any anyval;
    bool has_action = false;
};

template <typename T>
class DerivedOption : public BaseOption{
public:
    ~DerivedOption() override = default;

    template <typename...args>
    DerivedOption(T(*func)(args...) = nullptr) {
        //check if args are const char*
        static_assert(are_same_v<const char*, args...>, "Error: only const char* allowed");
        t_action = reinterpret_cast<T (*)(const char *...)>(func);
        has_action = func != nullptr;
        anyval = value;
    }

    std::any action(const std::vector<const char*> &args) override{
        const char *argvCpy[MAX_ARGS+1] = {nullptr};
        for(int i=0; i<args.size();i++){
            argvCpy[i] = args[i];
        }
        if(t_action != nullptr)
            value = t_action(UNPACK_ARGS(argvCpy));
        anyval = value;
        return anyval;
    }

    std::any increment() override {
        if constexpr (std::is_arithmetic<T>::value){
            value+=1;
        }
        anyval = value;
        return anyval;
    }

    std::string get_str_val() override{
        std::string res;
        try{
            if constexpr (std::is_arithmetic<T>::value){
                res = std::to_string(value);
            }
            else if constexpr (std::is_convertible<T, std::string>::value){
                res = std::string(value);
            }
        }catch(...){
            res = ""; //if null or other exception, set empty
        }

        return res;
    }

    void set() override {
        value = std::any_cast<T>(anyval);
    }

private:
    T(*t_action)(const char*...) = nullptr;
    T value{};
};


struct ARG_DEFS{

    ~ARG_DEFS() {
        delete option;
    }
    friend class argParser;

    ARG_DEFS &help(std::string hlp){
        m_help = std::move(hlp);
        return *this;
    }
    ARG_DEFS &advanced_help(std::string hlp){
        m_advanced_help = std::move(hlp);
        return *this;
    }
    ARG_DEFS &hidden(){
        m_hidden = true;
        return *this;
    }
    ARG_DEFS &final(){
        m_final = true;
        return *this;
    }
    ARG_DEFS &default_value(std::any val){
        if (val.type() != option->anyval.type()){
            throw std::logic_error(std::string(__func__) + "(" + typeStr + "): cannot add default value of different type");
        }
        if(!positional){
            option->anyval = std::move(val);
            option->set();
        }
        return *this;
    }
private:
    std::string m_help;
    std::string m_advanced_help;
    //hidden option
    bool m_hidden = false;
    //Final
    bool m_final = false;
    //list of options
    std::vector<std::string> m_options;

    std::string typeStr;
    ///Option/flag
    BaseOption* option = nullptr;
    //in use
    bool set = false;
    //alias
    std::string m_alias;
    //Positional
    bool positional = false;
    //Flag
    bool flag = false;
    //Implicit
    bool implicit = false;
};

class argParser
{
public:
    argParser(){
        argMap[HELP_NAME] = new ARG_DEFS();
        argMap[HELP_NAME]->typeStr = ARG_TYPE_HELP;
        argMap[HELP_NAME]->m_help = std::string(HELP_GENERIC_MESSAGE);
        argMap[HELP_NAME]->m_options = {HELP_ADVANCED_OPT_BRACED};
        argMap[HELP_NAME]->flag = true;
        argMap[HELP_NAME]->m_final = true;
        setAlias(HELP_NAME, HELP_ALIAS);
        //Count max number of arguments
        max_args = MAX_ARGS;
    }
    ~argParser(){
        std::vector<std::string> aliases;
        for (auto &x : argMap){
            delete x.second;
        }
        argMap.clear();
    }

    template <typename T, typename...args>
    ARG_DEFS &addPositional(const std::string &key,
                        T(*func)(args...) = nullptr){

        auto splitKey = parseKey(key, __func__);
        if(splitKey.alias.empty()){
            throw std::logic_error(std::string(__func__) + ": " + key + " positional argument cannot have aliases");
        }

        for(auto &x : argMap){
            for(auto &y : x.second->m_options){
                if(!isOptMandatory(y) && !x.second->m_final){
                    throw std::invalid_argument(std::string(__func__) + ": " + key
                    + " cannot add positional argument: conflict with non-final arg " + x.first + " with arbitraty option " + y);
                }
            }
        }

        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        ///check if default parser for this type is present
        if(func == nullptr){
            try{
                scan(nullptr, std::type_index(typeid(T)));
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }else{
            static_assert(sizeof...(args) > 1, " too many arguments in function");
        }

        auto x = new DerivedOption<T>(func);

        auto option = new ARG_DEFS();
        option->typeStr = strType;
        option->m_options = {};
        option->option = x;
        option->positional = true;

        argMap[splitKey.key] = option;
        posMap.emplace_back(splitKey.key);

        return *option;
    }

    /**
     *
     * @tparam T function return type
     * @tparam args function arguments (const char*)
     * @param key argument key ("-f" adds arbitrary argument, "f" adds mandatory argument)
     * @param opts list of options ({"foo"} - mandatory, {"[foo]"} = arbitrary). {} or {"[foo]"} treated as implicit argument
     * @param func function pointer or nullptr
     * @return
     */
    template <typename T, typename...args>
    ARG_DEFS &addArgument(const std::string &key,
                      const std::vector<std::string>& opts = {},
                      T(*func)(args...) = nullptr){

        auto splitKey = parseKey(key, __func__);
        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        if(opts.size() > max_args || sizeof...(args) > max_args){
            throw std::invalid_argument("Too many arguments for " + key
                                     + ", provided " + std::to_string(opts.size())
                                     + ", max " + std::to_string(max_args) + " allowed."
                                     + " Consider changing " + std::string(STRINGIFY_IMPL(UNPACK_ARGUMENTS)));
        }

        ///Check for invalid sequence order of arguments
        std::string last_arbitrary_arg;
        std::string last_mandatory_arg;
        for(auto & opt : opts){
            std::string sopt = opt;
            if(sopt.empty()){
                throw std::invalid_argument(key + " argument name cannot be empty");
            }
            if(isOptMandatory(sopt)){
                last_mandatory_arg = sopt;
                if(!last_arbitrary_arg.empty()){
                    throw std::invalid_argument(key
                                             + ": arbitrary argument "
                                             + last_arbitrary_arg
                                             + " cannot be followed by mandatory argument "
                                             + sopt);
                }
            }
            else{
                last_arbitrary_arg = sopt;
            }
        }

        bool final = false;
        if(!last_arbitrary_arg.empty()){
            if(!posMap.empty()){
                final = true;
            }
        }

        bool flag = (splitKey.key[0] == '-');
        if(!splitKey.alias.empty()){
            bool match = (splitKey.alias[0] == '-');
            if (match != flag){
                throw std::invalid_argument(key + ": cannot add alias " + splitKey.alias + ": different type");
            }
        }

        if(last_mandatory_arg.empty() && !flag){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " should have at least 1 mandatory parameter");
        }

        bool implicit = opts.empty() || (opts.size() == 1 && last_mandatory_arg.empty());

        if(func == nullptr){
            if(implicit && !std::is_arithmetic<T>::value){ //typeid(T) != typeid(bool)
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no function provided for non-arithmetic arg with implicit option");
            }
            if(opts.size() > 1){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no function provided for arg with " + std::to_string(opts.size()) + " options");
            }

            ///check if default parser for this type is present
            try{
                scan(nullptr, std::type_index(typeid(T)));
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }else if(sizeof...(args) != opts.size()){
                throw std::invalid_argument(std::string(__func__) + " " + key + " opts size != function arguments");
        }

        auto x = new DerivedOption<T>(func);

        auto option = new ARG_DEFS();
        option->typeStr = strType;
        option->option = x;
        option->m_options = opts;
        option->flag = flag;
        option->m_final = final;
        option->implicit = implicit;

        argMap[splitKey.key] = option;

        if(!splitKey.alias.empty()){
            //add alias
            setAlias(splitKey.key, splitKey.alias);
        }

        if(!flag)
            mandatory_option = true;

        return *option;
    }

    template <typename T>
    T getValue(const std::string &key){
        parsedCheck(__func__);
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__);
        auto r = getArg(key);
        try{
            auto base_opt = r.option;
            return std::any_cast<T>(base_opt->anyval);
        }catch(const std::bad_any_cast& e){
            throw std::runtime_error(std::string(__func__) + ": " + key + " cannot cast to " + strType);
        }
    }

    bool isSet(const std::string &key){
        if(argMap.find(key) != argMap.end()){
            return argMap[key]->set;
        }
        throw std::runtime_error(key + " not defined");
    }

    ///Set alias for option
    void setAlias(const std::string &key, const std::string &alias){
        if(argMap.find(key) == argMap.end()){
            throw std::runtime_error(std::string(__func__) + ": " + key + " not defined");
        }
        if(!argMap[key]->m_alias.empty()){
            throw std::runtime_error(std::string(__func__) + ": " + key + " alias already defined: " + argMap[key]->m_alias);
        }
        if(argMap[key]->positional){
            throw std::runtime_error(std::string(__func__) + ": " + key + " positional argument cannot have alias");
        }

        sanityCheck(alias, __func__);
        argMap[key]->m_alias = alias;
    }

    /// Self exec name
    auto getSelfName(){
        parsedCheck(__func__);
        return this->binary_name;
    }

    /// Parse arguments
    int parseArgs(int argc, char *argv[], bool allow_zero_options = false)
    {
        ///Retrieve binary self-name
        std::string self_name = std::string(argv[0]);
        binary_name = self_name.substr(self_name.find_last_of('/') + 1, self_name.length()-1);
        mandatory_option &= !allow_zero_options;

        auto parseArgument = [this, &argv, &argc](const char *key, const char *value, int idx){
            std::vector<const char*> argvCpy;
            for(int i=0;i<argMap[key]->m_options.size(); i++){
                if(idx+i >= argc){
                    break;
                }
                argvCpy.push_back(argv[idx+i]);
            }

            if(argMap[key]->option->has_action){
                argMap[key]->option->action(argvCpy);
            }else{
                auto val = argMap[key]->option->anyval;
                argMap[key]->option->anyval = scan(value, val.type());
            }
        };

        for (int i = 1; i < argc; i++){
            const char *pName = argv[i];
            const char *pValue = argv[i+1];

            std::string s = pName;
            std::string s2 = (pValue == nullptr) ? "" : pValue;

            auto c = s.find('=');
            if(c != std::string::npos){
                s2 = s.substr(c+1);
                s = s.substr(0, c);
                pName = s.c_str();
                pValue = s2.c_str();
            }

            ///Find alias
            if(argMap.find(pName) == argMap.end()){
                for(auto &x : argMap){
                    if(x.second->m_alias == pName){
                        pName = x.first.c_str();
                        break;
                    }
                }
            }

            if(argMap.find(pName) == argMap.end()){
                ///Try parsing positional args
                int pos_idx = i;
                if(!posMap.empty()){
                    for(auto &x : posMap){
                        if(pos_idx >= argc){
                            break;
                        }
                        parseArgument(x.c_str(), argv[pos_idx], pos_idx);
                        positional_cnt++;
                        pos_idx++;
                    }
                    break;
                }

                dummyFunc(nullptr);
                throw std::runtime_error("Unknown argument: " + std::string(pName));
            }

            if(argMap[pName]->typeStr == ARG_TYPE_HELP){
                helpDefault(argv[0], pValue);
                exit(0);
            }
            else
            {
                ///Parse other types

                int mandatory_opts = 0;
                for(auto & x: argMap[pName]->m_options){
                    if(isOptMandatory(x)){
                        mandatory_opts++;
                    }
                }

                ///Check if string null or next key
                if(mandatory_opts
                   && (pValue == nullptr
                       || argMap.find(pValue) != argMap.end())){
                    throw std::runtime_error("Error: no argument provided for " + std::string(pName));
                }
                //todo: implicit
                ///Parse arg with implicit option
                if(argMap[pName]->implicit){
                    if(argMap[pName]->option->has_action){
                        argMap[pName]->option->action({});
                    }
                    else{
                        argMap[pName]->option->increment();
                    }
                    continue;
                }

                int opts_cnt = 0;
                int cnt = i+1;

                for(int j=0; j<argMap[pName]->m_options.size(); j++){
                    if(cnt >= argc){
                        break;
                    }
                    if(argMap.find(argv[cnt]) != argMap.end()){
                        break;
                    }
                    cnt++;
                    opts_cnt++;
                }

                if(opts_cnt < mandatory_opts){
                    throw std::runtime_error(std::string(pName) + " requires "
                                             + std::to_string(mandatory_opts) + " arguments, but " + std::to_string(opts_cnt) + " were provided");
                }

                parseArgument(pName, pValue, i+1);
                i += opts_cnt;
                ///return if final
                if(argMap[pName]->m_final){
                    posMap.clear();
                    break;
                }

                argMap[pName]->set = true;
                //count mandatory options
                if(!argMap[pName]->flag){
                    option_cnt++;
                }
            }
        }
        args_parsed = true;
        //error if no option was set
        if(!option_cnt && mandatory_option){
            throw std::runtime_error("No option specified");
        }
        if(positional_cnt < posMap.size()){
            throw std::runtime_error("Not enough positional arguments provided");
        }
        return 0;
    }

    auto operator [](const char *key) const {return getArg(key);}

private:

    struct KEY_ALIAS{
        std::string key;
        std::string alias;
    };

    std::map<std::string, ARG_DEFS*> argMap;
    std::vector<std::string>posMap;

    std::string binary_name;
    bool args_parsed = false;
    int max_args = 0;
    int option_cnt = 0;
    bool mandatory_option = false;
    int positional_cnt = 0;

    ARG_DEFS &getArg(const std::string &key) const {
        std::string skey = key;
        if(argMap.find(skey) == argMap.end()){
            for(auto &x : argMap){
                if(x.second->m_alias == skey){
                    skey = x.first;
                    break;
                }
            }
        }

        if(argMap.find(skey) != argMap.end()){
            return *argMap.find(skey)->second;
        }

        throw std::runtime_error(key + " not defined");
    }

    static std::string getFuncTemplateType(const char *pretty_func, const char *specifier = "T"){
        std::string ref = std::string(specifier) + " = ";
        std::string y = pretty_func;
        if(y.find(ref)){
            y = y.substr(y.find(ref) + ref.length());
            auto semi = y.find(';');
            if(semi == std::string::npos){
                semi = y.find(']');
            }
            y = y.substr(0, semi);
        }else{
            y = "";
        }
        return y;
    }

    static std::any scan(const char *arg, std::type_index type){

        std::string temp = (arg == nullptr) ? "" : std::string(arg);

        if(type == std::type_index(typeid(const char *))){
            return arg;
        }else if(type == std::type_index(typeid(std::string))){
            return temp;
        }else if(type == std::type_index(typeid(bool))){
            auto isTrue = [func=__func__](const char *s){
                const char *pos_buf[] = {BOOL_POSITIVES, nullptr};
                const char *neg_buf[] = {BOOL_NEGATIVES, nullptr};
                for(auto &i : pos_buf){
                    if(i == nullptr) break;
                    if(!strcmp(s, i)) return true;
                }
                for(auto &i : neg_buf){
                    if(i == nullptr) break;
                    if(!strcmp(s, i)) return false;
                }
                throw std::runtime_error(std::string(func) + ": unable to convert " + s + " to bool");
            };

            std::string lVal;
            //convert to lower case
            for(auto elem : temp) {
                lVal += std::tolower(elem);
            }
            bool val = isTrue(lVal.c_str());
            return val;
        }

        ///numbers
        char *tmp = (char*)temp.c_str();
        std::any res;
        if(type == std::type_index(typeid(int))){
            res = (int)strtol(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(char))){
            res = (char)strtol(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(short int))){
            res = (short int)strtol(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(long))){
            res = strtol(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(long long))){
            res = strtoll(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(unsigned int))){
            res = (unsigned int)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(unsigned long))){
            res = strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(unsigned char))){
            res = (unsigned char)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(short unsigned int))){
            res = (short unsigned int)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(float))){
            res = strtof(temp.c_str(), &tmp);
        }else if(type == std::type_index(typeid(double))){
            res = strtod(temp.c_str(), &tmp);
        }else{
            throw std::logic_error(std::string(__func__) + ": value of unknown type " + temp);
        }

        if(temp.c_str() == tmp || *tmp){
            throw std::runtime_error(std::string(__func__) + ": could not convert " + temp);
        }
        return res;
    }

    void parsedCheck(const char* func = nullptr) const{

        if(func == nullptr){
            func = __func__;
        }
        if(!args_parsed){
            throw std::runtime_error(std::string(func) +": Invalid call. Arguments not parsed yet");
        }
    }

    void sanityCheck(const std::string &key, const char* func = nullptr){

        if(func == nullptr){
            func = __func__;
        }

        if(key.empty()){
            throw std::runtime_error(std::string(func) +": Key cannot be empty!");
        }

        if(std::string(key) == std::string(HELP_HIDDEN_OPT)){
            throw std::runtime_error(std::string(func) + ": " + HELP_HIDDEN_OPT + " key is reserved");
        }

        //Check previous definition
        if(argMap.find(key) != argMap.end()){
            throw std::runtime_error(std::string(func) + ": " + std::string(key) + " already defined");
        }

        for(auto &x : argMap){
            if(x.second->m_alias == key){
                throw std::runtime_error(std::string(func) + ": " + std::string(key) + " already defined");
            }
        }

    }

    KEY_ALIAS parseKey(const std::string &key, const char* func = nullptr){

        if(func == nullptr){
            func = __func__;
        }

        auto stripSpaces = [] (const std::string& word) -> std::string{
            std::string newWord;
            for (char i : word) {
                if (i != ' ') {
                    newWord += i;
                }
            }
            return newWord;
        };

        std::string skey = stripSpaces(key);
        std::string alias;
        auto split = skey.find(KEY_ALIAS_DELIMITER);
        if(split != std::string::npos){
            alias = skey.substr(split+1);
            sanityCheck(alias, func);
            skey = skey.substr(0, split);
        }
        sanityCheck(skey, func);
        //key is always longer
        if(alias.length() > skey.length()){
            std::swap(skey, alias);
        }
        return {skey, alias};
    }

    static bool isOptMandatory(const std::string &sopt){
        return !sopt.empty() && (sopt.at(0) != '[')
                                && (sopt.at(sopt.length() - 1) != ']');
    }

    ///These are voids in case funcType is changed later
    static void dummyFunc(void*...){
        std::cout << "Use '" + std::string(HELP_NAME) + "' for list of available options" << std::endl;
    }

    void helpDefault(const char* name, const char *param = nullptr){

        bool advanced = false;

        auto printParam = [](auto j, const std::string& alias){
            std::string alias_str = "\t" + (alias.empty() ? "" : (alias + KEY_ALIAS_DELIMITER + " "));
            std::cout <<  alias_str + j.first;
            for(auto x : j.second->m_options){
                std::string opt = std::string(x);
                    if(isOptMandatory(opt))
                        opt = "<" + opt + ">";

                std::cout << " " + opt;
            }
        };

        auto sorted_usage = [this, &advanced, &printParam](bool empty, bool flag){

            auto print_usage = [&advanced, &printParam](auto j, const std::string& alias = ""){
                //skip already printed
                if(j.second->set){
                    return;
                }
                //skip hidden
                if(j.second->m_hidden && !advanced){
                    return;
                }
                //skip positional
                if(j.second->positional){
                    return;
                }

                printParam(j, alias);

                std::string def_val = (j.second->option == nullptr) ? "" : j.second->option->get_str_val();
                def_val = def_val.empty() ? "" : " (default " + def_val + ")";

                std::cout << " : " + j.second->m_help + def_val << std::endl;
                j.second->set = true; //help_set
            };

            for (auto & j : argMap)
            {
                if((j.second->flag == flag)
                   && (j.second->m_options.empty() == empty))
                {
                    //find alias
                    std::string alias = j.second->m_alias;
                    print_usage(j, alias);
                }

            }
        };

        if(param != nullptr){
            if(std::string(param) == HELP_HIDDEN_OPT){
                advanced = true;
            }
            else{
                //param advanced help
                auto j = argMap.find(param);
                //seek alias
                if(j == argMap.end()){
                    for(auto &x : argMap){
                        if(x.second->m_alias == param){
                            j = argMap.find(x.first);
                            break;
                        }
                    }
                }

                if(j != argMap.end()){
                    printParam(*j, j->second->m_alias);
                    std::cout << ":" << std::endl;
                    std::cout << "Type: " + j->second->typeStr << std::endl;
                    std::cout << j->second->m_help << std::endl;
                    std::cout << j->second->m_advanced_help << std::endl;
                }else{
                    std::cout << "Unknown parameter " + std::string(param) << std::endl;
                }
                return;
            }
        }

        std::string positional;
        for (auto &x : posMap){
            positional += " " + x;
        }

        int flag_cnt = 0, opt_cnt = 0;
        for(auto &x : argMap){
            if(x.second->flag){
                flag_cnt++;
            }else if(!x.second->positional){
                opt_cnt++;
            }
        }

        std::cout << "Usage: " + std::string(name)
                     + (flag_cnt ? " [flags]" : "")
                     + (opt_cnt ? (!mandatory_option ? " [options]" : " options") : "")
                     + (opt_cnt ? " [arguments...]" : "") + positional << std::endl;

        if(!posMap.empty()){
            std::cout << "Positional arguments:" << std::endl;
            for(auto &x : posMap){
                std::cout << "\t" + x + " : " + argMap[x]->m_help << std::endl;
            }
        }

        if(flag_cnt){
            std::cout << "Flags (arbitrary):" << std::endl;
            sorted_usage(true, true);
            sorted_usage(false, true);
        }
        if(opt_cnt){
            std::cout << "Options (mandatory):" << std::endl;
            sorted_usage(true, false);
            sorted_usage(false, false);
        }
    }
};


#endif //VALLAB_PRINTER_ARGPARSER_H
