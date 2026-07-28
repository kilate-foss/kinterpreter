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

#include "function.hpp"

#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "error.hpp"

namespace kpp
{

std::string function::get_name(void) const
{
    if (is_common())
    {
        auto *expr = std::get<ast::function_expr const *>(m_value);
        return expr->name;
    }
    auto n = std::get<std::shared_ptr<native_function>>(m_value);
    return n->name;
}

std::vector<std::string> const &function::get_params(void) const
{
    if (!is_common())
        panic("native functions haven't params");

    auto *expr = as_common();
    return expr->params;
}

} // namespace kpp
