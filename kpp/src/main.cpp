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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <vector>

#include "error.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

static int g_argc = -1;
static char **g_argv = nullptr;

static bool g_dump = false;

static void compile(std::string const &name)
{
    namespace fs = std::filesystem;
    if (!fs::exists(fs::current_path() / name))
        panic("provided file {} not exists!", name);

    auto filedir = fs::current_path() / name;

    std::ifstream ifs{filedir, std::ios::in};
    if (!ifs.is_open())
        panic("cannot open file {}", filedir.string());

    std::string file_content, line;
    while (std::getline(ifs, line))
        file_content.append(line + '\n');

    kpp::file_info fi{filedir, file_content};
    kpp::lexer l{fi};
    fi = l.tokenize();

    kpp::parser p{fi};
    auto program = p.parse();

    if (g_dump)
    {
        std::println("tokens: ");
        for (auto &token : fi.tokens)
            std::println("{}", token);

        std::println("expressions: ");
        for (auto &expr : program.body)
            std::println("{}", *expr);

        std::exit(0);
    }

    kpp::interpreter i{std::move(program)};
    auto ret = i.run();
    (void)ret;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
        panic("you must provide at least one argument");

    ::g_argc = argc;
    ::g_argv = argv;

    auto op = std::string{argv[1]};
    if (op == "compile" || op == "c")
    {
        std::vector<std::string> files;

        for (auto i{2}; i < argc; i++)
        {
            auto value = std::string{argv[i]};
            if (value.starts_with('-'))
            {
                value.erase(0, 1);
                if (value == "d" || value == "dump")
                    g_dump = true;
            }
            else
                files.push_back(argv[i]);
        }
        for (auto &file : files)
        {
            if (files.size() > 1)
                std::println("{}:", file);
            compile(file);
        }
    }
}
