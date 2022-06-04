#pragma once

#include <tmpl/tmpl.hpp>

#include "arguments.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<bool b, class... Ts>
struct RawBase
{
    static_assert(b, "Types are not unique.");
};

template<class... Ts>
struct RawBase<true, Ts...>
{
    using type = Seq::UniqueTypes_t<TMPL::TypeList_t<Ts...>>;
};

template<class... Ts>
struct Base_t : RawBase<TMPL::AreUnique_v<Ts...>, Ts...> {  };

template<bool b, class... Ts>
struct RawDerived
{
    static_assert(b, "Derived not derived from Base.");
};

template<class... Ts>
struct RawDerived<true, Ts...>
{
    using type = Seq::UniqueTypes_t<Seq::SeqCat_t<Ts...>>;
};

template<class T>
struct IsBase : std::false_type {  };

template<template<class...> class T, class... Ts>
struct IsBase<T<Ts...>> : std::bool_constant<std::is_same_v<T<Ts...>, Base_t<Ts...>>> {  };

template<class T>
constexpr static inline auto IsBase_v { IsBase<T>::value };

template<class... Ts>
struct Derived_t : RawDerived<std::conjunction_v<IsBase<Ts>...>, Ts...> {  };

template<class... EntitySignatures_t>
using ComponentsFrom_t = Seq::UniqueTypes_t<Seq::SeqCat_t<typename EntitySignatures_t::type...>>;

template<class... EntitySignatures_t>
struct ECSManager_t
{
private:

    template<class SystemSignature_t>
    struct EntityTraversal_t
    {
        template<class EntitySignature_t, class Callback_t, class ECSMan_t>
        constexpr auto operator()(Callback_t&& cb, ECSMan_t&& ecs_man) const -> void
        {
            using RequiredEntity_t = Entity_t<EntitySignature_t>;
            if constexpr (TMPL::IsSubsetOf_v<typename SystemSignature_t::type,
                                             typename EntitySignature_t::type>) {
                auto i { ecs_man.mEntityMan.template size<RequiredEntity_t>() };
                while (i--) {
                    const ID_t<EntitySignature_t, std::size_t> idx { i };
                    std::apply(cb, ecs_man.template GetArgumentsFor<typename SystemSignature_t::type>(idx));
                }
                //auto it  { ecs_man.mEntityMan.template rbegin<RequiredEntity_t>() };
                //auto end { ecs_man.mEntityMan.template rend<RequiredEntity_t>() };
                //for (; it != end; ++it) {
                //    std::apply(cb, ecs_man.template GetArgumentsFor<typename SystemSignature_t::type>(std::next(it).base()));
                //}
            }
        }
    };

    struct SystemArguments_t
    {
        template<class... Args_t, class It_t, class ECSMan_t>
        constexpr auto operator()(It_t&& it, ECSMan_t&& ecs_man) const
        {
            using EntSig_t = typename std::remove_reference_t<It_t>::type;
            auto ent_key { ecs_man.mEntityMan.template GetKey<EntSig_t>(it.mID) };
            auto& ent { ecs_man.mEntityMan.template operator[]<Entity_t<EntSig_t>>(it.mID) };
            return std::tuple<AddConstIf_t<ECSMan_t, Args_t>&..., decltype(ent_key)>{
                ecs_man.GetComponent(ent.template GetComponentID<Args_t>())...,
                ent_key };
        }
    };

    struct ComponentCreator_t
    {
    private:
        ECSManager_t& mECSMan {  };
    public:
        constexpr explicit ComponentCreator_t(ECSManager_t& ecs_man)
            : mECSMan { ecs_man } {  }

        template<class... Components_t> constexpr auto operator()() const
        {
            return std::tuple {
                mECSMan.mComponentMan.template Create<Components_t>()... };
        }
    };

    struct ComponentDestroyer_t
    {
    private:
        ECSManager_t& mECSMan {  };
    public:
        constexpr explicit ComponentDestroyer_t(ECSManager_t& ecs_man)
            : mECSMan { ecs_man } {  }

        template<class... Components_t, class ComponentKeys_t>
        constexpr auto operator()(ComponentKeys_t&& cmp_keys) const
        {
            (mECSMan.mComponentMan.Destroy(std::get<typename ECSMap_t<ComponentWrapper<Components_t>>::Key_t>
                                           (std::forward<ComponentKeys_t>(cmp_keys))),
             ...);
        }
    };

    template<class ComponentArgs_t> constexpr auto
    CreateComponent(ComponentArgs_t&& arg)
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::type;

        auto cmp_creator {
            [&](auto&&... args) {
                return mComponentMan.template Create<RequieredComponent_t>
                    (std::forward<decltype(args)>(args)...);
            }
        };

