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

#include "interpreter.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <memory>
#include <print>
#include <string>
#include <vector>

#include "ast.hpp"
#include "error.hpp"
#include "ffi/fn.hpp"
#include "ffi/so.hpp"
#include "function.hpp"
#include "method.hpp"
#include "object/o_block.hpp"
#include "object/o_bool.hpp"
#include "object/o_function.hpp"
#include "object/o_int.hpp"
#include "object/o_method.hpp"
#include "object/o_null.hpp"
#include "object/o_string.hpp"
#include "object/o_struct.hpp"
#include "object/object.hpp"

namespace kpp
{

inline std::string upper_case(std::string s)
{
    std::transform(s.begin(),
                   s.end(),
                   s.begin(),
                   [](unsigned char c) -> char
                   {
                       return std::toupper(c);
                   });
    return s;
}

static std::string get_lib_full(std::string const &name)
{
#define prefix "lib"
#define sufix ".so"

    return prefix + name + sufix;
}

interpreter::interpreter(ast::program program_)
    : m_program{std::move(program_)}
{
    m_scopes.emplace_back();
}

obj::object_ref<object> interpreter::run(void)
{
    auto &body = m_program.body;
    if (body.empty())
        return obj::make_object<obj::null_object>();

    obj::object_ref<object> last;
    for (auto &expr : body)
        last = expr->visit(*this);

    return last;
}

std::expected<obj::object_ref<object>, std::string> interpreter::lookup(std::string const &name)
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); it++)
        if (it->contains(name))
            return it->get(name);
    return std::unexpected{std::format("undefined object: {}", name)};
}

std::expected<obj::object_ref<object>, std::string> interpreter::assign(std::string const &name,
                                                                        obj::object_ref<object> obj)
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); it++)
        if (it->contains(name))
            if (it->set(name, obj))
                return obj;

    return std::unexpected{std::format("undefined object: {}", name)};
}

obj::object_ref<object> interpreter::visit(ast::literal_expr<std::string> const &expr)
{
    return obj::make_object<obj::string_object>(expr.val);
}

obj::object_ref<object> interpreter::visit(ast::literal_expr<int> const &expr)
{
    return obj::make_object<obj::int_object>(expr.val);
}

obj::object_ref<object> interpreter::visit(ast::literal_expr<bool> const &expr)
{
    return obj::make_object<obj::bool_object>(expr.val);
}

obj::object_ref<object> interpreter::visit(ast::literal_expr<std::nullptr_t> const &expr)
{
    return obj::make_object<obj::null_object>();
}

obj::object_ref<object> interpreter::visit(ast::block_expr const &expr)
{
    return obj::make_object<obj::block_object>(&expr);
}

obj::object_ref<object> interpreter::visit(ast::function_expr const &expr)
{
    auto obj = obj::make_object<obj::function_object>(function{&expr});
    current_scope().define(expr.name, obj);
    return obj;
}

inline std::vector<obj::object_ref<object>>
to_objects(ast::expr_visitor &visitor, std::vector<ast::expr_ref<ast::expr>> const &args)
{
    std::vector<obj::object_ref<object>> vs;
    for (auto &expr : args)
    {
        auto v = expr->visit(visitor);
        vs.push_back(v);
    }
    return vs;
}

obj::object_ref<object> interpreter::run_block(ast::block_expr const &block)
{
    m_scopes.emplace_back();

    obj::object_ref<object> last;
    for (auto &e : block.body)
        /** the last expr is the return value of block */
        last = e->visit(*this);

    m_scopes.pop_back();

    return last;
}

obj::object_ref<object> interpreter::call_function(function const &fn,
                                                   std::vector<obj::object_ref<object>> args)
{
    /** run native function */
    if (fn.is_native())
    {
        auto nfn = fn.as_native();

        /** ffi is used when using extern declared function */
        if (auto ffi_fn = nfn->get_ffi_fn())
            return (ffi_fn.value())->call(args);

        if (auto cb = nfn->get_callback())
            return (cb.value())(args);
    }

    /** run kpp writeen functions */
    if (fn.is_common())
    {
        m_scopes.emplace_back();

        auto *cfn = fn.as_common();

        /** define arguments in scope */
        for (size_t i = 0; i < cfn->params.size(); ++i)
            current_scope().define(cfn->params[i], args[i]);

        return run_block(*cfn->block);
    }

    panic("invalid function");
}

obj::object_ref<object> interpreter::visit(ast::call_expr const &expr)
{
    auto callee = expr.callee->visit(*this);

    /** call function */
    if (callee->get_type() == obj::get_function_type())
    {
        auto args = to_objects(*this, expr.args);
        return call_function(obj::obj_cast<function const &>(*callee), std::move(args));
    }

    /** call method */
    if (callee->get_type() == obj::get_method_type())
    {
        auto const &mtd = obj::obj_cast<method const &>(*callee);

        auto args = to_objects(*this, expr.args);
        args.insert(args.begin(), mtd.self);

        return call_function(mtd.fn, std::move(args));
    }

    /** run block */
    if (callee->get_type() == obj::get_block_type())
    {
        auto const *block = obj::obj_cast<ast::block_expr const *>(*callee);
        return run_block(*block);
    }

    std::println("{}", *callee->get_type());

    todo();
}

