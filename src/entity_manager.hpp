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
        using ReqEntity_t = entity_type<EntSig_t>;
        auto id { Base_t::template emplace_back<ReqEntity_t>(cmp_ids).key() };
        CreateBases(GetBases_t<EntSig_t>{}, id, cmp_ids);
        return id;
    }

    template<class EntID_t> constexpr auto 
    Destroy(EntID_t ent_id) -> void
    {
        using Sign_t = typename EntID_t::value_type::Signature_t; 
        auto& ent { Base_t::template operator[]<typename EntID_t::value_type>(ent_id) };

        std::visit([&]<class T>(T eid) {
                    if constexpr (not IsInstanceOf_v<Sign_t, typename T::value_type::Signature_t>) {
                        Destroy(eid);
                    } else {
                        Seq::ForEach_t<GetBases_t<typename EntID_t::value_type::Signature_t>>::Do([&]<class Base_t>() {
                            auto id { ent.template GetBaseID<Base_t>() };
                            (*this).template erase<typename decltype(id)::value_type>(id);
                            //Base_t::template GetRequiredContainer<typename decltype(id)::value_type>().erase(id);
                        });
                        (*this).template erase<typename EntID_t::value_type>(ent_id);
                    }
                }, ent.GetParentID());
    }

    template<class DestSig_t, class EntID_t> constexpr auto
    TransformTo(EntID_t ent_id, auto cmp_ids) -> auto
    {
        using SrcSig_t   = typename EntID_t::value_type::Signature_t;
        using DestBs_t = GetBases_t<DestSig_t>;
        using SrcBs_t  = GetBases_t<SrcSig_t>;
        using RmBs_t   = Seq::RemoveTypes_t<SrcBs_t, DestBs_t>;
        using Bs_t     = Seq::RemoveTypes_t<SrcBs_t, RmBs_t>;
        using MkBs_t   = Seq::RemoveTypes_t<DestBs_t, SrcBs_t>;

        auto& ent { GetEntity(ent_id) };
        auto bases { ent.GetBaseIDs() };
        auto id { Base_t::template emplace_back<entity_type<DestSig_t>>(cmp_ids).key() };
        Seq::ForEach_t<SrcBs_t>::Do([&]<class Base_t>() {
                    auto bid { ent.template GetBaseID<Base_t>() };
                    GetEntity(bid).SetParentID(id);
                });
        CreateBases(MkBs_t{}, id, cmp_ids, bases);
        Seq::ForEach_t<RmBs_t>::Do([&]<class Base_t>() {
                            auto bid { ent.template GetBaseID<Base_t>() };
                            (*this).template erase<typename decltype(bid)::value_type>(bid);
                            //Base_t::template GetRequiredContainer<typename decltype(id)::value_type>().erase(id);
                        });
        Base_t::template erase<entity_type<SrcSig_t>>(ent_id);
        return id;
    }

    template<class EntID_t> constexpr auto
    GetEntity(EntID_t pos) const -> const auto&
    {
        return Base_t::template operator[]<typename EntID_t::value_type>(pos);
    }

    template<class EntID_t> constexpr auto
    GetEntity(EntID_t pos) -> auto&
    {
        return Base_t::template operator[]<typename EntID_t::value_type>(pos);
    }

    template<class EntSig_t> constexpr auto
    GetKey(size_t pos) const -> auto
    {
        auto& base { Base_t::template GetRequiredContainer<entity_type<EntSig_t>>() };
        return base.get_key(pos);
    }


private:
    template<template<class...> class TList_t, class... Bases_t, class Tp = std::tuple<>> constexpr auto
    CreateBases(TList_t<Bases_t...>, [[maybe_unused]] auto parent_id, [[maybe_unused]] auto cmp_ids, Tp bases = {}) -> auto
    {
        std::tuple ids { Base_t::template emplace_back<entity_type<Bases_t>>(cmp_ids, parent_id).key()... };
        auto bases_ids { std::tuple_cat(ids, bases) };
        (GetEntity(std::get<EntityID_t<Bases_t>>(ids)).SetBasesIDs(bases_ids), ...);
        GetEntity(parent_id).SetBasesIDs(bases_ids);
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
