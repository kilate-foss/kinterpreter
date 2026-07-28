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

#include <string>
#include <unordered_map>
#include <vector>

#include "object/o_int.hpp"
#include "object/object.hpp"

namespace kpp::obj
{

template <>
inline std::string obj_cast<std::string>(object const &value)
{
    return value.repr();
}

struct string_object : public object
{
    explicit string_object(std::string value)
        : object{get_string_type()},
          m_value(std::move(value))
    {
        auto ms = get_str_methods();
        for (auto const &[k, v] : ms)
            methods.insert_or_assign(k, v);
    }

    std::string get_name(void) const override
    {
        return "string";
    }

    std::string to_str() const override
    {
        return repr();
    }

    std::string repr() const override
    {
        return m_value;
    }

    std::string const &get_value() const
    {
        return m_value;
    }

    static auto get_str_methods() -> std::unordered_map<std::string, object_ref<object>>
    {
        static std::unordered_map<std::string, object_ref<object>> methods{
            {"get_len",
             make_method("get_len",
                         [](std::vector<object_ref<object>> const &args) -> object_ref<object>
                         {
                             return make_object<int_object>(
                                 obj_cast<std::string>(*args.at(0)).size());
                         })}};
        return methods;
    }

private:
    std::string m_value;
};

} // namespace kpp::obj
