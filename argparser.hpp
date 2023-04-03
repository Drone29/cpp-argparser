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
#include <tuple>
#include <ctime>
#include <iomanip>

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
/// additional key/option delimiter (default is space)
#define KEY_OPTION_DELIMITER "="
/// bool parsable strings
#define BOOL_POSITIVES "true", "1", "yes", "on"
#define BOOL_NEGATIVES "false", "0", "no", "off"

#define REQUIRED_OPTION_SIGN "(*)"

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
//constexpr int MAX_ARGS = countChars(ARG_STRING, ']');

#define GET_TYPE(x)  std::type_index(typeid(x))

#define ARG_TYPE_HELP "HELP"
//date format by default
#define DEFAULT_DATE_FORMAT "%Y-%m-%dT%H:%M:%S"
// isodate special type
using date_t = std::tm;

template <typename ...>
struct are_same : std::true_type {};

template <typename S, typename T, typename ... Ts>
struct are_same <S, T, Ts...> : std::false_type {};

template <typename T, typename ... Ts>
struct are_same <T, T, Ts...> : are_same<T, Ts...> {};

template <typename ... Ts>
inline constexpr bool are_same_v = are_same<Ts...>::value;

class BaseOption{
protected:
    friend class argParser;
    friend struct ARG_DEFS;
    BaseOption() = default;
    virtual ~BaseOption() = default;
    virtual std::any action (const std::string *args, int size) = 0;
    virtual std::any increment() = 0;
    virtual void set(std::any x) = 0;
    virtual std::string get_str_val() = 0;
    virtual void set_global_ptr(std::any ptr) = 0;
    std::any anyval;
    bool has_action = false;
};

template <typename T, class...Targs>
class DerivedOption : public BaseOption{
private:
    friend class argParser;
    std::any action(const std::string *args, int size) override{
        const char *argvCpy[MAX_ARGS+1] = {nullptr};
        const std::string *ptr = args;
        for(int i=0; i<size;i++){
            argvCpy[i] = (*ptr++).c_str();
        }

        auto cchar_tpl = std::make_tuple(UNPACK_ARGS(argvCpy));
        auto tplres = std::tuple_cat(tpl, cchar_tpl);

        if(t_action != nullptr){
            value = std::apply(t_action, tplres);
        }
        set_global();
        anyval = value;
        return anyval;
    }

