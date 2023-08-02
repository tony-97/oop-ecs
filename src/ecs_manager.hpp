#pragma once

#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "entity.hpp"
#include "tmpl/sequence.hpp"
#include "traits.hpp"
#include "type_aliases.hpp"

#include <cstdint>
#include <execution>
#include <algorithm>
#include <ranges>
#include <type_traits>

namespace ECS
{

template<class Config_t>
struct ECSManager_t : Uncopyable_t
{
private:
    template<class Sign_t> struct EntityConfig_t;

    template<class... Ts> using BaseComponentContainer_t = SoA_t<ECSMap_t, Ts...>;
    template<class... Ts> using BaseEntityContainer_t    = SoA_t<ECSMap_t, Entity_t<EntityConfig_t<Ts>>...>;

    using EntSignatures_t = typename Config_t::Signatures_t;
    using ComponentList_t = Seq::As_t<Traits::Components_t, EntSignatures_t>;

    template<class T> using ToEntity_t = std::type_identity<Entity_t<EntityConfig_t<T>>>;
    template<class T> using ToID_t     = std::type_identity<ID_t<T>>;

    struct ComponentManagerConfig_t
    {
        using base = Seq::As_t<BaseComponentContainer_t, ComponentList_t>;
    };

    template<class Sign_t>
    struct EntityConfig_t
    {
        template<class T>
        using Self_t          = EntityConfig_t<T>;
        using Signature_t     = Sign_t;
        using Signatures_t    = EntSignatures_t;
        using Components_t    = Traits::Components_t<Signature_t>;
        using Bases_t         = Traits::Bases_t<Signature_t>;
        using ComponentIDs_t  = Seq::As_t<std::tuple, Seq::Map_t<Components_t, ToID_t>>;
        using BasesIDs_t      = Seq::As_t<std::tuple, Seq::Map_t<Seq::Map_t<Bases_t, ToEntity_t>, ToID_t>>;
        using ParentVariant_t = Seq::As_t<std::variant, Seq::Map_t<Seq::Map_t<Signatures_t, ToEntity_t>, ToID_t>>;
    };

    struct EntityManagerConfig_t
    {
        using base = Seq::As_t<BaseEntityContainer_t, EntSignatures_t>;
        template<class T> using entity_type = Entity_t<EntityConfig_t<T>>;
    };

    using ComponentMan_t = ComponentManager_t<ComponentManagerConfig_t>;
    using EntityMan_t    = EntityManager_t<EntityManagerConfig_t>;

    template<class T>
    using entity_type = typename EntityMan_t::template entity_type<T>;

    template<class T>
    using EntityID_t = typename EntityMan_t::template EntityID_t<T>;

    template<class SysSig_t, class EntID_t, class Callback_t> constexpr static auto
    ProcessEntity(EntID_t eid, Callback_t cb, auto& ecs_man) -> void
    {
        using EntSig_t  = Traits::Signature_t<EntID_t>;
        using Cmps_t    = Traits::Components_t<SysSig_t>;
        using EntHandle = std::conditional_t<std::is_same_v<SysSig_t, EntSig_t>, Handle_t<EntSig_t>, Handle_t<SysSig_t>>;
        EntHandle ent_handle { 0 };
        if constexpr (std::is_same_v<SysSig_t, EntSig_t>) {
            ent_handle = EntHandle{ eid };
        } else {
            ent_handle = ecs_man.template GetBase<SysSig_t>(eid);
        }
        if constexpr (Traits::IsInvocable_v<Callback_t, Cmps_t, EntHandle>) {
            Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts>(auto fn) {
                        fn(ecs_man.template GetComponent<Ts>(ent_handle)..., ent_handle);
                    }, cb);
        } else if constexpr (Traits::ConditionalIsInvocable_v<(Seq::Size_v<Cmps_t> > 1), Callback_t, Cmps_t>) {
            Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts>(auto fn) {
                        fn(ecs_man.template GetComponent<Ts>(ent_handle)...);
                    }, cb);
        } else if constexpr (Traits::IsInvocable_v<Callback_t, EntHandle>) {
            cb(ent_handle);
        }
    }

    template<class EntSig_t> constexpr static auto
    TraverseEntities(auto policy, auto cb, auto& ecs_man) -> void
    {
        auto size { ecs_man.mEntityMan.template size<entity_type<EntSig_t>>() };
        std::ranges::iota_view indexes { decltype(size){ 0 }, size };
        std::for_each(policy, std::make_reverse_iterator(indexes.end()), std::make_reverse_iterator(indexes.begin()), [&](auto i) {
                    ProcessEntity<EntSig_t>(ecs_man.mEntityMan.template GetKey<EntSig_t>(i), cb, ecs_man);
                });
    }

    template<class Cmpt_t> constexpr auto
    CreateComponent(Cmpt_t cmp) -> auto
    {
        return mComponentMan.template Create<Cmpt_t>(cmp);
    }

    template<template<class...> class TList_t, class... Cmps_t> constexpr auto
    CreateComponents(TList_t<Cmps_t...>, auto... cmps) -> auto
    {
       return std::tuple { CreateComponent(cmps)..., CreateComponent(Cmps_t{})... };
    }

    template<template<class...> class TList_t, class... Cmps_t> constexpr auto
    DestroyComponents(TList_t<Cmps_t...>, [[maybe_unused]] auto tp_ids) -> void
    {
        (mComponentMan.Destroy(std::get<ID_t<Cmps_t>>(tp_ids)), ...);
    }

    constexpr auto GetEntity(auto eid) const -> const auto& { return mEntityMan.GetEntity(eid); }
    constexpr auto GetEntity(auto eid)       -> auto&       { return mEntityMan.GetEntity(eid); }

