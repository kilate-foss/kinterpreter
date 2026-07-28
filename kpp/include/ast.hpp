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

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <format>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "ffi/type.hpp"
#include "object/object.hpp"

namespace kpp::ast
{

/** ahhh forward decls ☠️ */
template <typename T>
struct literal_expr;

struct block_expr;
struct function_expr;
struct call_expr;
struct directive_expr;
struct var_decl_expr;
struct return_expr;
struct binary_expr;
struct assignment_expr;
struct identifier_expr;
struct member_access_expr;
struct namespace_access_expr;
struct aggregate_expr;
struct struct_aggregate_expr;
struct namespace_aggregate_expr;
struct extern_decl_expr;

struct expr_visitor
{
    /** literals */
    virtual obj::object_ref<object> visit(literal_expr<std::string> const &) = 0;
    virtual obj::object_ref<object> visit(literal_expr<int> const &) = 0;
    virtual obj::object_ref<object> visit(literal_expr<bool> const &) = 0;
    virtual obj::object_ref<object> visit(literal_expr<std::nullptr_t> const &) = 0;

    virtual obj::object_ref<object> visit(block_expr const &) = 0;
    virtual obj::object_ref<object> visit(function_expr const &) = 0;
    virtual obj::object_ref<object> visit(call_expr const &) = 0;
    virtual obj::object_ref<object> visit(directive_expr const &) = 0;
    virtual obj::object_ref<object> visit(var_decl_expr const &) = 0;
    virtual obj::object_ref<object> visit(return_expr const &) = 0;
    virtual obj::object_ref<object> visit(binary_expr const &) = 0;
    virtual obj::object_ref<object> visit(assignment_expr const &) = 0;
    virtual obj::object_ref<object> visit(identifier_expr const &) = 0;
    virtual obj::object_ref<object> visit(member_access_expr const &) = 0;
    virtual obj::object_ref<object> visit(namespace_access_expr const &) = 0;
    virtual obj::object_ref<object> visit(aggregate_expr const &) = 0;
    virtual obj::object_ref<object> visit(struct_aggregate_expr const &) = 0;
    virtual obj::object_ref<object> visit(namespace_aggregate_expr const &) = 0;
    virtual obj::object_ref<object> visit(extern_decl_expr const &) = 0;
};

struct expr
{
    virtual ~expr() = default;
    virtual obj::object_ref<object> visit(expr_visitor &) const = 0;
    virtual void print_tree(std::string &, const std::string &, bool) const = 0;
};

template <typename T>
    requires std::derived_from<T, expr>
using expr_ref = std::unique_ptr<T>;

template <std::derived_from<expr> Type, typename... Args>
expr_ref<Type> make_expr(Args &&...args)
{
    return std::make_unique<Type>(std::forward<Args>(args)...);
}

inline void append_tree_prefix(std::string &out, const std::string &prefix, bool is_last)
{
    out += prefix;
    out += is_last ? "└── " : "├── ";
}

/** Represents a literal value
 *  For example:
 *    "Hello, World!" as std::string,
 *    10 as int,
 *    true (or false) as bool
 */
template <typename T>
struct literal_expr : expr
{
    T val;

    literal_expr(T value_)
        : val{std::move(value_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &, const std::string &, bool) const override;
};

template <>
inline void literal_expr<std::string>::print_tree(std::string &out,
                                                  const std::string &prefix,
                                                  bool is_last) const
{
    append_tree_prefix(out, prefix, is_last);

    out += std::format("string {}\n", val);
}

template <>
inline void
literal_expr<int>::print_tree(std::string &out, const std::string &prefix, bool is_last) const
{
    append_tree_prefix(out, prefix, is_last);

    out += std::format("int {}\n", val);
}

template <>
inline void
literal_expr<bool>::print_tree(std::string &out, const std::string &prefix, bool is_last) const
{
    append_tree_prefix(out, prefix, is_last);

    out += std::format("bool {}\n", val);
}

template <>
inline void
literal_expr<std::nullptr_t>::print_tree(std::string &out, const std::string &prefix, bool is_last) const
{
    append_tree_prefix(out, prefix, is_last);

    out += std::format("null\n");
}

/** Represents a Block in the code
 *  Blocks are denominated by braces.
 *  For example:
 *    {
 *      # code here
 *    }
 */
struct block_expr : expr
{
    std::vector<expr_ref<expr>> body;

