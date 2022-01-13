#pragma once

#include <sequence.hpp>

#include <array>
#include <type_traits>

#include "type_aliases.hpp"
#include "helpers.hpp"

namespace ECS
{

template<class Components_t>
struct ComponentsToIDs;

template<template<class...> class Components_t, class... Ts>
struct ComponentsToIDs<Components_t<Ts...>>
{
    using type = TMPL::TypeList_t<ID_t<Ts, IndexSize_t>...>;
};

template<class Components_t>
using ComponentsToIDs_t = typename ComponentsToIDs<Components_t>::type;

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

    template<class Component_t>
    constexpr auto GetComponentID() const { return std::get<ID_t<Component_t, IndexSize_t>>(mComponentIDs); } 

    constexpr auto GetComponentIDs() const -> const ComponentIDs_t& { return mComponentIDs; }

private:

    ComponentIDs_t mComponentIDs {  };
};

} // namespace ECS
