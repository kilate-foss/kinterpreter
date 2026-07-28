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

#include "lexer.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "error.hpp"
#include "token.hpp"

namespace kpp
{

file_info &lexer::tokenize()
{
    std::vector<token> tokens;

    auto advance = [&](std::size_t of = 1)
    {
        if (m_fi.content.at(m_pos) == '\n')
        {
            m_line++;
            m_col = 1;
        }
        else
        {
            m_col += of;
        }
        m_pos += of;
    };

    auto clen = m_fi.content.length();

    auto next = [&]() -> char
    {
        if (m_pos + 1 >= clen)
        {
            return '\0';
        }

        return m_fi.content.at(m_pos + 1);
    };

    auto read_string = [&]() -> std::pair<std::string, bool>
    {
        std::string buf;
        advance();
        while (m_pos < clen)
        {
            auto ch = m_fi.content.at(m_pos);
            if (ch == '\\')
            {
                advance();
                if (m_pos >= clen)
                    break;
                auto next = m_fi.content.at(m_pos);
                switch (next)
                {
                case 'n':
                    buf.append("\n");
                    break;
                case 't':
                    buf.append("\t");
                    break;
                case 'r':
                    buf.append("\r");
                    break;
                case '"':
                    buf.append("\"");
                    break;
                case '\\':
                    buf.append("\\");
                    break;
                default:
                    buf.append(std::string(1, next));
                    break;
                };
                advance();
            }
            else if (ch == '"')
            {
                advance();
                return {buf, true};
            }
            else
            {
                buf.append(std::string(1, ch));
                advance();
            }
        }
        return {buf, false};
    };

    auto push_token = [&](const auto &str, token::kind kind)
    {
        tokens.push_back({m_line, m_col, std::string{str}, kind});
    };

    while (m_pos < clen)
    {
        auto ch = m_fi.content.at(m_pos);
        if (std::isspace(ch))
        {
            advance();
            continue;
        }

        if (auto kind = token::symbols[static_cast<unsigned char>(ch)])
        {
            push_token(ch, *kind);
            advance();
            continue;
        }

        // strings
        if (ch == '"')
        {
            auto [str, closed] = read_string();
            if (!closed)
            {
                panic("Unclosed string literal at [{},{}]", m_line, m_col);
            }

            push_token(str, token::kind::string_lit);
            continue;
        }

        // comments
        else if (ch == '/' && next() == '/')
        {
            advance(2);
            while (m_pos < clen && m_fi.content.at(m_pos) != '\n')
                advance();
            continue;
        }
        else if (ch == '\n')
        {
            advance();
            continue;
        }
        else if (ch == ';')
        {
            advance();
            continue;
        }

        if (m_fi.content.substr(m_pos).starts_with("->"))
        {
            advance(2);
            push_token("->", token::kind::arrow);
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch)))
        {
            auto dot = false;
            auto start = m_pos;
            while (m_pos < clen)
            {
                auto ch = m_fi.content.at(m_pos);
                if (std::isdigit(static_cast<unsigned char>(ch)))
                    advance();
                else if (ch == '.' && !dot && (m_pos + 1 < clen) &&
                         std::isdigit(static_cast<unsigned char>(m_fi.content.at(m_pos + 1))))
                {
                    dot = true;
                    advance();
                }
                else
                    break;
            }

            auto num = m_fi.content.substr(start, m_pos - start);
            if (num.empty())
            {
                panic("Can't extract number at [{}, {}]", m_line, m_col);
            }

            push_token(num, token::kind::number_lit);
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_' || ch == '@')
        {
            auto start = m_pos;
            while (m_pos < clen &&
                   (std::isalpha(m_fi.content.at(m_pos)) || std::isdigit(m_fi.content.at(m_pos)) ||
                    m_fi.content.at(m_pos) == '_' || m_fi.content.at(m_pos) == '@'))
            {
                advance();
            }

            auto word = m_fi.content.substr(start, m_pos - start);
            if (word.empty())
            {
                panic("Can't get identifier at [{}, {}]", m_line, m_col);
            }

            if (token::is_keyword(word))
            {
                push_token(word, token::kind::keyword);
                continue;
            }
            else if (token::is_directive(word))
            {
                push_token(word.substr(1), token::kind::directive);
                continue;
            }
            else if (token::is_bool(word))
            {
                push_token(word, token::kind::bool_lit);
                continue;
            }
            else
            {
                push_token(word, token::kind::identifier);
                continue;
            }
        }
        panic("unexpected character {} at [{}, {}, pos = {}]", ch, m_line, m_col, m_pos);
    }
    push_token("", token::kind::eof);
    m_fi.tokens = tokens;
    return m_fi;
}

} // namespace kpp
