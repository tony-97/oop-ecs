#pragma once

#include <tmpl/tmpl.hpp>
#include <tmpl/sequence.hpp>
#include <tmpl/type_list.hpp>

#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "helpers.hpp"
#include "traits.hpp"

#include <type_traits>
#include <execution>
#include <algorithm>
#include <tuple>
#include <unistd.h>

namespace ECS
{

template<class... EntSigs_t>
struct ECSManager_t
{
private:
    using ComponentList_t = ComponentsFrom_t<EntSigs_t...>;
    using EntityList_t    = TMPL::TypeList_t<EntSigs_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;
    using EntityMan_t     = EntityManager_t<EntSigs_t...>;

    template<class T>
    using entity_type = typename EntityMan_t::template entity_type<T>;

    template<class SysSig_t>
    struct EntityTraversal_t
    {
        template<class EntSig_t, class Callback_t, class ECSMan_t> constexpr auto
        operator()(Callback_t&& cb, ECSMan_t& ecs_man) const -> void
        {
            using Cmps_t = typename SysSig_t::type;
            if constexpr (ECS::IsInstanceOf_v<SysSig_t, EntSig_t>) {
                auto begin { ecs_man.mEntityMan.template begin<entity_type<EntSig_t>>() };
                auto end   { ecs_man.mEntityMan.template end<entity_type<EntSig_t>>() };
                for (auto it { end }; it-- != begin;) {
                    auto eid { ecs_man.mEntityMan.GetEntityID(it) };
                    if constexpr (IsSystemCallable_v<Callback_t, Cmps_t, ID_t<entity_type<EntSig_t>>>) {
                        Seq::Unpacker_t<Cmps_t>::Call([eid,&ecs_man]<class... Ts>(auto&& fn) {
                                    fn(ecs_man.template GetComponent<Ts>(eid)..., eid);
                                }, std::forward<Callback_t>(cb));
                    } else if constexpr (   IsSystemCallable_v<Callback_t, Cmps_t>
                                         && Seq::Size_v<Cmps_t> > 1) {
                        Seq::Unpacker_t<Cmps_t>::Call([eid,&ecs_man]<class... Ts>(auto&& fn) {
                                    fn(ecs_man.template GetComponent<Ts>(eid)...);
                                }, std::forward<Callback_t>(cb));
                    } else {
                        cb(eid);
                    }
                }
            }
        }
    };

    template<class CmptArgs_t> constexpr auto
    CreateComponent(CmptArgs_t arg) -> auto
    {
        return mComponentMan.template Create<CmptArgs_t>(arg);
    }

    template<class EmptyCmps_t, std::size_t... Is, class... Args_t> constexpr auto 
    CreateComponentsIMPL(std::index_sequence<Is...>, Args_t... args) -> auto
    {
       return std::tuple {
           CreateComponent(args)..., 
           CreateComponent(Seq::TypeAt_t<Is, EmptyCmps_t>{})... };
    }

    template<class EmptyCmps_t, class... Args_t> constexpr auto
    CreateComponents(Args_t... args) -> auto
    {
        return CreateComponentsIMPL<EmptyCmps_t>(
                std::make_index_sequence<Seq::Size_v<EmptyCmps_t>>{},
                args...);
    }

    template<class EntSig_t, class... IDs> constexpr auto
    CreateEntityIMPL(IDs... ids) -> auto
    {
        return mEntityMan.template Create<EntSig_t>(ids...);
    }

public:
    template<class EntSig_t, class... Args_t> constexpr auto
    CreateEntity(Args_t... args) -> auto
    {
        using ArgsTypes = TMPL::TypeList_t<Args_t...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntSig_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        using ComponentIDs_t = typename entity_type<EntSig_t>::ComponentIDs_t;

        auto cmp_ids { TupleAs<ComponentIDs_t>(CreateComponents<RemainingComponents_t>(args...)) };

        return std::apply([&](auto... ids) {
                    return CreateEntityIMPL<EntSig_t>(ids...);
                    }, cmp_ids);
    }

    template<class EntID_t> constexpr auto&
    GetEntity(EntID_t eid)
    {
        return mEntityMan.GetEntity(eid);
    }