    std::any increment() override {
        if constexpr (std::is_arithmetic<T>::value){
            value+=1;
        }
        set_global();
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

    void set(std::any x) override {
        try{
            anyval = x;
            value = std::any_cast<T>(anyval);
            set_global();
        }catch(std::bad_any_cast &e){
            throw std::invalid_argument("invalid type");
        }
    }

    void set_global_ptr(std::any ptr) override {
        try{
            global = std::any_cast<T*>(ptr);
        }catch(std::bad_any_cast &e){
            throw std::invalid_argument("invalid pointer type");
        }
    }

    template <typename...args>
    DerivedOption(T(*func)(Targs..., args...), std::tuple<Targs...> targs) { //Targs...targs

        static_assert(sizeof...(args) < MAX_ARGS, " too many arguments");
        //check if args are const char*
        static_assert(are_same_v<const char*, args...>, "Error: only const char* allowed");
        t_action = reinterpret_cast<T (*)(Targs...,const char *...)>(func);
        has_action = func != nullptr;
        anyval = value;

        tpl = targs;
    }
    ~DerivedOption() override = default;

    void set_global(){
        if(global != nullptr){
            //set global
            *global = value;
        }
    }
    T(*t_action)(Targs...,const char*...) = nullptr;
    T value{};
    std::tuple<Targs...> tpl;
    T *global = nullptr;
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
    /// positional, mandatory or required options cannot be hidden
    ARG_DEFS &hidden(){
        if(!m_positional && m_arbitrary && !m_required)
            m_hidden = true;
        return *this;
    }
    ARG_DEFS &repeatable(){
        if(!m_positional)
            m_repeatable = true;
        return *this;
    }
    ARG_DEFS &default_value(std::any val){
        try{
            if(m_arbitrary && !m_positional){
                option->set(std::move(val));
                show_default = true;
            }
        }catch(std::invalid_argument &e){
            throw std::logic_error(std::string(__func__) + "(" + typeStr + "): error: " + e.what());
        }
        return *this;
    }
    ARG_DEFS &global_ptr(std::any ptr){
        try {
            option->set_global_ptr(std::move(ptr));
        }catch(std::invalid_argument &e){
            throw std::logic_error(std::string(__func__) + "(" + typeStr + "): error: " + e.what());
        }
        return *this;
    }
    ///display as mandatory in help
    ///user should specify all of mandatory options
    ARG_DEFS &mandatory(){
        /// hidden cannot be mandatory
        if(m_arbitrary && !m_positional && !m_hidden)
            m_arbitrary = false;
        return *this;
    }
    ///only for arbitrary options
    ///user should specify at least one required option
    ARG_DEFS &required(){
        /// hidden cannot be required
        if(!m_positional && !m_hidden){
            m_required = true;
            /// if mandatory option forced to be required, set flag to arbitrary
            m_arbitrary = true;
        }
        return *this;
    }
    ///only for date_t
    ///specify date format in terms of strptime()
    ARG_DEFS &date_format(const char *format, bool hide_in_help = false){
        if(option->anyval.type() == typeid(date_t)
        && m_options.size() == 1){

            if(format != nullptr){
                //check if contains spaces
                for(auto &c : std::string(format)){
                    if(c == ' '){
                        throw std::logic_error(std::string(__func__) + ": invalid date format " + format + " (cannot contain spaces)");
                    }
                }
                auto t = std::time(nullptr);
                std::tm tt = *std::localtime(&t);
                std::stringstream ss;
                ss << std::put_time(&tt, format);
                ss >> std::get_time(&tt, format);
                if (ss.fail()){
                    throw std::logic_error(std::string(__func__) + ": invalid date format " + format);
                }
                m_date_format = format;
            }
            m_hide_date_format = hide_in_help;
        }
        return *this;
    }

    [[nodiscard]] bool is_set() const{
        return m_set;
    }
    [[nodiscard]] bool is_arbitrary() const{
        return m_arbitrary;
    }
    [[nodiscard]] bool is_required() const{
        return m_required;
    }
    [[nodiscard]] bool is_positional() const{
        return m_positional;
    }
    [[nodiscard]] bool is_implicit() const{
        return m_implicit;
    }
    [[nodiscard]] bool is_repeatable() const{
        return m_repeatable;
    }
    // applicable only for date_t type
    [[nodiscard]] const char *get_date_format() const{
        return m_date_format;
    }
    [[nodiscard]] auto options_size() const{
        return m_options.size();
    }

private:
    std::string m_help;
    std::string m_advanced_help;
    //hidden option
    bool m_hidden = false;
    //list of options
    std::vector<std::string> m_options;

    std::string typeStr;
    ///Option/flag
    BaseOption* option = nullptr;
    //in use
    bool m_set = false;
    //alias
    std::string m_alias;
    //Positional
    bool m_positional = false;
    //Flag
    bool m_arbitrary = false;
    //Required != !arbitrary. if !arbitrary, user should specify all of them. If required, at least one is enough
    bool m_required = false;
    //Implicit
    bool m_implicit = false;
    //Non-repeatable
    bool m_repeatable = false;
    //If starts with minus
    bool m_starts_with_minus = false;
    //Date format (only for isodate_t type)
    const char *m_date_format = nullptr;
    //If needed to be shown in help
    bool m_hide_date_format = false;
    //Mandatory opts
    int mandatory_options = 0;
    //show default
    bool show_default = false;
};

class argParser
{
public:
    argParser(){
        argMap[HELP_NAME] = new ARG_DEFS();
        argMap[HELP_NAME]->typeStr = ARG_TYPE_HELP;
        argMap[HELP_NAME]->m_help = std::string(HELP_GENERIC_MESSAGE);
        argMap[HELP_NAME]->m_options = {HELP_ADVANCED_OPT_BRACED};
        argMap[HELP_NAME]->m_arbitrary = true;
        setAlias(HELP_NAME, HELP_ALIAS);
    }
    ~argParser(){
        std::vector<std::string> aliases;
        for (auto &x : argMap){
            delete x.second;
        }
        argMap.clear();
        argVec.clear();
    }

    template <typename T, class...Targs, typename...args>
    ARG_DEFS &addPositional(const std::string &key,
                            T(*func)(args...) = nullptr,
                            std::tuple<Targs...> targs = std::tuple<>()){

        auto splitKey = parseKey(key, __func__);
        if(!splitKey.alias.empty()){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " positional argument cannot have aliases");
        }

