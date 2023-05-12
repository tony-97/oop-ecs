#pragma once

#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"

namespace ECS
{

template<class CmptList_t>
struct ComponentManager_t;

template<class T> using ComponentKey_t = typename ECSMap_t<T>::Key_t;

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
        //return Base_t::template emplace_back<ReqCmpt_t>(std::forward<Args_t>(args)...);
        return Base_t::template emplace_back<ReqCmpt_t>(cmp);
        //return Base_t::template push_back<ReqCmpt_t>(cmp);
    }

    template<class K_t> constexpr auto
    Destroy(K_t cmp_key) -> void
    {
        auto& cont {
            Base_t::template GetRequiredContainer<typename K_t::value_type>()
        };

        cont.erase(cmp_key);
    }

    template<class K_t> constexpr auto
    GetComponent(K_t cmp_key) const -> const auto&
    {
        using Component_t = typename K_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_key);
    }

    template<class K_t> constexpr auto
    GetComponent(K_t cmp_key) -> auto& 
    {
        using Component_t = typename K_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_key);
    }

    template<class Cmpt_t> constexpr auto
    size() const -> auto
    { return Base_t::template size<Cmpt_t>(); }
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
