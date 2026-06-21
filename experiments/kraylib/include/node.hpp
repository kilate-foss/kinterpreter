#pragma once

#include <initializer_list>
#include <string>

#include <kilate/node.h>

namespace kilate::node
{

inline static node_t arg(const std::string &name, node_value_kind_t kind)
{
    return {.type = NODE_ARG, .arg_n = value_t{.type = kind, .s = strdup(name.c_str())}};
}

struct vector
{
    vector(std::initializer_list<node_t>);
    vector(node_vector_t *);

    vector(const vector &other) noexcept;
    vector(vector &&other) noexcept;

    ~vector() noexcept;

    vector &operator=(const vector &other);
    vector &operator=(vector &&other);

    const node_t *operator[](std::size_t idx) const
    {
        return reinterpret_cast<const node_t *>(vector_get(m_raw, idx));
    }

    node_vector_t *get_raw() const
    {
        return m_raw;
    }

    bool empty() const
    {
        return m_raw->size == 0;
    }

private:
    node_vector_t *m_raw;
    bool m_managed_by_me;
};

} // namespace node
