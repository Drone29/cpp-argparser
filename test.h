//
// Created by andrey on 20.12.24.
//

#ifndef ARGPARSER_TEST_H
#define ARGPARSER_TEST_H

#include <array>

class Base{
public:
    virtual void action(const std::initializer_list<const char*> &args) = 0;
    virtual ~Base() = default;
};

// Composite class using variadic inheritance
template <typename... Types>
class Composite : public Base {
public:
    std::tuple<Types...> data;
    void action(const std::initializer_list<const char*> &args) override {
        // get number of str args
        const auto str_args = std::get<1>(data); // arguments should always be first
        const auto args_num = std::tuple_size_v<decltype(str_args)>;
        // func + side args
        auto func_and_side_args = std::get<2>(data);
        auto func = std::get<0>(func_and_side_args); // function
        auto side_args = std::get<1>(func_and_side_args); // arguments in a tuple

        std::array<const char*, args_num> str_arr;
        int idx = 0;
        // populate array
        for(auto s : args){
            str_arr[idx++] = s;
        }
        // resulting tuple (side args + string array)
        auto tpl_res = std::tuple_cat(side_args, str_arr);
        //call function with result
        std::apply(func, tpl_res);
    }
    explicit Composite(Types... values)
            : data(std::move(values)...) {}
};

// Forward declaration for the builder
template <typename... Types>
class CompositeBuilder;

// Base builder type
class BasicBuilder {
public:
    template <typename A, typename... Keys>
    auto AddArgument(Keys ...keys) {
        static_assert(sizeof...(keys) > 0, "Keys' set cannot be empty");
        static_assert((std::is_same_v<Keys, const char*> && ...), "Keys must be strings");
        return CompositeBuilder<A>(
                std::vector<std::string>{keys...},
                std::make_tuple(A{})
                ); // create a composite builder
    }
};

template <typename... Types>
class CompositeBuilder {
private:
    std::tuple<Types...> components;
    std::vector<std::string> keys;

    template <std::size_t... Is>
    Base* finalizeHelper(std::index_sequence<Is...>) {
        return new Composite<Types...>(std::get<Is>(components)...);
    }
    template <typename NewType>
    auto add(NewType newComponent) {
        return CompositeBuilder<Types..., NewType>(
                std::move(keys),
                std::tuple_cat(components, std::make_tuple(newComponent)));
    }

public:
    // composite constructor
    explicit CompositeBuilder(std::vector<std::string> &&ks, std::tuple<Types...> &&comps)
            : keys(std::move(ks)),
            components(std::move(comps)) {}

    // add arguments
    template <typename... Args>
    auto SetArguments(Args ...strArgs) {
        static_assert((std::is_same_v<Args, const char*> && ...), "Args must be strings");
        return add(std::make_tuple(strArgs...)); // add arguments as a tuple
    }

    // add function and side args (if any)
    template <typename Callable, typename... IntArgs>
    auto SetCallable(Callable && callable, IntArgs ...intArgs) {
        static_assert(std::tuple_size_v<decltype(components)> == 2, "Call SetArguments() first");
        using funcType = std::conditional_t<std::is_function_v<Callable>, std::add_pointer_t<Callable>, Callable>;
        funcType func = std::forward<Callable>(callable);
        return add(std::make_tuple(
                func,
                std::make_tuple(intArgs...)
                ));
    }

    Base* Finalize() {
        return finalizeHelper(std::make_index_sequence<sizeof...(Types)>());
    }
};

#endif //ARGPARSER_TEST_H
