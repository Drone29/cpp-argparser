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
#include <sstream>
#include <algorithm>

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
//get type index
#define GET_TYPE(x) std::type_index(typeid(std::remove_cv_t<x>))

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

// visible only for current file
namespace{
    /// format is applicable only to date_t type
    std::any scan(std::type_index type, const char *arg, const char *date_format = nullptr) {

        std::string temp = (arg == nullptr) ? "" : std::string(arg);

        if(type == GET_TYPE(char*) || type == GET_TYPE(const char*)){
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
            // todo: not all conversion errors handled
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
}

class BaseOption{
protected:
    friend class argParser;
    friend struct ARG_DEFS;
    BaseOption() = default;
    virtual ~BaseOption() = default;
    virtual std::any action () = 0; // for implicit args
    virtual std::any action (const std::string *args, int size) = 0; // for common args
    virtual std::any action (const std::string *args, int size, const char *date_format) = 0; // for infinite args
    virtual void set(std::any x) = 0;
    virtual std::string get_str_val() = 0;
    virtual void set_global_ptr(std::any ptr) = 0;
    virtual std::type_index get_type() = 0;
    std::any anyval;
    bool has_action = false;
    bool infinite_options = false;
};

template <typename T, class...Targs>
class DerivedOption : public BaseOption{
private:
    friend class argParser;
    std::any increment() {
        if constexpr (std::is_arithmetic<T>::value){
            value+=1;
        }
        set_global();
        anyval = value;
        return anyval;
    }
    // parse infinite params and single scan. returns vector of Ts or single value
    std::any action(const std::string *args, int size, const char *date_format) override{
        std::vector<T> res;
        const std::string *ptr = args;
        if(!has_action && !infinite_options){
            set(scan(GET_TYPE(T), (*ptr).c_str(), date_format));
            return anyval;
        }
        for(int i=0; i<size;i++){
            // create tuple from side args + current const char* arg
            auto cchar_tpl = std::make_tuple((*ptr).c_str());
            auto tplres = std::tuple_cat(tpl, cchar_tpl);
            //scan or apply action
            T val = has_action
                    ? std::apply(t_action, tplres)
                    : std::any_cast<T>(scan(GET_TYPE(T), (*ptr).c_str(), date_format));
            res.push_back(val);
            ptr++;
        }
        anyval = res;
        return anyval;
    }
    // for implicit args only
    std::any action() override{
        if(!has_action){
            return increment();
        }
        return action(nullptr, 0);
    }
    std::any action(const std::string *args, int size) override{

        const char *argvCpy[MAX_ARGS+1] = {nullptr};
        const std::string *ptr = args;
        for(int i=0; i<size;i++){
            argvCpy[i] = (*ptr++).c_str();
        }
        // create resulting tuple
        auto cchar_tpl = std::make_tuple(UNPACK_ARGS(argvCpy));
        auto tplres = std::tuple_cat(tpl, cchar_tpl);

        //t_action != nullptr
        if(has_action){
            value = std::apply(t_action, tplres);
        }
        set_global();
        anyval = value;
        return anyval;
    }

    std::string get_str_val() override{
        std::string res;
        try{
            if constexpr (std::is_arithmetic_v<T>){
                res = std::to_string(value);
            }
            else if constexpr (std::is_convertible_v<T, std::string>){
                if constexpr(std::is_pointer_v<T>){
                    res = (value == NULL || value == nullptr) ? "" : res = std::string(value);
                }else{
                    res = std::string(value);
                }
            }
        }catch(...){
            res = ""; //if null or other exception, set empty
        }

        return res;
    }

    std::type_index get_type() override{
        return GET_TYPE(decltype(value));
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
        if(infinite_options) return;
        try{
            global = std::any_cast<T*>(ptr);
        }catch(std::bad_any_cast &e){
            throw std::invalid_argument("invalid pointer type");
        }
    }

    void set_global(){
        if(global != nullptr && !infinite_options){
            //set global
            *global = value;
        }
    }

    template <typename...args>
    DerivedOption(T(*func)(Targs..., args...), std::tuple<Targs...> targs, bool infinite_opts = false) { //Targs...targs

        static_assert(sizeof...(args) < MAX_ARGS, " too many arguments");
        //check if args are const char*
        static_assert(are_same_v<const char*, args...>, "Error: only const char* allowed");
        t_action = reinterpret_cast<T (*)(Targs...,const char *...)>(func);
        has_action = func != nullptr;
        anyval = value;

        tpl = targs;
        infinite_options = infinite_opts;
    }
    ~DerivedOption() override = default;

    T(*t_action)(Targs...,const char*...) = nullptr;
    T value {};
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
    ARG_DEFS &default_value(std::any val, bool hide_in_help = false){
        try{
            if(m_arbitrary && !m_positional){
                option->set(std::move(val));
                show_default = !hide_in_help;
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
           && (m_positional || m_options.size() == 1)){

            if(format != nullptr){
                //check if contains spaces
                for(auto &c : std::string(format)){
                    if(c == ' '){
                        throw std::logic_error(std::string(__func__) + ": invalid date format " + format + " (cannot contain spaces)");
                    }
                }
                time_t t = 0;
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
    [[nodiscard]] bool has_infinite_options() const{
        return option == nullptr ? false : option->infinite_options;
    };
    // get raw string parameters passed from cli
    [[nodiscard]] std::vector<std::string> get_cli_params() const{
        return m_cli_params;
    }

private:
    std::string m_help;
    std::string m_advanced_help;
    //hidden option
    bool m_hidden = false;
    //list of options
    std::vector<std::string> m_options;
    //raw cli parameters
    std::vector<std::string> m_cli_params;
    //stringified type
    std::string typeStr;
    //Option/flag
    BaseOption* option = nullptr;
    //in use
    bool m_set = false;
    //alias
    std::vector<std::string> m_aliases;
    //Positional
    bool m_positional = false;
    //Flag
    bool m_arbitrary = false;
    //Required != !arbitrary. if !arbitrary, user should specify all of them. If required, at least one is enough
    bool m_required = false;
    //Implicit
    bool m_implicit = false;
    //repeatable
    bool m_repeatable = false;
    //If starts with minus
    bool m_starts_with_minus = false;
    //Date format (only for date_t type)
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
        setAlias(HELP_NAME, {HELP_ALIAS});
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

        argMap[key] = option;
        posMap.emplace_back(key);

        return *option;
    }

    template <typename T, class...Targs, typename...args>
    ARG_DEFS &addArgument(const std::vector<std::string> &names,
                          const std::initializer_list<std::string>& opts_lst = {},
                          T(*func)(args...) = nullptr,
                          std::tuple<Targs...> targs = std::tuple<>()){

        if(names.empty()){
            throw std::invalid_argument(std::string(__func__) + ": argument must have a name");
        }
        for(auto &el : names){
            checkForbiddenSymbols(el, __func__);
        }

        KEY_ALIAS splitKey;
        splitKey.aliases = {names};
        splitKey.key = splitKey.aliases[0];
        // find longest entry in vector, it'll be key
        auto idx = splitKey.aliases.begin();
        for(auto it = splitKey.aliases.begin(); it != splitKey.aliases.end(); it++){
            if((*it).length() > splitKey.key.length()){
                splitKey.key = *it;
                idx = it;
            }
        }
        // remove key from aliases vector
        splitKey.aliases.erase(idx);

        /// get template type string
        auto strType = getFuncTemplateType(__PRETTY_FUNCTION__, "T");

        std::vector<std::string> opts = opts_lst;
        ///Check for invalid sequence order of arguments
        std::string last_arbitrary_arg;
        std::string last_mandatory_arg;
        int mnd_vals = 0;
        bool infinite_opts = false;

        for(auto & sopt : opts){
            if(sopt.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " option name cannot be empty");
            }
            if(sopt.at(0) == ' ' || sopt.at(sopt.length()-1) == ' '){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " option " + sopt + " cannot begin or end with space");
            }
            // infinite options
            if(sopt == "..."){
                infinite_opts = true;
                if(sopt != opts.back()){
                    throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " '...' should be the last in the list");
                }
                if(sopt == opts.front()){
                    throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " infinite list must have a name");
                }
                if(opts.size() > 2){
                    throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " infinite list must have exactly 2 arguments");
                }
                if(!isOptMandatory(opts.front())){
                    throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " infinite list name cannot be arbitrary");
                }
                // remove last ...
                opts.pop_back();
                break;
            }

            if(isOptMandatory(sopt)){
                mnd_vals++;
                last_mandatory_arg = sopt;
                if(!last_arbitrary_arg.empty()){
                    throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key
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

        bool flag = splitKey.key[0] == '-';
        bool starts_with_minus = flag;
        for(auto &el : splitKey.aliases){
            bool match = el[0] == '-';
            if (match != flag){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + ": cannot add alias " + el + ": different type");
            }
        }

        if(last_mandatory_arg.empty() && !flag){
            throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " should have at least 1 mandatory parameter");
        }

        bool implicit = opts.size() == 0;

        if(func == nullptr){
            if(implicit && !std::is_arithmetic<T>::value){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no function provided for non-arithmetic arg with implicit option");
            }
            if(opts.size() > 1){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no function provided for arg with " + std::to_string(opts.size()) + " options");
            }
            if(!implicit && last_mandatory_arg.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no function provided for arg with arbitrary options");
            }

            ///check if default parser for this type is present
            try{
                scan(GET_TYPE(T), nullptr);
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }
        else if((sizeof...(args) - sizeof...(Targs)) != opts.size()){
            throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " opts size != function arguments");
        }

        auto x = new DerivedOption<T,Targs...>(func, targs, infinite_opts);
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

        // add aliases
        for(auto &alias : splitKey.aliases){
            setAlias(splitKey.key, alias);
        }

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
    ARG_DEFS &addArgument(const char *key,
                          const std::initializer_list<std::string>& opts = {},
                          T(*func)(args...) = nullptr,
                          std::tuple<Targs...> targs = std::tuple<>()){

        std::string keys = key == nullptr ? "" : std::string(key);
        std::vector<std::string> vec;
        std::string::iterator s;
        size_t c;

        auto checkChars = [](int c) -> bool{
            return std::isalnum(c) || c == '-' || c == '_';
        };

        // split string
        while((s = std::find_if_not(keys.begin(), keys.end(), checkChars)) != keys.end()){
            c = s - keys.begin();
            auto t = keys.substr(0, c);
            if(!t.empty()) {
                vec.push_back(t);
            }
            keys = keys.substr(c+1);
        }

        // add last portion of the string if not empty
        if(!keys.empty()){
            vec.push_back(keys);
        }

        return addArgument(vec, opts, func, targs);
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

    /// Get last unparsed argument
    [[nodiscard]] const ARG_DEFS *getLastUnparsed() const{
        return last_unparsed_arg;
    }

    ///Set alias for option
    void setAlias(const std::string &key, const std::string &alias){
        if(argMap.find(key) == argMap.end()){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " not defined");
        }
        if(argMap[key]->m_positional){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " positional argument cannot have alias");
        }

        sanityCheck(alias, __func__);

        auto sortingFunc = [](const std::string& first, const std::string& second){
            return first.size() < second.size();
        };

        argMap[key]->m_aliases.push_back(alias);
        //sort vector by length after inserting new alias
        std::sort(argMap[key]->m_aliases.begin(), argMap[key]->m_aliases.end(), sortingFunc);
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
            try{
                // save raw cli parameters
                argMap[key]->m_cli_params = {&argVec[start], &argVec[end]};
                if(argMap[key]->option->has_action && !argMap[key]->option->infinite_options){
                    argMap[key]->option->action(&argVec[start], end - start);
                }else{
                    argMap[key]->option->action(&argVec[start], end - start, argMap[key]->m_date_format);
                }
            }catch(std::exception &e){
                //save last unparsed arg
                last_unparsed_arg = argMap[key];
                throw unparsed_param(e.what());
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
                for(auto &el : x.second->m_aliases){
                    if(el == key){
                        return x.first;
                    }
                }
            }
            return "";
        };

