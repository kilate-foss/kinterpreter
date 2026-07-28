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

#include "token.hpp"

#include <ostream>
#include <string>

namespace kpp
{
    
std::string kind_to_string(const token::kind &kind)
{
    switch (kind)
    {
    case token::kind::eof:
        return "eof";
    case token::kind::keyword:
        return "keyword";
    case token::kind::identifier:
        return "identifier";
    case token::kind::directive:
        return "directive";
    case token::kind::equals:
        return "equals";
    case token::kind::open_paren:
        return "open_paren";
    case token::kind::close_paren:
        return "close_paren";
    case token::kind::star:
        return "star";
    case token::kind::arrow:
        return "arrow";
    case token::kind::colon:
        return "colon";
    case token::kind::comma:
        return "comma";
    case token::kind::open_brace:
        return "open_brace";
    case token::kind::close_brace:
        return "close_brace";
    case token::kind::dot:
        return "dot";
    case token::kind::string_lit:
        return "string";
    case token::kind::number_lit:
        return "number";
    case token::kind::bool_lit:
        return "bool";
    };
    return "???";
}

std::ostream &operator<<(std::ostream &os, const token::kind &kind)
{
    os << kind_to_string(kind);
    return os;
}

std::ostream &operator<<(std::ostream &os, const token &obj)
{
    os << "Token {" << std::endl;
    os << "  kind: " << obj.get_kind() << std::endl;
    os << "  value: " << obj.get_value() << std::endl;
    os << "  line: " << obj.get_line() << std::endl;
    os << "  column: " << obj.get_column() << std::endl;
    os << "}" << std::endl;
    return os;
}

}
