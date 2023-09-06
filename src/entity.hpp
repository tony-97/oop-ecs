#pragma once

#include "type_aliases.hpp"
#include "helpers.hpp"

#include <tmpl/sequence.hpp>

#include <tuple>

namespace ECS
{

template<class Config_t>
struct Entity_t final : Uncopyable_t
{
public:
    using Signature_t     = typename Config_t::Signature_t;
    using Components_t    = typename Config_t::Components_t;
    using Bases_t         = typename Config_t::Bases_t;
    using ComponentIDs_t  = typename Config_t::ComponentIDs_t;
    using BasesIDs_t      = typename Config_t::BasesIDs_t;
    using ParentVariant_t = typename Config_t::ParentVariant_t;

    template<class T> using EntityID_t = ID_t<Entity_t<typename Config_t::template Self_t<T>>>;

    template<class ParentID_t = Handle_t<Signature_t>>
    constexpr explicit Entity_t(auto cmp_ids, ParentID_t parent_id = { })
    : Entity_t(Components_t{}, cmp_ids, parent_id) {  }

    constexpr explicit Entity_t(Entity_t&& other)
        : mComponentIDs { std::move(other.mComponentIDs) },
          mBases        { std::move(other.mBases)        },
          mParent       { std::move(other.mParent)       } {  }

    constexpr auto operator=(Entity_t&& other) -> Entity_t&
    {
        mComponentIDs = std::move(other.mComponentIDs);
        mBases        = std::move(other.mBases);
        mParent       = std::move(other.mParent);

        return *this;
    }

    template<class Cmpt_t>    constexpr auto GetComponentID() const -> auto { return std::get<Handle_t<Cmpt_t>>(mComponentIDs); }
    template<class EntSign_t> constexpr auto GetBaseID()      const -> auto { return std::get<Handle_t<EntSign_t>>(mBases); }

    constexpr auto GetComponentIDs() const -> ComponentIDs_t { return mComponentIDs; }
    constexpr auto GetBaseIDs()      const -> BasesIDs_t     { return mBases; }
    constexpr auto GetParentID()     const -> auto           { return mParent; }

    constexpr auto SetParentID(auto parent_id) -> void { mParent = parent_id; }
    constexpr auto SetBasesIDs(auto bs_ids)    -> void
    {
        TMPL::Sequence::ForEach_t<Bases_t>::Do([&]<class T>() {
                    std::get<Handle_t<T>>(mBases) = std::get<Handle_t<T>>(bs_ids);
                });
    }

private:
    template<template<class...> class TList_t, class... Cmps_t>
    constexpr explicit Entity_t(TList_t<Cmps_t...>, [[maybe_unused]] auto cmp_ids, auto parent_id)
        : mComponentIDs{ std::get<Handle_t<Cmps_t>>(cmp_ids)... }, mParent { parent_id } {  }

    ComponentIDs_t  mComponentIDs {  };
    BasesIDs_t      mBases        {  };
    ParentVariant_t mParent       {  };
};

} // namespace ECS