        static_assert((sizeof...(args) - sizeof...(Targs)) <= 1, " too many arguments in function");

        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        ///check if default parser for this type is present
        if(func == nullptr){
            try{
                scan(GET_TYPE(T), nullptr);
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }

        auto x = new DerivedOption<T,Targs...>(func, targs); //std::make_tuple(targs...)

        auto option = new ARG_DEFS();
        option->typeStr = strType;
        option->m_options = {};
        option->option = x;
        option->m_positional = true;
        if(GET_TYPE(T) == GET_TYPE(date_t)){
            option->m_date_format = DEFAULT_DATE_FORMAT;
        }

        argMap[splitKey.key] = option;
        posMap.emplace_back(splitKey.key);

        return *option;
    }

    /**
     *
     * @tparam T function return type
     * @tparam Targs function side arguments (any)
     * @tparam args function arguments (const char*)
     * @param key argument key ("-f" adds arbitrary argument, "f" adds mandatory argument)
     * @param opts list of options ({"foo"} - mandatory, {"[foo]"} = arbitrary). {} treated as implicit argument
     * @param func function pointer or nullptr
     * @return
     */
    template <typename T, class...Targs, typename...args>
    ARG_DEFS &addArgument(const std::string &key,
                          const std::vector<std::string>& opts = {},
                          T(*func)(args...) = nullptr,
                          std::tuple<Targs...> targs = std::tuple<>()){ //Targs&&...targs

        auto splitKey = parseKey(key, __func__);
        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        ///Check for invalid sequence order of arguments
        std::string last_arbitrary_arg;
        std::string last_mandatory_arg;
        int mnd_vals = 0;

        for(auto & opt : opts){
            const std::string& sopt = opt;
            if(sopt.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " option name cannot be empty");
            }
            if(sopt.at(0) == ' ' || sopt.at(sopt.length()-1) == ' '){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " option " + sopt + " cannot begin or end with space");
            }

            if(isOptMandatory(sopt)){
                mnd_vals++;
                last_mandatory_arg = sopt;
                if(!last_arbitrary_arg.empty()){
                    throw std::invalid_argument(std::string(__func__) + ": " + key
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
        bool starts_with_minus = flag;
        if(!splitKey.alias.empty()){
            bool match = (splitKey.alias[0] == '-');
            if (match != flag){
                throw std::invalid_argument(std::string(__func__) + ": " + key + ": cannot add alias " + splitKey.alias + ": different type");
            }
        }

        if(last_mandatory_arg.empty() && !flag){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " should have at least 1 mandatory parameter");
        }

        bool implicit = opts.empty();

        if(func == nullptr){
            if(implicit && !std::is_arithmetic<T>::value){ //typeid(T) != typeid(bool)
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no function provided for non-arithmetic arg with implicit option");
            }
            if(opts.size() > 1){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no function provided for arg with " + std::to_string(opts.size()) + " options");
            }
            if(!implicit && last_mandatory_arg.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no function provided for arg with arbitrary options");
            }

            ///check if default parser for this type is present
            try{
                scan(GET_TYPE(T), nullptr);
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }
        else if((sizeof...(args) - sizeof...(Targs)) != opts.size()){
            throw std::invalid_argument(std::string(__func__) + " " + key + " opts size != function arguments");
        }

        auto x = new DerivedOption<T,Targs...>(func, targs); //std::make_tuple(targs...)

        auto option = new ARG_DEFS();
        option->typeStr = strType;
        option->option = x;
        option->m_options = opts;
        option->m_arbitrary = flag;
        option->m_implicit = implicit;
        option->m_starts_with_minus = starts_with_minus;
        option->mandatory_options = mnd_vals;
        if(GET_TYPE(T) == GET_TYPE(date_t)
        && opts.size() == 1){
            option->m_date_format = DEFAULT_DATE_FORMAT;
        }

        argMap[splitKey.key] = option;

        if(!splitKey.alias.empty()){
            //add alias
            setAlias(splitKey.key, splitKey.alias);
        }

        return *option;
    }

    template <typename T>
    T getValue(const std::string &key){
        parsedCheck(__func__);
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__);
        auto &r = getArg(key);
        try{
            auto base_opt = r.option;
            return std::any_cast<T>(base_opt->anyval);
        }catch(const std::bad_any_cast& e){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot cast to " + strType);
        }
    }

    /**
     *
     * @tparam T conversion type
     * @param value - string needed to be converted
     * @return
     */
    template <typename T>
    static T scanValue(const char *value, const char *date_format = DEFAULT_DATE_FORMAT){
        if(value == nullptr){
            return T{};
        }
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__);
        try{
            std::any val = scan(GET_TYPE(T), value, date_format);
            return std::any_cast<T>(val);
        }catch(const std::bad_any_cast& e){
            throw std::invalid_argument(std::string(__func__) + ": cannot cast to " + strType);
        }
    }

