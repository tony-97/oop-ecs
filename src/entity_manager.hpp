#pragma once

#include <list>
#include <type_traits>
#include <memory>

#include "helpers.hpp"
#include "type_aliases.hpp"
#include "type_list.hpp"
#include "entity.hpp"

#include "component_storage.hpp"
#include "argument_packer.hpp"

template<class TListOut_t, class... Ts>
struct SubstractTypeListsIMPL_t
{
    using type = TListOut_t;
};

template<template<class...> class TListOut_t, class... Ts,
         template<class...> class TListA_t, class A, class... As,
         template<class...> class TListB_t, class... Bs>
struct SubstractTypeListsIMPL_t<TListOut_t<Ts...>, TListA_t<A, As...>, TListB_t<Bs...>>
    : std::conditional_t<(std::is_same_v<A, Bs> || ...),
                         SubstractTypeListsIMPL_t<TListOut_t<Ts...>, TListA_t<As...>, TListB_t<Bs...>>,
                         SubstractTypeListsIMPL_t<TListOut_t<Ts..., A>, TListA_t<As...>, TListB_t<Bs...>>> {  };

template<class TList1_t, class TList2_t>
struct SubstractTypeLists
    : SubstractTypeListsIMPL_t<TMPL::TypeList_t<>, TList1_t, TList2_t> {  };

template<class TList1_t, class TList2_t>
using SubstractTypeLists_t = typename SubstractTypeLists<TList1_t, TList2_t>::type;

namespace ECS
{

template<class... SystemsSignature_t>
using ComponentExtractor_t = TMPL::UniqueTypeList_t<TMPL::TypeListCat_t<SystemsSignature_t...>>;

// SystemSignature_t is a variadic template argument that each one is a list of components
template<class... SystemSignature_t>
struct EntityManager_t
{
private:

    using Self_t = EntityManager_t;

    using OwnEntitity_t = Entity_t<Self_t>;

    using ComponentList_t = ComponentExtractor_t<SystemSignature_t...>;
    using ComponentWarehouse_t = ComponentStorage_t<ComponentList_t>;

    using EntityVector_t = Vector_t<std::unique_ptr<OwnEntitity_t>>;

    struct ComponentCreator_t
    {
        Self_t& ent_man {  };
        template<class RequieredComponent_t, class... Args_t>
        constexpr auto
        operator()(OwnEntitity_t& ent, Args_t&&... args)
        -> void
        {
            ent_man.CreateRequieredComponent<RequieredComponent_t>(ent, std::forward<Args_t>(args)...);
        }
    };

    struct ComponentTraversal_t
    {
        template<class Head_t, class... Tail_t, class Callable_t, class EntMan_t>
        constexpr auto
        operator()(Callable_t&& callback, EntMan_t&& ent_man) const -> void
        {
            auto it  { ent_man.mComponents.template begin<Head_t>() };
            auto end { ent_man.mComponents.template end<Head_t>() };
            for (; it != end; ++it) {
                auto& [ent_id, cmp] { *it };
                auto& ent { ent_man.GetRequiredEntity(ent_id) };
                callback(cmp, ent_man.template GetRequiredComponent<Tail_t>(ent)..., ent);
            }
        }

        template<class Callable_t, class EntityType_t>
        void operator()(Callable_t&&, EntityType_t&) {  }
    };

    template<class RequieredComponent_t, class... Args_t>
    constexpr auto
    CreateRequieredComponent(OwnEntitity_t& ent, Args_t&&... args)
    -> void
    {
        constexpr auto cmp_tp_id
        {
            ComponentWarehouse_t::template
                GetRequiredComponentTypeIndex<RequieredComponent_t>()
        };
        auto cmp_id
        {
            mComponents.template
                        CreateRequiredComponent<RequieredComponent_t>
                        (ent.GetEntityID(), std::forward<Args_t>(args)...)
        };

        ent.AttachComponentID(cmp_tp_id, cmp_id);
    }

    template<class ComponentArgs_t> constexpr auto
    CreateRequieredComponentFromArgs(OwnEntitity_t& ent, ComponentArgs_t&& args)
    -> void
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::For_type;

        auto cmp_creator
        {
            [&](auto&&... args)
            {
                CreateRequieredComponent<RequieredComponent_t>
                    (ent, std::forward<decltype(args)>(args)...);
            }
        };

        Args::apply(cmp_creator, std::forward<ComponentArgs_t>(args));
    }

    template<class RequieredComponent_t, class Ent_t> constexpr auto
    GetRequiredComponent(const Ent_t& ent) -> RequieredComponent_t&
    {
        auto cmp_tp_id { mComponents.template GetRequiredComponentTypeIndex<RequieredComponent_t>() };
        auto cmp_id { ent.GetRequiredComponentID(cmp_tp_id) };
        return mComponents.template GetRequiredComponent<RequieredComponent_t>(cmp_id);
    }

    template<class RequieredComponent_t, class Ent_t> constexpr auto
    GetRequiredComponent(const Ent_t& ent) const -> const RequieredComponent_t&
    {
        return SameAsConstMemFunc(this,
                                  &Self_t::GetRequiredComponent<RequieredComponent_t, const OwnEntitity_t>,
                                  ent);
    }

    constexpr auto GetRequiredEntity(EntityID_t eid) const -> const OwnEntitity_t&
    {
        return *mEntitites[eid];
    }

    constexpr auto GetRequiredEntity(EntityID_t eid) -> OwnEntitity_t&
    {
        return *mEntitites[eid];
    }

public:

    using EntityCreationKey_t = Key_t<Self_t>;

    explicit EntityManager_t() = default;

    template<class... Signatures_t , class... ComponentsArgs_t> constexpr auto
    CreateEntityForSystems(ComponentsArgs_t&&... args)
    -> OwnEntitity_t& 
    {
        static_assert(AreUnique_v<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>,
                      "Component arguments must be unique.");
        using RequiredComponents_t = ComponentExtractor_t<Signatures_t...>;
        using ComponentsArgsFor_t  = TMPL::TypeList_t<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>;
        using RemainingComponents_t [[maybe_unused]] = SubstractTypeLists_t<RequiredComponents_t, ComponentsArgsFor_t>;
        static_assert(IsSubsetOf_v<ComponentsArgsFor_t, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");
        auto& ent { *mEntitites.emplace_back(std::make_unique<OwnEntitity_t>(constructor_key, mEntitites.size())) };
        (CreateRequieredComponentFromArgs(ent, std::forward<ComponentsArgs_t>(args)), ...);

        TMPL::ForEach_t<RemainingComponents_t>::Do(ComponentCreator_t{ *this }, ent);
        return ent;
    }

    template<class Signature_t, class Callable_t> constexpr auto
    ForEachEntityType(Callable_t&& callback) -> void
    {
        using Extractor_t = TMPL::TypeListExtractor_t<Signature_t>;
        Extractor_t::InvokeFunctor(ComponentTraversal_t{  }, std::forward<Callable_t>(callback), *this);
    }

    template<class Signature_t, class Callable_t> constexpr auto
    ForEachEntityType(Callable_t&& callback) const -> void
    {
        using Extractor_t = TMPL::TypeListExtractor_t<Signature_t>;
        Extractor_t::InvokeFunctor(ComponentTraversal_t{  }, std::forward<Callable_t>(callback), *this);
    }


private:

    static constexpr Key_t<Self_t> constructor_key {  };

    EntityVector_t mEntitites {  };
    ComponentWarehouse_t mComponents {  };
};

} // namespace ECS

