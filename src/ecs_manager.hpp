#pragma once

#include <tmpl/tmpl.hpp>
#include <tmpl/sequence.hpp>
#include <tmpl/type_list.hpp>

#include "arguments.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"
#include "helpers.hpp"
#include "interface.hpp"

#include <type_traits>

namespace ECS
{
template<class... EntitySignatures_t>
using ComponentsFrom_t = Seq::UniqueTypes_t<Seq::SeqCat_t<typename EntitySignatures_t::type...>>;

template<class Fn_t, class... Args_t>
struct IsSystemCallable;

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t, class EntIdx_t>
struct IsSystemCallable<Fn_t, Sig_t<Sigs_t...>, EntIdx_t>
: std::bool_constant<std::is_invocable_v<Fn_t, Sigs_t&..., EntIdx_t>> {  };

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t>
struct IsSystemCallable<Fn_t, Sig_t<Sigs_t...>>
: std::bool_constant<std::is_invocable_v<Fn_t, Sigs_t&...>> {  };

template<class Fn_t, class... Args_t>
static inline constexpr auto IsSystemCallable_v { IsSystemCallable<Fn_t, Args_t...>::value };

template<class... EntSigs_t>
struct ECSManager_t
{
private:
    using ComponentList_t = ComponentsFrom_t<EntSigs_t...>;
    using EntityList_t    = TMPL::TypeList_t<EntSigs_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;
    using EntityMan_t     = EntityManager_t<ComponentKey_t, EntSigs_t...>;

    template<class T>
    using EntityIndex_t = Identifier_t<ECSManager_t, T,
                                       typename EntityMan_t::template EntityID_t<T>>;

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
                auto i { ecs_man.mEntityMan.template size<EntSig_t>() };
                while (i--) {
                    EntityIndex_t<EntSig_t> ent_idx { mConstructorKey_t, i };
                    if constexpr (IsSystemCallable_v<Callback_t, Cmps_t, EntityIndex_t<EntSig_t>>) {
                        std::apply(std::forward<Callback_t>(cb), ecs_man.template GetArgumentsFor<SysSig_t>(ent_idx));
                    } else if constexpr (   IsSystemCallable_v<Callback_t, Cmps_t>
                                         && Seq::Size_v<Cmps_t> > 1) {
                        std::apply(std::forward<Callback_t>(cb), ecs_man.template GetComponentsFor<SysSig_t>(ent_idx));
                    } else {
                        std::apply(std::forward<Callback_t>(cb), std::tuple{ ent_idx });
                    }
                }
            }
        }
    };

    struct SystemArguments_t
    {
        template<class... Args_t, class EntIdx_t, class ECSMan_t> constexpr auto
        operator()(EntIdx_t ent_idx, ECSMan_t& ecs_man) const -> auto
        {
            return std::tuple<AddConstIf_t<ECSMan_t, Args_t>&..., EntIdx_t>{
                ecs_man.template GetComponent<Args_t>(ent_idx)...,
                ent_idx };
        }
    };

    struct SystemComponents_t
    {
        template<class... Args_t, class EntIdx_t, class ECSMan_t> constexpr auto 
        operator()(EntIdx_t ent_idx, ECSMan_t& ecs_man) const -> auto
        {
            return ecs_man.template GetComponents<Args_t...>(ent_idx);
        }
    };

    struct ComponentDestroyer_t
    {
    private:
        ECSManager_t& mECSMan {  };
    public:
        constexpr explicit ComponentDestroyer_t(ECSManager_t& ecs_man)
            : mECSMan { ecs_man } {  }

        template<class... Cmps_t, class TupKeys_t> constexpr auto
        operator()([[maybe_unused]] TupKeys_t cmp_keys) const -> auto
        {
            (mECSMan.mComponentMan.Destroy(std::get<ComponentKey_t<Cmps_t>>(cmp_keys)),
             ...);
        }
    };

    template<class CmptArgs_t> constexpr auto
    CreateComponent(const CmptArgs_t& arg) -> auto
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

    template<class EmptyCmps_t, std::size_t... Is, class... Args_t> constexpr auto 
    CreateComponentsIMPL(std::index_sequence<Is...>, const Args_t&... args) -> auto
    {
       return std::tuple {
           CreateComponent(args)..., 
           CreateComponent(Args::Arguments_t<Seq::TypeAt_t<Is, EmptyCmps_t>>{})... };
    }

    template<class EmptyCmps_t, class... Args_t> constexpr auto
    CreateComponents(const Args_t&... args) -> auto
    {
        return CreateComponentsIMPL<EmptyCmps_t>(
                std::make_index_sequence<Seq::Size_v<EmptyCmps_t>>{},
                args...);
    }

    template<class EntSig_t, class... IDs> constexpr auto
    CreateEntityIMPL(IDs... ids) -> const auto&
    {
        return mEntityMan.template Create<EntSig_t>(ids...);
    }

    template<class Signature_t, class EntIdx_t> constexpr auto
    GetArgumentsFor(EntIdx_t it) const -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call(SystemArguments_t{  }, it, *this);
    }

    template<class Signature_t, class EntIdx_t> constexpr auto
    GetArgumentsFor(EntIdx_t it) -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call(SystemArguments_t{  }, it, *this);
    }