    ///Set alias for option
    void setAlias(const std::string &key, const std::string &alias){
        if(argMap.find(key) == argMap.end()){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " not defined");
        }
        if(!argMap[key]->m_alias.empty()){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " alias already defined: " + argMap[key]->m_alias);
        }
        if(argMap[key]->m_positional){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " positional argument cannot have alias");
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
        argVec = {argv + 1, argv + argc};
        //should be enough for any reasonable number of arguments
        argVec.reserve(100);
        ///Retrieve binary self-name
        std::string self_name = std::string(argv[0]);
        binary_name = self_name.substr(self_name.find_last_of('/') + 1);

        int parsed_mnd_args = 0;
        int parsed_required_args = 0;

        //count mandatory/required arguments
        for(auto &x : argMap){
            if(!x.second->m_arbitrary
               && !x.second->m_positional){
                mandatory_args++;
            }else if(x.second->m_arbitrary
                     && x.second->m_required){
                required_args++;
            }
        }

        mandatory_option = mandatory_args || required_args;
        mandatory_option &= !allow_zero_options;

        auto parseArgument = [this](const std::string &key, int start, int end){
            if(argMap[key]->option->has_action){
                argMap[key]->option->action(&argVec[start], end - start);
            }else{
                auto val = argMap[key]->option->anyval;
                argMap[key]->option->set(scan(val.type(), argVec[start].c_str(), argMap[key]->m_date_format));
            }
        };

        auto setArgument = [this, &parsed_mnd_args, &parsed_required_args](const std::string &pName){
            argMap[pName]->m_set = true;
            //count mandatory/required options
            if(!argMap[pName]->m_arbitrary){
                parsed_mnd_args++;
            }else if(argMap[pName]->m_required){
                parsed_required_args++;
            }
        };

        auto strMismatch = [](const std::string &compareWhat, const std::string &compareWith) -> size_t{
            auto tmpWhat = compareWhat;
            size_t result = 0;
            int idx = 0;
            if(compareWith.length() < compareWhat.length()){
                return std::string::npos;
            }
            while(tmpWhat.length() < compareWith.length()){
                tmpWhat += ' ';
            }
            for(auto &x : compareWith){
                auto remStr = compareWith.substr(idx+1);
                char c = tmpWhat[idx++];
                if(x != c){
                    result++;
                    if(remStr.find(c) != std::string::npos){
                        result--;
                    }
                }
            }
            return result;
        };

        auto findKeyByAlias = [this](const std::string &key) -> std::string{
            for(auto &x : argMap){
                if(x.second->m_alias == key){
                    return x.first;
                }
            }
            return "";
        };

        auto checkParsedNonPos = [this, &parsed_mnd_args, &parsed_required_args](){
            if(mandatory_option){
                if(parsed_mnd_args != mandatory_args){
                    for(auto &x : argMap){
                        if(!x.second->m_arbitrary && !x.second->m_positional && !x.second->m_set){
                            throw std::runtime_error(x.first + " not specified");
                        }
                    }
                }
                if(required_args > 0 && parsed_required_args < 1){
                    throw std::runtime_error("Missing required option " + std::string(REQUIRED_OPTION_SIGN));
                }
            }
        };

