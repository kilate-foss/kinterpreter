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
#include <format>
#include <ostream>
#include <string>

namespace kpp
{

struct token
{
    enum class kind
    {
        eof,
        keyword,
        identifier,
        directive,
        equals,
        open_paren,
        close_paren,
        star,
        arrow,
        comma,
        colon,
        open_brace,
        close_brace,
        dot,
        string_lit,
        number_lit,
        bool_lit,
    };

    token(std::size_t line, std::size_t col, const std::string &value, kind kind)
        : m_value{value},
          m_kind{kind},
          m_line{line},
          m_col{col}
    {
    }

    token(std::size_t line, std::size_t col, char ch, kind kind)
        : token{line, col, std::string{1, ch}, kind}
    {
    }

    std::string get_value() const
    {
        return m_value;
    }

    kind get_kind() const
    {
        return m_kind;
    }

    std::size_t get_line() const
    {
        return m_line;
    }

    std::size_t get_column() const
    {
        return m_col;
    }

    bool is_block_start(void) const
    {
        return (get_kind() == kind::keyword && get_value() == "do") ||
               (get_kind() == kind::open_brace);
    };

    bool is_block_end(void) const
    {
        return (get_kind() == kind::keyword && get_value() == "end") ||
               (get_kind() == kind::close_brace);
    }

    static constexpr auto symbols = []
    {
        std::array<std::optional<token::kind>, 256> arr{};
        arr['('] = token::kind::open_paren;
        arr[')'] = token::kind::close_paren;
        arr['='] = token::kind::equals;
        arr['*'] = token::kind::star;
        arr[':'] = token::kind::colon;
        arr[','] = token::kind::comma;
        arr['{'] = token::kind::open_brace;
        arr['}'] = token::kind::close_brace;
        arr['.'] = token::kind::dot;
        return arr;
    }();

    static constexpr auto is_keyword = [](const auto &word) -> bool
    {
        return (word == "work" || word == "end" || word == "let" || word == "return" ||
                word == "do" || word == "struct" || word == "namespace" || word == "extern" ||
                word == "from" || word == "null");
    };

    static constexpr auto is_directive = [](const auto &word) -> bool
    {
        return (word == "@import");
    };

    static constexpr auto is_bool = [](const auto &word) -> bool
    {
        return (word == "true" || word == "false");
    };

private:
    std::string m_value;
    kind m_kind;

    std::size_t m_line;
    std::size_t m_col;
};

std::string kind_to_string(const token::kind &);

std::ostream &operator<<(std::ostream &, const token::kind &);
std::ostream &operator<<(std::ostream &, const token &);

} // namespace kpp

template <>
struct std::formatter<kpp::token::kind> : std::formatter<std::string>
{
    auto format(const kpp::token::kind &kind, format_context &ctx) const
    {
        return std::formatter<std::string>::format(kind_to_string(kind), ctx);
    }
};

template <>
struct std::formatter<kpp::token> : std::formatter<std::string>
{
    auto format(const kpp::token &t, format_context &ctx) const
    {
        return formatter<std::string>::format(
            std::format(
                "[{}, {}, [{}, {}]]", t.get_value(), t.get_kind(), t.get_line(), t.get_column()),
            ctx);
    }
};
