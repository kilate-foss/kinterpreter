/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 Aquiles Trindade
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <expected>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace kpp
{

namespace ast
{
struct block_expr;
}

struct object;

namespace obj
{

struct type_object;

template <typename T>
T obj_cast(object const &);

template <typename T>
using object_ref = std::shared_ptr<T>;

template <std::derived_from<object> T, typename... Args>
object_ref<T> make_object(Args &&...args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace obj

struct object
{
    virtual ~object(void) = default;

    virtual std::string get_name() const = 0;
    virtual std::string to_str(void) const = 0;
    virtual std::string repr(void) const = 0;

    auto get_method(std::string const &) -> std::expected<obj::object_ref<object>, std::string>;
    auto get_property(std::string const &) -> std::expected<obj::object_ref<object>, std::string>;

    void define_method(std::string const &name, obj::object_ref<object> m)
    {
        methods[name] = m;
    }

    void define_property(std::string const &name, obj::object_ref<object> p)
    {
        properties[name] = p;
    }

    auto set_property(std::string const &name, obj::object_ref<object> &value)
        -> std::expected<void, std::string>
    {
        auto it = properties.find(name);

        if (it == properties.end())
            return std::unexpected{std::format("property {} not defined", name)};

        it->second = std::move(value);
        return {};
    }

    auto get_type(void) -> obj::object_ref<obj::type_object>
    {
        return m_type;
    }

    void set_type(obj::object_ref<obj::type_object> type_)
    {
        m_type = type_;
    }

protected:
    auto find_property(std::string const &name)
        -> std::unordered_map<std::string, obj::object_ref<object>>::iterator
    {
        return properties.find(name);
    }

    auto find_method(std::string const &name)
        -> std::unordered_map<std::string, obj::object_ref<object>>::iterator
    {
        return methods.find(name);
    }

    auto properties_end() -> std::unordered_map<std::string, obj::object_ref<object>>::iterator
    {
        return properties.end();
    }

    auto methods_end() -> std::unordered_map<std::string, obj::object_ref<object>>::iterator
    {
        return methods.end();
    }

protected:
    explicit object(obj::object_ref<obj::type_object>);

    obj::object_ref<obj::type_object> m_type;
    std::unordered_map<std::string, obj::object_ref<object>> methods;
    std::unordered_map<std::string, obj::object_ref<object>> properties;
};

namespace obj
{

struct type_object : public object
{
    std::string name;

    /** static methods from the class, they're called from the type and not from the instance
     *  for example:
     *    int.from(<string>)
     */
    std::unordered_map<std::string, obj::object_ref<object>> static_methods;

    /** static property from the class, they're called from the type and not from the instance
     *  for example:
     *    int.MAX
     */
    std::unordered_map<std::string, obj::object_ref<object>> static_properties;

    type_object(std::string name_, object_ref<type_object> meta_)
        : object{meta_},
          name{name_}
    {
    }

    std::string get_name(void) const override
    {
        return name;
    }

    std::string to_str(void) const override
    {
        return get_name();
    }

    std::string repr(void) const override
    {
        return std::format("<type {}>", get_name());
    }

    auto get_static_method(std::string const &)
        -> std::expected<obj::object_ref<object>, std::string>;
    auto get_static_property(std::string const &)
        -> std::expected<obj::object_ref<object>, std::string>;

    void define_static_method(std::string const &name, obj::object_ref<object>);
    void define_static_property(std::string const &name, obj::object_ref<object>);

    auto set_static_property(std::string const &, obj::object_ref<object>)
        -> std::expected<void, std::string>;

    static void register_methods(object_ref<type_object>);
};

inline object_ref<type_object> _create(std::string name, object_ref<type_object> meta)
{
    return make_object<type_object>(name, meta);
}

struct _types
{
    std::unordered_map<std::string, object_ref<type_object>> builtin_types;
    std::unordered_map<std::string, object_ref<type_object>> user_types;

    static auto create() -> _types;
};

extern _types deftypes;

static inline auto get_builtin_type(std::string const &name)
    -> std::expected<object_ref<type_object>, std::string>
{
    auto t = deftypes.builtin_types.find(name);
    if (t != deftypes.builtin_types.end())
        return t->second;
    return std::unexpected{std::format("no built-in type named {}", name)};
}

static inline auto get_user_type(std::string const &name)
    -> std::expected<object_ref<type_object>, std::string>
{
    auto t = deftypes.user_types.find(name);
    if (t != deftypes.user_types.end())
        return t->second;
    return std::unexpected{std::format("no user type named {}", name)};
}

static inline object_ref<type_object> get_type_type()
{
    return get_builtin_type("type").value_or(nullptr);
}

static inline object_ref<type_object> get_null_type()
{
    return get_builtin_type("null").value_or(nullptr);
}

static inline object_ref<type_object> get_int_type()
{
    return get_builtin_type("int").value_or(nullptr);
}

static inline object_ref<type_object> get_bool_type()
{
    return get_builtin_type("bool").value_or(nullptr);
}

static inline object_ref<type_object> get_string_type()
{
    return get_builtin_type("string").value_or(nullptr);
}

static inline object_ref<type_object> get_block_type()
{
    return get_builtin_type("block").value_or(nullptr);
}

static inline object_ref<type_object> get_function_type()
{
    return get_builtin_type("function").value_or(nullptr);
}

static inline object_ref<type_object> get_method_type()
{
    return get_builtin_type("method").value_or(nullptr);
}

static inline object_ref<type_object> get_struct_type()
{
    return get_builtin_type("struct").value_or(nullptr);
}

static inline auto get_type(std::string const &name)
    -> std::expected<object_ref<object>, std::string>
{
    if (auto b = get_builtin_type(name))
        return b;
    return get_user_type(name);
}

auto make_method(object_ref<object> self,
                 std::string const &name,
                 std::function<object_ref<object>(std::vector<object_ref<object>> const &)>)
    -> object_ref<object>;

inline auto
make_method(std::string const &name,
            std::function<object_ref<object>(std::vector<object_ref<object>> const &)> fn)
    -> object_ref<object>
{
    return make_method(nullptr, name, fn);
}

} // namespace obj

} // namespace kpp

template <>
struct std::formatter<kpp::obj::type_object> : std::formatter<std::string>
{
    auto format(kpp::obj::type_object const &to, format_context &ctx) const
    {
        return std::formatter<std::string>::format(to.repr(), ctx);
    }
};