        for(auto index = 0; index < argVec.size(); index++){

            auto insertKeyValue = [this, &index](const std::string &key, const std::string &val){
                argVec[index] = key;
                argVec.insert(argVec.begin() + index + 1, val);
                //move pointer
                index--;
            };

            std::string pName = argVec[index];
            std::string pValue = index+1 >= argVec.size() ? "" : argVec[index + 1];
            std::string newKey;

            ///Handle '='
            auto c = pName.find(KEY_OPTION_DELIMITER);
            if(c != std::string::npos){
                pValue = pName.substr(c+1);
                pName = pName.substr(0, c);
                //change current key and insert value to vector
                insertKeyValue(pName, pValue);
                continue;
            }

            if(argMap.find(pName) == argMap.end()){
                ///Find alias
                newKey = findKeyByAlias(pName);
                if(!newKey.empty()){
                    pName = newKey;
                }else{
                    ///check contiguous or combined arguments
                    bool contiguous = false;
                    for(auto &x : argMap){
                        //contiguous format isn't allowed for options that doesn't start with '-'
                        if(!x.second->m_starts_with_minus){
                            continue;
                        }
                        auto key = x.first;
                        auto alias = x.second->m_alias;
                        std::string contKey = pName.substr(0, key.length());
                        std::string contAlias = pName.substr(0, alias.length());
                        std::string first2dig = pName.substr(0,2); //first 2 digits of current key
                        // if key or alias is 2 digits long arg with implicit value,
                        // starts with '-' and match first 2 digits,
                        // it's a combined (-vvv style) argument
                        if(first2dig.length() == 2
                        && x.second->m_implicit
                        && (first2dig == key || first2dig == alias)){
                            //set '-' to other portion to extract it later
                            pValue = "-" + pName.substr(first2dig.length());
                            pName = first2dig;
                            insertKeyValue(pName, pValue);
                            contiguous = true;
                            break;
                        }
                        //check if it's a contiguous keyValue pair
                        if(key == contKey){
                            pValue = pName.substr(contKey.length());
                            pName = contKey;
                            insertKeyValue(pName, pValue);
                            contiguous = true;
                            break;
                        }
                        //check if it's a contiguous aliasValue pair
                        if(!contAlias.empty()
                        && alias == contAlias){
                            pValue = pName.substr(contAlias.length());
                            pName = contAlias;
                            insertKeyValue(pName, pValue);
                            contiguous = true;
                            break;
                        }
                    }
                    if(contiguous){
                        continue;
                    }
                }
            }


            if(argMap.find(pName) == argMap.end()){
                ///Try parsing positional args
                int pos_idx = index;
                if(!posMap.empty()){
                    ///check if non-positional options were parsed
                    checkParsedNonPos();
                    for(auto &x : posMap){
                        if(pos_idx >= argVec.size()){
                            break;
                        }
                        parseArgument(x, index, index+1);
                        positional_cnt++;
                        pos_idx++;
                    }
                    break;
                }

                std::string thrError = "Unknown argument: " + std::string(pName);
                ///find closest key
                auto proposed_value = argMap.begin()->first;
                auto mismatch = strMismatch(pName, proposed_value);
                for(auto &x : argMap){
                    auto tmp = strMismatch(pName, x.first);
                    if(tmp < mismatch){
                        mismatch = tmp;
                        proposed_value = x.first;
                    }
                }

                dummyFunc(nullptr);
                if(mismatch < 2){
                    thrError += ". Did you mean " + proposed_value + "?";
                }
                throw std::runtime_error(thrError);
            }

            if(argMap[pName]->typeStr == ARG_TYPE_HELP){
                helpDefault(argv[0], pValue);
                exit(0);
            }
            else
            {
                ///Parse other types

                ///If non-repeatable and occurred again, throw error
                if(argMap[pName]->m_set
                   && !argMap[pName]->m_repeatable){
                    throw std::runtime_error("Error: redefinition of non-repeatable arg " + std::string(pName));
                }

                ///Parse arg with implicit option
                if(argMap[pName]->m_implicit){
                    if(argMap[pName]->option->has_action){
                        argMap[pName]->option->action(nullptr, 0);
                    }
                    else{
                        argMap[pName]->option->increment();
                    }
                    setArgument(pName);
                    continue;
                }

                int opts_cnt = 0;
                auto cnt = index + 1;
                bool arbitrary_values = false;

                for(int j=0; j<argMap[pName]->m_options.size(); j++){
                    if(cnt >= argVec.size()){
                        break;
                    }

                    //check if next value is also a key
                    bool next_is_key = argMap.find(argVec[cnt]) != argMap.end();
                    //find alias
                    for(auto &x : argMap){
                        next_is_key |= x.second->m_alias == argVec[cnt];
                    }

                    if(next_is_key
                       && argMap[pName]->mandatory_options != argMap[pName]->m_options.size()){
                        arbitrary_values = true;
                        break;
                    }
                    cnt++;
                    opts_cnt++;
                }

                if(opts_cnt < argMap[pName]->mandatory_options){
                    throw std::runtime_error(std::string(pName) + " requires "
                                             + std::to_string(argMap[pName]->mandatory_options) + " options, but " + std::to_string(opts_cnt) + " were provided");
                }

                if(arbitrary_values){
                    ///parse arg with partial arbitrary values list
                    if(argMap[pName]->option->has_action){
                        argMap[pName]->option->action(&argVec[index+1], opts_cnt);
                    }else{
                        throw std::runtime_error(std::string(pName) + " arbitrary value conflicts with arg " + argVec[cnt]);
                    }
                }else{
                    parseArgument(pName, index + 1, index + 1 + opts_cnt);
                }

                index += opts_cnt;
                setArgument(pName);
            }
        }
        args_parsed = true;
        checkParsedNonPos();