    template<class EntID_t> constexpr auto
    Destroy(EntID_t ent_id) -> void
    {
        using EntSig_t = typename EntID_t::value_type;
        auto cmp_ids {
            mEntityMan.template Destroy(ent_id)
        };

        std::apply([&](auto... ids) {
                    (mComponentMan.Destroy(ids), ...);
                }, cmp_ids);
    }

    template<class DestSig_t, class EntID_t, class... Args_t> constexpr auto
    TransformTo(EntID_t ent_id, Args_t... args) -> auto
    {
        using SrcSig_t   = typename EntID_t::value_type::Signature_t;
        using DestCmps_t = ComponentsFrom_t<DestSig_t>;
        using SrcCmps_t  = ComponentsFrom_t<SrcSig_t>;
        using RmCmps_t   = Seq::RemoveTypes_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::RemoveTypes_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<Args_t...>;
        using RemainingComponents_t = Seq::RemoveTypes_t<MkCmps_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, MkCmps_t>,
                      "Components arguments does not match the requiered components");
        using ComponentIDs_t = typename entity_type<DestSig_t>::ComponentIDs_t;

        auto old_ids {
            mEntityMan.template Destroy(ent_id)
        };
        auto new_ids { CreateComponents<RemainingComponents_t>(args...) };

        auto ids { TupleAs<ComponentIDs_t>(std::tuple_cat(new_ids, old_ids)) };

        Seq::Unpacker_t<RmCmps_t>::Call([&]<class... Cmps_t>() {
                    (mComponentMan.Destroy(std::get<ID_t<Cmps_t>>(old_ids)),
                    ...);
                });

        return std::apply([&](auto... ids) {
                    return CreateEntityIMPL<DestSig_t>(ids...);
                    }, ids);
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
        using NewSig_t = Seq::RemoveTypes_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
        static_assert(IsInstanceOf_v<SrcSig_t, BaseSig_t>, "Not base from ent.");
    }

    template<class Cmpt_t, class EntID_t> constexpr auto
    GetComponent(EntID_t ent_id) const -> const Cmpt_t&
    {
        using EntSig_t = typename EntID_t::value_type;
        auto& ent { mEntityMan.template GetEntity(ent_id) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());
    }

    template<class Cmpt_t, class EntID_t> constexpr auto
    GetComponent(EntID_t ent_id) -> auto&
    {
        using EntSig_t = typename EntID_t::value_type;
        auto& ent { mEntityMan.template GetEntity(ent_id) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());;
    }

    template<class CmpID_t> constexpr auto
    GetComponent(CmpID_t cmp_id) const -> const typename CmpID_t::value_type&
    {
        return mComponentMan.GetComponent(cmp_id);
    }

    template<class CmpID_t> constexpr auto
    GetComponent(CmpID_t cmp_id) -> typename CmpID_t::value_type&
    {
        return mComponentMan.GetComponent(cmp_id);
    }

    template<class Cmpt_t, class EntID_t> constexpr auto
    GetComponentKey(EntID_t ent_id) const -> auto
    {
        using EntSig_t = typename EntID_t::value_type;
        return mEntityMan.template GetEntity(ent_id)
                         .template GetComponentID<Cmpt_t>();
    }

    template<class... Cmps_t, class EntID_t> constexpr auto
    GetComponents(EntID_t ent_id) const -> std::tuple<const Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_id)... };
    }

    template<class... Cmps_t, class EntID_t> constexpr auto
    GetComponents(EntID_t ent_id) -> std::tuple<Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_id)... };
    }

    template<class Signature_t, class EntID_t> constexpr auto
    GetComponentsFor(EntID_t ent_id) const -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts_t>() {
                    return std::forward_as_tuple(GetComponent<Ts_t>(ent_id)...);
                });
    }

    template<class Signature_t, class EntID_t> constexpr auto
    GetComponentsFor(EntID_t ent_id) -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call([&]<class... Ts_t>() {
                    return std::forward_as_tuple(GetComponent<Ts_t>(ent_id)...);
                });
    }

    template<class EntSig_t, class Callable_t> constexpr auto
    ForEachEntity(Callable_t&& cb) const -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntSig_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

    template<class EntSig_t, class Callable_t> constexpr auto 
    ForEachEntity(Callable_t&& cb) -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntSig_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }
private:
    ComponentMan_t mComponentMan {  };
    EntityMan_t    mEntityMan    {  };
};

} // namespace ECS
