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

#include <string>
#include <dlfcn.h>

namespace kpp::ffi
{

struct so
{
    so(std::string const &name, int flags = RTLD_LAZY)
    {

        auto handle = dlopen(name.c_str(), RTLD_LAZY);
        if (!handle)
            panic("cannot open library {} due to {}", name, get_error());
        m_handle = handle;
    }

    ~so()
    {
        if (m_handle)
            dlclose(m_handle);
    }

    so(const so &other) = delete;
    so &operator=(const so &other) = delete;

    so(so &&other)
    {
        if (this == &other)
            return;

        this->m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    so &operator=(so &&other)
    {
        if (this == &other)
            return *this;
        if (m_handle)
            dlclose(m_handle);
        this->m_handle = other.m_handle;
        other.m_handle = nullptr;
        return *this;
    }

    static inline std::string get_error(void)
    {
        return dlerror();
    }

    void *sym(std::string const &name)
    {
        void *sym = dlsym(m_handle, name.c_str());
        return sym;
    }

private:
    void *m_handle;
};

}