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

#include <format>
#include <string>

#include "ast.hpp"
#include "object/object.hpp"

namespace kpp::obj
{

struct struct_object : public object
{
    explicit struct_object(ast::struct_aggregate_expr const *struct_)
        : object{get_struct_type()},
          m_value(struct_)
    {
    }

    std::string get_name(void) const override
    {
        return m_value->name;
    }

    std::string to_str() const override
    {
        return repr();
    }

    std::string repr() const override
    {
        return std::format("<struct {}>", m_value->name);
    }

    ast::struct_aggregate_expr const *get_value() const
    {
        return m_value;
    }

private:
    ast::struct_aggregate_expr const *m_value;
};

template <>
inline ast::struct_aggregate_expr const *obj_cast<ast::struct_aggregate_expr const *>(object const &value)
{
    return static_cast<struct_object const &>(value).get_value();
}

} // namespace kpp::obj
