#pragma once

#include <iterator>
#include <variant>
#include <array>

#include "internal_component.hpp"
#include "type_aliases.hpp"
#include "type_list.hpp"
#include "helpers.hpp"

namespace ECS
{

template<class ComponentTypes_t>
struct ComponentStorage_t;

template<template <class...> class ComponentTypes_t, class... Components_t>
struct ComponentStorage_t<ComponentTypes_t<Components_t...>> : public Uncopyable_t
{
private:

    using ComponentList_t = TMPL::TypeList_t<Components_t...>;

    template<class Component_t>
    using ComponentVector_t = Vector_t<InternalComponent_t<Component_t>>;

    using ComponentMatrix_t = std::array<std::variant<ComponentVector_t<Components_t>...>,
                                         sizeof...(Components_t)>;

    ComponentMatrix_t mComponentTable {  };

    template<class RequiredComponent_t> constexpr auto
    GetRequiredComponentVector() const
    -> const ComponentVector_t<RequiredComponent_t>& 
    {
        CheckIfComponentExists<RequiredComponent_t>();
        constexpr auto index { GetRequiredComponentTypeIndex<RequiredComponent_t>() };
        return std::get<ComponentVector_t<RequiredComponent_t>>(mComponentTable[index]);
    }

    template<class RequiredComponent_t> constexpr auto
    GetRequiredComponentVector()
    ->ComponentVector_t<RequiredComponent_t>&
    {
        return SameAsConstMemFunc(this, &ComponentStorage_t::GetRequiredComponentVector<RequiredComponent_t>);
    }

    constexpr static void CheckIfComponentsAreUnique()
    {
        static_assert(AreUnique_v<Components_t...>, "Components must be unique.");
    }

    template<class Component_t>
    constexpr static void CheckIfComponentExists()
    {
        static_assert(IsOneOf_v<Component_t, Components_t...>,
                      "The requiered component does not exists in this instance.");
    }

public:

    explicit ComponentStorage_t()
        : mComponentTable { ComponentVector_t<Components_t>{  }... }
    {
        CheckIfComponentsAreUnique();
    }

    template<class RequiredComponent_t, class... Args_t> constexpr auto
    CreateRequiredComponent(EntityID_t eid, Args_t&&... args)
    -> ComponentID_t
    {
        auto& cmp_vec { GetRequiredComponentVector<RequiredComponent_t>() };
        auto cmp_id { cmp_vec.size() };
        cmp_vec.emplace_back(eid, std::forward<Args_t>(args)...);

        return cmp_id;
    }

    template<class RequiredComponent_t> constexpr static auto
    GetRequiredComponentTypeIndex()
    -> ComponentTypeID_t
    {
        return TMPL::IndexOf_v<RequiredComponent_t, ComponentList_t>;
    }

    template<class RequiredComponent_t> constexpr auto
    RemoveRequiredComponent(ComponentID_t cmp_id)
    -> EntityID_t
    {
        auto cmp_tp_idx { GetRequiredComponentTypeIndex<RequiredComponent_t>() };
        return RemoveRequiredComponent(cmp_tp_idx, cmp_id);
    }

    constexpr auto
    RemoveRequiredComponent(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id)
    -> EntityID_t
    {
        auto& variant_vec { mComponentTable[cmp_tp_id] };
        auto remove_cmp
        {
            [&cmp_id](auto& cmp_vec) -> EntityID_t {
                auto  it   = std::next(cmp_vec.begin(), cmp_id);
                auto& last = cmp_vec.back();
                *it = std::move(last);
                return it->mEntityID;
            }
        };

        return std::visit(remove_cmp, variant_vec);
    }

    template<class RequiredComponent_t> constexpr auto
    GetRequiredComponent(ComponentID_t cmp_id) const
    -> const RequiredComponent_t&
    {
        return GetRequiredComponentVector<RequiredComponent_t>()[cmp_id].mSelf;
    }

    template<class RequiredComponent_t> constexpr auto
    GetRequiredComponent(ComponentID_t cmp_id)
    -> RequiredComponent_t&
    {
        return SameAsConstMemFunc(this,
                                  &ComponentStorage_t::GetRequiredComponent<RequiredComponent_t>,
                                  cmp_id);
    }
};

} // namespace ECS