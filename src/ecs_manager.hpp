#pragma once

#include <tmpl/tmpl.hpp>
#include <type_traits>

#include "arguments.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "helpers.hpp"
#include "interface.hpp"

namespace ECS
{

template<class... EntitySignatures_t>
using ComponentsFrom_t = Seq::UniqueTypes_t<Seq::SeqCat_t<typename EntitySignatures_t::type...>>;

template<class... EntSigs_t>
struct ECSManager_t
{

private:
    using ComponentList_t = ComponentsFrom_t<EntSigs_t...>;
    using EntityList_t    = TMPL::TypeList_t<EntSigs_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;

    template<class T>
    using ComponentKey_t = typename ComponentMan_t::template ComponentKey_t<T>;
    
    using EntityMan_t = EntityManager_t<ComponentKey_t, EntSigs_t...>;

    template<class T>
    using EntityIndex_t = Identifier_t<ECSManager_t, T,
                                       typename EntityMan_t::template EntityID_t<T>>;

    template<class T>
    using entity_type = typename EntityMan_t::template entity_type<T>;

    template<class SysSig_t>
    struct EntityTraversal_t
    {
        template<class EntSig_t, class Callback_t, class ECSMan_t>
        constexpr auto operator()(Callback_t&& cb, ECSMan_t& ecs_man) const -> void
        {
            if constexpr (TMPL::IsSubsetOf_v<typename SysSig_t::type,
                                             typename EntSig_t::type>) {
                auto i { ecs_man.mEntityMan.template size<EntSig_t>() };
                while (i--) {
                    EntityIndex_t<EntSig_t> ent_idx { mConstructorKey_t, i };
                    std::apply(cb, ecs_man.template GetArgumentsFor<typename SysSig_t::type>(ent_idx));
                }
            }
        }
    };

    struct SystemArguments_t
    {
        template<class... Args_t, class EntIdx_t, class ECSMan_t>
        constexpr auto operator()(EntIdx_t ent_idx, ECSMan_t&& ecs_man) const
        {
            using EntSig_t = typename EntIdx_t::type;

            auto& ent { ecs_man.mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
            return std::tuple<AddConstIf_t<ECSMan_t, Args_t>&..., EntIdx_t>{
                ecs_man.mComponentMan.GetComponent(ent.template GetComponentID<Args_t>())...,
                ent_idx };
        }
    };

    struct ComponentCreator_t
    {
    private:
        ECSManager_t& mECSMan {  };
    public:
        constexpr explicit ComponentCreator_t(ECSManager_t& ecs_man)
            : mECSMan { ecs_man } {  }

        template<class... Cmps_t> constexpr auto operator()() const
        {
            return std::tuple {
                mECSMan.mComponentMan.template Create<Cmps_t>()... };
        }
    };

    struct ComponentDestroyer_t
    {
    private:
        ECSManager_t& mECSMan {  };
    public:
        constexpr explicit ComponentDestroyer_t(ECSManager_t& ecs_man)
            : mECSMan { ecs_man } {  }

        template<class... Cmps_t, class TupKeys_t>
        constexpr auto operator()(TupKeys_t&& cmp_keys) const
        {
            (mECSMan.mComponentMan.Destroy(std::get<ComponentKey_t<Cmps_t>>
                                           (std::forward<TupKeys_t>(cmp_keys))),
             ...);
        }
    };

    template<class CmptArgs_t> constexpr auto
    CreateComponent(const CmptArgs_t& arg)
    {
        using RequieredComponent_t = typename CmptArgs_t::type;

        auto create_cmp {
            [&](auto&&... args) {
                return mComponentMan.template Create<RequieredComponent_t>
                    (std::forward<decltype(args)>(args)...);
            }
        };

        return Args::apply(create_cmp, arg);
    }

    template<class Signature_t, class EntIdx_t>
    constexpr auto GetArgumentsFor(EntIdx_t&& it) const
    {
        return Seq::Unpacker_t<Signature_t>::Call(SystemArguments_t{  },
                                                  std::forward<EntIdx_t>(it),
                                                  *this);
    }