    block_expr(std::vector<expr_ref<expr>> body_)
        : body{std::move(body_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("block \n");

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        for (auto i{0}; i < body.size(); ++i)
        {
            body[i]->print_tree(out, child_prefix, i == body.size() - 1);
        }
    }
};

/** Represents a function in the code
 *  functions start with 'work' keyword, and end with 'end' keyword
 *  They can have paramethers, declared between parenthesis
 *  For example:
 *    work main(param1, patam2)
 *      # body here
 *    end
 */

struct function_expr : expr
{
    std::string name;
    std::vector<std::string> params;
    expr_ref<block_expr> block;

    function_expr(std::string name_, std::vector<std::string> params_, expr_ref<block_expr> block_)
        : name{name_},
          params{std::move(params_)},
          block{std::move(block_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);

        out += std::format("function {}\n", name);

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        for (auto i{0}; i < params.size(); ++i)
        {
            append_tree_prefix(out, child_prefix, i == params.size() - 1);
            out += "param " + params.at(i) + "\n";
        }

        block->print_tree(out, child_prefix, true);
    }
};

/** Represents a function call in the code
 *  calls have the name of function, and the params, wrapped in parenthesis
 *  For example:
 *    print("Hello, World")
 */
struct call_expr : expr
{
    expr_ref<expr> callee;
    std::vector<expr_ref<expr>> args;

    call_expr(expr_ref<expr> callee_, std::vector<expr_ref<expr>> args_)
        : callee{std::move(callee_)},
          args{std::move(args_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);

        out += std::format("call \n");

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        callee->print_tree(out, child_prefix, false);
        for (std::size_t i = 0; i < args.size(); ++i)
        {
            args[i]->print_tree(out, child_prefix, i == args.size() - 1);
        }
    }
};

/** Represents a directive
 *  Directive are special statements
 *  For example:
 *    @import("android") is the import directive
 */
struct directive_expr : expr
{
    std::string name;
    expr_ref<expr> val;

    directive_expr(std::string name_, expr_ref<expr> value_)
        : name{std::move(name_)},
          val{std::move(value_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("directive {}\n", name);

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        val->print_tree(out, child_prefix, true);
    }
};

/** Represents a Variable Declaration
 *  a variable declaration starts with let keyword
 *  For example:
 *    let name = "Aquiles"
 */
struct var_decl_expr : expr
{
    std::string name;
    expr_ref<expr> val;

    var_decl_expr(std::string name_, expr_ref<expr> value_)
        : name{std::move(name_)},
          val{std::move(value_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("var decl {}\n", name);

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        val->print_tree(out, child_prefix, true);
    }
};

/** Represents a return statement in the code
 *  For example:
 *    return "Hi"
 */
struct return_expr : expr
{
    expr_ref<expr> val;

    return_expr(expr_ref<expr> value_)
        : val{std::move(value_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("return \n");

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        val->print_tree(out, child_prefix, true);
    }
};

/** Represents any Binary Expression
 *  A Binary Expr has Left and Right Exprs and Returns a Expr
 *  For example:
 *    5 + 5
 *    10 - 5
 */
struct binary_expr : expr
{
    enum class op_kind
    {
        add,
        sub,
        mul,
        div,
        mod,
        equals,
        not_equals,
        greater_than,
        less_than,
        greater_than_or_equal,
        less_than_or_equal
    };

    expr_ref<expr> left;
    expr_ref<expr> right;
    op_kind kind;

    binary_expr(expr_ref<expr> left_, expr_ref<expr> right_, op_kind kind_)
        : left{std::move(left_)},
          right{std::move(right_)},
          kind{kind_}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("binary expr \n");

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        left->print_tree(out, child_prefix, false);
        right->print_tree(out, child_prefix, true);
    }
};

/** Represents a Assignment Expression
 *  Assignment is composed by <l-value> = <expr>
 *  For example:
 *    name = "Aquiles"
 *    age = 15
 */
struct assignment_expr : expr
{
    expr_ref<expr> target;
    expr_ref<expr> val;

    assignment_expr(expr_ref<expr> target_, expr_ref<expr> value_)
        : target{std::move(target_)},
          val{std::move(value_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("assignment expr \n");

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        target->print_tree(out, child_prefix, false);
        val->print_tree(out, child_prefix, true);
    }
};

/** Represents a simple identifier
 *  For example:
 *    foo
 */
struct identifier_expr : expr
{
    std::string name;

