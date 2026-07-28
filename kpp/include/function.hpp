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
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "ffi/type.hpp"
#include "object/object.hpp"

namespace kpp
{

/** forward declares ast */
namespace ast
{
struct function_expr;
}

namespace ffi
{
struct fn;
}

struct native_function
{
    using callback_t =
        std::function<obj::object_ref<object>(std::vector<obj::object_ref<object>> const &)>;
    using callable_t = std::variant<std::shared_ptr<ffi::fn>, callback_t>;

    std::string name;
    callable_t callable;
    ffi::c_type ret_type;

    native_function(std::string name_, std::shared_ptr<ffi::fn> fn_, ffi::c_type ret_type_)
        : name(std::move(name_)),
          callable(fn_),
          ret_type(ret_type_)
    {
    }

    native_function(std::string name_, callback_t callback_)
        : name(std::move(name_)),
          callable(std::move(callback_)),
          ret_type(ffi::c_type::void_)
    {
    }

    bool is_ffi(void) const
    {
        return std::holds_alternative<std::shared_ptr<ffi::fn>>(callable);
    }

    auto get_ffi_fn(void) -> std::expected<std::shared_ptr<ffi::fn>, std::string>
    {
        if (is_ffi())
            return std::get<std::shared_ptr<ffi::fn>>(callable);
        return std::unexpected{"native function is not ffi."};
    }

    auto get_callback(void) -> std::expected<callback_t, std::string>
    {
        if (!is_ffi())
            return std::get<callback_t>(callable);
        return std::unexpected {"native function is not callback."};
    }
};

struct function
{
    function(ast::function_expr const *fn_)
        : m_value{fn_}
    {
    }

    function(std::shared_ptr<native_function> nf_)
        : m_value{nf_}
    {
    }

    bool is_common(void) const
    {
        return std::holds_alternative<ast::function_expr const *>(m_value);
    }

    bool is_native(void) const
    {
        return std::holds_alternative<std::shared_ptr<native_function>>(m_value);
    }

    ast::function_expr const *as_common(void) const
    {
        return std::get<ast::function_expr const *>(m_value);
    }

    std::shared_ptr<native_function> as_native(void) const
    {
        return std::get<std::shared_ptr<native_function>>(m_value);
    }

    std::string get_name(void) const;
    std::vector<std::string> const &get_params(void) const;

private:
    std::variant<ast::function_expr const *, std::shared_ptr<native_function>> m_value;
};

} // namespace kpp
