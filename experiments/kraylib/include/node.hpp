#pragma once

#include <initializer_list>
#include <string>

#include <kilate/node.h>

namespace kilate::node
{

inline static node_t param(const std::string &name, node_value_kind_t kind)
{
    auto n{make_node(NODE_PARAM)};
    n.param_n.name = strdup(name.c_str());
    n.param_n.kind = kind;
    return n;
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

} // namespace kilate::node
