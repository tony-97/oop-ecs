#pragma once

#include "helpers.hpp"

namespace ECS
{

template<class Config_t>
struct ComponentManager_t final : Config_t::base, Uncopyable_t
{
public:
    using Self_t = ComponentManager_t;
    using Base_t = typename Config_t::base;

    constexpr explicit ComponentManager_t() : Base_t{  } {  }

    template<class ReqCmpt_t> constexpr auto
    Create(ReqCmpt_t cmp) -> auto
    {
        return Base_t::template emplace_back<ReqCmpt_t>(cmp).key();
    }

    template<class CmpID_t> constexpr auto
    Destroy(CmpID_t cmp_id) -> void
    {
        Base_t::template erase<typename CmpID_t::value_type>(cmp_id);
    }

    template<class CmpID_t> constexpr auto
    GetComponent(CmpID_t cmp_id) const -> const auto&
    {
        using Component_t = typename CmpID_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_id);
    }

    template<class CmpID_t> constexpr auto
    GetComponent(CmpID_t cmp_id) -> auto& 
    {
        using Component_t = typename CmpID_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_id);
    }

private:
    using Base_t::operator[];
    using Base_t::at;
    using Base_t::data;
    using Base_t::push_back;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
