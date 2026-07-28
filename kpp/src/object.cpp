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

#include "object/object.hpp"
#include "function.hpp"
#include "method.hpp"
#include "object/o_method.hpp"
#include "object/o_null.hpp"
#include "object/o_string.hpp"

#include <memory>
#include <string>
#include <vector>

namespace kpp
{

object::object(obj::object_ref<obj::type_object> type_)
    : m_type{type_}
{
}

std::expected<obj::object_ref<object>, std::string>
obj::type_object::get_static_method(std::string const &name)
{
    if (m_type->static_methods.contains(name))
        return m_type->static_methods.at(name);
    return std::unexpected{std::format("static method {} not defined", name)};
}

std::expected<obj::object_ref<object>, std::string>
obj::type_object::get_static_property(std::string const &name)
{
    if (m_type->static_properties.contains(name))
        return m_type->static_properties.at(name);
    return std::unexpected{std::format("static property {} not defined", name)};
}

void obj::type_object::define_static_method(std::string const &name, obj::object_ref<object> m)
{
    m_type->static_methods[name] = m;
}

void obj::type_object::define_static_property(std::string const &name, obj::object_ref<object> p)
{
    m_type->static_properties[name] = p;
}

std::expected<void, std::string>
obj::type_object::set_static_property(std::string const &name, obj::object_ref<object> value)
{
    auto it = m_type->static_properties.find(name);

    if (it == m_type->static_properties.end())
        return std::unexpected{std::format("property {} not defined", name)};

    it->second = std::move(value);
    return {};
}

std::expected<obj::object_ref<object>, std::string> object::get_method(std::string const &name)
{
    if (auto it = find_method(name); it != methods_end())
        return it->second;

    return std::unexpected{std::format("undefined method: {}", name)};
}

std::expected<obj::object_ref<object>, std::string> object::get_property(std::string const &name)
{
    if (auto it = find_property(name); it != properties_end())
        return it->second;

    return std::unexpected{std::format("undefined variable: {}", name)};
}

/********** BEGIN _TYPES ***********/
obj::_types obj::deftypes = _types::create();

obj::_types obj::_types::create()
{
    /** helper to make a type obj */
    static auto _cr = [](auto const &name, auto meta) -> object_ref<type_object>
    {
        return make_object<type_object>(name, meta);
    };

    _types ts;

    /** create type */
    auto &base = ts.builtin_types["type"] = _cr("type", nullptr);
    base->set_type(base);

    ts.builtin_types["null"] = _cr("null", base);
    ts.builtin_types["int"] = _cr("int", base);
    ts.builtin_types["bool"] = _cr("bool", base);
    ts.builtin_types["string"] = _cr("string", base);
    ts.builtin_types["block"] = _cr("block", base);
    ts.builtin_types["function"] = _cr("function", base);
    ts.builtin_types["method"] = _cr("method", base);
    ts.builtin_types["struct"] = _cr("struct", base);

    type_object::register_methods(get_type_type());

    return ts;
}

/********** BEGIN TYPE OBJECT ******/

obj::object_ref<object> obj::make_method(
    obj::object_ref<object> self,
    std::string const &name,
    std::function<obj::object_ref<object>(std::vector<obj::object_ref<object>> const &)> fn)
{
    auto n = std::make_shared<native_function>(name, fn);
    auto f = function{n};
    auto m = method{self, std::move(f)};
    return obj::make_object<obj::method_object>(m);
};

void obj::type_object::register_methods(obj::object_ref<type_object> to)
{
    to->define_static_method(
        "new",
        make_method("new",
                    [](std::vector<object_ref<object>> const &args) -> object_ref<object>
                    {
                        return make_object<null_object>();
                    }));

    /** get_type instance method */
    to->define_method(
        "get_type",
        make_method(to,
                    "get_type",
                    [](std::vector<object_ref<object>> const &args) -> object_ref<object>
                    {
                        return args.at(0)->get_type();
                    }));

    /** get_type type method */
    to->define_static_method(
        "get_type",
        make_method(to,
                    "get_type",
                    [](std::vector<object_ref<object>> const &args) -> object_ref<object>
                    {
                        return args.at(0)->get_type();
                    }));

    /** to_str instance method */
    to->define_method(
        "to_str",
        make_method(to,
                    "to_str",
                    [](std::vector<object_ref<object>> const &args) -> object_ref<object>
                    {
                        return make_object<string_object>(args.at(0)->to_str());
                    }));
}

/********** END TYPE OBJECT ********/

} // namespace kpp
