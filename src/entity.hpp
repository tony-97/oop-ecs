#pragma once

#include <tmpl/sequence.hpp>

#include "helpers.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<class Sign_t, template<class> class CmptKey_t>
struct Entity_t final : Uncopyable_t
{
    template<class Cmps_t> struct ComponentKeys;

    template<template<class...> class Cmps_t, class... Ts>
    struct ComponentKeys<Cmps_t<Ts...>>
    {
        using type = TMPL::TypeList_t<CmptKey_t<Ts>...>;
    };
    
    template<class Cmps_t>
    using ComponentKeys_t = typename ComponentKeys<Cmps_t>::type;
public:
    using Signature_t    = Sign_t;
    using Components_t   = typename Sign_t::type;
    using ComponentIDs_t = Seq::ConvertTo_t<std::tuple<>,
                                            ComponentKeys_t<Components_t>>;

    template<class... IDs_t>
    constexpr explicit Entity_t(IDs_t... ids)   : mComponentIDs { ids... } {  }
    constexpr explicit Entity_t(Entity_t&& other) : mComponentIDs { std::move(other.mComponentIDs) } {  }

    constexpr Entity_t& operator=(Entity_t&& other)
    {
        mComponentIDs = std::move(other.mComponentIDs);

        return *this;
    }

    template<class Cmpt_t>
    constexpr const auto& GetComponentID() const
    {
        return std::get<CmptKey_t<Cmpt_t>>(mComponentIDs);
    }

    constexpr auto GetComponentIDs() const -> const ComponentIDs_t&
    { return mComponentIDs; }

private:

    ComponentIDs_t mComponentIDs {  };
};

} // namespace ECS