public:
    template<class EntSig_t, class... Args_t> constexpr auto
    CreateEntity(const Args_t&... args) -> const auto&
    {
        using ArgsTypes = TMPL::TypeList_t<typename Args_t::type...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntSig_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        using ComponentKeys_t = typename entity_type<EntSig_t>::ComponentIDs_t;

        auto cmp_ids { TupleAs<ComponentKeys_t>(CreateComponents<RemainingComponents_t>(args...)) };

        return std::apply([&](auto... ids) -> const auto& {
                    return CreateEntityIMPL<EntSig_t>(ids...);
                    }, cmp_ids);
    }

    template<class EntIdx_t> constexpr auto
    Destroy(EntIdx_t ent_idx) -> void
    {
        using EntSig_t = typename EntIdx_t::type;
        auto cmp_ids {
            mEntityMan.template Destroy<EntSig_t>(ent_idx.mID)
        };

        std::apply([&](auto... keys) {
                    (mComponentMan.Destroy(keys), ...);
                }, cmp_ids);
    }

    template<class DestSig_t, class EntIdx_t, class... Args_t> constexpr auto
    TransformTo(EntIdx_t ent_idx, const Args_t&... args) -> const auto& 
    {
        using SrcSig_t   = typename EntIdx_t::type;
        using DestCmps_t = typename DestSig_t::type;
        using SrcCmps_t  = typename SrcSig_t::type;
        using RmCmps_t   = Seq::RemoveTypes_t<SrcCmps_t, DestCmps_t>;
        using MkCmps_t   = Seq::RemoveTypes_t<DestCmps_t, SrcCmps_t>;

        using ArgsTypes = TMPL::TypeList_t<typename Args_t::type...>;
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

        return std::apply([&](auto... ids) -> const auto& {
                    return CreateEntityIMPL<DestSig_t>(ids...);
                    }, ids);
    }

    template<class BaseSig_t, class EntIdx_t, class... Args_t> constexpr auto
    AddBase(EntIdx_t ent_idx, Args_t&&... args) -> void
    {
        using SrcSig_t = typename EntIdx_t::type;
        using NewSig_t = Seq::SeqCat_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
    }

    template<class BaseSig_t, class EntIdx_t> constexpr auto
    RemoveBase(EntIdx_t ent_idx) -> void
    {
        using SrcSig_t = typename EntIdx_t::type;
        using NewSig_t = Seq::RemoveTypes_t<SrcSig_t, TMPL::TypeList_t<BaseSig_t>>;
        static_assert(IsInstanceOf_v<SrcSig_t, BaseSig_t>, "Not base from ent.");
    }

    template<class Cmpt_t, class EntIdx_t> constexpr auto
    GetComponent(EntIdx_t ent_idx) const -> const Cmpt_t&
    {
        using EntSig_t = typename EntIdx_t::type;
        auto& ent { mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());
    }

    template<class Cmpt_t, class EntIdx_t> constexpr auto
    GetComponent(EntIdx_t ent_idx) -> auto&
    {
        using EntSig_t = typename EntIdx_t::type;
        auto& ent { mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID) };
        
        return mComponentMan.GetComponent(ent.template GetComponentID<Cmpt_t>());;
    }

    template<class CmpKey_t> constexpr auto
    GetComponent(CmpKey_t cmp_key) const -> const typename CmpKey_t::value_type::type&
    {
        return mComponentMan.GetComponent(cmp_key);
    }

    template<class CmpKey_t> constexpr auto
    GetComponent(CmpKey_t cmp_key) -> typename CmpKey_t::value_type::type&
    {
        return mComponentMan.GetComponent(cmp_key);
    }

    template<class Cmpt_t, class EntIdx_t> constexpr auto
    GetComponentKey(EntIdx_t ent_idx) const -> auto
    {
        using EntSig_t = typename EntIdx_t::type;
        return mEntityMan.template GetEntity<EntSig_t>(ent_idx.mID)
                         .template GetComponentID<Cmpt_t>();
    }

    template<class... Cmps_t, class EntIdx_t> constexpr auto
    GetComponents(EntIdx_t ent_idx) const -> std::tuple<const Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_idx)... };
    }

    template<class... Cmps_t, class EntIdx_t> constexpr auto
    GetComponents(EntIdx_t ent_idx) -> std::tuple<Cmps_t&...>
    {
        return { GetComponent<Cmps_t>(ent_idx)... };
    }

    template<class Signature_t, class EntIdx_t> constexpr auto
    GetComponentsFor(EntIdx_t it) const -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call(SystemComponents_t{  },
                                             it,
                                             *this);
    }

    template<class Signature_t, class EntIdx_t> constexpr auto
    GetComponentsFor(EntIdx_t it) -> auto
    {
        using Cmps_t = typename Signature_t::type;
        return Seq::Unpacker_t<Cmps_t>::Call(SystemComponents_t{  },
                                             it,
                                             *this);
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
    static inline constexpr Key_t<ECSManager_t> mConstructorKey_t {  };
    ComponentMan_t mComponentMan {  };
    EntityMan_t mEntityMan {  };
};

} // namespace ECS
