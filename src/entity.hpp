#pragma once

#include <tmpl/sequence.hpp>
#include <tuple>

#include "type_aliases.hpp"
#include "traits.hpp"
#include "helpers.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<class Sign_t>
struct Entity_t final : Uncopyable_t
{
    public:
    using Signature_t    = Sign_t;
    using Components_t   = ComponentsFrom_t<Signature_t>;
    using Bases_t        = GetBases_t<Signature_t>;
    using ComponentIDs_t = Seq::ConvertTo_t<std::tuple, Seq::Map_t<Components_t, ToID_t>>;
    using BasesIDs_t     = Seq::ConvertTo_t<std::tuple, Seq::Map_t<Bases_t, ToID_t>>;

    template<class... IDs_t>
    constexpr explicit Entity_t(IDs_t... ids) : mComponentIDs { ids... } {  }

    template<class TupleIDs_t>
    constexpr explicit Entity_t(TupleIDs_t tp_ids) : Entity_t(Components_t{}, tp_ids) {  }

    constexpr explicit Entity_t(Entity_t&& other) : mComponentIDs { std::move(other.mComponentIDs) } {  }

    constexpr auto operator=(Entity_t&& other) -> Entity_t&
    {
        mComponentIDs = std::move(other.mComponentIDs);

        return *this;
    }

    template<class Cmpt_t>
    constexpr auto GetComponentID() const -> auto
    {
        return std::get<ID_t<Cmpt_t>>(mComponentIDs);
    }

    constexpr auto GetComponentIDs() const -> ComponentIDs_t
    { return mComponentIDs; }

private:

    template<template<class...> class TList_t, class... Ts, class TupleIDs_t>
    constexpr explicit Entity_t(TList_t<Ts...>, TupleIDs_t tp_ids) 
        : mComponentIDs{ std::get<Ts>(tp_ids)... } {  }

    ComponentIDs_t mComponentIDs {  };
    BasesIDs_t mBases {  };
};

} // namespace ECS
