#pragma once

#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"

namespace ECS
{

template<class Cmpt_t>
struct ComponentWrapper_t
{
    Cmpt_t mSelf {  };

    template<class... Args_t> constexpr
    ComponentWrapper_t(Args_t&&... args) : mSelf { std::forward<Args_t>(args)... } {  }
};

template<class CmptList_t>
struct ComponentManager_t;

template<template <class...> class CmptList_t, class... Cmpts_t>
struct ComponentManager_t<CmptList_t<Cmpts_t...>> final
    : SoA_t<ECSMap_t, ComponentWrapper_t<Cmpts_t>...>, Uncopyable_t
{
public:
                      using Self_t         = ComponentManager_t;
                      using Base_t         = SoA_t<ECSMap_t, ComponentWrapper_t<Cmpts_t>...>;
    template<class T> using ComponentKey_t = typename ECSMap_t<ComponentWrapper_t<T>>::Key_t;

    constexpr explicit ComponentManager_t() : Base_t{  } {  }

    template<class ReqCmpt_t, class... Args_t>
    constexpr auto Create(Args_t&&... args)
    {
        using Component_t = ComponentWrapper_t<ReqCmpt_t>;

        return Base_t::template emplace_back<Component_t>(std::forward<Args_t>(args)...);
    }

    template<class K_t>
    constexpr void Destroy(const K_t& cmp_key)
    {
        auto& cont {
            Base_t::template GetRequiredContainer<typename K_t::value_type>()
        };

        cont.erase(cmp_key);
    }

    template<class K_t>
    constexpr const auto& GetComponent(const K_t& cmp_key) const
    {
        using Component_t = typename K_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_key).mSelf;
    }

    template<class K_t>
    constexpr auto& GetComponent(const K_t& cmp_key)
    {
        using Component_t = typename K_t::value_type;
        return Base_t::template operator[]<Component_t>(cmp_key).mSelf;
    }

    template<class Cmpt_t> constexpr auto
    size() const { return Base_t::template size<ComponentWrapper_t<Cmpt_t>>(); }
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
