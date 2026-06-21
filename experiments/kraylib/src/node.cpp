#include "node.hpp"

#include <initializer_list>
#include <string>

#include <kilate/node.h>

namespace kilate::node
{
vector::vector(std::initializer_list<node_t> nodes) : m_managed_by_me(true)
{
    m_raw = vector_make(sizeof(node_t));
    vector_reserve(m_raw, nodes.size());
    for (auto &node : nodes)
    {
        vector_push_back(m_raw, &node); // it will copy, so dont worry
    }
}

vector::vector(node_vector_t *raw) : m_raw{raw}, m_managed_by_me{false}
{
}

vector::vector(const vector &other) noexcept
{
    auto new_raw{vector_make(other.m_raw->itemSize)};
    vector_reserve(new_raw, other.m_raw->size);
    for (std::size_t i{0}; i < other.m_raw->size; ++i)
    {
        auto node{reinterpret_cast<const node_t *>(vector_get(other.m_raw, i))};
        vector_push_back(new_raw, node);
    }
    this->m_raw = new_raw;
    this->m_managed_by_me = true;
}

vector::vector(vector &&other) noexcept
{
    m_raw = other.m_raw;
    m_managed_by_me = true;
    other.m_raw = nullptr;
    other.m_managed_by_me = false;
}

vector &vector::operator=(const vector& other)
{
    if (this == &other)
        return *this;

    if (m_managed_by_me)
        vector_delete(m_raw);

    auto new_raw = vector_make(other.m_raw->itemSize);
    vector_reserve(new_raw, other.m_raw->size);

    for (std::size_t i = 0; i < other.m_raw->size; ++i)
    {
        auto node = reinterpret_cast<const node_t*>(vector_get(other.m_raw, i));
        vector_push_back(new_raw, node);
    }

    m_raw = new_raw;
    m_managed_by_me = true;

    return *this;
}

vector &vector::operator=(vector &&other)
{
    if (this == &other)
        return *this;

    vector_delete(m_raw);

    m_raw = other.m_raw;
    m_managed_by_me = other.m_managed_by_me;
    other.m_raw = nullptr;
    other.m_managed_by_me = false;

    return *this;
}

vector::~vector() noexcept
{
    if (m_managed_by_me)
        vector_delete(m_raw);
}


}
