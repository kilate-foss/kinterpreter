#pragma once

#include <vector>

#include <kilate/native.h>
#include <kilate/node.h>

#include "node.hpp"

namespace kilate::native
{

struct native_entry
{
    const char *name;
    const char *return_type;
    const node::vector *params;
    return_node_t (*fn)(native_fndata_t *);
};

inline static auto &registry()
{
    static std::vector<native_entry *> r{};
    return r;
}

#define DECLARE_NATIVE(NAME, RETTYPE, ...)                                                         \
    struct __##NAME##__                                                                            \
    {                                                                                              \
        inline static constexpr const char *name = #NAME;                                          \
        inline static constexpr const char *return_type = RETTYPE;                                 \
        inline static const kilate::node::vector params{__VA_ARGS__};                               \
                                                                                                   \
        static return_node_t fn(native_fndata_t *d);                               \
                                                                                                   \
        inline static kilate::native::native_entry entry{name, return_type, &params, &fn};         \
                                                                                                   \
        struct __register                                                                          \
        {                                                                                          \
            __register()                                                                           \
            {                                                                                      \
                kilate::native::registry().push_back(&entry);                                      \
            }                                                                                      \
        };                                                                                         \
                                                                                                   \
        inline static __register __register__;                                                     \
    };                                                                                             \
                                                                                                   \
    return_node_t __##NAME##__::fn(native_fndata_t *d)

inline static void register_fn(const char *name, const char *return_type, const node::vector &args,
                               native_fn_t fn)
{
    native_register_fn(name, return_type, (!args.empty()) ? args.get_raw() : nullptr, fn);
}

} // namespace kilate::native
