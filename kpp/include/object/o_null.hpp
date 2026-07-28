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

struct null_object : public object
{
    null_object(): object{get_null_type()}
    {
    }

    std::string get_name(void) const override
    {
        return "null";
    }

    std::string to_str() const override
    {
        return repr();
    }

    std::string repr() const override
    {
        return "null";
    }
};

template <>
inline std::nullptr_t obj_cast<std::nullptr_t>(object const &value)
{
    return nullptr;
}

} // namespace kpp::obj
