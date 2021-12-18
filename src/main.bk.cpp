#include <tuple>
#include <type_traits>
#include <utility>
#include <list>

#include "helpers.hpp"
#include "type_aliases.hpp"
#include "type_list.hpp"
#include "entity.hpp"

#include "component_storage.hpp"
#include "argument_packer.hpp"

#include <type_traits>

template <class R, class TList_t, class... Ts>
struct RemoveTypeTypeListIMPL_t { using type = TList_t; };

template<class R,
         template<class...> class TList_t, class... Ts,
         class U, class... Us>
struct RemoveTypeTypeListIMPL_t<R, TList_t<Ts...>, U, Us...>
    : std::conditional_t<std::is_same_v<R, U>
                       , RemoveTypeTypeListIMPL_t<TList_t<Ts...>, Us...>
                       , RemoveTypeTypeListIMPL_t<TList_t<Ts..., U>, Us...>> {  };

template <class TList_t, class R>
struct RemoveTypeTypeList;

template <template<class...>class TList_t, class... Ts, class R>
struct RemoveTypeTypeList<TList_t<Ts...>, R>
    : public RemoveTypeTypeListIMPL_t<R, TList_t<>, Ts...> {  };

template <class TList_t, class T>
using RemoveTypeTypeList_t = typename RemoveTypeTypeList<TList_t, T>::type;

//==============================================================================

template<class TListOut_t, class... Ts>
struct SubstractTypeListsIMPL_t
{
    using type = TListOut_t;
};

template<template<class...> class TListOut_t, class... Ts,
         class TListA_t,
         template<class...> class TListB_t, class B, class... Bs>
struct SubstractTypeListsIMPL_t<TListOut_t<Ts...>, TListA_t, TListB_t<B, Bs...>>
    : SubstractTypeListsIMPL_t<RemoveTypeTypeList_t<TListA_t, B>, TListA_t, TListB_t<Bs...>> {  };

template<class TList1_t, class TList2_t>
struct SubstractTypeLists
    : SubstractTypeListsIMPL_t<TMPL::TypeList_t<>, TList1_t, TList2_t> {  };

template<class TList1_t, class TList2_t>
using SubstractTypeLists_t = typename SubstractTypeLists<TList1_t, TList2_t>::type;

namespace ECS
{

template<class Owner_t>
struct Key_t
{
private:
friend Owner_t;
    explicit constexpr Key_t() = default;
};

template<class... EntityList_>
using ComponentExtractor_t = TMPL::UniqueTypeList_t<TMPL::TypeListCat_t<EntityList_...>>;

// EntityList_t is a variadic template argument that each one is a list of components
template<class... EntityList_t>
struct EntityManager_t
{
private:

    using Self_t = EntityManager_t;
    using OwnEntitity_t = Entity_t<Self_t>;

    using ComponentList_t = ComponentExtractor_t<EntityList_t...>;
    using ComponentWarehouse = ComponentStorage_t<ComponentList_t>;

    static constexpr Key_t<Self_t> constructor_key {  };

    std::list<OwnEntitity_t> mEntities {  };
    ComponentWarehouse mComponents {  };

    template<class ComponentArgs_t> constexpr auto
    CreateRequieredComponent(OwnEntitity_t& ent, ComponentArgs_t&& args)
    -> void
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::For_type;

        constexpr auto cmp_tp_id
        {
            ComponentWarehouse::template
                GetRequiredComponentTypeIndex<RequieredComponent_t>()
        };
        auto cmp_creator
        {
            [&](auto&&... args) -> ComponentID_t {
                return mComponents.template CreateRequiredComponent<RequieredComponent_t>(ent.GetEntityID(), args...);
            }
        };
        auto cmp_id { Args::apply(cmp_creator, std::forward<ComponentArgs_t>(args)) };
        ent.AttachComponentID(cmp_tp_id, cmp_id);
    }

public:

    using EntityCreationKey_t = Key_t<Self_t>;

    explicit EntityManager_t()
    {
        static_assert(AreUnique_v<EntityList_t...>, "Entities should not repeat.");
    }

    template<class RequiredEntitiy_t, class... ComponentsArgs_t> constexpr auto
    CreateRequieredEntity(ComponentsArgs_t&&... args)
    -> OwnEntitity_t& 
    {
        static_assert(IsOneOf_v<RequiredEntitiy_t, EntityList_t...>,
                     "The requiered entity does not exists in this instance.");
        static_assert(AreUnique_v<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>,
                      "Component arguments must be unique.");
        using RequiredComponents_t = ComponentExtractor_t<RequiredEntitiy_t>;
        using ComponentsArgsFor_t  = TMPL::TypeList_t<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>;
        //using RemainingComponents_t = SubstractTypeLists_t<RequiredComponents_t, ComponentsArgsFor_t>;
        static_assert(IsSubsetOf_v<ComponentsArgsFor_t, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");
        auto& ent { mEntities.emplace_back(constructor_key, mEntities.size()) };
        (CreateRequieredComponent(ent, std::forward<ComponentsArgs_t>(args)), ...);
        return ent;
    }
};

} // namespace ECS

struct RenderComponent_t
{
    char c;
};

struct PositionComponent_t
{
    int x, y;
};

struct PhysicsComponent_t
{
    int vx, vy;
};

#include <iostream>
int main()
{
    using Rendereable_t = TMPL::TypeList_t<RenderComponent_t>;
    using Movable_t     = TMPL::TypeList_t<PositionComponent_t>;
    using Physicable_t  = TMPL::TypeList_t<PhysicsComponent_t>;
    using Character_t   = TMPL::TypeListCat<Rendereable_t, Movable_t, Physicable_t>;
    ECS::EntityManager_t<Rendereable_t, Movable_t, Physicable_t, Character_t> ent_man {  };
    Args::Arguments_t ren_args { Args::Wrapper_v<RenderComponent_t>, 'c' };

    ent_man.CreateRequieredEntity<Rendereable_t>(ren_args);

    using substracted = SubstractTypeLists_t<TMPL::TypeList_t<int, double, float>, TMPL::TypeList_t<int, double>>;
    static_assert(std::is_same_v<substracted, TMPL::TypeList_t<float>>, "NO!");

    ECS::ComponentStorage_t<TMPL::TypeList_t<int, double, float>> cmp_store {  };
    auto cmp_id_1 { cmp_store.CreateRequiredComponent<int>(0, 1) };
    auto cmp1 { cmp_store.GetRequiredComponent<int>(cmp_id_1) };
    auto cmp_id_2 { cmp_store.CreateRequiredComponent<double>(1, 3.1415) };
    auto cmp2 { cmp_store.GetRequiredComponent<double>(cmp_id_2) };
    std::cout << "CmpID1: " << cmp_id_1 << "Cmp1: " << cmp1 << std::endl;
    std::cout << "CmpID2: " << cmp_id_2 << "Cmp2: " << cmp2 << std::endl;
    return 0;
}
