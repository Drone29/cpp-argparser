//
// Created by andrey on 17.12.2021.
//

#pragma once

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
#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

/// advanced help option (show hidden)
#define HELP_HIDDEN_OPT "-h"
/// help param name
#define HELP_PARAM_NAME "arg"
/// arg explanation
#define HELP_PARAM_EXPLANATION "'" HELP_PARAM_NAME "' to get help about certain arg"
/// braced help option
#define HELP_OPT_BRACED "[" HELP_PARAM_NAME "]"
#define HELP_ADVANCED_OPT_BRACED "[" HELP_PARAM_NAME " | " HELP_HIDDEN_OPT "]"
/// help self-explanation
#define HELP_GENERIC_MESSAGE \
    "Show this message and exit. " HELP_PARAM_EXPLANATION
/// callable macro, handles both function pointers and lambdas
#define CALLABLE(Callable) std::conditional_t<std::is_function_v<Callable>, \
        std::add_pointer_t<Callable>, Callable>
/// bool parsable strings
constexpr const char* BOOL_POSITIVES[] = {"true", "1", "yes", "on", "enable"};
constexpr const char* BOOL_NEGATIVES[] = {"false", "0", "no", "off", "disable"};
/// mark for required options
constexpr const char* REQUIRED_OPTION_SIGN  = "(*)";
/// help key
constexpr const char* HELP_NAME = "--help";
constexpr const char* HELP_ALIAS = "-h";
/// delimiter for key aliases
constexpr const char* KEY_ALIAS_DELIMITER = ",";
/// additional key/option delimiter (default is space)
constexpr const char* KEY_OPTION_DELIMITER = "=";
/// help argument identifier
constexpr const char* ARG_TYPE_HELP = "HELP";
/// date format by default
constexpr const char* DEFAULT_DATE_FORMAT = "%Y-%m-%dT%H:%M:%S";
/// max size of function string arguments
constexpr size_t MAX_ARGS = 10;
/// magic number for options array in addArgument
constexpr size_t OPTS_SZ_MAGIC = MAX_ARGS + 1;

/// Useful aliases ///
/// identifier for implicit argument
static const char * const IMPLICIT_ARG[OPTS_SZ_MAGIC] = {};
/// std::tm alias
using date_t = std::tm;

namespace parser_internal{
    /// fold expression to check if pack parameters are of the same type S
    template <typename S, typename ...T>
    inline constexpr bool are_same_type = (std::is_same_v<S, T> && ...);

    // dummy function for default template parameter
    void dummy(){}

    using no_action_t = decltype(dummy);

    namespace internal
    {
        template <typename TypeToStringify>
        struct GetTypeNameHelper{
            static std::string GetTypeName(){
#ifdef __GNUC__
                //For GCC
                std::string y = __PRETTY_FUNCTION__;
                std::string start = std::string("TypeToStringify") + " = ";
                std::string end = ";";
#elif defined _MSC_VER
                //For MSVC
                std::string y = __FUNCTION__;
                std::string start = "internal::GetTypeNameHelper<";
                std::string end = ">::GetTypeName";
#else
#warning "Unsupported compiler. Cannot stringify type"
                //For unsupported compilers return empty string
                std::string y = "";
                std::string start = "";
                std::string end = "";
                return "";
#endif
                if(y.find(start) == std::string::npos) return "";
                y = y.substr(y.find(start) + start.length());
                return y.substr(0, y.find(end));
            }
        };
    }

    template <typename T>
    inline std::string GetTypeName(){
        return internal::GetTypeNameHelper<T>::GetTypeName();
    }

    inline bool starts_with(const std::string &prefix, const std::string &s) noexcept{
        return s.substr(0, prefix.size()) == prefix;
    }

