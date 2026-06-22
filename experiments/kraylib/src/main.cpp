#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <thread>

#include <raylib.h>

#include "native.hpp"
#include "node.hpp"
#include "value.hpp"

namespace kraylib
{

DECLARE_NATIVE(init_window, nullptr, kilate::node::arg("width", NODE_VALUE_TYPE_UINT),
               kilate::node::arg("height", NODE_VALUE_TYPE_UINT),
               kilate::node::arg("name", NODE_VALUE_TYPE_STRING))
{
    assert(d != nullptr && "native function data is null.");

    auto width = kilate::value::get_arg<int>(d, 0, "width must be provided as first argument");
    auto height = kilate::value::get_arg<int>(d, 1, "height must be provided ad second argument");
    auto title = kilate::value::get_arg<char *>(d, 2, "title must be provided as third argument");

    InitWindow(width, height, title);

    return make_node(NODE_RETURN);
}

DECLARE_NATIVE(clear_background, nullptr, kilate::node::arg("color", NODE_VALUE_TYPE_UINT))
{
    auto unpack_color = [](std::uint32_t c) -> Color
    {
        return {(unsigned char)((c >> 24) & 0xFF), (unsigned char)((c >> 16) & 0xFF),
                (unsigned char)((c >> 8) & 0xFF), (unsigned char)(c & 0xFF)};
    };

    assert(d != nullptr && "native function data is null.");

    auto packed_color{kilate::value::get_arg<std::uint32_t>(
        d, 0, "color hex must be provided as first argument")};

    auto color{unpack_color(packed_color)};

    ClearBackground(color);

    return make_node(NODE_RETURN);
}

DECLARE_NATIVE(begin_drawing, nullptr)
{
    assert(d != nullptr && "native function data is null.");
    BeginDrawing();
    return make_node(NODE_RETURN);
}

DECLARE_NATIVE(end_drawing, nullptr)
{
    assert(d != nullptr && "native function data is null.");
    EndDrawing();
    return make_node(NODE_RETURN);
}

DECLARE_NATIVE(Color, "UInt", kilate::node::arg("red", NODE_VALUE_TYPE_UINT),
               kilate::node::arg("green", NODE_VALUE_TYPE_UINT),
               kilate::node::arg("blue", NODE_VALUE_TYPE_UINT),
               kilate::node::arg("alpha", NODE_VALUE_TYPE_UINT))
{
    auto pack_color = [](unsigned char r, unsigned char g, unsigned char b,
                         unsigned char a) -> std::uint32_t
    {
        return ((std::uint32_t)r << 24 | (std::uint32_t)g << 16 | (std::uint32_t)b << 8 |
                (std::uint32_t)a);
    };
    assert(d != nullptr && "native function data is null.");
    auto red{kilate::value::get_arg<std::uint32_t>(
        d, 0, "red channel must br provided as first argument")};
    auto green{kilate::value::get_arg<std::uint32_t>(
        d, 1, "green channel must be provided as second argument")};
    auto blue{kilate::value::get_arg<std::uint32_t>(
        d, 2, "blue channel must be provided as third argument")};
    auto alpha{kilate::value::get_arg<std::uint32_t>(
        d, 3, "alpha channel must be provided as fourth argument")};
    auto color{pack_color(red, green, blue, alpha)};
    auto ret{make_node(NODE_RETURN)};
    ret.return_n.type = NODE_VALUE_TYPE_UINT;
    ret.return_n.u = color;
    return ret;
}

DECLARE_NATIVE(close_window, nullptr)
{
    assert(d != nullptr && "native function data is null.");
    CloseWindow();
    return make_node(NODE_RETURN);
}

DECLARE_NATIVE(test_loop, nullptr)
{
    assert(d != nullptr && "native function data is null.");

    /*while (!WindowShouldClose())
    {
        ClearBackground(clear_background::g_color);
    }*/

    std::this_thread::sleep_for(std::chrono::seconds{5});

    return make_node(NODE_RETURN);
}

} // namespace kraylib

extern "C" KILATE_NATIVE_REGISTER()
{
    for (auto *entry : kilate::native::registry())
    {
        kilate::native::register_fn(entry->name, entry->return_type, *entry->params, entry->fn);
    }
}
