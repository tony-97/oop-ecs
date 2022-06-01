#pragma once

#include "helpers.hpp"
#include "type_aliases.hpp"
#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"

namespace ECS
{

template<class Component_t>
struct ComponentWrapper
{
    Component_t mSelf {  };

    template<class... Args_t> constexpr
    ComponentWrapper(Args_t&&... args) : mSelf { std::forward<Args_t>(args)... } {  }
};

template<class ComponentTypes_t>
struct ComponentManager_t;

template<template <class...> class ComponentTypes_t, class... Components_t>
struct ComponentManager_t<ComponentTypes_t<Components_t...>> final
    : SoA_t<ECSMap_t, ComponentWrapper<Components_t>...>, Uncopyable_t
{
public:

                      using Self_t           = ComponentManager_t;
                      using Base_t           = SoA_t<ECSMap_t, ComponentWrapper<Components_t>...>;
                      using ConstructorKey_t = Key_t<Self_t>;
    template<class T> using ComponentID_t    = typename ECSMap_t<T>::Key_t;

    constexpr explicit ComponentManager_t() : Base_t{  } {  }

    template<class RequiredComponent_t, class... Args_t> constexpr auto
    Create(Args_t&&... args)
    {
        using Component_t = ComponentWrapper<RequiredComponent_t>;

        return Base_t::template emplace_back<Component_t>(std::forward<Args_t>(args)...);
    }

    template<class K>
    constexpr void Destroy(K& key)
    {
        auto& cont {
            Base_t::template GetRequiredContainer<typename K::value_type>()
        };

        cont.erase(std::move(key));
    }

    template<class K>
    constexpr auto& GetComponent(const K& cmp_id)
    {
        using Component_t = typename K::value_type;
        return Base_t::template operator[]<Component_t>(cmp_id).mSelf;
    }

    template<class K>
    constexpr const auto& GetComponent(const K& cmp_id) const
    {
        using Component_t = typename K::value_type;
        return Base_t::template operator[]<Component_t>(cmp_id).mSelf;
    }

private:

    using Base_t::data;
    using Base_t::push_back;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
