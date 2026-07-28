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
#include <string>
#include <string_view>
#include <vector>

#include "token.hpp"

namespace kpp
{

struct file_info
{
    std::string filename;
    std::string content;
    std::vector<token> tokens;

    file_info(std::string filename_, std::string content_, std::vector<token> tokens_)
        : filename{std::move(filename_)},
          content{std::move(content_)},
          tokens{std::move(tokens_)}
    {
    }

    file_info(std::string filename_, std::string content_)
        : filename{std::move(filename_)},
          content{std::move(content_)},
          tokens{}
    {
    }
};

struct lexer
{
    lexer(file_info &fi)
        : m_fi{fi},
          m_pos{0},
          m_line{1},
          m_col{1}
    {
    }

    file_info &tokenize();

private:
    file_info &m_fi;
    std::size_t m_pos;
    std::size_t m_line;
    std::size_t m_col;
};

} // namespace kpp