    identifier_expr(std::string name_)
        : name{std::move(name_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);

        out += std::format("identifier {}\n", name);
    }
};

/** Represents a object member access
 *  For Example
 *    struct Foo
 *      work bar()
 *      end
 *    end
 *
 *    fun main()
 *      let foo = Foo.new() # also here
 *      foo.bar() # here
 *    end
 */
struct member_access_expr : expr
{
    expr_ref<expr> obj;
    std::string name;

    member_access_expr(expr_ref<expr> obj_, std::string name_)
        : obj{std::move(obj_)},
          name{std::move(name_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);

        out += "member access\n";

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        obj->print_tree(out, child_prefix, false);

        append_tree_prefix(out, child_prefix, true);
        out += std::format("{}\n", name);
    }
};

/** Represents a namespace member access
 *  For Example
 *    namespace Foo
 *      work bar()
 *      end
 *    end
 *
 *    fun main()
 *      Foo::bar()
 *    end
 */
struct namespace_access_expr : expr
{
    expr_ref<expr> obj;
    std::string name;

    namespace_access_expr(expr_ref<expr> obj_, std::string name_)
        : obj{std::move(obj_)},
          name{std::move(name_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);

        out += "namespace access\n";

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        obj->print_tree(out, child_prefix, false);

        append_tree_prefix(out, child_prefix, true);
        out += std::format("{}\n", name);
    }
};

/** Represents any Aggregate
 * For example:
 * # an Struct
 * struct People do
 *   let name = "Aquiles"
 *   let age = "0"
 *
 *   work new(name, age) do
 *   end
 * end
 *
 * or an Namespace
 * namespace Android do
 *   work people(name, age)
 *     return People.new(name, age)
 *   end
 * end
 */
struct aggregate_expr : expr
{
    expr_ref<block_expr> block;

    aggregate_expr(expr_ref<block_expr> block_)
        : block{std::move(block_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }
};

/** Represents an Namespace, see @aggregate_expr */
struct namespace_aggregate_expr : aggregate_expr
{
    std::string name;

    namespace_aggregate_expr(std::string name_, expr_ref<block_expr> block_)
        : aggregate_expr(std::move(block_)),
          name{name_}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("namespace {} \n", name);

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        block->print_tree(out, child_prefix, true);
    }
};

/** Represents an Struct, see @aggregate_expr */
struct struct_aggregate_expr : aggregate_expr
{
    std::string name;

    struct_aggregate_expr(std::string name_, expr_ref<block_expr> block_)
        : aggregate_expr(std::move(block_)),
          name{std::move(name_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("struct {} \n", name);

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        block->print_tree(out, child_prefix, true);
    }
};

/** Represents a extern function declaration
 * For Example:
 *   extern puts(char*) -> int from "c"
 */
struct extern_decl_expr : expr
{
    std::string name;
    ffi::c_type ret_type;
    std::vector<ffi::c_type> params_types;
    expr_ref<expr> lib; // from<

    extern_decl_expr(std::string name_,
                     ffi::c_type ret_type_,
                     std::vector<ffi::c_type> params_types_,
                     expr_ref<expr> lib_)
        : name{std::move(name_)},
          ret_type{std::move(ret_type_)},
          params_types{std::move(params_types_)},
          lib{std::move(lib_)}
    {
    }

    obj::object_ref<object> visit(expr_visitor &visitor) const override
    {
        return visitor.visit(*this);
    }

    void print_tree(std::string &out, const std::string &prefix, bool is_last) const override
    {
        append_tree_prefix(out, prefix, is_last);
        out += std::format("extern {} -> {}  \n", name, ffi::to_string(ret_type));

        auto child_prefix = prefix + (is_last ? "    " : "│   ");
        lib->print_tree(out, prefix, false);
        for (auto i{0}; i < params_types.size(); i++)
        {
            append_tree_prefix(out, child_prefix, i == params_types.size() - 1);
            out += "param " + to_string(params_types.at(i)) + "\n";
        }
    }
};

struct program
{
    std::vector<expr_ref<expr>> body;
};

inline std::ostream &operator<<(std::ostream &os, const expr &expr)
{
    std::string out;
    expr.print_tree(out, "", true);
    os << out;
    return os;
}

} // namespace kpp::ast

template <>
struct std::formatter<kpp::ast::expr> : std::formatter<std::string>
{
    auto format(const kpp::ast::expr &expr, format_context &ctx) const
    {
        std::string out;
        expr.print_tree(out, "", true);
        return std::formatter<std::string>::format(out, ctx);
    }
};
