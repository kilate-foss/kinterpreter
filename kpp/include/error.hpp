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

#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <utility>

namespace kpp::error
{

struct __file_info
{
    __file_info(const char *fl, const char *fc, int l)
        : file{fl},
          func{fc},
          line{l}
    {
    }
    const char *file;
    const char *func;
    int line;
};

[[noreturn]]
static __attribute__((unused)) void panic_n(const __file_info &f, const std::string &msg)
{
    std::cerr << "panic(" << f.file << ":" << f.func << ":" << f.line << "): " << msg << std::endl;
    std::exit(1);
}

[[noreturn]]
static __attribute__((unused)) void fatal(const std::string &msg)
{
    std::cerr << msg;
    std::exit(1);
}

#define panic(fmt, ...)                                                                            \
    kpp::error::panic_n({__FILE__, __func__, __LINE__}, fmt __VA_OPT__(, ) __VA_ARGS__)

#define todo() panic("TODO")

template <typename... Args>
[[noreturn]]
void panic_n(const __file_info &f, std::format_string<Args...> fmt, Args &&...args)
{
    panic_n(f, std::format(fmt, std::forward<Args>(args)...));
}

} // namespace lpx::error