    template<class Signature_t, class EntIdx_t>
    constexpr auto GetArgumentsFor(EntIdx_t it)
    {
        return Seq::Unpacker_t<Signature_t>::Call(SystemArguments_t{  },
                                                  std::forward<EntIdx_t>(it),
                                                  *this);
    }
public:
    template<class EntSig_t, class EntIdx_t>
    struct StaticCast
    : std::bool_constant<std::is_same_v<EntSig_t,
                         typename std::remove_reference_t<EntIdx_t>::type>>
                          {  };

    template<class EntSig_t, class EntKey_t>
    static inline constexpr bool StaticCast_v { StaticCast<EntSig_t, EntKey_t>::value };

    template<class EntSig_t, class... Args_t> constexpr auto
    CreateEntity(const Args_t&... args) -> void
    {
        using ArgsTypes = TMPL::TypeList_t<typename Args_t::type...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntSig_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        auto create_entity {
            [&](auto&&... ids) -> void {
                mEntityMan.template
                    Create<EntSig_t>(std::forward<decltype(ids)>(ids)...);
            }
        };

        using ComponentKeys_t = typename entity_type<EntSig_t>::ComponentIDs_t;
        auto create_components { 
            [&]() {
                return TupleAs<ComponentKeys_t>(
                        std::tuple_cat(std::tuple{
                            CreateComponent(args)...
                            },
                            Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{ *this }))
                        );
            }
        };

        std::apply(create_entity, create_components());
    }

    template<class EntIdx_t>
    constexpr auto Destroy(EntIdx_t ent_idx) -> void
    {
        using EntSig_t = typename EntIdx_t::type;
        auto cmp_ids {
            mEntityMan.template Destroy<EntSig_t>(ent_idx.mID)
        };

        std::apply([&](auto&&... keys) {
                    (mComponentMan.Destroy(keys), ...);
                }, cmp_ids);
    }

    template<class DestSig_t, class EntIdx_t, class... Args_t>
    constexpr auto TransformTo(EntIdx_t ent_idx, const Args_t&... args) -> void
    {
        using SrcSig_t   = typename EntIdx_t::type;
        using DestCmps_t = ComponentsFrom_t<DestSig_t>;
        using SrcCmps_t  = ComponentsFrom_t<SrcSig_t>;
        using RmCmps_t   = Seq::RemoveTypes_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::RemoveTypes_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Args_t>::type...>;
        using RemainingComponents_t = Seq::RemoveTypes_t<MkCmps_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, MkCmps_t>,
                      "Components arguments does not match the requiered components");

        auto cmp_ids {
            mEntityMan.template Destroy<SrcSig_t>(ent_idx)
        };

        auto create_entity {
            [&](auto&&... ids) -> void {
                return mEntityMan.template
                    Create<DestSig_t>(std::forward<decltype(ids)>(ids)...);
            }
        };

        using ComponentKeys_t = typename entity_type<DestSig_t>::ComponentIDs_t;
        auto create_components { 
            [&]() {
                return TupleAs<ComponentKeys_t>(
                        std::tuple_cat(std::tuple{
                            CreateComponent(args)...
                            },
                            Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{ *this }),
                            cmp_ids)
                        );
            }
        };

        Seq::Unpacker_t<RmCmps_t>::Call(ComponentDestroyer_t{ *this }, cmp_ids);

        std::apply(create_entity, create_components());
    }

    template<class Cmpt_t, class EntIdx_t> constexpr const auto&
    GetComponent(EntIdx_t ent_idx) const
    {
        using EntSig_t = typename EntIdx_t::type;
        auto& ent { mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());;
    }

    template<class Cmpt_t, class EntIdx_t> constexpr auto&
    GetComponent(EntIdx_t ent_idx)
    {
        return SameAsConstMemFunc(*this, &ECSManager_t::GetComponent<Cmpt_t, EntIdx_t>, ent_idx);
    }

    template<class EntSig_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) const -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntSig_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

    template<class EntSig_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntSig_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }
private:
    static inline constexpr Key_t<ECSManager_t> mConstructorKey_t {  };
    ComponentMan_t mComponentMan {  };
    EntityMan_t mEntityMan {  };
};

} // namespace ECS