        return Args::apply(cmp_creator, std::forward<ComponentArgs_t>(arg));
    }

    template<class CmpID_t>
    constexpr auto GetComponent(const CmpID_t& cmp_id) const -> const auto&
    {
        return mComponentMan.GetComponent(cmp_id);
    }

    template<class CmpID_t>
    constexpr auto GetComponent(const CmpID_t& cmp_id) -> auto&
    {
        return mComponentMan.GetComponent(cmp_id);
    }

    template<class PrxEnt_t>
    constexpr auto GetEntity(const PrxEnt_t prx_ent) const -> const Entity_t<typename PrxEnt_t::type>&
    {
        return mEntityMan.template operator[]<Entity_t<typename PrxEnt_t::type>>(prx_ent.mID);
    }

    template<class PrxEnt_t>
    constexpr auto GetEntity(const PrxEnt_t prx_ent) -> Entity_t<typename PrxEnt_t::type>&
    {
        return SameAsConstMemFunc(this, &ECSManager_t::GetEntity<PrxEnt_t>, prx_ent);
    }

    template<class Signature_t, class It_t>
    constexpr auto GetArgumentsFor(It_t&& it) const
    {
        return Seq::Unpacker_t<Signature_t>::Call(SystemArguments_t{  },
                                                  std::forward<It_t>(it),
                                                  *this);
    }

    template<class Signature_t, class It_t>
    constexpr auto GetArgumentsFor(It_t&& it)
    {
        return Seq::Unpacker_t<Signature_t>::Call(SystemArguments_t{  },
                                                  std::forward<It_t>(it),
                                                  *this);
    }

public:

    using ComponentList_t = ComponentsFrom_t<EntitySignatures_t...>;
    using EntityList_t    = TMPL::TypeList_t<EntitySignatures_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;
    using EntityMan_t     = EntityManager_t<Entity_t<EntitySignatures_t>...>;

    template<class EntitySignature_t, class... Arguments_t> constexpr auto
    CreateEntity(Arguments_t&&... args)
    {
        using RequiredEntity_t = Entity_t<EntitySignature_t>;
        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Arguments_t>::type...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntitySignature_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        auto create_entity {
            [&](auto&&... ids) {
                return mEntityMan.template
                    Create<RequiredEntity_t>(std::forward<decltype(ids)>(ids)...);
            }
        };

        using ComponentIDs_t = typename RequiredEntity_t::ComponentIDs_t;

        auto create_components { 
            [&]() {
                return ConvertTo<ComponentIDs_t>(
                        std::tuple_cat(std::tuple{
                            CreateComponent(std::forward<Arguments_t>(args))...
                            },
                            Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{ *this }))
                        );
            }
        };

        return std::apply(create_entity, create_components());
    }

    template<class EntKey_t>
    void Destroy(EntKey_t& ent_key)
    {
        auto cmp_ids {
            mEntityMan.Destroy(std::move(ent_key))
        };

        std::apply([&](auto&&... keys) {
                    (mComponentMan.Destroy(keys), ...);
                }, cmp_ids);
    }

    template<class DestSig_t, class EntKey_t, class... Args_t>
    constexpr auto TransformTo(EntKey_t&& ent_key, Args_t&&... args)
    {
        using SrcEnt_t   = typename std::remove_reference_t<EntKey_t>::value_type;
        using DestEnt_t  = Entity_t<DestSig_t>;
        using DestCmps_t = ComponentsFrom_t<DestSig_t>;
        using SrcCmps_t  = ComponentsFrom_t<typename SrcEnt_t::signature_type>;
        using RmCmps_t   = Seq::RemoveTypes_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::RemoveTypes_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Args_t>::type...>;
        using RemainingComponents_t = Seq::RemoveTypes_t<MkCmps_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, MkCmps_t>,
                      "Components arguments does not match the requiered components");

        auto cmp_ids {
            mEntityMan.Destroy(ent_key)
        };

        auto create_entity {
            [&](auto&&... ids) {
                return mEntityMan.template
                    Create<DestEnt_t>(std::forward<decltype(ids)>(ids)...);
            }
        };

        using ComponentIDs_t = typename DestEnt_t::ComponentIDs_t;

        auto create_components { 
            [&]() {
                return ConvertTo<ComponentIDs_t>(
                        std::tuple_cat(std::tuple{
                            CreateComponent(std::forward<Args_t>(args))...
                            },
                            Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{ *this }),
                            cmp_ids)
                        );
            }
        };

        Seq::Unpacker_t<RmCmps_t>::Call(ComponentDestroyer_t{ *this }, cmp_ids);

        return std::apply(create_entity, create_components());
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) const -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntitySignature_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntitySignature_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

private:

    ComponentMan_t mComponentMan {  };
    EntityMan_t mEntityMan {  };
};

} // namespace ECS