obj::object_ref<object> interpreter::visit(ast::directive_expr const &expr)
{
    todo();
}

obj::object_ref<object> interpreter::visit(ast::var_decl_expr const &expr)
{
    auto val = expr.val->visit(*this);
    current_scope().define(expr.name, val);
    return val;
}

obj::object_ref<object> interpreter::visit(ast::return_expr const &expr)
{
    todo();
}

obj::object_ref<object> interpreter::visit(ast::binary_expr const &expr)
{
    todo();
}

obj::object_ref<object> interpreter::visit(ast::assignment_expr const &expr)
{
    auto value = expr.val->visit(*this);
    if (auto *id = dynamic_cast<ast::identifier_expr *>(expr.target.get()))
    {
        if (auto res = assign(id->name, value))
            return value;
        else
            panic("{}", res.error());
    }

    if (auto *member = dynamic_cast<ast::member_access_expr *>(expr.target.get()))
    {
        auto obj = member->obj->visit(*this);
        if (auto res = obj->set_property(member->name, value))
            return value;
        else
            panic("{}", res.error());
    }
    todo();
}

obj::object_ref<object> interpreter::visit(ast::identifier_expr const &expr)
{
    /** lookuo in the current scope */
    if (auto s = lookup(expr.name))
        return s.value();

    /** look in type table */
    if (auto t = obj::get_type(expr.name))
        return t.value();

    panic("undefined object or type {}", expr.name);
}

obj::object_ref<object> interpreter::visit(ast::member_access_expr const &expr)
{
    auto obj = expr.obj->visit(*this);

    if (auto mtd = obj->get_method(expr.name); mtd)
    {
        auto const &mt = obj::obj_cast<method const &>(*mtd.value());
        return obj::make_object<obj::method_object>(method{obj, mt.fn});
    }

    std::println("{}", *obj->get_type());
    if (obj->get_type() != obj::get_type_type())
    {
        if (auto mtd = obj->get_type()->get_method(expr.name); mtd)
        {
            auto const &mt = obj::obj_cast<method const &>(*mtd.value());
            return obj::make_object<obj::method_object>(method{obj, mt.fn});
        }
    }

    if (auto prop = obj->get_property(expr.name); prop)
        return prop.value();

    panic("neither property nor method found for name {}", expr.name);
}

obj::object_ref<object> interpreter::visit(ast::namespace_access_expr const &expr)
{
    auto obj = expr.obj->visit(*this);

    auto type = std::dynamic_pointer_cast<obj::type_object>(obj);
    if (!type)
        panic("'::' can only be used on types");

    /** static method */
    if (auto mtd = type->get_static_method(expr.name); mtd)
        return mtd.value();

    /** static property */
    if (auto prop = type->get_static_property(expr.name); prop)
        return prop.value();

    panic("neither static property nor static method found for name {}", expr.name);
}

obj::object_ref<object> interpreter::visit(ast::aggregate_expr const &expr)
{
    todo();
}

obj::object_ref<object> interpreter::visit(ast::struct_aggregate_expr const &expr)
{
    auto st_obj = obj::make_object<obj::struct_object>(&expr);
    auto struct_type = obj::make_object<obj::type_object>(expr.name, obj::get_type_type());

    for (auto &e : expr.block->body)
    {
        if (auto *fn = dynamic_cast<ast::function_expr *>(e.get()))
        {
            auto obj_fn = obj::make_object<obj::function_object>(function{fn});
            if (!fn->params.empty() && fn->params.at(0) == "self")
                struct_type->define_method(fn->name, obj_fn);
            else
                struct_type->define_static_method(fn->name, obj_fn);
        }
        else if (auto *var = dynamic_cast<ast::var_decl_expr *>(e.get()))
        {
            if (var->name != upper_case(var->name))
                st_obj->define_property(var->name, var->val->visit(*this));
            else
                struct_type->define_static_property(var->name, var->val->visit(*this));
        }
    }

    st_obj->set_type(struct_type);
    obj::deftypes.user_types[expr.name] = struct_type;

    return st_obj;
}

obj::object_ref<object> interpreter::visit(ast::namespace_aggregate_expr const &expr)
{
    todo();
}

obj::object_ref<object> interpreter::visit(ast::extern_decl_expr const &expr)
{
    /** load the library */
    auto from = expr.lib->visit(*this)->to_str();
    ffi::so so{get_lib_full(from)};

    /** get the symbol */
    auto sym = so.sym(expr.name);
    if (!sym)
        panic("no sym found for {} from {} due to {}", expr.name, from, ffi::so::get_error());

    auto f = std::make_shared<ffi::fn>(expr.params_types, expr.ret_type, sym);
    auto n = std::make_shared<native_function>(expr.name, f, expr.ret_type);

    auto obj = obj::make_object<obj::function_object>(n);
    current_scope().define(expr.name, obj);
    return obj;
}

} // namespace kpp