public:
    template<class EntSig_t, class... Args_t> constexpr auto
    CreateEntity(Args_t... args) -> Handle_t<EntSig_t>
    {
        using ArgsTypes_t           = TMPL::TypeList_t<Args_t...>;
        using RequiredComponents_t  = Traits::Components_t<EntSig_t>;
        using RemainingComponents_t = Seq::Difference_t<RequiredComponents_t, ArgsTypes_t>;
        static_assert(Seq::IsSet_v<ArgsTypes_t>, "Component arguments must be unique.");
        static_assert(Seq::IsSubsetOf_v<ArgsTypes_t, RequiredComponents_t>,
                      "Components arguments does not match the entity components");

        auto cmp_ids { CreateComponents(RemainingComponents_t{}, args...) };

        return { mEntityMan.template Create<EntSig_t>(cmp_ids) };
    }

    template<class EntSig_t> constexpr auto
    Destroy(Handle_t<EntSig_t> ent_handle) -> void
    {
        EntityID_t<EntSig_t> ent_id { ent_handle };
        auto& ent { GetEntity(ent_id) };
        std::visit([&]<class T>(T eid) {
                    DestroyComponents(Traits::Components_t<Traits::Signature_t<T>>{}, GetEntity(eid).GetComponentIDs());
                    mEntityMan.template Destroy(eid);
                }, ent.GetParentID());
    }

    template<class DestSig_t, class EntSig_t, class... Args_t> constexpr auto
    TransformTo(Handle_t<EntSig_t> ent_handle, Args_t... args) -> Handle_t<DestSig_t>
    {
        using SrcSig_t   = EntSig_t;
        using DestCmps_t = Traits::Components_t<DestSig_t>;
        using SrcCmps_t  = Traits::Components_t<SrcSig_t>;
        using RmCmps_t   = Seq::Difference_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::Difference_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<Args_t...>;
        using RemainingComponents_t = Seq::Difference_t<MkCmps_t, ArgsTypes>;
        static_assert(Seq::IsSet_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(Seq::IsSubsetOf_v<ArgsTypes, MkCmps_t>,
                      "Components arguments does not match the requiered components");
        EntityID_t<EntSig_t> ent_id { ent_handle.GetIndex() };
        auto old_ids { GetEntity(ent_id).GetComponentIDs() };
        auto new_ids { CreateComponents(RemainingComponents_t{}, args...) };
        auto ids { std::tuple_cat(new_ids, old_ids) };
        DestroyComponents(RmCmps_t{}, old_ids);
        return { mEntityMan.template TransformTo<DestSig_t>(ent_id, ids) };
    }

    template<class BaseSig_t, class EntID_t, class... Args_t> constexpr auto
    AddBase(EntID_t ent_id, Args_t&&... args) -> void
    {
        using SrcSig_t = typename EntID_t::type;
        using NewSig_t = Seq::Cat_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
    }

    template<class BaseSig_t, class EntID_t> constexpr auto
    RemoveBase(EntID_t ent_id) -> void
    {
        using SrcSig_t = typename EntID_t::type;
        using NewSig_t = Seq::Difference_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
        static_assert(Traits::IsInstanceOf_v<SrcSig_t, BaseSig_t>, "Not base from ent.");
    }

    template<class Base_t, class EntSig_t> constexpr auto
    GetBase(Handle_t<EntSig_t> ent_handle) const -> Handle_t<Base_t>
    {
        auto& ent { GetEntity(EntityID_t<EntSig_t>{ ent_handle }) };
        return { ent.template GetBaseID<Base_t>().GetIndex() };
    }

    template<class Cmpt_t, class EntSig_t> constexpr auto
    GetComponent(Handle_t<EntSig_t> ent_handle) const -> const Cmpt_t&
    {
        static_assert(Seq::Contains_v<Cmpt_t, Traits::Components_t<EntSig_t>>,
                      "This entity doesn't have this component");
        auto ent_id { EntityID_t<EntSig_t>{ ent_handle.GetIndex() } };
        auto& ent { GetEntity(ent_id) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());
    }

    template<class Cmpt_t, class EntSig_t> constexpr auto
    GetComponent(Handle_t<EntSig_t> ent_handle) -> Cmpt_t&
    {
        return SameAsConstMemFunc(*this, &ECSManager_t::GetComponent<Cmpt_t, EntSig_t>, ent_handle);
    }

    template<class Cmp_t> constexpr auto
    GetComponent(Handle_t<Cmp_t> cmp_handle) const -> const Cmp_t&
    {
        return mComponentMan.GetComponent(ID_t<Cmp_t>{ cmp_handle.GetIndex() });
    }

    template<class Cmp_t> constexpr auto
    GetComponent(Handle_t<Cmp_t> cmp_handle) -> Cmp_t&
    {
        return SameAsConstMemFunc(*this, &ECSManager_t::GetComponent<Cmp_t>, cmp_handle);
    }

    template<class Cmpt_t, class EntSig_t> constexpr auto
    GetComponentID(Handle_t<EntSig_t> ent_handle) const -> auto
    {
        static_assert(Seq::Contains_v<Cmpt_t, Traits::Components_t<EntSig_t>>,
                      "This entity doesn't have this component");
        return GetEntity(EntityID_t<EntSig_t>{ ent_handle.GetIndex() }).template GetComponentID<Cmpt_t>();
    }

    template<class... Cmps_t> constexpr auto
    GetComponents(auto ent_id) const -> std::tuple<const Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_id)... };
    }

    template<class... Cmps_t> constexpr auto
    GetComponents(auto ent_id) -> std::tuple<Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_id)... };
    }

    template<class Sign_t> constexpr auto
    GetComponentsFor(auto ent_id) const -> auto
    {
        using Cmps_t = typename Sign_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts_t>() {
                    return std::forward_as_tuple(GetComponent<Ts_t>(ent_id)...);
                });
    }

    template<class Sign_t> constexpr auto
    GetComponentsFor(auto ent_id) -> auto
    {
        using Cmps_t = typename Sign_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts_t>() {
                    return std::forward_as_tuple(GetComponent<Ts_t>(ent_id)...);
                });
    }

    template<class EntSig_t> constexpr auto
    ForEach(auto cb) const -> void
    {
        TraverseEntities<EntSig_t>(std::execution::seq, cb, *this);
    }

    template<class EntSig_t> constexpr auto 
    ForEach(auto cb) -> void
    {
        TraverseEntities<EntSig_t>(std::execution::seq, cb, *this);
    }

    template<class EntSig_t> constexpr auto 
    ASYNCForEach(auto cb) -> void
    {
        TraverseEntities<EntSig_t>(std::execution::par_unseq, cb, *this);
    }

    template<class EntSig_t> constexpr auto 
    ASYNCForEach(auto cb) const -> void
    {
        TraverseEntities<EntSig_t>(std::execution::par_unseq, cb, *this);
    }

    template<class SysSig_t, class EntSig_t> constexpr auto
    Match(Handle_t<EntSig_t> ent_handle, auto cb) const -> void
    {
        std::visit([&]<class T>(T eid) {
                    if constexpr (Traits::IsInstanceOf_v<SysSig_t, Traits::Signature_t<T>>) {
                        ProcessEntity<SysSig_t>(eid, cb, *this);
                    }
                }, GetEntity(EntityID_t<EntSig_t>{ ent_handle }).GetParentID());
    }

    template<class SysSig_t, class EntSig_t> constexpr auto
    Match(Handle_t<EntSig_t> ent_handle, auto cb) -> void
    {
        std::visit([&]<class T>(T eid) {
                    if constexpr (Traits::IsInstanceOf_v<SysSig_t, Traits::Signature_t<T>>) {
                        ProcessEntity<SysSig_t>(eid, cb, *this);
                    }
                }, GetEntity(EntityID_t<EntSig_t>{ ent_handle }).GetParentID());
    }

    template<class EntSig_t> constexpr auto
    Match(Handle_t<EntSig_t> ent_handle, auto... cbs) const -> void
    {
        std::visit([&]<class T>(T eid) {
                    Seq::ForEach_t<Traits::Bases_t<T>>::Do(
                        [&]<class Bs_t>(){
                            auto bs { GetEntity(eid).template GetBase<Bs_t>() };
                            ProcessEntity<Bs_t>(bs, overloaded{ cbs... }, *this);
                        });
                }, GetEntity(EntityID_t<EntSig_t>{ ent_handle }).GetParentID());
    }

    template<class EntSig_t> constexpr auto
    Match(Handle_t<EntSig_t> ent_handle, auto... cbs) -> void
    {
        std::visit([&]<class T>(T eid) {
                    Seq::ForEach_t<Traits::Bases_t<Traits::Signature_t<T>>>::Do(
                        [&]<class Bs_t>(){
                            auto bs { GetEntity(eid).template GetBaseID<Bs_t>() };
                            ProcessEntity<Bs_t>(bs, overloaded{ cbs... }, *this);
                        });
                }, GetEntity(EntityID_t<EntSig_t>{ ent_handle }).GetParentID());
    }

    template<class Sign_t> constexpr auto
    Size() const -> std::uintmax_t
    {
        return mEntityMan.template size<entity_type<Sign_t>>();
    }

    constexpr auto
    Size() const -> std::uintmax_t
    {
        return Seq::Unpacker_t<EntSignatures_t>::Call([&]<class... Signs_t>() {
                    return (Size<Signs_t>() + ...);
                });
    }
private:
    ComponentMan_t mComponentMan {  };
    EntityMan_t    mEntityMan    {  };
};

} // namespace ECS
