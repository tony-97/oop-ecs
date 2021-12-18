#pragma once

#include <list>

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

template<class... EntityList_>
using ComponentExtractor_t = TMPL::UniqueTypeList_t<TMPL::TypeListCat_t<EntityList_...>>;

// EntityList_t is a variadic template argument that each one is a list of components
template<class... EntityList_t>
struct EntityManager_t
{
private:

    using Self_t = EntityManager_t;

    template<class EntityType_t>
    using OwnEntitity_t = Entity_t<Self_t, EntityType_t>;

    using ComponentList_t = ComponentExtractor_t<EntityList_t...>;
    using ComponentWarehouse = ComponentStorage_t<ComponentList_t>;

    struct ComponentCreator_t
    {
        Self_t& ent_man {  };
        template<class RequieredComponent_t, class EntityType_t, class... Args_t>
        constexpr auto
        operator()(OwnEntitity_t<EntityType_t>& ent, Args_t&&... args)
        -> void
        {
            ent_man.CreateRequieredComponent<RequieredComponent_t>(ent, std::forward<Args_t>(args)...);
        }
    };

    template<class RequieredComponent_t, class EntityType_t, class... Args_t>
    constexpr auto
    CreateRequieredComponent(OwnEntitity_t<EntityType_t>& ent, Args_t&&... args)
    -> void
    {
        constexpr auto cmp_tp_id
        {
            ComponentWarehouse::template
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

    template<class ComponentArgs_t, class EntityType_t> constexpr auto
    CreateRequieredComponentFromArgs(OwnEntitity_t<EntityType_t>& ent,
                                     ComponentArgs_t&& args)
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

    template<class RequiredEntitiy_t> static constexpr auto
    CheckIfEntityExist() -> void
    {
        static_assert(IsOneOf_v<RequiredEntitiy_t, EntityList_t...>,
                     "The requiered entity does not exists in this instance.");       
    }

    
    static constexpr auto CheckIfEntitiesAreUnique() -> void
    {
        static_assert(AreUnique_v<EntityList_t...>, "Entities should not repeat.");
    }

    template<class RequiredEntitiy_t> constexpr auto
    GetRequiredEntityList() -> std::list<RequiredEntitiy_t>&
    {
        CheckIfEntityExist<RequiredEntitiy_t>();
        constexpr auto index { TMPL::IndexOf_v<RequiredEntitiy_t, TMPL::TypeList_t<EntityList_t...>> };
        return std::get<std::list<RequiredEntitiy_t>>(mEntityTable[index]);
    }

public:

    using EntityCreationKey_t = Key_t<Self_t>;

    explicit EntityManager_t() : mEntityTable { std::list<EntityList_t>{  }... }
    {
        CheckIfEntitiesAreUnique();
    }

    template<class RequiredEntitiy_t, class... ComponentsArgs_t> constexpr auto
    CreateRequieredEntity(ComponentsArgs_t&&... args)
    -> OwnEntitity_t<RequiredEntitiy_t>& 
    {
        static_assert(AreUnique_v<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>,
                      "Component arguments must be unique.");
        using RequiredComponents_t = ComponentExtractor_t<RequiredEntitiy_t>;
        using ComponentsArgsFor_t  = TMPL::TypeList_t<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>;
        using RemainingComponents_t [[maybe_unused]] = SubstractTypeLists_t<RequiredComponents_t, ComponentsArgsFor_t>;
        static_assert(IsSubsetOf_v<ComponentsArgsFor_t, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");
        auto& ents { GetRequiredEntityList<RequiredEntitiy_t>() };
        auto& ent { ents.emplace_back(constructor_key, mEntityTable.size()) };
        (CreateRequieredComponentFromArgs(ent, std::forward<ComponentsArgs_t>(args)), ...);

        TMPL::ForEach_t<RemainingComponents_t>::Do(ComponentCreator_t{ *this }, ent);
        return ent;
    }

private:

    static constexpr Key_t<Self_t> constructor_key {  };

    std::array<std::variant<std::list<OwnEntitity_t<EntityList_t>>...>, sizeof...(EntityList_t)> mEntityTable;
    ComponentWarehouse mComponents {  };
};

} // namespace ECS

