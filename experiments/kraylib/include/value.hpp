#pragma once

#include <cassert>
#include <exception>
#include <iostream>
#include <string>
#include <type_traits>

#include <kilate/native.h>
#include <kilate/node.h>

#include "node.hpp"

namespace kilate::value
{

template <typename T>
struct value_traits;

template <typename T>
concept Integral = std::is_integral_v<T>;

template <Integral T>
struct value_traits<T>
{
    static T get(value_t *v)
    {
        switch (v->type)
        {
        case NODE_VALUE_TYPE_UINT:
            return static_cast<T>(v->u);
        case NODE_VALUE_TYPE_INT:
            return static_cast<T>(v->i);
        case NODE_VALUE_TYPE_LONG:
            return static_cast<T>(v->l);
        default:
            assert(false);
            return {};
        }
    }
};

template <>
struct value_traits<char *>
{
    static inline constexpr auto kind{NODE_VALUE_TYPE_STRING};

    static char *get(value_t *v)
    {
        assert(v->type == kind);
        return v->s;
    }
};

inline static safe_value_t get_safe_value(interpreter_t *i, const arg_node_t *arg,
                                          const std::string &err_msg)
{
    if (!i)
        return {};
    if (!arg)
    {
        std::cout << "runtime error: " << err_msg;
        std::terminate();
    }
    return get_safe_value(i, (arg_node_t *)arg);
}

template <typename T>
T get_value(interpreter_t *inter, const arg_node_t *arg, const std::string &err_msg = {})
{
    auto sv = get_safe_value(inter, arg, err_msg);
    return value_traits<T>::get(&sv.value);
}

template <typename T>
T get_arg(native_fndata_t *d, std::size_t idx, const std::string &msg = {})
{
    node::vector args{d->args};
    return get_value<T>(d->inter, args[idx], msg);
}

} // namespace kilate::value
