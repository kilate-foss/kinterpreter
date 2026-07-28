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

#include <optional>
#include <string>

#include <ffi.h>

namespace kpp::ffi
{

enum class c_type
{
    void_,

    int8_t,
    int16_t,
    int32_t,
    int64_t,
    schar,
    sshort,
    slong,
    sint,

    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    uchar,
    ushort,
    ulong,
    uint,

    pointer
};

constexpr std::optional<c_type> to_c_type(std::string_view str)
{
    if (str == "void")
        return c_type::void_;

    if (str == "int8_t")
        return c_type::int8_t;
    if (str == "int16_t")
        return c_type::int16_t;
    if (str == "int32_t")
        return c_type::int32_t;
    if (str == "int64_t")
        return c_type::int64_t;

    if (str == "char")
        return c_type::schar;
    if (str == "short")
        return c_type::sshort;
    if (str == "int")
        return c_type::sint;
    if (str == "long")
        return c_type::slong;

    if (str == "uint8_t")
        return c_type::uint8_t;
    if (str == "uint16_t")
        return c_type::uint16_t;
    if (str == "uint32_t")
        return c_type::uint32_t;
    if (str == "uint64_t")
        return c_type::uint64_t;

    if (str == "uchar")
        return c_type::uchar;
    if (str == "ushort")
        return c_type::ushort;
    if (str == "uint")
        return c_type::uint;
    if (str == "ulong")
        return c_type::ulong;

    if (str == "pointer")
        return c_type::pointer;

    return std::nullopt;
}

constexpr std::string to_string(c_type type)
{
    switch (type)
    {
    case c_type::void_:
        return "void";

    case c_type::int8_t:
        return "int8_t";
    case c_type::int16_t:
        return "int16_t";
    case c_type::int32_t:
        return "int32_t";
    case c_type::int64_t:
        return "int64_t";

    case c_type::schar:
        return "char";
    case c_type::sshort:
        return "short";
    case c_type::sint:
        return "int";
    case c_type::slong:
        return "long";

    case c_type::uint8_t:
        return "uint8_t";
    case c_type::uint16_t:
        return "uint16_t";
    case c_type::uint32_t:
        return "uint32_t";
    case c_type::uint64_t:
        return "uint64_t";

    case c_type::uchar:
        return "uchar";
    case c_type::ushort:
        return "ushort";
    case c_type::uint:
        return "uint";
    case c_type::ulong:
        return "ulong";

    case c_type::pointer:
        return "pointer";
    }

    return "<unknown>";
}

inline ffi_type *to_ffi_type(c_type type)
{
    switch (type)
    {
    case c_type::void_:
        return &ffi_type_void;

    case c_type::int8_t:
        return &ffi_type_sint8;
    case c_type::int16_t:
        return &ffi_type_sint16;
    case c_type::int32_t:
        return &ffi_type_sint32;
    case c_type::int64_t:
        return &ffi_type_sint64;

    case c_type::schar:
        return &ffi_type_schar;
    case c_type::sshort:
        return &ffi_type_sshort;
    case c_type::sint:
        return &ffi_type_sint;
    case c_type::slong:
        return &ffi_type_slong;

    case c_type::uint8_t:
        return &ffi_type_uint8;
    case c_type::uint16_t:
        return &ffi_type_uint16;
    case c_type::uint32_t:
        return &ffi_type_uint32;
    case c_type::uint64_t:
        return &ffi_type_uint64;

    case c_type::uchar:
        return &ffi_type_uchar;
    case c_type::ushort:
        return &ffi_type_ushort;
    case c_type::uint:
        return &ffi_type_uint;
    case c_type::ulong:
        return &ffi_type_ulong;

    case c_type::pointer:
        return &ffi_type_pointer;
    }

    return nullptr;
}

} // namespace kpp::ffi
