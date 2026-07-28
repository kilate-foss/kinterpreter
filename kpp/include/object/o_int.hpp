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

#include "object/object.hpp"

namespace kpp::obj
{

struct int_object : public object
{

    explicit int_object(int value)
        : object{get_int_type()},
          m_value(value)
    {
    }

    std::string get_name(void) const override
    {
        return "int";
    }

    std::string to_str() const override
    {
        return std::to_string(m_value);
    }

    std::string repr() const override
    {
        return std::to_string(m_value);
    }

    int get_value() const
    {
        return m_value;
    }

private:
    int m_value;
};

template <>
inline int obj_cast<int>(object const &value)
{
    return static_cast<int_object const &>(value).get_value();
}

} // namespace kpp::obj
