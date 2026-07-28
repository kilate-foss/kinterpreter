#pragma once

#include "object/object.hpp"
#include <string>

namespace kpp::obj
{

struct instance_object : public object
{
    explicit instance_object(object_ref<object> obj_)
        : object{obj_->get_type()},
          m_value{obj_}
    {
    }

    std::string get_name(void) const override
    {
        return m_value->get_name();
    }

    std::string to_str(void) const override
    {
        return m_value->to_str();
    }

    std::string repr(void) const override
    {
        return m_value->repr();
    }

    object_ref<object> get_value() const
    {
        return m_value;
    }

private:
    object_ref<object> m_value;
};

template <>
inline object_ref<object> obj_cast<object_ref<object>>(object const &value)
{
    return static_cast<instance_object const &>(value).get_value();
}

} // namespace kpp::obj
