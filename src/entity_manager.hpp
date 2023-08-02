#pragma once

#include "helpers.hpp"
#include "traits.hpp"
#include "type_aliases.hpp"

#include <cstddef>
#include <tuple>
#include <variant>

namespace ECS
{

template<class Config_t>
struct EntityManager_t final : Config_t::base, Uncopyable_t
{
public:
                      using Self_t      = EntityManager_t;
                      using Base_t      = typename Config_t::base;
    template<class T> using entity_type = typename Config_t::template entity_type<T>;
    template<class T> using EntityID_t  = ID_t<entity_type<T>>;

    constexpr explicit EntityManager_t() : Base_t{  } {  }

    template<class EntSig_t> constexpr auto
    Create(auto cmp_ids) -> auto
    {
        auto id { CreateParent<EntSig_t>(cmp_ids) };
        CreateBases(Traits::Bases_t<EntSig_t>{}, id, cmp_ids);
        return id;
    }

    // only call on the parent entity id
    template<class EntID_t> constexpr auto 
    Destroy(EntID_t ent_id) -> void
    {
        auto& ent { GetEntity(ent_id) };
        DestroyBases(Traits::Bases_t<Traits::Signature_t<EntID_t>>{}, ent);
        DestroyRaw(ent_id);
    }

    template<class DestSig_t, class EntID_t> constexpr auto
    TransformTo(EntID_t ent_id, auto cmp_ids) -> auto
    {
        using SrcSig_t = Traits::Signature_t<EntID_t>;
        using DestBs_t = Traits::Bases_t<DestSig_t>;
        using SrcBs_t  = Traits::Bases_t<SrcSig_t>;
        using RmBs_t   = TMPL::Sequence::Difference_t<SrcBs_t, DestBs_t>;
        using Bs_t     = TMPL::Sequence::Difference_t<SrcBs_t, RmBs_t>;
        using MkBs_t   = TMPL::Sequence::Difference_t<DestBs_t, SrcBs_t>;

        auto& ent { GetEntity(ent_id) };
        auto id { CreateParent<DestSig_t>(cmp_ids) };
        // change the to the new parent
        TMPL::Sequence::ForEach_t<Bs_t>::Do([&]<class Base_t>() {
                    auto bid { ent.template GetBaseID<Base_t>() };
                    GetEntity(bid).SetParentID(id);
                });
        CreateBases(MkBs_t{}, id, cmp_ids, ent.GetBaseIDs());
        DestroyBases(RmBs_t{}, ent);
        DestroyRaw(ent_id);
        return id;
    }

    template<class EntID_t> constexpr auto
    GetEntity(EntID_t pos) const -> const auto&
    {
        return Base_t::template operator[]<Traits::Entity_t<EntID_t>>(pos);
    }

    template<class EntID_t> constexpr auto
    GetEntity(EntID_t pos) -> auto&
    {
        return Base_t::template operator[]<Traits::Entity_t<EntID_t>>(pos);
    }

    template<class EntSig_t> constexpr auto
    GetKey(size_t pos) const -> auto
    {
        auto& base { Base_t::template GetRequiredContainer<entity_type<EntSig_t>>() };
        return base.get_key(pos);
    }

private:
    template<class EntID_t> constexpr auto
    DestroyRaw(EntID_t eid) -> void
    {
        Base_t::template erase<Traits::Entity_t<EntID_t>>(eid);
    }

    template<class EntSig_t> constexpr auto
    CreateRawEntity(auto... args) -> auto&
    {
        using ReqEntity_t = entity_type<EntSig_t>;
        return Base_t::template emplace_back<ReqEntity_t>(args...);
    }

    template<class EntSig_t> constexpr auto
    CreateBase(auto cmp_ids, auto parent_id) -> auto
    {
        return CreateRawEntity<EntSig_t>(cmp_ids, parent_id).key();
    }

    template<class EntSig_t> constexpr auto
    CreateParent(auto cmp_ids) -> auto
    {
        auto& slot { CreateRawEntity<EntSig_t>(cmp_ids) };
        slot.value().SetParentID(slot.key());

        return slot.key();
    }

    template<template<class...> class TList_t, class... Bases_t, class Tp = std::tuple<>> constexpr auto
    CreateBases(TList_t<Bases_t...>, [[maybe_unused]] auto parent_id, [[maybe_unused]] auto cmp_ids, Tp bases = {}) -> auto
    {
        std::tuple ids { CreateBase<Bases_t>(cmp_ids, parent_id)... };
        auto bases_ids { std::tuple_cat(ids, bases) };
        (GetEntity(std::get<EntityID_t<Bases_t>>(ids)).SetBasesIDs(bases_ids), ...);
        GetEntity(parent_id).SetBasesIDs(bases_ids);
    }

    template<template<class...> class TList_t, class... Bases_t> constexpr auto
    DestroyBases(TList_t<Bases_t...>, auto& ent) -> void
    {
        (DestroyRaw(ent.template GetBaseID<Bases_t>()), ...);
    }

    using Base_t::operator[];
    using Base_t::at;
    using Base_t::push_back;
    using Base_t::emplace;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
