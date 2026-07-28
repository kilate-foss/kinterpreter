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

#include "function.hpp"
#include "object/object.hpp"

namespace kpp::obj
{

struct function_object : public object
{
    explicit function_object(function fn)
        : object{get_function_type()},
          m_value(std::move(fn))
    {
    }

    std::string get_name(void) const override
    {
        return get_value().get_name();
    }

    std::string to_str() const override
    {
        return m_value.get_name();
    }

    std::string repr() const override
    {
        return std::format("<function {}>", m_value.get_name());
    }

    function const &get_value() const
    {
        return m_value;
    }

private:
    function m_value;
};

template <>
inline function const &obj_cast<function const &>(object const &value)
{
    return static_cast<function_object const &>(value).get_value();
}

} // namespace kpp::obj
