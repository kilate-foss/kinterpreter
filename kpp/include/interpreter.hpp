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

#include <cstddef>
#include <expected>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "function.hpp"
#include "object/object.hpp"

namespace kpp
{

class scope
{
public:
    void define(std::string name, obj::object_ref<object> obj)
    {
        m_values.emplace(std::move(name), std::move(obj));
    }

    std::expected<void, std::string> set(std::string const &name, obj::object_ref<object> obj)
    {
        auto it = m_values.find(name);
        if (it == m_values.end())
            return std::unexpected{std::format("no object {} in current scopes", name)};

        it->second = std::move(obj);
        return {};
    }

    bool contains(std::string const &name) const
    {
        return m_values.contains(name);
    }

    obj::object_ref<object> &get(std::string const &name)
    {
        return m_values.at(name);
    }

private:
    std::unordered_map<std::string, obj::object_ref<object>> m_values;
};

struct interpreter : ast::expr_visitor
{
    interpreter(ast::program);

    obj::object_ref<object> run(void);

private:
    enum class context
    {
        default_ = 0,
        call
    };

    ast::program m_program;
    std::vector<scope> m_scopes;

    scope &current_scope(void)
    {
        return m_scopes.back();
    }

    std::expected<obj::object_ref<object>, std::string> lookup(std::string const &);
    std::expected<obj::object_ref<object>, std::string> assign(std::string const &,
                                                               obj::object_ref<object>);

    obj::object_ref<object> call_function(function const &, std::vector<obj::object_ref<object>>);
    obj::object_ref<object> run_block(ast::block_expr const &);

    obj::object_ref<object> visit(ast::literal_expr<std::string> const &) override;
    obj::object_ref<object> visit(ast::literal_expr<int> const &) override;
    obj::object_ref<object> visit(ast::literal_expr<bool> const &) override;
    obj::object_ref<object> visit(ast::literal_expr<std::nullptr_t> const &) override;
    obj::object_ref<object> visit(ast::block_expr const &) override;
    obj::object_ref<object> visit(ast::function_expr const &) override;
    obj::object_ref<object> visit(ast::call_expr const &) override;
    obj::object_ref<object> visit(ast::directive_expr const &) override;
    obj::object_ref<object> visit(ast::var_decl_expr const &) override;
    obj::object_ref<object> visit(ast::return_expr const &) override;
    obj::object_ref<object> visit(ast::binary_expr const &) override;
    obj::object_ref<object> visit(ast::assignment_expr const &) override;
    obj::object_ref<object> visit(ast::identifier_expr const &) override;
    obj::object_ref<object> visit(ast::member_access_expr const &) override;
    obj::object_ref<object> visit(ast::namespace_access_expr const &) override;
    obj::object_ref<object> visit(ast::aggregate_expr const &) override;
    obj::object_ref<object> visit(ast::struct_aggregate_expr const &) override;
    obj::object_ref<object> visit(ast::namespace_aggregate_expr const &) override;
    obj::object_ref<object> visit(ast::extern_decl_expr const &) override;
};

} // namespace kpp
