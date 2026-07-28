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

#include "ast.hpp"
#include "object/object.hpp"

namespace kpp::obj
{

struct block_object : public object
{
    explicit block_object(ast::block_expr const *block)
        : object{get_block_type()},
          m_value(block)
    {
    }

    std::string get_name(void) const override
    {
        return "block";
    }

    std::string to_str() const override
    {
        return "<block>";
    }

    std::string repr() const override
    {
        return "<block>";
    }

    ast::block_expr const *get_value() const
    {
        return m_value;
    }

private:
    ast::block_expr const *m_value;
};

template <>
inline ast::block_expr const *obj_cast<ast::block_expr const *>(object const &value)
{
    return static_cast<block_object const &>(value).get_value();
}

} // namespace kpp::obj
