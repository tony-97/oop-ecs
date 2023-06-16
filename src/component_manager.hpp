#pragma once

#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"

namespace ECS
{

template<class CmptList_t>
struct ComponentManager_t;

template<template <class...> class CmptList_t, class... Cmpts_t>
struct ComponentManager_t<CmptList_t<Cmpts_t...>> final
    : SoA_t<ECSMap_t, Cmpts_t...>, Uncopyable_t
{
public:
    using Self_t         = ComponentManager_t;
    using Base_t         = SoA_t<ECSMap_t, Cmpts_t...>;

    constexpr explicit ComponentManager_t() : Base_t{  } {  }

    template<class ReqCmpt_t> constexpr auto
    Create(ReqCmpt_t cmp) -> auto
    {
        return Base_t::template emplace_back<ReqCmpt_t>(cmp);
    }

    template<class CmpID_t> constexpr auto
    Destroy(CmpID_t cmp_id) -> void
    {
        auto& cont {
            Base_t::template GetRequiredContainer<typename CmpID_t::value_type>()
        };

        cont.erase(cmp_id);
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
