#pragma once

#include <tmpl/tmpl.hpp>

#include "arguments.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "helpers.hpp"
#include "interface.hpp"
#include "tmpl/sequence.hpp"
#include "tmpl/type_list.hpp"

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

    template<class EmptyCmps_t, std::size_t... Is, class... Args_t>
    constexpr auto CreateComponentsIMPL(std::index_sequence<Is...>, Args_t&&... args)
    {
       return std::tuple {
           CreateComponent(std::forward<Args_t>(args))..., 
           CreateComponent(Args::Arguments_t<Seq::TypeAt_t<Is, EmptyCmps_t>>{})... };
    }

    template<class EmptyCmps_t, class... Args_t>
    constexpr auto CreateComponents(Args_t&&... args)
    {
        return CreateComponentsIMPL<EmptyCmps_t>(
                std::make_index_sequence<Seq::Size_v<EmptyCmps_t>>{},
                std::forward<Args_t>(args)...);
    }

    template<class EntSig_t, class... IDs>
    constexpr auto CreateEntityIMPL(IDs... ids)
    {
        mEntityMan.template Create<EntSig_t>(ids...);
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
    template<class EntSig_t, class... Args_t> constexpr auto
    CreateEntity(const Args_t&... args) -> void
    {
        using ArgsTypes = TMPL::TypeList_t<typename Args_t::type...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntSig_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        using ComponentKeys_t = typename entity_type<EntSig_t>::ComponentIDs_t;

        auto cmp_ids { TupleAs<ComponentKeys_t>(CreateComponents<RemainingComponents_t>(args...)) };

        std::apply([&](auto... ids) {
                CreateEntityIMPL<EntSig_t>(ids...);
                }, cmp_ids);
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
        using DestCmps_t = typename DestSig_t::type;
        using SrcCmps_t  = typename SrcSig_t::type;
        using RmCmps_t   = Seq::RemoveTypes_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::RemoveTypes_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Args_t>::type...>;
        using RemainingComponents_t = Seq::RemoveTypes_t<MkCmps_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, MkCmps_t>,
                      "Components arguments does not match the requiered components");
        using ComponentKeys_t = typename entity_type<DestSig_t>::ComponentIDs_t;

        auto old_ids {
            mEntityMan.template Destroy<SrcSig_t>(ent_idx.mID)
        };
        auto new_ids { CreateComponents<RemainingComponents_t>(args...) };

        auto ids { TupleAs<ComponentKeys_t>(std::tuple_cat(new_ids, old_ids)) };

        Seq::Unpacker_t<RmCmps_t>::Call(ComponentDestroyer_t{ *this }, old_ids);

        std::apply([&](auto... ids) {
                CreateEntityIMPL<DestSig_t>(ids...);
                }, ids);
    }

    template<class BaseSig_t, class EntIdx_t, class... Args_t>
    constexpr auto AddBase(EntIdx_t ent_idx, Args_t&&... args) -> void
    {
        using SrcSig_t = typename EntIdx_t::type;
        using NewSig_t = Seq::SeqCat_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
        
    }

    template<class BaseSig_t, class EntIdx_t>
    constexpr auto RemoveBase(EntIdx_t ent_idx) -> void
    {
        using SrcSig_t = typename EntIdx_t::type;
        using NewSig_t = Seq::RemoveTypes_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
        static_assert(IsInstanceOf_v<SrcSig_t, BaseSig_t>, "Not base from ent.");
    }

    template<class Cmpt_t, class EntIdx_t> constexpr const auto&
    GetComponent(EntIdx_t ent_idx) const
    {
        using EntSig_t = typename EntIdx_t::type;
        auto& ent { mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());
    }

    template<class Cmpt_t, class EntIdx_t> constexpr auto&
    GetComponent(EntIdx_t ent_idx)
    {
        using EntSig_t = typename EntIdx_t::type;
        auto& ent { mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());;
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