        auto checkParsedNonPos = [this, &parsed_mnd_args, &parsed_required_args](){
            if(mandatory_option){
                if(parsed_mnd_args != mandatory_args){
                    for(auto &x : argMap){
                        if(!x.second->m_arbitrary && !x.second->m_positional && !x.second->m_set){
                            throw parse_error(x.first + " not specified");
                        }
                    }
                }
                if(required_args > 0 && parsed_required_args < 1){
                    throw parse_error("Missing required option " + std::string(REQUIRED_OPTION_SIGN));
                }
            }
        };

        /// Handle '=' and combined args
        for(auto index = 0; index < argVec.size(); index++){

            auto insertKeyValue = [this, &index](const std::string &key, const std::string &val){
                argVec[index] = key;
                argVec.insert(argVec.begin() + index + 1, val);
                index--;
            };
            std::string pName = argVec[index];
            std::string pValue = index+1 >= argVec.size() ? "" : argVec[index + 1];

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
                std::string name = findKeyByAlias(pName);
                if(!name.empty()){
                    // change alias to key
                    argVec[index] = name;
                }
                else{
                    ///check contiguous or combined arguments
                    bool contiguous = false;
                    for(auto &x : argMap){
                        //contiguous format isn't allowed for options that doesn't start with '-'
                        if(!x.second->m_starts_with_minus){
                            continue;
                        }
                        auto key = x.first;
                        std::string contKey = pName.substr(0, key.length());
                        for(auto &alias : x.second->m_aliases){

                            std::string contAlias = pName.substr(0, alias.length());
                            std::string first2dig = pName.substr(0,2); //first 2 digits of current key
                            // if key or alias is 2 digits long arg with implicit value,
                            // starts with '-' and matches first 2 digits,
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
                            //check if it's a contiguous keyValue pair (-k123 style)
                            if(key == contKey){
                                pValue = pName.substr(contKey.length());
                                pName = contKey;
                                insertKeyValue(pName, pValue);
                                contiguous = true;
                                break;
                            }
                            //check if it's a contiguous aliasValue pair (-a123 style)
                            if(!contAlias.empty()
                               && alias == contAlias){
                                pValue = pName.substr(contAlias.length());
                                pName = contAlias;
                                insertKeyValue(pName, pValue);
                                contiguous = true;
                                break;
                            }
                        }
                        if(contiguous)
                            break;
                    }
                }
            }
        }


        /// Main parser loop
        for(auto index = 0; index < argVec.size(); index++){

            std::string pName = argVec[index];
            std::string pValue = index+1 >= argVec.size() ? "" : argVec[index + 1];

            ///Try parsing positional args
            if(argMap.find(pName) == argMap.end()){
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
                throw parse_error(thrError);
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
                    throw parse_error("Error: redefinition of non-repeatable arg " + std::string(pName));
                }

                ///Parse arg with implicit option
                if(argMap[pName]->m_implicit){
                    argMap[pName]->option->action();
                    setArgument(pName);
                    continue;
                }

                int opts_cnt = 0;
                auto cnt = index;
                bool infinite_opts = argMap[pName]->has_infinite_options();

                while(++cnt < argVec.size()){
                    // if all options found, break
                    if(!infinite_opts && opts_cnt >= argMap[pName]->m_options.size()){
                        break;
                    }
                    // leave space for positionals
                    if((infinite_opts || opts_cnt == argMap[pName]->mandatory_options)
                       && argVec.size() - cnt <= posMap.size()){
                        break;
                    }
                    //check if next value is also a key
                    bool next_is_key = argMap.find(argVec[cnt]) != argMap.end();
                    if(next_is_key){
                        break;
                    }
                    ++opts_cnt;
                }

                if(opts_cnt < argMap[pName]->mandatory_options){
                    throw parse_error(std::string(pName) + " requires "
                                             + std::to_string(argMap[pName]->mandatory_options) + " options, but " + std::to_string(opts_cnt) + " were provided");
                }

                parseArgument(pName, index + 1, index + 1 + opts_cnt);

                index += opts_cnt;
                setArgument(pName);
            }
        }
        args_parsed = true;
        checkParsedNonPos();

        if(positional_cnt < posMap.size()){
            throw parse_error("Not enough positional arguments provided");
        }

        return 0;
    }

    const ARG_DEFS &operator [] (const std::string &key) const { return getArg(key); }

    /// Custom exception class (unparsed parameters)
    class unparsed_param : public std::runtime_error{
    public:
        explicit unparsed_param(const char *msg) : std::runtime_error(msg) {}
        explicit unparsed_param(const std::string& s) : std::runtime_error(s){}
    };
    /// Custom exception class (other parse errors)
    class parse_error : public std::runtime_error{
    public:
        explicit parse_error(const char *msg) : std::runtime_error(msg) {}
        explicit parse_error(const std::string& s) : std::runtime_error(s){}
    };

