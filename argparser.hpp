#pragma once

#include <map>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <cstring>
#include <any>
#include <tuple>
#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>

namespace parser_internal{

    /// bool parsable strings
    constexpr const char* BOOL_POSITIVES[] = {"true", "1", "yes", "on", "enable"};
    constexpr const char* BOOL_NEGATIVES[] = {"false", "0", "no", "off", "disable"};

    namespace internal
    {
        template <typename TypeToStringify>
        struct GetTypeNameHelper{
            static std::string GetTypeName(){
#ifdef __clang__
                // For Clang
                std::string y = __PRETTY_FUNCTION__;
                std::string start = "TypeToStringify = ";
                std::string end = "]";
#elif defined(__GNUC__)
                // For GCC
                std::string y = __PRETTY_FUNCTION__;
                std::string start = "TypeToStringify = ";
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

    inline bool starts_with(const std::string &prefix, const std::string &s) noexcept {
        return s.substr(0, prefix.size()) == prefix;
    }

    inline bool isOptMandatory(const std::string &sopt) noexcept {
        return !sopt.empty() && (sopt.front() != '[')
               && (sopt.back() != ']');
    }

    template <typename T>
    std::string GetTypeName(){
        return internal::GetTypeNameHelper<T>::GetTypeName();
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

    template<typename T>
    T scan(const char* arg){
        std::string temp = (arg == nullptr) ? "" : std::string(arg);
        T res;
        if constexpr(std::is_convertible_v<T, const char*>){
            return arg;
        }else if constexpr(std::is_same_v<T, std::string>){
            return temp;
        }else if constexpr(std::is_same_v<T, bool>){
            auto isTrue = [func=__func__](const char *s){
                for(const auto &i : BOOL_POSITIVES){
                    if(!strcmp(s, i)) return true;
                }
                for(const auto &i : BOOL_NEGATIVES){
                    if(!strcmp(s, i)) return false;
                }
                throw std::runtime_error(std::string(func) + ": unable to convert " + s + " to bool");
            };
            std::string lVal;
            //convert to lower case
            for(auto elem : temp) {
                lVal += char(std::tolower(elem));
            }
            return isTrue(lVal.c_str());
        }/// arithmetic
        else if constexpr(std::is_arithmetic_v<T>){
            /// char special
            if constexpr(std::is_same_v<T, char>){
                // special - if single char, treat as symbol
                if(temp.length() == 1){
                    res = temp[0];
                }else{
                    res = narrow_cast<char>(scan_number<int>(temp), temp);
                }
            }/// for types whose size less than int
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
        }/// not convertible
        else{
            throw std::logic_error(std::string(__func__) + ": no converter for " + temp + " of type " + GetTypeName<T>());
        }
        return res;
    }

    template<typename T>
    struct hasScanHandler {
        static constexpr bool value =
                std::is_convertible_v<T, const char*> ||      // Convertible to const char*
                std::is_same_v<T, std::string> ||             // std::string
                std::is_arithmetic_v<T>;                      // arithmetic
    };

    inline void validateKeyOrParam(const std::string &key, bool is_param, const char* func){
        auto isValidKeyChar = [](int c) -> bool {
            return std::isalnum(c) || c == '-' || c == '_';
        };
        auto isValidParamChar = [](int c) -> bool {
            return std::isalnum(c) || std::ispunct(c) || std::isspace(c);
        };
        auto isDigitOrPunct = [](int c) -> bool {
            return std::isdigit(c) || std::ispunct(c);
        };

        auto isValidCharCond = is_param ? isValidParamChar : isValidKeyChar;
        auto invalidChar = std::find_if_not(key.begin(), key.end(), isValidCharCond);
        auto allDigitsAndPunct = std::all_of(key.begin(), key.end(), isDigitOrPunct);
        if(key.empty())
            throw std::invalid_argument(std::string(func) + ": empty key or param");
        if(invalidChar != key.end())
            throw std::invalid_argument(std::string(func) + ": " + key + " cannot contain " + *invalidChar);
        if(allDigitsAndPunct)
            throw std::invalid_argument(std::string(func) + ": " + key + " cannot consist only of digits and punctuation chars");
        if(key.back() == '-')
            throw std::invalid_argument(std::string(func) + ": " + key + " shouldn't end with '-'");
    }
}

class ArgHandleBase{
protected:
    friend class argParser;
    friend struct Argument;
    friend class ArgBuilderBase;
    ArgHandleBase() = default;

    virtual void action (const std::string *args, int size) {}
    virtual void set_value(const std::any &x) {}
    virtual std::string get_str_val() const {return "";}
    virtual std::vector<std::string> get_str_choices() const {return {};}
    virtual void set_global_ptr(const std::any &ptr) {}
    virtual void set_choices(std::vector<std::any> &&choices_list) {}
    virtual void make_variadic() {}
    virtual bool is_variadic() {return false;}
    virtual void set_nargs(unsigned int n) {}
    virtual unsigned int get_nargs() {return 0;}
    virtual std::any get_any_val() const {return {};}

public:
    virtual ~ArgHandleBase() = default;
};

template <typename T, size_t STR_ARGS, typename...Targs>
class ArgHandle : public ArgHandleBase{
private:
    friend class argParser;
    friend class ArgBuilderBase;
    using NContainer = std::vector<T>;

    T m_value;
    std::any m_anyval;
    std::tuple<Targs...> m_action_and_args; //holds action (function, lambda, etc) and side args supplied to it
    T *m_global = nullptr;
    NContainer m_choices {};
    bool m_variadic = false;
    unsigned int m_nargs = 0;
    bool m_single_narg = false;

    [[nodiscard]] static constexpr bool has_action() {
        // if tuple is not empty, it has function
        return std::tuple_size_v<decltype(m_action_and_args)> > 0;
    }
    [[nodiscard]] static constexpr bool not_void(){
        return !std::is_void_v<T>;
    }
    [[nodiscard]] static constexpr bool choices_viable() {
        return std::is_arithmetic_v<T> || std::is_same_v<T, std::string>;
    }

    static_assert(not_void(), "Argument type cannot be void");

    void check_choices() const {
        // applicable only to arithmetic or strings
        if constexpr(choices_viable()) {
            if(!m_choices.empty()){
                // check if result corresponds to one of the choices
                for(const auto &v : m_choices){
                    if(m_value == v){
                        return;
                    }
                }
                throw std::runtime_error("value does not correspond to any of given choices");
            }
        }
    }

    void containerize(NContainer &&container = NContainer{}) {
        m_anyval = std::move(container);
    }

    void set_any_val() {
        set_global();
        m_anyval = m_value;
    }

    // parse variadic params, single scan and common action
    void action(const std::string *args, int size) override {
        if(!m_variadic && m_nargs == 0) {
            // if implicit
            bool implicit = STR_ARGS == 0 && !m_single_narg;
            if(implicit || size <= 0){
                parse_implicit();
                return;
            }
            if constexpr(has_action()){
                // non-variadic action
                parse_common(args, size);
            }else{
                // simple scan of single value
                m_value = parser_internal::scan<T>(args[0].c_str());
                set_any_val();
            }
            check_choices();
        } else {
            parse_variadic(args, size);
        }
    }
    // parse variadic
    void parse_variadic(const std::string *args, int size) {
        // parse variadic
        NContainer res;
        // variadic action
        for(int i=0; i<size; ++i){
            if constexpr(has_action()){
                parse_common(&args[i], 1);
            }else{
                m_value = parser_internal::scan<T>(args[i].c_str());
            }
            check_choices();
            res.push_back(m_value);
        }
        containerize(std::move(res));
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
        if constexpr(not_void() && has_action()) {
            // obtain action and side args from tuple
            auto && [func, side_args] = m_action_and_args;
            if constexpr(STR_ARGS > 0) {
                // create array of STR_ARGS size
                std::array<const char*, STR_ARGS> str_arr {};
                // fill array with vector values
                for(int i=0; i<size; ++i){
                    str_arr[i] = (args + i)->c_str();
                }
                // resulting tuple
                auto tpl_res = std::tuple_cat(side_args, str_arr);
                // call function with resulting tuple
                using rType = decltype(std::apply(func, tpl_res));
                if constexpr(!std::is_same_v<rType, void>) {
                    m_value = std::apply(func, tpl_res);
                } else {
                    std::apply(func, tpl_res); // if void
                }
            }
            else{
                // call function with initial tuple
                using rType = decltype(std::apply(func, side_args));
                if constexpr(!std::is_same_v<rType, void>) {
                    m_value = std::apply(func, side_args);
                } else {
                    std::apply(func, side_args); // if void
                }
            }
        }
        set_any_val();
    }
    void increment() {
        if constexpr(std::is_arithmetic_v<T>) {
            if constexpr(std::is_same_v<T, bool>) {
                // treat bool as special type and toggle value
                m_value = !m_value;
            }else{
                // increment other arithmetic types
                m_value += 1;
            }
        }
        set_any_val();
    }

    std::string get_str_val(T val) const {
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

    [[nodiscard]] std::string get_str_val() const override{
        return get_str_val(m_value);
    }

    [[nodiscard]] std::vector<std::string> get_str_choices() const override{
        std::vector<std::string> res;
        for(const auto &v : m_choices){
            std::string s = get_str_val(v);
            if(!s.empty()){
                res.push_back(get_str_val(v));
            }
        }
        return res;
    }

    void set_value(const std::any &x) override {
        m_value = std::any_cast<T>(x);
        set_any_val();
    }

    void set_global_ptr(const std::any &ptr) override {
        m_global = std::any_cast<T*>(ptr);
    }

    void set_global() {
        if(m_global != nullptr) {
            *m_global = m_value;
        }
    }

    void set_choices(std::vector<std::any> &&choices_list) override {
        for(auto &&c : choices_list) {
            m_choices.push_back(std::any_cast<T>(c));
        }
    }

    void make_variadic() override {
        containerize();
        m_variadic = true;
    }
    bool is_variadic() override {return m_variadic;}
    void set_nargs(unsigned int n) override {
        if(n > 1){
            // return vector for m_nargs > 1
            containerize();
            m_nargs = n;
        }else if(n == 1 && !m_variadic){
            // single narg treated as parameter
            m_single_narg = true;
        }
    }

    unsigned int get_nargs() override {return m_nargs;}

    std::any get_any_val() const override {
        return m_anyval;
    }

    explicit ArgHandle(std::tuple<Targs...> &&tpl) :
            m_value(),
            m_anyval(m_value),
            m_action_and_args(std::move(tpl)) {}

    ~ArgHandle() override = default;
};

struct Argument{

    [[nodiscard]] bool isSet() const{
        return m_set;
    }
    [[nodiscard]] bool isOptional() const{
        return m_optional;
    }
    [[nodiscard]] bool isRequired() const{
        return m_required;
    }
    [[nodiscard]] bool isPositional() const{
        return m_positional;
    }
    [[nodiscard]] bool isImplicit() const{
        return m_implicit;
    }
    [[nodiscard]] bool isRepeatable() const{
        return m_repeatable;
    }
    [[nodiscard]] bool isVariadic() const{
        return m_arg_handle->is_variadic();
    };
    [[nodiscard]] std::string getName() const{
        return m_name;
    }
    // get nargs
    [[nodiscard]] unsigned int getNargs() const{
        return m_arg_handle->get_nargs();
    };

    // conversion operator template
    template<typename T>
    operator T() const {
        return std::any_cast<T>(m_arg_handle->get_any_val());
    }

    ~Argument() = default;
private:

    explicit Argument(std::string name) noexcept
            : m_name(std::move(name)){}

    friend class argParser;
    friend class ArgBuilderBase;
    std::string m_name;
    std::string m_help;
    std::string m_advanced_help;
    std::string m_nargs_var;
    //hidden option
    bool m_hidden = false;
    //list of options
    std::vector<std::string> m_options;
    //stringified type
    std::string m_type_str;
    //Option/flag
    std::unique_ptr<ArgHandleBase> m_arg_handle;
    //in use
    bool m_set = false;
    //alias
    std::vector<std::string> m_aliases;
    //Positional
    bool m_positional = false;
    //Flag
    bool m_optional = false;
    //Required != !optional. if !optional, user should specify all of them. If required, at least one is enough
    bool m_required = false;
    //Implicit
    bool m_implicit = false;
    //repeatable
    bool m_repeatable = false;
    //If starts with minus
    bool m_starts_with_minus = false;
    //Mandatory opts
    int m_mandatory_options = 0;
    //show default
    bool m_show_default = false;
};

// forward-declare for arg builder
class argParser;

class ArgBuilderBase {
private:
    bool m_is_variadic = false;
    int m_nargs_size = 0;
    std::any m_default_val;
    std::any m_global_ptr;
    std::vector<std::any> m_choices;
protected:
    std::function<void(std::unique_ptr<Argument> &&)> m_callback;
    std::unique_ptr<Argument> m_arg;

    ArgBuilderBase(const std::string &key,
                   std::vector<std::string> &&aliases,
                   bool is_positional,
                   std::function<void(std::unique_ptr<Argument> &&)> &&callback)
                        : m_callback(std::move(callback)) {
        m_arg.reset(new Argument(key));
        m_arg->m_positional = is_positional;
        m_arg->m_aliases = std::move(aliases);
        m_arg->m_optional = key.front() == '-';
        m_arg->m_starts_with_minus = m_arg->m_optional;
    }

    void setArgOpts(std::vector<std::string> &&opts, int mandatory_opts){
        m_arg->m_options = std::move(opts);
        m_arg->m_mandatory_options = mandatory_opts;
    }
    void setArgStrType(const std::string &strType){
        m_arg->m_type_str = strType;
    }
    void setArgNargTraits(const std::string &narg_name, int nargs_size, bool is_variadic){
        m_arg->m_nargs_var = narg_name;
        m_nargs_size = nargs_size;
        m_is_variadic = is_variadic;
    }
    void setArgHelp(const std::string &help){
        m_arg->m_help = help;
    }
    void setArgAdvancedHelp(const std::string &adv_help){
        m_arg->m_advanced_help = adv_help;
    }
    void makeArgHidden(){
        if(!m_arg->m_positional && m_arg->m_optional)
            m_arg->m_hidden = true;
    }
    void makeArgRepeatable(){
        if(!m_arg->m_positional && !m_is_variadic)
            m_arg->m_repeatable = true;
    }
    void setArgDefaultValue(std::any &&val, bool hide_in_help){
        if(m_arg->m_optional && !m_arg->m_positional && !m_is_variadic) {
            m_default_val = std::move(val);
            m_arg->m_show_default = !hide_in_help;
        }
    }
    void setArgGlobPtr(std::any &&ptr){
        m_global_ptr = std::move(ptr);
    }
    void makeArgMandatory(){
        if(m_arg->m_optional && !m_arg->m_positional && !m_arg->m_hidden)
            m_arg->m_optional = false;
    }
    void makeArgRequired(){
        if(!m_arg->m_positional) {
            m_arg->m_required = true;
            m_arg->m_optional = true;
        }
    }
    void setArgChoices(std::vector<std::any> &&choices){
        m_choices = std::move(choices);
    }

    void createArg(ArgHandleBase *handle) {

        bool is_implicit = m_arg->m_options.empty();

        if(!m_arg->m_mandatory_options && !m_arg->m_positional && !m_arg->m_optional){
            throw std::invalid_argument(std::string(__func__) + ": " + m_arg->m_name + " should have at least 1 mandatory parameter");
        }

        if (m_is_variadic)
            handle->make_variadic();
        if (m_default_val.has_value())
            handle->set_value(m_default_val);
        if (m_global_ptr.has_value())
            handle->set_global_ptr(m_global_ptr);
        if (!m_choices.empty())
            handle->set_choices(std::move(m_choices));
        handle->set_nargs(m_nargs_size);
        m_arg->m_arg_handle = std::unique_ptr<ArgHandleBase>(handle);
        m_arg->m_implicit = is_implicit;

        m_callback(std::move(m_arg));
    }

    // Helper function to forward all types in the tuple
    template<typename VType, size_t STR_PARAMS, size_t... I, typename TupleType>
    ArgHandleBase *createOption(std::index_sequence<I...>, TupleType &&t) {
        return new ArgHandle<
                VType,
                STR_PARAMS,
                std::tuple_element_t<I, TupleType>...
        >(std::forward<TupleType>(t));
    }
};

// arg builder
template<size_t STR_PARAM_IDX, size_t CALLABLE_IDX, bool POSITIONAL, typename... Types>
class ArgBuilder : public ArgBuilderBase {
protected:
    friend class argParser;
    std::tuple<Types...> m_components;

    // add new component
    template<size_t FIRST_IDX, size_t SECOND_IDX, bool POS, typename NewType>
    auto addComponent(NewType &&newComponent) {
        return ArgBuilder<FIRST_IDX, SECOND_IDX, POS, Types..., NewType>(
                this, // pass current obj pointer further to create a new object with already existing data
                std::tuple_cat(std::move(m_components), std::make_tuple(std::forward<NewType>(newComponent))
                ));
    }

public:
    // ctor
    explicit ArgBuilder(std::string &&key,
                        std::vector<std::string> &&aliases,
                        std::function<void(std::unique_ptr<Argument> &&)> &&callback,
                        std::tuple<Types...> &&comps)
            : ArgBuilderBase(std::move(key), std::move(aliases), POSITIONAL, std::move(callback)),
              m_components(std::move(comps)){}
    // 'move' ctor
    explicit ArgBuilder(ArgBuilderBase *prev, std::tuple<Types...> &&comps)
    // use move semantics to construct the helper
            : ArgBuilderBase(std::move(*prev)),
              m_components(std::move(comps)){}

    // add arguments
    template<typename... Params>
    decltype(auto) parameters(Params ...strArgs) {
        if constexpr (sizeof...(strArgs) > 0) {
            static_assert((std::is_same_v<Params, const char*> && ...), "Params must be strings");
        }
        static_assert(STR_PARAM_IDX == 0, "Params or nargs already set");
        const size_t current_size = std::tuple_size_v<decltype(m_components)>;
        std::string m_last_optional_arg;
        int mandatory_opts = 0;
        // func to check options
        auto checkOpts = [this, func=__func__, &m_last_optional_arg, &mandatory_opts](const char *k) {
            if (!k){
                throw std::invalid_argument("Arg cannot be null!");
            }
            auto sopt = std::string(k);
            parser_internal::validateKeyOrParam(sopt, /*is_param=*/true, func);
            if(parser_internal::isOptMandatory(sopt)){
                mandatory_opts++;
                if(!m_last_optional_arg.empty()){
                    throw std::invalid_argument(std::string(func) + ": " + m_arg->getName()
                                                + ": optional argument "
                                                + m_last_optional_arg
                                                + " cannot be followed by mandatory argument "
                                                + sopt);
                }
            }
            else{
                m_last_optional_arg = sopt;
            }
            return sopt;
        };

        // populate opts
        std::vector<std::string> opts = {checkOpts(strArgs)...};
        setArgOpts(std::move(opts), mandatory_opts);

        return addComponent<current_size,CALLABLE_IDX,POSITIONAL>(std::make_tuple(strArgs...));
    }

    // set NArgs
    template<unsigned int FRO, int TO = 0>
    decltype(auto) nargs() {
        static_assert(FRO != 0 || TO != 0, "nargs cannot be zero!");
        auto prepareNargs = [this](const std::string &narg_name){
            const bool is_variadic = TO < 0;
            int max_size = TO > int(FRO) ? TO : int(FRO);
            auto opts = std::vector<std::string>(max_size, narg_name);
            for(int i = int(FRO); i < TO; ++i) {
                opts[i] = "[" + opts[i] + "]";
            }
            setArgOpts(std::move(opts), FRO);
            setArgNargTraits(narg_name, max_size, is_variadic);
        };

        //if single parameter provided, it's ok
        if constexpr (STR_PARAM_IDX > 0) {
            auto str_params = std::get<STR_PARAM_IDX>(m_components); // string params
            const size_t str_params_size = std::tuple_size_v<decltype(str_params)>;
            // provided param is metavar
            auto &[param_name] = str_params;
            if (!parser_internal::isOptMandatory(param_name)) {
                throw std::invalid_argument(std::string(__func__) + ": " + m_arg->getName() + " ambiguity detected: optional param used along with nargs");
            }
            static_assert(str_params_size < 2, "Nargs only applicable to args with 0 or 1 parameters");
            prepareNargs(param_name);
            return (*this);
        } else {
            // provide our own param idx
            const size_t current_size = std::tuple_size_v<decltype(m_components)>;
            auto narg_name = m_arg->getName();
            // convert non-positional to upper case
            if(!m_arg->isPositional()){
                //remove --
                narg_name = narg_name.substr(narg_name.find_first_not_of('-'));
                //convert to upper case
                for(auto &elem : narg_name) {
                    elem = char(std::toupper(elem));
                }
            }
            prepareNargs(narg_name);
            return addComponent<current_size,CALLABLE_IDX,POSITIONAL>(
                    std::make_tuple(std::make_tuple(narg_name.c_str()))
            );
        }
    }

    // add callable and side args (if any)
    template<typename Callable, typename... SideArgs>
    decltype(auto) callable(Callable && clbl, SideArgs ...sideArgs) {
        static_assert(CALLABLE_IDX == 0, "Callable already set");
        const size_t current_size = std::tuple_size_v<decltype(m_components)>;
        return addComponent<STR_PARAM_IDX,current_size,POSITIONAL>(
                std::make_tuple(
                std::forward<Callable>(clbl),
                std::make_tuple(std::forward<SideArgs>(sideArgs)...))
        );
    }

    decltype(auto) help(const std::string &help_message) {
        setArgHelp(help_message);
        return (*this);
    }

    decltype(auto) advancedHelp(const std::string &adv_help_message) {
        setArgAdvancedHelp(adv_help_message);
        return (*this);
    }

    decltype(auto) hidden() {
        makeArgHidden();
        return (*this);
    }

    decltype(auto) repeatable() {
        makeArgRepeatable();
        return (*this);
    }

    template<typename T>
    decltype(auto) defaultValue(T &&default_val, bool hide_in_help = false) {
        auto val = std::get<0>(m_components);
        using VType = decltype(val);
        static_assert(std::is_same_v<VType, T>, "Default value type mismatch");
        setArgDefaultValue(std::forward<T>(default_val), hide_in_help);
        return (*this);
    }

    template<typename T>
    decltype(auto) globalPtr(T *glob_ptr) {
        auto val = std::get<0>(m_components);
        using VType = decltype(val);
        static_assert(std::is_same_v<VType, T>, "Pointer type mismatch");
        setArgGlobPtr(glob_ptr);
        return (*this);
    }

    decltype(auto) mandatory() {
        makeArgMandatory();
        return (*this);
    }

    decltype(auto) required() {
        makeArgRequired();
        return (*this);
    }

    template<typename... Choices>
    decltype(auto) choices(Choices ...choices){
        auto val = std::get<0>(m_components);
        auto str_params = std::get<STR_PARAM_IDX>(m_components); // string params
        using VType = decltype(val);
        const size_t str_params_size = std::tuple_size_v<decltype(str_params)>;

        static_assert(std::is_arithmetic_v<VType> || std::is_same_v<VType, std::string>,
                      "Choices are applicable only to arithmetic types and strings");
        static_assert((std::is_convertible_v<Choices, VType> && ...),
                      "Choices should be convertible to the type of the argument");
        static_assert(STR_PARAM_IDX > 0, "Params or nargs should be set before choices");
        static_assert(str_params_size < 2, "Choices only applicable to args with 1 parameter");

        auto validate_and_convert = [](auto &&choice) -> std::any {
            using T = decltype(choice);
            return VType(std::forward<T>(choice));
        };
        setArgChoices({validate_and_convert(choices)...});
        return (*this);
    }

    // finalize argument declaration
    void finalize() {
        auto val = std::get<0>(m_components);
        using VType = decltype(val);
        const size_t comp_size = std::tuple_size_v<decltype(m_components)>;
        static_assert(comp_size > 0, "Should have at least 1 component");
        const bool has_params = STR_PARAM_IDX > 0;
        const bool has_callable = CALLABLE_IDX > 0;
        /// get template type string
        const auto str_type = parser_internal::GetTypeName<VType>();
        ///check if default parser for this type is present
        const bool has_default_parser = parser_internal::hasScanHandler<VType>::value;

        ArgHandleBase *option = nullptr;
        // either default parser or callable should be present
        static_assert(has_default_parser || has_callable, "No default parser for this type");
        // if there are string params
        if constexpr (has_params) {
            auto str_params = std::get<STR_PARAM_IDX>(m_components); // string params
            const size_t str_params_size = std::tuple_size_v<decltype(str_params)>;
            // if function not provided
            if constexpr (!has_callable) {
                static_assert(str_params_size < 2, "A parsing function should be provided for arguments with more than 1 parameter");
                option = createOption<VType, str_params_size>
                        (std::make_index_sequence<0>{}, std::tuple<>()); //empty tuple for no function
            } else {
                auto func_tpl = std::get<CALLABLE_IDX>(m_components);
                const size_t func_tpl_size = std::tuple_size_v<decltype(func_tpl)>;
                option = createOption<VType, str_params_size>
                        (std::make_index_sequence<func_tpl_size>{}, std::move(func_tpl));
            }
        } else {
            // no params = implicit
            if constexpr(!has_callable) {
                static_assert(std::is_arithmetic_v<VType>, "Function should be provided for non-arithmetic implicit arg");
                option = createOption<VType, 0>
                        (std::make_index_sequence<0>{}, std::tuple<>()); //empty tuple for no function
            } else {
                auto func_tpl = std::get<CALLABLE_IDX>(m_components);
                const size_t func_tpl_size = std::tuple_size_v<decltype(func_tpl)>;
                option = createOption<VType, 0>
                        (std::make_index_sequence<func_tpl_size>{}, std::move(func_tpl));
            }
        }
        setArgStrType(str_type);
        createArg(option);
    }
};

class argParser
{
public:
    explicit argParser(const std::string &name = "", const std::string &descr = ""){
        m_argMap[help_key] = std::unique_ptr<Argument>(new Argument(help_key));
        m_argMap[help_key]->m_help = "Show this message and exit. 'arg' to get help about certain argument";
        m_argMap[help_key]->m_options = {"[arg]"};
        m_argMap[help_key]->m_optional = true;
        m_argMap[help_key]->m_arg_handle = std::unique_ptr<ArgHandleBase>(new ArgHandleBase());
        m_argMap[help_key]->m_aliases = {help_alias};

        m_binary_name = name;
        m_description = descr;

        m_callback = []{}; // default callback
    }
    ~argParser(){
        m_argMap.clear();
    }

    argParser(const argParser&) = delete;
    argParser &operator=(const argParser&) = delete;
    argParser(argParser &&) = delete;
    argParser &operator=(argParser &&) = delete;

    // new arg builder
    template<typename T, typename... Keys>
    auto addArgument(Keys ...keys) {
        static_assert(sizeof...(keys) > 0, "Keys' set cannot be empty");
        static_assert((std::is_same_v<Keys, const char*> && ...), "Keys must be strings");
        // sanity checks
        auto checkKeys = [this, func=__func__](const char *k) {
            if (!k) {
                throw std::invalid_argument("Key cannot be null!");
            }
            auto skey = std::string(k);
            parser_internal::validateKeyOrParam(skey, /*is_param=*/false, func);
            checkDuplicates(skey, func);
            return skey;
        };
        //populate aliases
        auto aliases = std::vector<std::string>{checkKeys(keys)...};
        auto key = std::move(aliases.back());
        aliases.pop_back();

        bool flag = key.front() == '-';

        for(const auto &el : aliases){
            bool match = el.front() == '-';
            if (match != flag){
                throw std::invalid_argument(std::string(__func__) + ": " + key + ": cannot add alias " + el + ": different type");
            }
        }

        auto callback = [this,m_key=key](std::unique_ptr<Argument> &&arg) {
            m_argMap[m_key] = std::move(arg);
        };

        return ArgBuilder<0,0,false,T>(
                std::move(key),
                std::move(aliases),
                std::forward<decltype(callback)>(callback),
                std::make_tuple(T{})
        );
    }
    // add positional
    template<typename T>
    auto addPositional(const char *ckey) {
        std::string key = ckey;
        /// check if variadic pos already defined
        for(const auto &p : m_posMap){
            const auto &x = m_argMap.at(p);
            if(x->isVariadic()){
                throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot add positional argument after variadic positional argument " + p);
            }
            for(const auto &o : x->m_options){
                if(!parser_internal::isOptMandatory(o)){
                    throw std::invalid_argument(std::string(__func__) + ": " + key + " cannot add positional argument after positional argument with optional nargs " + p);
                }
            }
        }

        checkDuplicates(key, __func__);
        parser_internal::validateKeyOrParam(key, /*is_param=*/false, __func__);
        if(key.front() == '-'){
            throw std::invalid_argument(std::string(__func__) + ": " + key + " positional argument cannot start with '-'");
        }

        auto callback = [this,m_key=key](std::unique_ptr<Argument> &&arg) {
            m_argMap[m_key] = std::move(arg);
            m_posMap.push_back(m_key);
        };

        return ArgBuilder<0,0,true,T>(
                std::move(key),
                std::vector<std::string>(),
                std::forward<decltype(callback)>(callback),
                std::make_tuple(T{})
        ).parameters(ckey); // set single parameter for positional (for size)
    }

    template <typename C>
    argParser &setCallback(C &&callback){
        static_assert(std::is_invocable_r_v<void, C>, "Callback must be void()");
        m_callback = std::forward<C>(callback);
        return *this;
    }

    static void hiddenSecret(const std::string &secret){
        help_hidden_secret = secret;
    }

    template <typename T>
    T getValue(const std::string &key){
        parsedCheck(__func__);
        auto strType = parser_internal::GetTypeName<T>();
        auto &r = getArg(key);
        try{
            return std::any_cast<T>(r.m_arg_handle->get_any_val());
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
    static T scanValue(const char *value){
        if(value == nullptr){
            return T{};
        }
        return parser_internal::scan<T>(value);
    }

    /// Self exec name
    auto getSelfName(){
        parsedCheck(__func__);
        return this->m_binary_name;
    }

    argParser &addCommand(const std::string &name, const std::string &descr){
        parser_internal::validateKeyOrParam(name, /*is_param=*/false, __func__);

        if(!parser_internal::isOptMandatory(name)){
            throw std::invalid_argument(std::string(__func__) + ": " + name + " child command cannot be optional");
        }
        m_commandMap[name] = std::make_unique<argParser>(name, descr);
        return *m_commandMap.at(name);
    }

    /// Parse arguments
    int parseArgs(int argc, char *argv[])
    {
        if(m_args_parsed){
            throw parse_error("Repeated attempt to run " + std::string(__func__));
        }
        ///Retrieve binary self-name
        if(m_binary_name.empty()){
            std::string self_name = std::string(argv[0]);
            size_t pos = self_name.find_last_of("/\\"); // Handles both Windows and UNIX
            m_binary_name = (pos == std::string::npos) ? self_name : self_name.substr(pos + 1);
        }
        try{
            return parseArgs({argv + 1, argv + argc});
        }catch(const parse_error &e){
            std::cout << e.what() << std::endl;
            std::cout << "Try '" << help_key << "' for more information" << std::endl;
            throw;
        }
    }

    /// Returns true if arguments were fully parsed
    [[nodiscard]] bool parsed() const noexcept {
        return m_args_parsed;
    }

    const Argument &operator [] (const std::string &key) const { return getArg(key); }

    /// Custom exception class (unparsed parameters)
    class unparsed_param : public std::runtime_error{
        std::string key;
        std::vector<std::string> cli_params{};
    public:
        explicit unparsed_param(std::string _name, const std::string& msg, std::vector<std::string> _cli_params)
                : std::runtime_error(_name + " : " + msg),
                  key(std::move(_name)),
                  cli_params(std::move(_cli_params)) {}
        [[nodiscard]] const std::string &name() const noexcept { return key; }
        [[nodiscard]] const std::vector<std::string> &cli() const noexcept { return cli_params; }
    };
    /// Custom exception class (other parse errors)
    class parse_error : public std::runtime_error{
    public:
        explicit parse_error(const char *msg) : std::runtime_error(msg) {}
        explicit parse_error(const std::string& s) : std::runtime_error(s){}
    };

protected:
    friend class ArgBuilderBase;
    enum class IS_REQUIRED {
        DONT_CHECK,
        FALSE,
        TRUE
    };

    std::map<std::string, std::unique_ptr<Argument>> m_argMap;
    std::map<std::string, std::unique_ptr<argParser>> m_commandMap;
    std::vector<std::string> m_posMap;
    std::vector<std::string> m_argVec;
    std::function<void()> m_callback;

    std::string m_binary_name;
    std::string m_description;
    inline static std::string help_hidden_secret;
    inline static const std::string help_key = "--help";
    inline static const std::string help_alias = "-h";
    bool m_args_parsed = false;
    bool m_mandatory_option = false;
    bool m_command_parsed = false;
    int m_positional_args_parsed = 0;
    int m_unparsed_mandatory_positionals = 0;
    int m_mandatory_args = 0;
    int m_required_args = 0;
    int m_hidden_args = 0;
    int m_command_offset = 0;
    int m_parsed_mnd_args = 0;
    int m_parsed_required_args = 0;

    [[nodiscard]] Argument &getArg(const std::string &key) const {
        auto it = m_argMap.find(key);
        if (it == m_argMap.end()) {
            it = [&key, this]() {
                for(auto x = m_argMap.begin(); x != m_argMap.end(); ++x) {
                    const auto &aliases = x->second->m_aliases;
                    if (std::find(aliases.begin(), aliases.end(), key) != aliases.end()) {
                        return x;
                    }
                }
                return m_argMap.end();
            }();
        }
        if(it != m_argMap.end()){
            return *it->second;
        }
        throw std::invalid_argument(key + " not defined");
    }

    void parsedCheck(const char* func = nullptr) const {
        if(func == nullptr){
            func = __func__;
        }
        if(!m_args_parsed){
            throw std::runtime_error(std::string(func) +": Invalid call. Arguments not parsed yet");
        }
    }

    void checkDuplicates(const std::string &key, const char* func = nullptr){
        if(func == nullptr){
            func = __func__;
        }
        //Check previous definition
        if(m_argMap.find(key) != m_argMap.end()){
            throw std::invalid_argument(std::string(func) + ": " + std::string(key) + " already defined");
        }
        for(const auto &x : m_argMap){
            for(const auto &alias : x.second->m_aliases){
                if(alias == key){
                    throw std::invalid_argument(std::string(func) + ": " + std::string(key) + " already defined");
                }
            }
        }
    }

    size_t calculateMismatch (const std::string &target, const std::string &candidate) {
        // use Levenstein distance
        auto targetLen = target.length();
        auto candidateLen = candidate.length();
        // distanceTable[i][j] is the minimum number of edits (Levenstein distance) required to convert
        // the first i chars of target into the first j chars of candidate
        std::vector<std::vector<size_t>> distanceTable(targetLen + 1, std::vector<size_t>(candidateLen + 1));
        // base case - one string is empty
        // (to convert first i chars of target into empty string requires i deletions)
        for (auto i = 0; i <= targetLen; ++i) distanceTable[i][0] = i; // Default deletion cost
        for (auto j = 0; j <= candidateLen; ++j) distanceTable[0][j] = j; // Default insertion cost
        // populate distanceTable table
        for (auto tIdx = 1; tIdx <= targetLen; ++tIdx) {
            for (auto cIdx = 1; cIdx <= candidateLen; ++cIdx) {
                char targetChar = target[tIdx - 1];
                char candidateChar = candidate[cIdx - 1];
                if (targetChar == candidateChar) {
                    //if chars match, no edit needed, copy previous value
                    distanceTable[tIdx][cIdx] = distanceTable[tIdx - 1][cIdx - 1];
                } else {
                    // if don't match, figure out which operation is less costly
                    size_t insertionCost = distanceTable[tIdx][cIdx - 1] + 1;
                    size_t deletionCost = distanceTable[tIdx - 1][cIdx] + 1;
                    size_t substitutionCost = distanceTable[tIdx - 1][cIdx - 1] + 1;
                    distanceTable[tIdx][cIdx] = std::min({insertionCost,deletionCost,substitutionCost});
                }
                // check transpositions (swapped adjacent chars)
                if (tIdx > 1 && cIdx > 1) {
                    char prevTargetChar = target[tIdx-2];
                    char prevCandidateChar = candidate[cIdx-2];
                    if (targetChar == prevCandidateChar && candidateChar == prevTargetChar) {
                        size_t transpositionCost = distanceTable[tIdx-2][cIdx-2] + 1;
                        distanceTable[tIdx][cIdx] = std::min(distanceTable[tIdx][cIdx], transpositionCost);
                    }
                }
            }
        }
        // return result
        return distanceTable[targetLen][candidateLen];
    }

    size_t calculateLexMismatch (const std::string &s1, const std::string &s2) {
        auto minLen = std::min(s1.length(), s2.length());
        int distance = 0;
        for(int i = 0; i < minLen; ++i) {
            distance += std::abs(s1[i] - s2[i]);
        }
        // remaining chars (if one string is longer)
        distance += std::abs(int(s1.length() - s2.length()));
        return distance;
    };

    std::string closestKey (const std::string &name) {
        std::string closestMatch;
        auto minMismatch = std::string::npos;
        auto minLexMismatch = std::string::npos;

        auto findClosest = [&](size_t mismatch, size_t lexMismatch, const std::string &candidate) {
            // check lexicographical distance
            bool lex_less = mismatch == minMismatch && lexMismatch < minLexMismatch;
            if(mismatch < minMismatch || lex_less){
                minMismatch = mismatch;
                minLexMismatch = lexMismatch;
                closestMatch = candidate;
            }
            return mismatch == 0;
        };

        auto processEntry = [&](const std::string &key, const std::vector<std::string> &aliases) {
            auto mismatch = calculateMismatch(name, key);
            auto lexMismatch = calculateLexMismatch(name, key);
            for(const auto &al : aliases){
                mismatch = std::min(mismatch, calculateMismatch(name, al));
                lexMismatch = std::min(lexMismatch, calculateLexMismatch(name, al));
            }
            return findClosest(mismatch, lexMismatch, key);
        };
        // check arguments
        for(const auto &it : m_argMap){
            if(processEntry(it.first, it.second->m_aliases)){
                // early exit for optimal match
                return closestMatch;
            }
        }
        //check commands
        for(const auto &it : m_commandMap){
            if(processEntry(it.first, {})) {
                // early exit for optimal match
                return closestMatch;
            }
        }
        return minMismatch < 2 ? closestMatch : "";
    };

    [[nodiscard]] argParser* findChildByName (const std::string &key) const {
            const auto &it = m_commandMap.find(key);
            if (it != m_commandMap.end()) {
                return it->second.get();
            }
            return nullptr;
    }

    static bool parseHandleEqualsSign(std::string &pName, std::string &pValue) {
        auto c = pName.find('=');
        if(c != std::string::npos){
            pValue = pName.substr(c+1);
            pValue.insert(0, 1, '\0'); //add null to mark as value
            pName = pName.substr(0, c);
            return true;
        }
        return false;
    }

    bool parseHandleContiguousAndCombinedArgs(std::string &pName, std::string &pValue, std::string &name) {
        auto len = pName.front() == '-' ? 2 : 1;
        std::string startsWith = pName.substr(0, len);
        if(m_argMap.find(startsWith) != m_argMap.end()){
            name = startsWith;
        }else{
            name = findKeyByAlias(startsWith);
        }
        if(!name.empty()){
            const auto &x = m_argMap.at(name);
            if(x->m_implicit){
                // implicit contiguous (-vvv/-it or vvv/it style) argument
                //set '-' to other portion to extract it later
                pValue = x->m_starts_with_minus ? "-" : "";
                pValue += pName.substr(startsWith.length());
                return true;
            } else if(!x->m_positional && x->m_options.size() == 1){
                //check if it's a contiguous keyValue or aliasValue pair (-k123 or k123 style)
                //only for non-pos args with 1 option
                pValue = pName.substr(startsWith.length());
                pValue.insert(0, 1, '\0'); //add null to mark as value
                pName = name;
                return true;
            }
        }
        return false;
    }

    void parsePreprocessArgVec() {

        for(auto index = 0; index < m_argVec.size(); ++index){
            auto insertKeyValue = [this, &index](const auto &key, const auto &val){
                m_argVec[index] = key;
                m_argVec.insert(m_argVec.begin() + index + 1, val);
            };
            std::string pName = m_argVec[index];
            std::string pValue = index+1 >= m_argVec.size() ? "" : m_argVec[index + 1];

            ///Handle '='
            if (parseHandleEqualsSign(pName, pValue)) {
                //change current m_key and insert value to vector
                insertKeyValue(pName, pValue);
            }
            if(m_argMap.find(pName) == m_argMap.end()){
                ///Find alias
                std::string name = findKeyByAlias(pName);
                if (findChildByName(pName) != nullptr) {
                    // if found child, break
                    m_command_offset = m_argVec.size() - index;
                    break;
                } else if (!name.empty()) {
                    // change alias to key
                    pName = name;
                    m_argVec[index] = name;
                    index += m_argMap[name]->m_mandatory_options; //skip mandatory opts
                } else if (parseHandleContiguousAndCombinedArgs(pName, pValue, name)) {
                    ///check contiguous or combined arguments
                    insertKeyValue(name, pValue);
                }
            } else{
                // if found in argMap, skip mandatory opts
                index += m_argMap[pName]->m_mandatory_options;
            }
            if(pName == help_key){
                // if found help key, break
                break;
            }
        }
    }

    [[nodiscard]] int parseHandlePositional(int index) {
        const auto &pos_name = m_posMap[m_positional_args_parsed];
        const auto &pos_arg = m_argMap[pos_name];
        int opts_cnt = 0;
        auto nargs = pos_arg->getNargs();
        bool variadic = pos_arg->isVariadic();
        const auto next_cmd_idx = m_argVec.size() - m_command_offset;
        const auto next_arg_idx = findNextArg(index);
        if(nargs > 0 || variadic){
            auto cnt = index-1;
            while(++cnt < m_argVec.size()){
                bool found_command = cnt >= next_cmd_idx;
                bool found_next_arg = cnt >= next_arg_idx;
                bool nargs_handled = nargs > 0 && opts_cnt >= nargs && !variadic;
                if(found_command || found_next_arg || nargs_handled){
                    break;
                }
                ++opts_cnt;
            }
            if(opts_cnt < pos_arg->m_mandatory_options){
                throw parse_error(m_binary_name + ": not enough " + pos_name + " arguments");
            }
        }else{
            opts_cnt = 1;
        }
        m_unparsed_mandatory_positionals = std::max(0, m_unparsed_mandatory_positionals - opts_cnt);
        m_positional_args_parsed++;
        index += parseSingleArgument(pos_name, index, index+opts_cnt);
        return index;
    }

    [[nodiscard]] bool positionalsParsed() const {
        return m_positional_args_parsed == m_posMap.size();
    }

    [[nodiscard]] int parseHandleChildAndPositional(int index) {
        while(index < m_argVec.size()){
            /// Parse children
            auto child = findChildByName(m_argVec[index]);
            if(child != nullptr){
                ++index; //skip command name itself
                index += child->parseArgs({m_argVec.begin() + index, m_argVec.end()});
                m_command_parsed = true;
                break;
            }
            ///Try parsing positional args
            if(!positionalsParsed()){
                index = parseHandlePositional(index);
            } else {
                break;
            }
        }
        return index;
    }

    int findNextArg(int index) {
        for(auto i = index; i < m_argVec.size(); ++i) {
            const auto &arg = m_argMap.find(m_argVec[i]);
            if(arg != m_argMap.end() && !arg->second->m_positional){
                return i;
            }
        }
        return m_argVec.size();
    }

    int parseHandleKnownArg(int index, const std::string &pName) {
        const auto &arg = m_argMap[pName];
        if(arg->m_positional){
            return parseHandlePositional(index);
        }
        ///If non-repeatable and occurred again, throw error
        if(arg->m_set && !arg->m_repeatable){
            throw parse_error("Error: redefinition of non-repeatable arg " + std::string(pName));
        }

        int opts_cnt = 0;
        auto cnt = index;
        ++index; //skip current key

        const auto next_arg_idx = findNextArg(index);
        const auto next_cmd_idx = m_argVec.size() - m_command_offset;

        while(++cnt < m_argVec.size()){
            bool all_params_found = opts_cnt >= arg->m_options.size();
            bool all_mandatory_found_or_variadic = (opts_cnt >= arg->m_mandatory_options) || arg->isVariadic();
            bool is_next_key = (cnt >= next_arg_idx || cnt >= next_cmd_idx);
            bool will_be_insufficient_for_positionals = (next_cmd_idx - cnt) <= m_unparsed_mandatory_positionals;
            // if all options found, break
            if(!arg->isVariadic() && all_params_found)
                break;
            // leave space for positionals
            if(all_mandatory_found_or_variadic && will_be_insufficient_for_positionals)
                break;
            // check if next value is an arg or command key
            if(all_mandatory_found_or_variadic && is_next_key)
                break;

            ++opts_cnt;
        }

        if(opts_cnt < arg->m_mandatory_options){
            throw parse_error(std::string(pName) + " requires "
                              + std::to_string(arg->m_mandatory_options) + " parameters, but " + std::to_string(opts_cnt) + " were provided");
        }

        index += parseSingleArgument(pName, index, index + opts_cnt);
        return index;
    }

    std::string findKeyByAlias(const std::string &key) {
        for(const auto &x : m_argMap){
            if(x.first == key){
                return key;
            }
            for(const auto &el : x.second->m_aliases){
                if(el == key){
                    return x.first;
                }
            }
        }
        return "";
    }

    void setArgument(const std::string &pName) {
        auto &arg = m_argMap[pName];
        arg->m_set = true;
        //count mandatory/required options
        if(!arg->m_optional){
            m_parsed_mnd_args++;
        }else if(arg->m_required){
            m_parsed_required_args++;
        }
    }

    void checkParsedNonPos() {
        if(!m_mandatory_option){
            return;
        }
        if(m_parsed_mnd_args != m_mandatory_args){
            for(const auto &arg : m_argMap){
                if(!arg.second->m_optional && !arg.second->m_positional && !arg.second->m_set){
                    throw parse_error(arg.first + " not specified");
                }
            }
        }
        if(m_required_args > 0 && m_parsed_required_args < 1){
            throw parse_error(m_binary_name + ": missing required option (*)");
        }
    }

    void checkTypos(const std::string& pName) {
        auto candidate = closestKey(pName);
        if(!candidate.empty() && candidate != pName){
            const auto &arg = m_argMap.find(candidate);
            if (arg != m_argMap.end()) {
                // if it's a minus argument
                if(arg->second->m_starts_with_minus) {
                    throw parse_error("Unknown argument: " + std::string(pName) + ". Did you mean " + candidate + "?");
                }
            } else if(findChildByName(candidate)) {
                // if it's a command
                throw parse_error("Unknown command: " + std::string(pName) + ". Did you mean " + candidate + "?");
            }
        }
    }

    void setParseCounters() {
        //count mandatory/required arguments
        for(const auto &x : m_argMap){
            if(!x.second->m_optional
               && !x.second->m_positional){
                m_mandatory_args++;
            }else if(x.second->m_optional
                     && x.second->m_required){
                m_required_args++;
            }
            if(x.second->m_hidden){
                m_hidden_args++;
            }
        }
        for(const auto &x : m_posMap){
            const auto &arg = m_argMap.at(x);
            m_unparsed_mandatory_positionals += arg->m_mandatory_options;
        }

        m_mandatory_option = m_mandatory_args || m_required_args;
    }

    int parseSingleArgument(const std::string &key, int start, int end) {
        try{
            // remove \0 added while preparing
            for(int i = start; i < end && i < m_argVec.size(); ++i){
                if(!m_argVec[i].empty() && m_argVec[i].front() == '\0'){
                    m_argVec[i].erase(0,1);
                }
            }
            const std::string *ptr = start < m_argVec.size() ? &m_argVec.at(start) : nullptr;
            auto &arg = m_argMap[key];
            arg->m_arg_handle->action(ptr, end - start);
        }catch(std::exception &e){
            throw unparsed_param(key, e.what(), {m_argVec.begin() + start, m_argVec.begin() + end});
        }catch(...){
            throw unparsed_param(key, "unknown error", {m_argVec.begin() + start, m_argVec.begin() + end});
        }
        return end-start;
    }

    int parseArgs(std::vector<std::string> &&arg_vec) {
        m_argVec = std::move(arg_vec);
        setParseCounters();
        /// Preprocess argVec (handle '=', aliases, combined args, etc)
        parsePreprocessArgVec();
        /// Main parser loop
        int index = 0;
        while(index < m_argVec.size()){
            const auto &pName = m_argVec[index];
            const auto &pValue = index+1 >= m_argVec.size() ? "" : m_argVec[index + 1];
            ///If found unknown key
            if(m_argMap.find(pName) == m_argMap.end()){
                ///Check if it's an arg with a typo
                checkTypos(pName);
                /// Handle positional args and child parsers
                const auto before_pos = index;
                index = parseHandleChildAndPositional(before_pos);
                /// If we just parsed all positional args, continue
                if (index - before_pos > 0 && positionalsParsed()){
                    continue;
                }
                break;
            }
            ///Show help
            else if(pName == help_key){
                printHelp(pValue);
                exit(0);
            }
            else{
                ///Parse other types
                index = parseHandleKnownArg(index, pName);
                setArgument(pName);
            }
        }

        checkParsedNonPos();
        if(index < m_argVec.size()){
            throw parse_error(m_argVec[index] + ": unknown argument");
        }
        if(m_unparsed_mandatory_positionals > 0){
            throw parse_error(m_binary_name + ": not enough positional arguments provided");
        }
        if(!m_commandMap.empty() && !m_command_parsed){
            throw parse_error(m_binary_name + ": no command provided");
        }

        m_args_parsed = true;
        m_callback(); //run callback
        return index;
    }

    static std::string formatChoices(const std::unique_ptr<Argument> &arg) {
        const auto &choices = arg->m_arg_handle->get_str_choices();
        if (choices.empty()){
            return "";
        }
        std::string opt;
        for(auto i = 0; i < choices.size(); ++i){
            if(i > 0){
                opt += "|";
            }
            opt += choices[i];
        }
        return opt;
    }

    static std::string formatAliases(const std::unique_ptr<Argument> &arg) {
        std::string aliases;
        for(const auto &alias : arg->m_aliases){
            aliases += alias + ",";
        }
        return aliases;
    }

    static std::string formatVariadic(const std::string &opt) {
        return "[" + opt + "]...";
    }

    static std::string formatOptions(const std::unique_ptr<Argument> &arg) {
        std::string result;
        std::string choices_str = formatChoices(arg);
        for(const auto &option : arg->m_options) {
            auto formatted = [&choices_str, &option](){
                bool is_mandatory = parser_internal::isOptMandatory(option);
                if (!choices_str.empty()) {
                    return is_mandatory ? ("<" + choices_str + ">") : ("[" + choices_str + "]");
                }
                return is_mandatory ? ("<" + option + ">") : option;
            }();
            if(!formatted.empty()) {
                result += " " + formatted;
            }
        }
        if (arg->isVariadic()) {
            result += " " + formatVariadic(!choices_str.empty() ? choices_str : arg->m_nargs_var);
        }
        return result;
    }

    static std::string formatPositionalOptions(const std::unique_ptr<Argument> &arg) {
        std::string choices_str = formatChoices(arg);
        return choices_str.empty() ? "" : " {" + choices_str + "}";
    }

    [[nodiscard]] std::string getPositionalArgsUsage() const {
        std::string usage;
        for(const auto &posArg : m_posMap){
            const auto &details = m_argMap.at(posArg);
            std::string opt = details->m_nargs_var.empty() ? posArg : details->m_nargs_var;
            for(const auto &option : details->m_options){
                std::string tmp = opt;
                tmp = !parser_internal::isOptMandatory(option) ? ("[" + tmp + "]") : tmp;
                usage += " " + tmp;
            }
            if(details->isVariadic()){
                usage += " " + formatVariadic(opt);
            }
        }
        return usage;
    }

    [[nodiscard]] auto filterArgs(bool flag, bool hidden, IS_REQUIRED check_required) const {
        std::vector<decltype(m_argMap)::const_iterator> result;
        for (auto it = m_argMap.cbegin(); it != m_argMap.cend(); ++it){
            const auto &arg = it->second;
            bool req = check_required == IS_REQUIRED::DONT_CHECK
                       ? arg->m_required
                       : check_required == IS_REQUIRED::TRUE;
            bool condition = (arg->m_optional && !arg->m_required) == flag
                             && arg->m_hidden == hidden
                             && arg->m_required == req;
            if(condition) {
                result.push_back(it);
            }
        }
        return result;
    }

    [[nodiscard]] auto findArgument(const std::string &param) const {
        auto it = m_argMap.find(param);
        if (it != m_argMap.end()) {
            return it;
        }
        for (it = m_argMap.begin(); it != m_argMap.end(); ++it) {
            const auto &entry = *it;
            auto start = entry.second->m_aliases.begin();
            auto end = entry.second->m_aliases.end();
            if (std::find(start, end, param) != end) {
                return it;
            }
        }
        return m_argMap.end();
    }

    static void printParamDetails(decltype(m_argMap)::const_iterator it, bool notab = false) {
        std::string alias_str = formatAliases(it->second);
        std::string formatted_options = formatOptions(it->second);

        std::cout << (notab ? "" : "\t");
        std::cout << alias_str;
        std::cout << it->first;
        std::cout << formatted_options;
    }

    void printUsageHeader() const {
        if(!m_description.empty()){
            std::cout << m_description << std::endl;
        }
        std::cout << "Usage: " << m_binary_name
                  << (hasFlags() ? " [flags]..." : "")
                  << (hasMandatoryParameters() ? " parameters..." : "")
                  << getPositionalArgsUsage()
                  << (hasCommands() ? " command [<args>]" : "")
                  << std::endl;
    }

    void printCommandsUsage() const {
        if(hasCommands()){
            std::cout << "Commands:" << std::endl;
            for(const auto &child : m_commandMap){
                std::cout << "\t" << child.first << " : " << child.second->m_description << std::endl;
            }
        }
    }

    void printPositionalUsage() const {
        if(!m_posMap.empty()){
            std::cout << "Positional arguments:" << std::endl;
            for(const auto &x : m_posMap){
                std::cout << "\t" << x << formatPositionalOptions(m_argMap.at(x)) << " : " << m_argMap.at(x)->m_help << std::endl;
            }
        }
    }

    void printFlagsUsage(bool advanced) const {
        if(hasFlags()){
            std::cout << "Flags (optional):" << std::endl;
            printFilteredUsage(filterArgs(true, false, IS_REQUIRED::DONT_CHECK), advanced);
            if(advanced){
                //show hidden
                printFilteredUsage(filterArgs(true, true, IS_REQUIRED::DONT_CHECK), advanced);
            }
        }
    }

    void printMandatoryParametersUsage(bool advanced) const {
        if(hasMandatoryParameters()){
            std::cout << "Parameters (mandatory):" << std::endl;
            printFilteredUsage(filterArgs(false, false, IS_REQUIRED::FALSE), advanced); //show options without *
            printFilteredUsage(filterArgs(false, false, IS_REQUIRED::TRUE), advanced); //show options with *
            if(advanced){
                //show hidden
                printFilteredUsage(filterArgs(false, true, IS_REQUIRED::FALSE), advanced);
                printFilteredUsage(filterArgs(false, true, IS_REQUIRED::TRUE), advanced);
            }
        }
    }

    void printFilteredUsage(const std::vector<decltype(m_argMap)::const_iterator> &filtered_args, bool advanced) const {
        for (auto it: filtered_args) {
            const auto &arg = it->second;
            //skip hidden
            if(arg->m_hidden && !advanced) return;
            //skip positional
            if(arg->m_positional) return;

            printParamDetails(it);

            std::string default_str = arg->m_show_default ? arg->m_arg_handle->get_str_val() : "";
            default_str = !default_str.empty() ? " (default " + default_str + ")" : "";
            std::string repeatable_str = arg->m_repeatable ? " [repeatable]" : "";
            std::string required_str = arg->m_required ? (m_required_args > 1 ? " (*)" : "") : "";

            std::cout << " : ";
            std::cout << arg->m_help;
            std::cout << repeatable_str;
            std::cout << default_str;
            std::cout << required_str;
            std::cout << std::endl;
        }
    }

    void printHelpCommon(bool advanced) const {
        printUsageHeader();
        printCommandsUsage();
        printPositionalUsage();
        printFlagsUsage(advanced);
        printMandatoryParametersUsage(advanced);
    }

    void printHelpForParameter(const std::string &param) const {
        //param advanced help
        auto j = findArgument(param);
        if(j != m_argMap.end()){
            printParamDetails(j, true);
            std::cout << " : " << j->second->m_help << std::endl;
            std::cout << j->second->m_advanced_help << std::endl;
        }else{
            //look for child
            auto child = findChildByName(param);
            if(child) child->printHelp(); //calls exit() already
            std::cout << "Unknown parameter " + param << std::endl;
        }
    }

    [[nodiscard]] bool hasFlags() const {
        return std::any_of(m_argMap.begin(), m_argMap.end(), [](const auto &p) {
            return p.second->m_optional && !p.second->m_required;
        });
    }
    [[nodiscard]] bool hasMandatoryParameters() const {
        return std::any_of(m_argMap.begin(), m_argMap.end(), [](const auto &p) {
            return !p.second->m_positional && (!p.second->m_optional || p.second->m_required);
        });
    }
    [[nodiscard]] bool hasCommands() const {
        return !m_commandMap.empty();
    }

    void printHelp(const std::string &param = "") {
        if(!help_hidden_secret.empty() && param == help_hidden_secret){
            // show hidden arguments
            printHelpCommon(/*advanced=*/true);
        } else if(!param.empty()) {
            printHelpForParameter(param);
        } else {
            printHelpCommon(/*advanced=*/false);
        }
        if(m_required_args > 1){
            std::cout << "For options marked with (*): at least one such option should be provided" << std::endl;
        }
    }
};