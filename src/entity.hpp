#pragma once

#include <array>
#include <type_traits>

#include <sequence.hpp>

#include "component_manager.hpp"
#include "type_aliases.hpp"
#include "helpers.hpp"
#include "ecs_map.hpp"

namespace ECS
{

template<class Components_t>
struct ComponentsToIDs;

template<template<class...> class Components_t, class... Ts>
struct ComponentsToIDs<Components_t<Ts...>>
{
    using type = TMPL::TypeList_t<typename ECSMap_t<ComponentWrapper<Ts>>::Key_t...>;
};

template<class Components_t>
using ComponentsToIDs_t = typename ComponentsToIDs<Components_t>::type;

// TODO: create a second template parameter for the component id type
// TODO: sort the ids passed in the constructor in any order for construct the
//       component ids
template<class Signature_t>
struct Entity_t final : Uncopyable_t
{
public:

    using Components_t   = typename Signature_t::type;
    using ComponentIDs_t = TMPL::Sequence::ConvertTo_t<std::tuple<>,
                                                      ComponentsToIDs_t<Components_t>>;

    template<class... IDs_t>
    constexpr explicit Entity_t(IDs_t&&... ids)   : mComponentIDs { ids... } {  }
    constexpr explicit Entity_t(Entity_t&& other) : mComponentIDs { std::move(other.mComponentIDs) } {  }

    constexpr Entity_t& operator=(Entity_t&& other)
    {
        mComponentIDs = std::move(other.mComponentIDs);

        return *this;
    }

    template<class Component_t>
    constexpr const auto& GetComponentID() const
    {
        return std::get<typename ECSMap_t<ComponentWrapper<Component_t>>::Key_t>(mComponentIDs);
    }

    constexpr auto GetComponentIDs() const -> const ComponentIDs_t&
    { return mComponentIDs; }

private:

    ComponentIDs_t mComponentIDs {  };
};

} // namespace ECS
