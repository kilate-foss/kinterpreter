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
#include <variant>
#include <vector>

#include <ffi.h>

#include "error.hpp"
#include "ffi/type.hpp"
#include "object/object.hpp"
#include "object/o_int.hpp"
#include "object/o_string.hpp"

namespace kpp::ffi
{

struct fn
{
    fn(std::vector<c_type> params_types_, c_type ret_type_, void *sym_)
        : m_sym(sym_),
          ret_type(ret_type_),
          m_params_types(std::move(params_types_))
    {
        if (!m_sym)
            panic("sym is nullptr");

        for (auto t : m_params_types)
            m_arg_types.push_back(to_ffi_type(t));

        if (ffi_prep_cif(&m_cif,
                         FFI_DEFAULT_ABI,
                         static_cast<unsigned>(m_arg_types.size()),
                         to_ffi_type(ret_type),
                         m_arg_types.data()) != FFI_OK)
        {
            panic("ffi_prep_cif failed");
        }
    }

    obj::object_ref<object> call(std::vector<obj::object_ref<object>> const &args)
    {
        if (args.size() != m_params_types.size())
            panic("expected {} arguments, got {}", m_params_types.size(), args.size());

        m_storage.clear();
        m_strings.clear();
        m_values.clear();

        m_storage.reserve(args.size());
        m_strings.reserve(args.size());
        m_values.reserve(args.size());

        for (size_t i = 0; i < args.size(); ++i)
        {
            switch (m_params_types[i])
            {
            case c_type::sint:
            {
                m_storage.emplace_back(obj::obj_cast<int>(*args[i]));

                m_values.push_back(&std::get<int32_t>(m_storage.back()));

                break;
            }

            case c_type::pointer:
            {
                m_strings.emplace_back(obj::obj_cast<std::string>(*args[i]));

                char *ptr = m_strings.back().data();

                m_storage.emplace_back(ptr);

                m_values.push_back(&std::get<char *>(m_storage.back()));

                break;
            }

            default:
                panic("unsupported ffi parameter");
            }
        }

        switch (ret_type)
        {
        case c_type::void_:
        {
            ffi_call(&m_cif, FFI_FN(m_sym), nullptr, m_values.data());

            return {};
        }

        case c_type::sint:
        {
            int32_t ret{};

            ffi_call(&m_cif, FFI_FN(m_sym), &ret, m_values.data());

            return obj::make_object<obj::int_object>(ret);
        }

        case c_type::pointer:
        {
            char *ret{};

            ffi_call(&m_cif, FFI_FN(m_sym), &ret, m_values.data());

            if (!ret)
                return {};

            return obj::make_object<obj::string_object>(ret);
        }

        default:
            panic("unsupported ffi return type");
        }
    }

private:
    using storage_t = std::variant<int32_t, char *>;

    void *m_sym{};
    ffi_cif m_cif{};

    c_type ret_type{};

    std::vector<c_type> m_params_types;
    std::vector<ffi_type *> m_arg_types;

    std::vector<storage_t> m_storage;
    std::vector<std::string> m_strings;
    std::vector<void *> m_values;
};

} // namespace kpp::ffi