        if(positional_cnt < posMap.size()){
            throw std::runtime_error("Not enough positional arguments provided");
        }
        return 0;
    }

    const ARG_DEFS &operator [] (const std::string &key) const { return getArg(key); }

private:

    struct KEY_ALIAS{
        std::string key;
        std::string alias;
    };

    std::map<std::string, ARG_DEFS*> argMap;
    std::vector<std::string>posMap;
    std::vector<std::string> argVec;

    std::string binary_name;
    bool args_parsed = false;
    bool mandatory_option = false;
    int positional_cnt = 0;
    int mandatory_args = 0;
    int required_args = 0;

    [[nodiscard]] ARG_DEFS &getArg(const std::string &key) const {
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

        throw std::invalid_argument(key + " not defined");
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
    /// format is applicable only to date_t type
    static std::any scan(std::type_index type, const char *arg, const char *date_format = nullptr) {

        std::string temp = (arg == nullptr) ? "" : std::string(arg);

        if(type == GET_TYPE(const char *)){
            return arg;
        }else if(type == GET_TYPE(std::string)){
            return temp;
        }else if(type == GET_TYPE(bool)){
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
            return isTrue(lVal.c_str());
        }else if(type == GET_TYPE(date_t)){
            date_t tt = {0};
            if(date_format == nullptr){
                date_format = DEFAULT_DATE_FORMAT;
            }
            std::stringstream ss(temp);
            ss >> std::get_time(&tt, date_format);
            if (ss.fail()){
                throw std::runtime_error(std::string(__func__) + ": unable to convert " + temp + " to date");
            }
            return tt;
        }

        ///numbers
        char *tmp = (char*)temp.c_str();
        std::any res;
        if(type == GET_TYPE(int)){
            res = (int)strtol(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(char)){
            res = (char)strtol(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(short int)){
            res = (short int)strtol(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(long)){
            res = strtol(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(long long)){
            res = strtoll(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(unsigned int)){
            res = (unsigned int)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(unsigned long)){
            res = strtoul(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(unsigned char)){
            res = (unsigned char)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(short unsigned int)){
            res = (short unsigned int)strtoul(temp.c_str(), &tmp, 0);
        }else if(type == GET_TYPE(float)){
            res = strtof(temp.c_str(), &tmp);
        }else if(type == GET_TYPE(double)){
            res = strtod(temp.c_str(), &tmp);
        }else{
            throw std::logic_error(std::string(__func__) + ": value of unknown type " + temp);
        }

        if (errno == ERANGE) {
            throw std::range_error{std::string(__func__) + ": " + temp + " not representable"};
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
            throw std::invalid_argument(std::string(func) + ": Key cannot be empty!");
        }

        //Check previous definition
        if(argMap.find(key) != argMap.end()){
            throw std::invalid_argument(std::string(func) + ": " + std::string(key) + " already defined");
        }

        for(auto &x : argMap){
            if(x.second->m_alias == key){
                throw std::invalid_argument(std::string(func) + ": " + std::string(key) + " already defined");
            }
        }
    }

    static void checkForEqualsSymbol(const std::string &key, const char* func = nullptr){
        if(func == nullptr){
            func = __func__;
        }
        if(!key.empty()){
            auto c = key.find(KEY_OPTION_DELIMITER);
            if(c == 0 || c == key.length()-1){
                throw std::invalid_argument(std::string(func) + ": " + key + " cannot begin or end with =");
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

        if(skey.empty()){
            throw std::invalid_argument(std::string(func) + ": key cannot be empty");
        }

        sanityCheck(skey, func);

        checkForEqualsSymbol(skey, func);
        checkForEqualsSymbol(alias, func);

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

    void helpDefault(const char* name, const std::string &param = ""){

        bool advanced = false;

        auto printParam = [](auto j, const std::string& alias, bool notab = false){
            std::string alias_str = notab ? "" : "\t";
            alias_str += (alias.empty() ? "" : (alias + KEY_ALIAS_DELIMITER + " "));
            std::cout <<  alias_str + j.first;
            for(auto &x : j.second->m_options){
                std::string opt = std::string(x);
                if(isOptMandatory(opt))
                    opt = "<" + opt + ">";

                std::cout << " " + opt;
            }
        };

        auto sorted_usage = [this, &advanced, &printParam](bool flag, bool hidden){

            auto print_usage = [&advanced, &printParam, this](auto j, const std::string& alias = ""){
                //skip already printed
                if(j.second->m_set){
                    return;
                }
                //skip hidden
                if(j.second->m_hidden && !advanced){
                    return;
                }
                //skip positional
                if(j.second->m_positional){
                    return;
                }

                printParam(j, alias);

                std::string def_val = (j.second->option == nullptr) ? "" : j.second->option->get_str_val();
                def_val = j.second->show_default ? (def_val.empty() ? "" : " (default " + def_val + ")") : "";
                std::string repeatable = j.second->m_repeatable ? " [repeatable]" : "";
                // show date format if not prohibited
                std::string date_format = j.second->m_hide_date_format
                        ? ""
                        : (j.second->m_date_format == nullptr
                            ? ""
                            : (" {" + j.second->m_options[0] + " format: " + std::string(j.second->m_date_format) + "}"));
                std::string required = j.second->m_required ? (required_args > 1 ? " " + std::string(REQUIRED_OPTION_SIGN) : "") : "";
                std::cout << " : " + j.second->m_help + repeatable + date_format + def_val + required << std::endl;
                j.second->m_set = true; //help_set
            };

            for (auto & j : argMap)
            {
                if((j.second->m_arbitrary && !j.second->m_required) == flag
                   && j.second->m_hidden == hidden)
                {
                    //find alias
                    std::string alias = j.second->m_alias;
                    print_usage(j, alias);
                }

            }
        };


        if(param == HELP_HIDDEN_OPT){
            advanced = true;
        }
        else if(!param.empty()){
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
                printParam(*j, j->second->m_alias, true);
                std::cout << ":" << std::endl;
                //std::cout << "Type: " + j->second->typeStr << std::endl;
                std::cout << j->second->m_help << std::endl;
                std::cout << j->second->m_advanced_help << std::endl;
            }else{
                std::cout << "Unknown parameter " + param << std::endl;
            }
            return;
        }

        std::string positional;
        for (auto &x : posMap){
            positional += " <" + x + ">";
        }

        int flag_cnt = 0, opt_cnt = 0;
        for(auto &x : argMap){
            if(x.second->m_arbitrary
               && !x.second->m_required){
                flag_cnt++;
            }else if(!x.second->m_positional){
                opt_cnt++;
            }
        }

        std::cout << "Usage: " + std::string(name)
                     + (flag_cnt ? " [flags...]" : "")
                     + (opt_cnt ? " options..." : "")
                     + positional << std::endl;

        if(!posMap.empty()){
            std::cout << "Positional arguments:" << std::endl;
            for(auto &x : posMap){
                std::string date_format = argMap[x]->m_date_format == nullptr ? "" : ("[" + std::string(argMap[x]->m_date_format) + "] ");
                std::cout << "\t" + x + " : " + date_format + argMap[x]->m_help << std::endl;
            }
        }

        if(flag_cnt){
            std::cout << "Flags (arbitrary):" << std::endl;
            sorted_usage(true, false);
            if(advanced){
                //show hidden
                sorted_usage(true, true);
            }
        }
        if(opt_cnt){
            std::cout << "Options (mandatory):" << std::endl;
            sorted_usage(false, false);
            if(advanced){
                //show hidden
                sorted_usage(false, true);
            }

        }

        if(required_args > 1){
            std::cout << "For options marked with " + std::string(REQUIRED_OPTION_SIGN) + ": at least one such option should be provided" << std::endl;
        }
    }
};


#endif //VALLAB_PRINTER_ARGPARSER_H
