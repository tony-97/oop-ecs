#pragma once

#include <sequence.hpp>
#include <utility>

#include "helpers.hpp"
#include "type_aliases.hpp"
#include "struct_of_arrays.hpp"

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
    : SoA_t<ComponentWrapper<Components_t>...>, Uncopyable_t
{
public:

                      using Self_t           = ComponentManager_t;
                      using Base_t           = SoA_t<ComponentWrapper<Components_t>...>;
                      using ConstructorKey_t = Key_t<Self_t>;
    template<class T> using ComponentID_t    = Identifier_t<Self_t, T>;

    constexpr explicit ComponentManager_t() : Base_t{  } {  }

    template<class RequiredComponent_t, class... Args_t> constexpr auto
    CreateComponent(Args_t&&... args)
    {
        using Component_t = ComponentWrapper<RequiredComponent_t>;
        ComponentID_t<Component_t> cmp_id {
            constructor_key,
            Base_t::template size<Component_t>() };

        Base_t::template emplace_back<Component_t>
            (std::forward<Args_t>(args)...);

        return cmp_id;
    }

private:

    constexpr static inline ConstructorKey_t constructor_key {  };
};

} // namespace ECS