    template<typename T>
    inline T scan_number(const std::string &s){
        T res = 0;
        // string stream to parse numbers in different notations
        std::stringstream ss(s);
        auto pos = ss.tellg();
        // parse hex (no hexfloat)
        if(starts_with("0x", s) || starts_with("0X", s)){
            ss.str(s.substr(2));
            pos = ss.tellg();
            ss >> std::hex >> res;
        }else{
            // parse regular
            ss >> res;
        }

        auto distance = ss.tellg() - pos;
        bool fail = ss.fail() && !ss.eof();
        if(fail || distance > 0){
            throw std::runtime_error(std::string(__func__) + ": could not convert " + s + " to " + GetTypeName<T>());
        }
        return res;
    }
    template<class Target, class Source>
    Target narrow_cast(Source v, const std::string &s = ""){
        auto r = static_cast<Target>(v); // convert the value to the target type
        if (static_cast<Source>(r) != v){
            throw std::runtime_error{std::string(__func__) + ": " + s + " not representable as " + GetTypeName<Target>()};
        }
        return r;
    }
    /// format is applicable only to date_t type
    template<typename T>
    T scan(const char* arg, const char* date_format = nullptr){
        std::string temp = (arg == nullptr) ? "" : std::string(arg);
        T res;

        if constexpr(std::is_convertible_v<T, const char*>){
            return arg;
        }else if constexpr(std::is_same_v<T, std::string>){
            return temp;
        }else if constexpr(std::is_same_v<T, bool>){
            auto isTrue = [func=__func__](const char *s){
                for(auto &i : BOOL_POSITIVES){
                    if(!strcmp(s, i)) return true;
                }
                for(auto &i : BOOL_NEGATIVES){
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
        }else if constexpr(std::is_same_v<T, date_t>){
            date_t tt = {0};
            if(date_format == nullptr){
                date_format = DEFAULT_DATE_FORMAT;
            }
            std::stringstream ss(temp);
            std::stringstream chk; //check stream
            ss >> std::get_time(&tt, date_format); //convert string to time
            // ss.fail() does not cover all possible errors
            // so we're also checking equality of strings before and after conversion
            if (ss.fail()){
                throw std::runtime_error(std::string(__func__) + ": unable to convert " + temp + " to date");
            }
            // std::put_time is not good with empty strings, so we're checking it after ss.fail()
            chk << std::put_time(&tt, date_format); //convert result to string with the same format
            if (chk.fail() || ss.str() != chk.str()){
                throw std::runtime_error(std::string(__func__) + ": unable to convert " + temp + " to date");
            }
            return tt;
        }
            /// arithmetic
        else if constexpr(std::is_arithmetic_v<T>){
            /// char special
            if constexpr(std::is_same_v<T, char>){
                // special - if single char, treat as symbol
                if(temp.length() == 1){
                    res = temp[0];
                }else{
                    res = narrow_cast<char>(scan_number<int>(temp), temp);
                }
            }
                // for types whose size less than int
            else if constexpr(sizeof(T) < sizeof(int)){
                if constexpr(std::is_signed_v<T>){
                    res = narrow_cast<T>(scan_number<int>(temp), temp);
                }else{
                    res = narrow_cast<T>(scan_number<unsigned int>(temp), temp);
                }
            }else{
                /// numbers
                res = scan_number<T>(temp);
            }
        }
            /// not convertible
        else{
            throw std::logic_error(std::string(__func__) + ": no converter for " + temp + " of type " + GetTypeName<T>());
        }
        return res;
    }

    inline bool isOptMandatory(const std::string &sopt){
        return !sopt.empty() && (sopt.front() != '[')
               && (sopt.back() != ']');
    }
}

class BaseOption{
protected:
    friend class argParser;
    friend struct ARG_DEFS;
    BaseOption() = default;
    virtual ~BaseOption() = default;
    virtual void action (const std::string *args, int size, const char *date_format) {} // for variadic args
    virtual void set(std::any x) {}
    virtual std::string get_str_val() {return "";}
    virtual std::vector<std::string> get_str_choices() {return {};}
    virtual void set_global_ptr(std::any ptr) {}
    virtual void set_choices(const std::initializer_list<std::any> &choices_list) {}
    virtual void make_variadic() {}
    virtual bool is_variadic() {return false;}
    virtual void set_nargs(uint8_t n) {}
    virtual uint8_t get_nargs() {return 0;}
    std::any anyval;
//    bool variadic = false;
//    uint8_t nargs = 0;
};

template <typename T, typename Callable, size_t STR_ARGS, typename...Targs>
class DerivedOption : public BaseOption{
private:
    friend class argParser;
    [[nodiscard]] static constexpr bool has_action(){
        return !std::is_same_v<Callable, parser_internal::no_action_t>;
    }
    [[nodiscard]] static constexpr bool not_void(){
        return !std::is_void_v<T>;
    }

    void check_choices(){
        // applicable only to arithmetic or strings
        if constexpr(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>){
            if(!choices.empty()){
                // check if result correspond to one of the choices
                for(auto v : choices){
                    if(value == v){
                        return;
                    }
                }
                throw std::runtime_error("value does not correspond to any of given choices");
            }
        }
    }

    // parse variadic params, single scan and common action
    void action(const std::string *args, int size, const char *date_format) override{

        if(!variadic && nargs == 0){
            // if implicit
            if(STR_ARGS == 0 && !single_narg){
                parse_implicit();
                return;
            }
            if constexpr(has_action()){
                // non-variadic action
                parse_common(args, size);
            }else{
                // simple scan of single value
                set(parser_internal::scan<T>(args[0].c_str(), date_format));
            }
            check_choices();
            return;
        }
        // parse variadic
        std::vector<T> res;
        // variadic action
        for(int i=0; i<size; ++i){
            if constexpr(has_action()){
                parse_common(&args[i], 1);
            }else{
                value = parser_internal::scan<T>(args[i].c_str(), date_format);
            }
            check_choices();
            res.push_back(value);
        }
        anyval = res;
    }
    // for implicit args only
    void parse_implicit(){
        if(!has_action()){
            increment();
        }else{
            parse_common(nullptr, 0);
        }
    }
    void parse_common(const std::string *args, int size){
        if constexpr(not_void() && has_action()){
            if constexpr(STR_ARGS > 0){
                // create array of STR_ARGS size
                std::array<const char*, STR_ARGS> str_arr {};
                // fill array with vector values
                for(int i=0; i<size; ++i){
                    str_arr[i] = args[i].c_str();
                }
                // create tuple from array
                auto tpl_str = std::tuple_cat(str_arr);
                // resulting tuple
                auto tpl_res = std::tuple_cat(tpl, tpl_str);
                // call function with resulting tuple
                value = std::apply(func, tpl_res);
            }else{
                // call function with initial tuple
                value = std::apply(func, tpl);
            }
        }
        set_global();
        anyval = value;
    }
    void increment() {
        if constexpr (std::is_arithmetic_v<T>){
            if constexpr(std::is_same_v<T, bool>){
                // treat bool as special type an toggle value
                value = !value;
            }else{
                // increment other arithmetic types
                value += 1;
            }
        }
        set_global();
        anyval = value;
    }

    std::string get_str_val(T val){
        std::string res;
        try{
            if constexpr (std::is_arithmetic_v<T>){
                res = std::to_string(val);
            }
            else if constexpr (std::is_convertible_v<T, std::string>){
                if constexpr(std::is_pointer_v<T>){
                    res = (val == NULL || val == nullptr) ? "" : res = std::string(val);
                }else{
                    res = std::string(val);
                }
            }
        }catch(...){
            res = ""; //if null or other exception, set empty
        }
        return res;
    }

    std::string get_str_val() override{
        return get_str_val(value);
    }

    std::vector<std::string> get_str_choices() override{
        std::vector<std::string> res;
        for(auto v : choices){
            std::string s = get_str_val(v);
            if(!s.empty()){
                res.push_back(get_str_val(v));
            }
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

    void set_global(){
        if(global != nullptr){
            //set global
            *global = value;
        }
    }

    void set_choices(const std::initializer_list<std::any> &choices_list) override{
        if constexpr(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>){
            if constexpr(STR_ARGS > 1){
                throw std::invalid_argument("choices are not applicable to args with more than 1 parameter");
            }
            try{
                for(auto c : choices_list){
                    choices.push_back(std::any_cast<T>(c));
                }
            }catch(std::bad_any_cast &){
                throw std::invalid_argument("invalid choices list");
            }
        }else{
            throw std::invalid_argument("type can be either arithmetic or string");
        }
    }

    DerivedOption(Callable &&func_, const std::tuple<Targs...> &targs) :
            func(func_),
            tpl(targs){
        anyval = value;
    }

    ~DerivedOption() override = default;

    void make_variadic() override {
        anyval = std::vector<T>{};
        variadic = true;
    }
    bool is_variadic() override {return variadic;}
    void set_nargs(uint8_t n) override {
        if(n > 1){
            // return vector for nargs > 1
            anyval = std::vector<T>{};
            nargs = n;
        }else if(n == 1 && !variadic){
            // single narg treated as parameter
            single_narg = true;
        }
    }
    uint8_t get_nargs() override {return nargs;}

    static_assert(not_void(), "Return type cannot be void");

    T value {};
    // func stores functions, function ptrs, lambdas, functors
    CALLABLE(Callable) func;
    std::tuple<Targs...> tpl;
    T *global = nullptr;
    std::vector<T> choices {};

    bool variadic = false;
    uint8_t nargs = 0;
    bool single_narg = false;
};

struct ARG_DEFS{

    ARG_DEFS &help(std::string hlp){
        m_help = std::move(hlp);
        return *this;
    }
    ARG_DEFS &advanced_help(std::string hlp){
        m_advanced_help = std::move(hlp);
        return *this;
    }
    /// positional, mandatory options cannot be hidden
    /// NOTE: if hidden AND required, use carefully
    /// DO NOT make ALL required args hidden!!!
    ARG_DEFS &hidden(){
        if(!m_positional && m_arbitrary)
            m_hidden = true;
        return *this;
    }
    ARG_DEFS &repeatable(){
        if(!m_positional && !is_variadic())
            m_repeatable = true;
        return *this;
    }
    ARG_DEFS &default_value(std::any val, bool hide_in_help = false){
        try{
            if(m_arbitrary && !m_positional && !is_variadic()){
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
        /// positional cannot be required
        if(!m_positional){
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
                time_t t = 0;
                // Microsoft marks localtime as deprecated for some reason,
                // but localtime_s is not portable, so I'll leave it as it is
                std::tm tt = *std::localtime(&t);
                std::stringstream ss;
                ss << std::put_time(&tt, format);
                ss >> std::get_time(&tt, format);
                if (ss.fail()){
                    throw std::logic_error(std::string(__func__) + ": " + m_name + " invalid date format " + format);
                }
                m_date_format = format;
            }
            m_hide_date_format = hide_in_help;
        }
        return *this;
    }
    /// choices
    ARG_DEFS &choices(const std::initializer_list<std::any> &choice_list){
        try{
            option->set_choices(choice_list);
        }catch(std::invalid_argument &e){
            throw std::logic_error(std::string(__func__) + "(" + typeStr + "): error: " + e.what());
        }
        return *this;
    }
    /// nargs
    ARG_DEFS &nargs(uint8_t from, int8_t to = 0, const std::string &metavar = "") {
        if(!m_options.empty()){
            throw std::logic_error(std::string(__func__) + ": " + m_name + " nargs can be applied only to args with 0 parameters");
        }

        std::string narg_name;
        if(metavar.empty()){
            narg_name = m_name;
            //remove --
            while(parser_internal::starts_with("-", narg_name)){
                narg_name.erase(0, 1);
            }
            //convert to upper case
            for(auto &elem : narg_name) {
                elem = std::toupper(elem);
            }
        }else{
            narg_name = metavar;
        }

        // handle variadic
        if(to < 0){
            option->make_variadic();
        }

        int max_size = to > from ? to : from;
        if(max_size > 0){
            m_options = std::vector<std::string>(max_size, narg_name);
            for(int i = from; i < to; ++i){
                m_options[i] = "[" + m_options[i] + "]";
            }
        }

        mandatory_options = from;
        option->set_nargs(max_size);
        m_nargs_var = narg_name;
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
    [[nodiscard]] bool is_variadic() const{
        return option->is_variadic();
    };
    // applicable only for date_t type
    [[nodiscard]] const char *get_date_format() const{
        return m_date_format;
    }
    [[nodiscard]] auto options_size() const{
        return m_options.size();
    }
    // get raw string parameters passed from cli
    [[nodiscard]] std::vector<std::string> get_cli_params() const{
        return m_cli_params;
    }
    [[nodiscard]] std::string get_name() const{
        return m_name;
    }
    // get nargs
    [[nodiscard]] uint8_t get_nargs() const{
        return option->get_nargs();
    };
    ~ARG_DEFS() {
        delete option;
    }
private:

    explicit ARG_DEFS(std::string name)
            : m_name(std::move(name)){}

    friend class argParser;
    std::string m_name;
    std::string m_help;
    std::string m_advanced_help;
    std::string m_nargs_var;
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
    explicit argParser(const std::string &name = "", const std::string &descr = ""){
        argMap[HELP_NAME] = std::unique_ptr<ARG_DEFS>(new ARG_DEFS(HELP_NAME));
        argMap[HELP_NAME]->typeStr = ARG_TYPE_HELP;
        argMap[HELP_NAME]->m_help = std::string(HELP_GENERIC_MESSAGE);
        argMap[HELP_NAME]->m_options = {HELP_OPT_BRACED};
        argMap[HELP_NAME]->m_arbitrary = true;
        argMap[HELP_NAME]->option = new BaseOption();
        setAlias(HELP_NAME, {HELP_ALIAS});

        binary_name = name;
        description = descr;
    }
    ~argParser(){
        argMap.clear();
    }
    /**
     *
     * @tparam T
     * @tparam Callable - func/func ptr/lambda (CANNOT RETURN VOID)
     * @tparam Targs
     * @param key
     * @param func
     * @param targs
     * @return
     */
    template <typename T, typename Callable = parser_internal::no_action_t, typename...Targs>
    ARG_DEFS &addPositional(const std::string &key,
                            Callable &&func = parser_internal::dummy,
                            const std::tuple<Targs...> &targs = std::tuple<>()){

        /// check if variadic pos already defined
        for(auto &p : posMap){
            auto &x = argMap[p];
            if(x->is_variadic()){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot add positional argument after variadic positional argument " + p);
            }
            for(auto &o : x->m_options){
                if(!parser_internal::isOptMandatory(o)){
                    throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot add positional argument after positional argument with arbitrary nargs " + p);
                }
            }
        }
        // check if positional name is valid
        checkForbiddenSymbols(key, __func__);
        if(!parser_internal::isOptMandatory(key)){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " positional argument cannot be arbitrary");
        }

        /// get template type string
        auto strType = parser_internal::GetTypeName<T>();

        ///check if default parser for this type is present
        if constexpr(std::is_same_v<Callable, parser_internal::no_action_t>){
            try{
                parser_internal::scan<T>(nullptr);
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }
        auto arg = std::unique_ptr<ARG_DEFS>(new ARG_DEFS(key));
        arg->typeStr = strType;
        arg->m_options = {};
        arg->option = new DerivedOption<T, Callable, 1, Targs...>
                (std::forward<Callable>(func), targs);
        arg->m_positional = true;
        if(parser_internal::are_same_type<date_t, T>){
            arg->m_date_format = DEFAULT_DATE_FORMAT;
        }

        argMap[key] = std::move(arg);
        posMap.emplace_back(key);

        return *argMap[key];
    }
    /**
     *
     * @tparam T - return type
     * @tparam Callable - func/func_ptr/lambda type (CANNOT RETURN VOID)
     * @tparam OPT_SZ - size of options array (aka function string args) (CANNOT BE MORE THAN OPTS_SZ_MAGIC)
     * @tparam Targs - side arguments' types for function/lambda
     * @param names - argument name + aliases
     * @param opts_arr - array of string options
     * @param func - callable itself
     * @param targs - tuple with side arguments
     * @return - reference to ARG_DEFS struct
     */
    //OPT_SZ cannot be 0 as c++ doesn't support zero-length arrays
    template <typename T, typename Callable = parser_internal::no_action_t, size_t OPT_SZ = OPTS_SZ_MAGIC, typename...Targs>
    ARG_DEFS &addArgument(const std::vector<std::string> &names,
                          const char * const (&opts_arr)[OPT_SZ] = IMPLICIT_ARG,
                          Callable &&func = parser_internal::dummy,
                          const std::tuple<Targs...> &targs = std::tuple<>()){

        constexpr size_t opt_size = OPT_SZ != OPTS_SZ_MAGIC ? OPT_SZ : 0; // number of options
        constexpr bool implicit = opt_size == 0; //implicit arg has 0 options

        static_assert(opt_size < MAX_ARGS, "Too many string arguments");

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

        /// check for nulls
        for(int c = 0; c < opt_size; ++c){
            if(opts_arr[c] == nullptr){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key +  std::to_string(c) + "th parameter is null");
            }
        }
        /// create opts vector
        std::vector<std::string> opts = {opts_arr, opts_arr + opt_size};
        /// get template type string
        auto strType = parser_internal::GetTypeName<T>();
        ///Check for invalid sequence order of arguments
        std::string last_arbitrary_arg;
        std::string last_mandatory_arg;
        int mnd_vals = 0;

        for(auto & sopt : opts){
            if(sopt.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " parameter name cannot be empty");
            }
            if(sopt.front() == ' ' || sopt.back() == ' '){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " parameter " + sopt + " cannot begin or end with space");
            }

            if(parser_internal::isOptMandatory(sopt)){
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

        if constexpr(std::is_same_v<Callable, parser_internal::no_action_t>){
            static_assert(!implicit || std::is_arithmetic_v<T>, "Function should be provided for non-arithmetic implicit arg");
            if(!implicit && last_mandatory_arg.empty()){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no function provided for arg with arbitrary parameters");
            }
            ///check if default parser for this type is present
            try{
                parser_internal::scan<T>(nullptr);
            }catch(std::logic_error &e){
                throw std::invalid_argument(std::string(__func__) + ": " + splitKey.key + " no default parser for " + strType);
            }catch(std::runtime_error &){
                //do nothing
            }
        }

        auto arg = std::unique_ptr<ARG_DEFS>(new ARG_DEFS(splitKey.key));
        arg->typeStr = strType;
        arg->option = new DerivedOption<T, Callable, opt_size, Targs...>
                (std::forward<Callable>(func), targs);
        arg->m_options = opts;
        arg->m_arbitrary = flag;
        arg->m_implicit = implicit;
        arg->m_starts_with_minus = starts_with_minus;
        arg->mandatory_options = mnd_vals;
        if(parser_internal::are_same_type<date_t, T>
           && opt_size == 1){
            arg->m_date_format = DEFAULT_DATE_FORMAT;
        }
        argMap[splitKey.key] = std::move(arg);

        // add aliases
        for(auto &alias : splitKey.aliases){
            setAlias(splitKey.key, alias);
        }

        return *argMap[splitKey.key];
    }

    // another implementation of addArgument with const char *key
    template <typename T, typename Callable = parser_internal::no_action_t, size_t OPT_SZ = OPTS_SZ_MAGIC, typename...Targs>
    ARG_DEFS &addArgument(const char *key,
                          const char * const (&opts_arr)[OPT_SZ] = IMPLICIT_ARG,
                          Callable &&func = parser_internal::dummy,
                          const std::tuple<Targs...> &targs = std::tuple<>()){

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

        return addArgument<T, Callable, OPT_SZ, Targs...>
                (vec, opts_arr, std::forward<Callable>(func), targs);
    }

    template <typename T>
    T getValue(const std::string &key){
        parsedCheck(__func__);
        auto strType = parser_internal::GetTypeName<T>();
        auto &r = getArg(key);
        try{
            return std::any_cast<T>(r.option->anyval);
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
        return parser_internal::scan<T>(value, date_format);
    }

    /// Get last unparsed argument
    [[nodiscard]] const ARG_DEFS &getLastUnparsed() const{
        static ARG_DEFS dummy("");
        return last_unparsed_arg == nullptr ? dummy : *last_unparsed_arg;
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

    argParser &addCommand(const std::string &name, const std::string &descr){
        checkForbiddenSymbols(name, __func__);
        if(!parser_internal::isOptMandatory(name)){
            throw std::invalid_argument(std::string(__func__) + ": " + name + " child command cannot be arbitrary");
        }
        commandMap.push_back(std::make_unique<argParser>(name, descr));
        return *commandMap.front();
    }

    /// Parse arguments
    int parseArgs(int argc, char *argv[], bool hide_hidden_hint = false)
    {
        if(args_parsed){
            throw parse_error("Repeated attempt to run " + std::string(__func__));
        }
        ///Retrieve binary self-name
        if(binary_name.empty()){
            std::string self_name = std::string(argv[0]);
            binary_name = self_name.substr(self_name.find_last_of('/') + 1);
        }
        std::vector<std::string> argVec = {argv + 1, argv + argc};

        return parseArgs(argVec, hide_hidden_hint);
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

    std::map<std::string, std::unique_ptr<ARG_DEFS>> argMap;
    std::vector<std::unique_ptr<argParser>> commandMap;
    std::vector<std::string> posMap;

    std::string binary_name;
    std::string description;
    bool args_parsed = false;
    bool mandatory_option = false;
    bool command_parsed = false;
    int positional_args_parsed = 0;
    int positional_cnt = 0;
    int positional_places = 0;
    int mandatory_args = 0;
    int required_args = 0;
    int hidden_args = 0;
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
            throw std::invalid_argument(std::string(func) + ": " + key + " cannot contain spaces");
        }
    }

    //dummy function
    static void dummyFunc(){
        std::cout << "Use '" + std::string(HELP_NAME) + "' for list of available options" << std::endl;
    }

    int parseArgs(std::vector<std::string> &argVec, bool hide_hidden_hint = false){

        int parsed_mnd_args = 0;
        int parsed_required_args = 0;
        size_t command_offset = 0;

        //count mandatory/required arguments
        for(auto &x : argMap){
            if(!x.second->m_arbitrary
               && !x.second->m_positional){
                mandatory_args++;
            }else if(x.second->m_arbitrary
                     && x.second->m_required){
                required_args++;
            }
            if(x.second->m_hidden && !hide_hidden_hint){
                hidden_args++;
            }
        }
        for(auto &x : posMap){
            auto &arg = argMap[x];
            if(arg->is_variadic() || arg->get_nargs() > 0){
                positional_places += arg->mandatory_options;
            }else{
                positional_places += 1;
            }
        }

        mandatory_option = mandatory_args || required_args;

        auto parseArgument = [this, &argVec](const std::string &key, int start, int end) -> int{
            try{
                // end not included
                // remove spaces added while preparing
                for(auto it = argVec.begin() + start; it != argVec.begin() + end; it++){
                    if((*it).front() == ' '){
                        *it = (*it).substr(1);
                    }
                }
                const std::string *ptr = nullptr;
                // preserve out of range vector error
                if(start < argVec.size()){
                    // save raw cli parameters
                    argMap[key]->m_cli_params = {argVec.begin() + start, argVec.begin() + end};
                    // set pointer to start
                    ptr = &argVec.at(start);
                }
                argMap[key]->option->action(ptr, end - start, argMap[key]->m_date_format);
            }catch(std::exception &e){
                //save last unparsed arg
                last_unparsed_arg = argMap[key].get();
                throw unparsed_param(key + " : " + e.what());
            }
            return end-start;
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
            //make string same length
            tmpWhat += std::string(compareWith.length()-tmpWhat.length(), ' ');
            for(auto x : compareWith){
                char c = tmpWhat[idx++];
                auto remStr = compareWith.substr(idx);
                if(x != c){
                    if(remStr.find(c) == std::string::npos){
                        ++result;
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

        auto findChildByName = [this](const std::string &key) -> argParser*{
            for(const auto &child : commandMap){
                if(child->binary_name == key){
                    return child.get();
                }
            }
            return nullptr;
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
                    throw parse_error(binary_name + ": missing required option " + std::string(REQUIRED_OPTION_SIGN));
                }
            }
        };

        /// Handle '=' and combined args
        for(auto index = 0; index < argVec.size(); index++){

            auto insertKeyValue = [this, &index, &argVec](const std::string &key, const std::string &val){
                argVec[index] = key;
                argVec.insert(argVec.begin() + index + 1, val);
            };
            std::string pName = argVec[index];
            std::string pValue = index+1 >= argVec.size() ? "" : argVec[index + 1];


            ///Handle '='
            auto c = pName.find(KEY_OPTION_DELIMITER);
            if(c != std::string::npos){
                pValue = " " + pName.substr(c+1); //treat string after '=' as value anyway
                pName = pName.substr(0, c);
                //change current key and insert value to vector
                insertKeyValue(pName, pValue);
                // check arg name on the next iteration
                index--;
                continue;
            }

            if(argMap.find(pName) == argMap.end()){
                ///Find alias
                std::string name = findKeyByAlias(pName);
                if(findChildByName(pName) != nullptr){
                    // if found child, break
                    command_offset = argVec.size() - index;
                    break;
                }
                else if(!name.empty()){
                    // change alias to key
                    pName = name;
                    argVec[index] = name;
                    index += argMap[name]->mandatory_options; //skip mandatory opts
                }
                else{
                    ///check contiguous or combined arguments
                    int len = pName.front() == '-' ? 2 : 1;
                    std::string startsWith = pName.substr(0, len);
                    if(argMap.find(startsWith) != argMap.end()){
                        name = startsWith;
                    }else{
                        name = findKeyByAlias(startsWith);
                    }

                    if(!name.empty()){
                        auto &x = argMap[name];
                        // implicit contiguous (-vvv/-it or vvv/it style) argument
                        if(x->m_implicit){
                            //set '-' to other portion to extract it later
                            pValue = x->m_starts_with_minus ? "-" : "";
                            pValue += pName.substr(startsWith.length());
                            insertKeyValue(name, pValue);
                        }
                            //check if it's a contiguous keyValue or aliasValue pair (-k123 or k123 style)
                            //only for args with 1 option
                        else if(x->m_options.size() == 1){
                            pValue = " " + pName.substr(startsWith.length()); //treat as value
                            pName = name;
                            insertKeyValue(name, pValue);
                        }
                    }
                }//if name.empty()
            } //argMap.find(pName) == argMap.end()
            else{
                // if found in argMap, skip mandatory opts
                index += argMap[pName]->mandatory_options;
            }

            if(pName == HELP_NAME){
                // if found help key, break
                break;
            }
        }

        /// Main parser loop
        for(auto index = 0; index < argVec.size(); index++){

            std::string pName = argVec[index];
            std::string pValue = index+1 >= argVec.size() ? "" : argVec[index + 1];

            ///If found unknown key
            if(argMap.find(pName) == argMap.end()){

                for(;index < argVec.size(); ++index){
                    /// Parse children
                    auto child = findChildByName(argVec[index]);
                    if(child != nullptr){
                        std::vector<std::string> restvec = {argVec.begin() + index + 1, argVec.end()};
                        child->parseArgs(restvec, hide_hidden_hint);
                        command_parsed = true;
                        break;
                    }
                    ///Try parsing positional args
                    if(positional_args_parsed < posMap.size()){
                        auto pos_name = posMap[positional_args_parsed++];
                        int opts_cnt = 0;
                        if(argMap[pos_name]->get_nargs() > 0
                        || argMap[pos_name]->is_variadic()){
                            auto cnt = index-1;
                            int nargs = argMap[pos_name]->get_nargs();
                            bool variadic = argMap[pos_name]->is_variadic();
                            while(++cnt < argVec.size()){
                                if(findChildByName(argVec[cnt]) != nullptr){
                                    break;
                                }
                                if(nargs > 0 && opts_cnt >= nargs && !variadic){
                                    break;
                                }
                                ++opts_cnt;
                            }
                            if(opts_cnt < argMap[pos_name]->mandatory_options){
                                throw parse_error(binary_name + ": not enough " + pos_name + " arguments");
                            }
                        }else{
                            opts_cnt = 1;
                        }
                        positional_cnt += opts_cnt;
                        index += parseArgument(pos_name, index, index+opts_cnt);
                        --index;
                    }else{  //if(!posMap.empty())
                        throw parse_error("Error: trailing argument after positionals: " + argVec[index]);
                    }
                }

                if(command_parsed){
                    break;
                }
                if(!posMap.empty() && positional_args_parsed == posMap.size()){
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

                dummyFunc();
                if(mismatch < 2){
                    thrError += ". Did you mean " + proposed_value + "?";
                }
                throw parse_error(thrError);
            }
            ///Show help
            if(argMap[pName]->typeStr == ARG_TYPE_HELP){
                helpDefault(pValue);
                exit(0);
            }
            else{
                ///Parse other types

                ///If non-repeatable and occurred again, throw error
                if(argMap[pName]->m_set
                   && !argMap[pName]->m_repeatable){
                    throw parse_error("Error: redefinition of non-repeatable arg " + std::string(pName));
                }

                int opts_cnt = 0;
                auto cnt = index;
                bool infinite_opts = argMap[pName]->is_variadic();

                while(++cnt < argVec.size()){
                    // if all options found, break
                    if(!infinite_opts && opts_cnt >= argMap[pName]->m_options.size()){
                        break;
                    }
                    // leave space for positionals
                    auto left = argVec.size() - cnt - command_offset;
                    if((infinite_opts || opts_cnt == argMap[pName]->mandatory_options)
                       && left <= positional_places){   //posMap.size()
                        break;
                    }
                    //check if next value is also a key
                    bool next_is_key = (argMap.find(argVec[cnt]) != argMap.end());  // || findChildByName(argVec[cnt]) != nullptr)
                    if(next_is_key){
                        break;
                    }
                    ++opts_cnt;
                }

                if(opts_cnt < argMap[pName]->mandatory_options){
                    throw parse_error(std::string(pName) + " requires "
                                      + std::to_string(argMap[pName]->mandatory_options) + " parameters, but " + std::to_string(opts_cnt) + " were provided");
                }

                parseArgument(pName, index + 1, index + 1 + opts_cnt);

                index += opts_cnt;
                setArgument(pName);
            }
        }

        checkParsedNonPos();
        if(positional_cnt < positional_places){
            throw parse_error(binary_name + ": not enough positional arguments provided");
        }
        if(!commandMap.empty() && !command_parsed){
            throw parse_error(binary_name + ": no command provided");
        }

        args_parsed = true;
        return 0;
    }

    void helpDefault(const std::string &param = ""){

        bool advanced = false;

        if(hidden_args > 0){
            argMap[HELP_NAME]->m_options = {HELP_ADVANCED_OPT_BRACED};
            argMap[HELP_NAME]->m_help += ", '" + std::string(HELP_HIDDEN_OPT) + "' to list hidden args as well";
        }

        auto get_choices = [](const auto &j) -> std::string{
            // List choices if applicable
            auto choices = j->option->get_str_choices();
            std::string opt;
            if(!choices.empty()){
                opt = "{";
                bool notFirst = false;
                for(auto &c : choices){
                    if(notFirst){
                        opt += ",";
                    }
                    opt += c;
                    notFirst = true;
                }
                opt += "}";
            }
            return opt;
        };

        auto printParam = [&get_choices](const auto &j, bool notab = false){
            std::string alias_str = notab ? "" : "\t";
            for(auto &alias : j.second->m_aliases){
                alias_str += alias + KEY_ALIAS_DELIMITER + " ";
            }
            std::cout <<  alias_str + j.first;
            std::string opt = j.second->m_nargs_var;
            std::string choices = get_choices(j.second);
            if(!choices.empty()){
                // override metavar
                opt = choices;
            }
            for(auto &x : j.second->m_options){
                opt = std::string(x);
                if(!choices.empty()){
                    opt = choices;
                    if(!parser_internal::isOptMandatory(x)){
                        opt = "[" + opt + "]";
                    }
                }
                std::string tmp = opt;
                if(choices.empty() && parser_internal::isOptMandatory(tmp)){
                    tmp = "<" + tmp + ">";
                }
                std::cout << " " + tmp;
            }

            if(j.second->is_variadic()){
                std::cout << " [" + opt + "...]";
            }
        };
        //check required: -1-don't check, 0-false, other-true
        auto sorted_usage = [this, &advanced, &printParam](bool flag, bool hidden, int check_required = -1){

            auto print_usage = [&advanced, &printParam, this](const auto &j){
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

                std::string def_val = j.second->option->get_str_val();
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

            for (const auto &j : argMap){
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
                std::cout << j->second->m_help << std::endl;
                std::cout << j->second->m_advanced_help << std::endl;
            }else{
                std::cout << "Unknown parameter " + param << std::endl;
            }
            return;
        }

        std::string positional;
        for(auto &x : posMap){
            std::string opt = x;
            auto choices = get_choices(argMap[x]);
            if(!choices.empty()){
                opt = choices;
            }
            for(auto &o : argMap[x]->m_options){
                std::string tmp = opt;
                tmp = !parser_internal::isOptMandatory(o) ? ("[" + tmp + "]") : tmp;
                positional += " " + tmp;
            }
            if(argMap[x]->m_options.empty()){
                positional += " " + opt;
            }
            if(argMap[x]->is_variadic()){
                positional += " [" + opt + "...]";
            }
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

        if(!description.empty()){
            std::cout << description << std::endl;
        }

        std::cout << "Usage: " << binary_name
                     << (flag_cnt ? " [flags...]" : "")
                     << (opt_cnt ? " options..." : "")
                     << positional
                     << (!commandMap.empty() ? " command [<args>]" : "")
                     << std::endl;

        if(!commandMap.empty()){
            std::cout << "Commands:" << std::endl;
            for(const auto &child : commandMap){
                std::cout << "\t" << child->binary_name << " : " << child->description << std::endl;
            }
        }

        if(!posMap.empty()){
            std::cout << "Positional arguments:" << std::endl;
            for(auto &x : posMap){
                std::string date_format = argMap[x]->m_date_format == nullptr ? "" : (" { format: " + std::string(argMap[x]->m_date_format) + "}");
                std::cout << "\t" << x << " : " << argMap[x]->m_help << date_format << std::endl;
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
            if(advanced){
                //show hidden
                sorted_usage(false, true, 0);
                sorted_usage(false, true, 1);
            }
        }

        if(required_args > 1){
            std::cout << "For options marked with " + std::string(REQUIRED_OPTION_SIGN) + ": at least one such option should be provided" << std::endl;
        }
    }
};