private:

    struct KEY_ALIAS{
        std::string key;
        std::vector<std::string> aliases;
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
    ARG_DEFS *last_unparsed_arg = nullptr;

    [[nodiscard]] ARG_DEFS &getArg(const std::string &key) const {
        std::string skey = key;
        if(argMap.find(skey) == argMap.end()){
            for(auto &x : argMap){
                for(auto &alias : x.second->m_aliases){
                    if(alias == skey){
                        skey = x.first;
                        break;
                    }
                }
                if(skey != key){
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
            for(auto &alias : x.second->m_aliases){
                if(alias == key){
                    throw std::invalid_argument(std::string(func) + ": " + std::string(key) + " already defined");
                }
            }
        }
    }

    static void checkForbiddenSymbols(const std::string &key, const char* func = nullptr){
        if(func == nullptr){
            func = __func__;
        }
        if(key.empty()){
            throw std::invalid_argument(std::string(func) + ": key cannot be empty");
        }
        auto c = key.find(KEY_OPTION_DELIMITER);
        auto s = key.find(' ');
        if(c == 0 || c == key.length()-1){
            throw std::invalid_argument(std::string(func) + ": " + key + " cannot begin or end with =");
        }
        if(s != std::string::npos){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot contain spaces");
        }
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

        auto printParam = [](auto j, bool notab = false){
            std::string alias_str = notab ? "" : "\t";
            for(auto &alias : j.second->m_aliases){
                alias_str += alias + KEY_ALIAS_DELIMITER + " ";
            }
            std::cout <<  alias_str + j.first;
            std::string opt;
            for(auto &x : j.second->m_options){
                opt = std::string(x);
                std::string tmp = opt;
                if(isOptMandatory(tmp))
                    tmp = "<" + tmp + ">";

                std::cout << " " + tmp;
            }

            if(j.second->has_infinite_options()){
                std::cout << " [" + opt + "...]";
            }
        };
        //check required: -1-don't check, 0-false, other-true
        auto sorted_usage = [this, &advanced, &printParam](bool flag, bool hidden, int check_required = -1){

            auto print_usage = [&advanced, &printParam, this](auto j){
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

                printParam(j);

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

            for (auto & j : argMap){
                bool req = check_required < 0 ? j.second->m_required : check_required > 0;
                if((j.second->m_arbitrary && !j.second->m_required) == flag
                   && j.second->m_hidden == hidden
                   && j.second->m_required == req){

                    print_usage(j);
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
                bool found = false;
                for(auto &x : argMap){
                    for(auto &alias : x.second->m_aliases){
                        if(alias == param){
                            j = argMap.find(x.first);
                            found = true;
                            break;
                        }
                    }
                    if(found)
                        break;
                }
            }

            if(j != argMap.end()){
                printParam(*j, true);
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
            sorted_usage(false, false, 0); //show options without *
            sorted_usage(false, false, 1); //show options with *
        }

        if(required_args > 1){
            std::cout << "For options marked with " + std::string(REQUIRED_OPTION_SIGN) + ": at least one such option should be provided" << std::endl;
        }
    }
};


#endif //VALLAB_PRINTER_ARGPARSER_H
