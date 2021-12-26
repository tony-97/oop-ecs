#pragma once

#include <iterator>
#include <variant>
#include <array>

#include <sequence.hpp>

#include "internal_component.hpp"
#include "type_aliases.hpp"
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
    -> ComponentVector_t<RequiredComponent_t>&
    {
        return SameAsConstMemFunc(this, &ComponentStorage_t::GetRequiredComponentVector<RequiredComponent_t>);
    }

    constexpr static void CheckIfComponentsAreUnique()
    {
        static_assert(TMPL::AreUnique_v<Components_t...>, "Components must be unique.");
    }

    template<class Component_t>
    constexpr static void CheckIfComponentExists()
    {
        static_assert(TMPL::IsOneOf_v<Component_t, Components_t...>,
                      "The requiered component does not exists in this instance.");
    }

public:

    template<class Component_t>
    using iterator = typename ComponentVector_t<Component_t>::iterator;

    template<class Component_t>
    using const_iterator = typename ComponentVector_t<Component_t>::const_iterator;

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
        const auto cmp_id { cmp_vec.size() };
        cmp_vec.emplace_back(eid, std::forward<Args_t>(args)...);

        return cmp_id;
    }

    template<class RequiredComponent_t> constexpr static auto
    GetRequiredComponentTypeIndex()
    -> ComponentTypeID_t
    {
        return TMPL::IndexOf_v<RequiredComponent_t, Components_t...>;
    }

    template<class RequiredComponent_t> [[nodiscard]] constexpr auto
    RemoveRequiredComponent(ComponentID_t cmp_id)
    -> EntityID_t
    {
        constexpr auto cmp_tp_idx { GetRequiredComponentTypeIndex<RequiredComponent_t>() };
        return RemoveRequiredComponent(cmp_tp_idx, cmp_id);
    }

    [[nodiscard]] constexpr auto
    RemoveRequiredComponent(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id)
    -> EntityID_t
    {
        auto& variant_vec { mComponentTable[cmp_tp_id] };
        auto remove_cmp
        {
            [&cmp_id](auto& cmp_vec) -> EntityID_t {
                auto&  cmp = cmp_vec[cmp_id];
                auto& last = cmp_vec.back();
                cmp = std::move(last);
                cmp_vec.pop_back();
                return cmp.mEntityID;
            }
        };

        return std::visit(remove_cmp, variant_vec);
    }

    constexpr auto
    SetEntityID(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id, EntityID_t eid)
    -> void
    {
        auto& variant_vec { mComponentTable[cmp_tp_id] };
        auto set_ent_id
        {
            [&cmp_id, &eid](auto& cmp_vec) {
                cmp_vec[cmp_id].mEntityID = eid;
            }
        };
        std::visit(set_ent_id, variant_vec);
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

    template<class RequiredComponent_t> constexpr auto
    begin() const -> const_iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().begin();
    }

    template<class RequiredComponent_t> constexpr auto
    begin() -> iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().begin();
    }

    template<class RequiredComponent_t> constexpr auto
    cbegin() -> const_iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().cbegin();
    }

    template<class RequiredComponent_t> constexpr auto
    end() const -> const_iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().end();
    }

    template<class RequiredComponent_t> constexpr auto
    end() -> iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().end();
    }

    template<class RequiredComponent_t> constexpr auto
    cend() -> const_iterator<RequiredComponent_t>
    {
        return GetRequiredComponentVector<RequiredComponent_t>().cend();
    }

};

} // namespace ECS
