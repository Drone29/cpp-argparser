//
// Created by andrey on 17.12.2021.
//

#ifndef VALLAB_PRINTER_ARGPARSER_H
#define VALLAB_PRINTER_ARGPARSER_H

#include <map>
//#include <functional>
//#include <utility>
//#include <cstdarg>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <cstring>
#include <any>
#include <memory>
#include <typeindex>

/// help key
#define HELP_NAME "--help"
#define HELP_ALIAS "-h"
/// advanced help option (show hidden)
#define HELP_HIDDEN_OPT "-a"
/// insert this into .help field in addOption/addFunction to add a hidden option
#define HELP_HIDDEN_KEY '\f'
/// brace some text in .help in {} to add per-key advanced help
#define HELP_ADVANCED_KEY_START '{'
#define HELP_ADVANCED_KEY_END '}'
/// braced help option
#define HELP_ADVANCED_OPT_BRACED "[" HELP_HIDDEN_OPT " | param]"
/// help self-explanation
#define HELP_GENERIC_MESSAGE \
    "Show this message. " HELP_HIDDEN_OPT " to list advanced options"
/// delimiter for key aliases
#define KEY_ALIAS_DELIMITER ","
/// bool parsable strings
#define BOOL_POSITIVES "true", "1", "yes", "on"
#define BOOL_NEGATIVES "false", "0", "no", "off"
/// cast to function pointer of type int (void*...)
#define FCAST reinterpret_cast<int (*)(void *, ...)>
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

class argParser
{
public:
    argParser(){
        argMap[HELP_NAME] = new ARG_DEFS{
                ARG_TYPE_HELP,
                {HELP_ADVANCED_OPT_BRACED},
                std::string(HELP_GENERIC_MESSAGE)};
        argMap[HELP_NAME]->flag = true;
        setAlias(HELP_NAME, HELP_ALIAS);
        //Count max number of arguments
        max_args = MAX_ARGS;
    }
    ~argParser(){
        std::vector<std::string> aliases;
        for (auto &x : argMap){
            bool nodelete = false;
            aliases.push_back(x.second->alias);
            for(auto &y : aliases){
                if(y == x.first){
                    nodelete = true;
                    break;
                }
            }
            if(!nodelete){
                delete x.second->option;
            }
            delete x.second;
        }
        argMap.clear();
    }

    template <typename T = const char*, typename...args>
    void addPositional(const std::string &key,
                        const std::string& help,
                        std::any(*func)(args...) = nullptr){

        auto splitKey = parseKey(key, __func__);
        if(!splitKey.alias.empty()){
            throw std::runtime_error(key + " positional argument cannot have aliases");
        }

        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        auto x = new DerivedOption<T>();
        x->action = reinterpret_cast<std::any (*)(const char *...)>(func);

        auto common_help = retrieveHelpText(help);
        auto adv_help = retrieveAdvancedHelp(help);

        auto option = new ARG_DEFS{strType, {}, common_help, adv_help, false, nullptr, x};
        option->positional = true;

        argMap[splitKey.key] = option;
        posMap.emplace_back(splitKey.key);

    }

    template <typename T = const char *, typename...args> //class F = std::any(*)(const char*)
    void addArgument(const std::string &key,
                      const std::string& help,
                      const std::vector<const char*>& opts,
                      std::any(*func)(args...) = nullptr){

        auto splitKey = parseKey(key, __func__);

        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        if(opts.size() > max_args){
            throw std::runtime_error("Too many arguments for " + std::string(key)
                                     + ", provided " + std::to_string(opts.size())
                                     + ", max " + std::to_string(max_args) + " allowed."
                                     + " Consider changing " + std::string(STRINGIFY_IMPL(UNPACK_ARGUMENTS)));
        }

        ///Check for invalid sequence order of arguments
        std::string last_arbitrary_arg;
        std::string last_mandatory_arg;
        for(auto & opt : opts){
            std::string sopt = opt;
            if(isOptMandatory(sopt)){
                last_mandatory_arg = sopt;
                if(!last_arbitrary_arg.empty()){
                    throw std::runtime_error(std::string(key)
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

        bool flag = (splitKey.key[0] == '-');
        if(!splitKey.alias.empty()){
            bool match = (splitKey.alias[0] == '-');
            if(match != flag){
                throw std::runtime_error(std::string(key) + ": cannot add alias " + splitKey.alias + ": different type");
            }
        }

        auto x = new DerivedOption<T>();
        x->action = reinterpret_cast<std::any (*)(const char *...)>(func); //reinterpret_cast<std::any (*)(const char *...)>(func)

        auto common_help = retrieveHelpText(help);
        auto adv_help = retrieveAdvancedHelp(help);
        auto hidden = isHelpHidden(help);

        auto option = new ARG_DEFS{strType, opts, common_help, adv_help, flag, nullptr, x};
        option->hidden = hidden;

        argMap[splitKey.key] = option;

        if(!splitKey.alias.empty()){
            //add alias
            setAlias(splitKey.key, splitKey.alias);
        }

        if(!flag)
            mandatory_option = true;

    }

    template <typename T = std::any>
    T getValue(const std::string &key){
        parsedCheck(__func__);
        if(argMap.find(key) != argMap.end()){
            auto base_opt = argMap[key]->option;
            auto strType = getFuncTemplateType(__PRETTY_FUNCTION__);
            ///if any
            if(std::type_index(typeid(T)) == std::type_index(typeid(std::any))){
                return base_opt->anyval;
            }
            ///otherwise check
            if(std::type_index(typeid(T)) != std::type_index(base_opt->anyval.type())){
                throw std::runtime_error(std::string(__func__) + ": " + key + " cannot cast to " + strType);
            }
            return std::any_cast<T>(base_opt->anyval);
        }
        throw std::runtime_error(key + " not defined");
    }

    ///Set advanced help
    void setAdvancedHelp(const std::string &key, const std::string& help){
        auto arg = getArg(key);
        arg->advanced_help += help;
    }

    ///Set alias for option
    void setAlias(const std::string &key, const std::string &alias){
        if(argMap.find(key) == argMap.end()){
            throw std::runtime_error(std::string(__func__) + ": " + key + " not defined");
        }
        if(!argMap[key]->alias.empty()){
            throw std::runtime_error(std::string(__func__) + ": " + key + " alias already defined: " + argMap[key]->alias);
        }
        if(argMap[key]->positional){
            throw std::runtime_error(std::string(__func__) + ": " + key + " positional argument cannot have alias");
        }

        sanityCheck(alias, __func__);
        argMap[alias] = new ARG_DEFS(*argMap[key]);
        ///.option just points to original object, so we need to free memory in smart way, see ~argParser()
        argMap[alias]->alias = key;
        argMap[key]->alias = alias;
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

        auto parseArgument = [this, &argv](const char *key, const char *value, int idx){
            auto action = *argMap[key]->option->action;
            if(action == nullptr){
                auto val = argMap[key]->option->anyval;
                argMap[key]->option->anyval = scan(value, val.type());
            }else{
                argMap[key]->option->anyval = action(UNPACK_ARGUMENTS(argv, idx));
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

            if(argMap.find(pName) == argMap.end()){
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
                ///Check if already defined
                if(argMap[pName]->set){
                    throw std::runtime_error("Error: redefinition of " + std::string(pName)
                                             + (argMap[pName]->alias.empty() ? "" : "(" + argMap[pName]->alias + ")"));
                }
                    ///Parse other types
                else{

                    int mandatory_opts = 0;
                    for(auto & x: argMap[pName]->options){
                        if(isOptMandatory(x)){
                            mandatory_opts++;
                        }
                    }

                    ///Parse bool with no arguments
                    if(std::type_index(argMap[pName]->option->anyval.type()) == std::type_index(typeid(bool))
                    &&
                    (argMap[pName]->options.empty()
                    //null, nex key or positional exist
                    || ((pValue == nullptr || argMap.find(pValue) != argMap.end() || !posMap.empty())
                    && !mandatory_opts)
                    )
                    ){
                        bool a = !std::any_cast<bool>(argMap[pName]->option->anyval);
                        argMap[pName]->option->anyval = a;
                        continue;
                    }

                    ///Check if string null or next key
                    if(mandatory_opts
                       && (pValue == nullptr
                       || argMap.find(pValue) != argMap.end())
                       ){
                        throw std::runtime_error("Error: no argument provided for " + std::string(pName));
                    }

                    int opts_cnt = 0;
                    int cnt = i+1;

                    for(int j=0; j<argMap[pName]->options.size();j++){
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
                }

                argMap[pName]->set = true;
                //copy to alias
                if(!argMap[pName]->alias.empty()){
                    *argMap[argMap[pName]->alias] = *argMap[pName];
                }
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
        return option_cnt;
    }

 //   auto operator [](const char *key) {return getArg(key);}

private:

    class BaseOption{
    public:
        virtual ~BaseOption() {}
        std::any anyval;
        std::any(*action)(const char*...) = nullptr;
    };

    template <typename T>
    class DerivedOption : public BaseOption{
    public:
        ~DerivedOption() override = default;
        DerivedOption(T val) : value{val}{
            anyval = value;
        }
        DerivedOption() {
            anyval = value;
        }
        T value;
    };

    struct ARG_DEFS{
        ///.option just points to original object, so we need to free memory in smart way, see ~argParser()
        ~ARG_DEFS() = default;
        std::string typeStr;
        std::vector<const char*> options;
        std::string help;
        std::string advanced_help;
        //Flag
        bool flag = false;
        //default value string
        const char* default_val = nullptr;
        ///Option/flag
        BaseOption* option = nullptr;
        //in use
        bool set = false;
        //hidden option
        bool hidden = false;
        //alias
        std::string alias;
        //Positional
        bool positional = false;
    };

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
        /////////////////////////
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
        }else if(type == std::type_index(typeid(long))){
            res = strtol(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(long long))){
            res = strtoll(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(unsigned int))){
            res = (unsigned int)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(unsigned long))){
            res = strtoul(temp.c_str(), &tmp, 0);
        }else if(type == std::type_index(typeid(float))){
            res = strtof(temp.c_str(), &tmp);
        }else if(type == std::type_index(typeid(double))){
            res = strtod(temp.c_str(), &tmp);
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
            throw std::runtime_error(std::string(key) + " already defined");
        }

    }

    ARG_DEFS *getArg(const std::string &key) {
        if(argMap.find(key) != argMap.end()){
            auto y = argMap[key];
            return y;
        }
        throw std::runtime_error(key + " not defined");
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

        return {skey, alias};
    }

    static bool isOptMandatory(const std::string &sopt){
        return (sopt.at(0) != '[')
               && (sopt.at(sopt.length()-1) != ']');
    }

    static bool isHelpHidden(const std::string &help_msg){
        bool res = false;
        auto control = help_msg.find(HELP_HIDDEN_KEY);
        if(control != std::string::npos){
            res = true;
        }
        return res;
    }

    static std::string retrieveHelpText(const std::string &help_msg){
        std::string res;
        auto control = help_msg.find(HELP_HIDDEN_KEY);
        if(control != std::string::npos){
            res = help_msg.substr(control + 1);
        }
        else{
            res = help_msg;
        }
        //remove advanced help
        control = res.find(HELP_ADVANCED_KEY_START);
        if(control != std::string::npos){
            res = res.substr(0, control);
            control = help_msg.find(HELP_ADVANCED_KEY_END);
            if(control != std::string::npos){
                res += help_msg.substr(control + 1);
            }
        }

        return res;
    }

    static std::string retrieveAdvancedHelp(const std::string &help_msg){
        std::string res;
        auto control = help_msg.find(HELP_ADVANCED_KEY_START);
        if(control != std::string::npos){
            res = help_msg.substr(control + 1);
            control = res.find(HELP_ADVANCED_KEY_END);
            if(control != std::string::npos){
                res = res.substr(0, control);
            }
        }
        else{
            res = "";
        }
        return res;
    }

    ///These are voids in case funcType is changed later
    static void dummyFunc(void*...){
        std::cout << "Use '" + std::string(HELP_NAME) + "' for list of available options" << std::endl;
    }

    void helpDefault(const char* name, const char *param = nullptr){

        bool advanced = false;

        auto printParam = [](auto j, const std::string& alias){
            std::string alias_str = alias.empty() ? "\t" : (alias + KEY_ALIAS_DELIMITER + " ");
            std::cout <<  alias_str + j.first;
            for(auto x : j.second->options){
                std::string opt = std::string(x);
//                if(!j.second->flag){
                    if(isOptMandatory(opt))
                        opt = "<" + opt + ">";
//                }
//                else{
//                    opt = opt.substr(opt.find('[')+1, opt.find(']')-1);
//                    opt = "<" + opt + ">";
//                }

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
                if(j.second->hidden && !advanced){
                    return;
                }
                //skip positional
                if(j.second->positional){
                    return;
                }

                printParam(j, alias);

                std::string def_val = (j.second->default_val == nullptr) ? ""
                                                                        : ( " (default " + std::string(j.second->default_val) + ")");

                std::cout <<" : " + j.second->help + def_val << std::endl;
                j.second->set = true; //help_set
            };

            for (auto & j : argMap)
            {
                if((j.second->flag == flag)
                   && (j.second->options.empty() == empty))
                {
                    //find alias
                    std::string alias = j.second->alias;
                    if(!alias.empty()){
                        argMap[alias]->set = true; //help_set
                    }

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
                if(j != argMap.end()){
                    printParam(*j, j->second->alias);
                    std::cout << ":" << std::endl;
                    std::cout << "Type: " + j->second->typeStr << std::endl;
                    std::cout << j->second->advanced_help << std::endl;
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
                std::cout << "\t" + x + " : " + argMap[x]->help << std::endl;
            }
        }

        if(flag_cnt){
            std::cout << "Flags (arbitrary):" << std::endl;
            sorted_usage(true, true);
            sorted_usage(false, true);
        }
        if(opt_cnt){
            std::cout << "Options:" << std::endl;
            sorted_usage(true, false);
            sorted_usage(false, false);
        }
    }
};


#endif //VALLAB_PRINTER_ARGPARSER_H
