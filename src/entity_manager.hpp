#pragma once

#include "entity.hpp"
#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"

namespace ECS
{

// Entities is a variadic template argument that each one is a list of components
template<template<class> class CmptKey_t, class... EntSigs_t>
struct EntityManager_t final : SoA_t<ECSMap_t, Entity_t<EntSigs_t, CmptKey_t>...>, Uncopyable_t
{
public:
                      using Self_t      = EntityManager_t;
                      using Base_t      = SoA_t<ECSMap_t, Entity_t<EntSigs_t, CmptKey_t>...>;
    template<class T> using Size_t      = typename ECSMap_t<T>::size_type;
    template<class T> using entity_type = Entity_t<T, CmptKey_t>;
    template<class T> using EntityKey_t = typename ECSMap_t<entity_type<T>>::Key_t;

    constexpr explicit EntityManager_t() : Base_t{  } {  }

    template<class EntSig_t, class... CmptKeys_t> constexpr auto
    Create(CmptKeys_t&&... keys)
    {
        return Base_t::template emplace_back<entity_type<EntSig_t>>(std::forward<CmptKeys_t>(keys)...);
    }

    template<class K_t> [[nodiscard]] constexpr auto 
    Destroy(const K_t& ent_key)
    {
        using RequiredEntity_t = typename K_t::value_type;
        auto& ent  { Base_t::template operator[]<RequiredEntity_t>(ent_key) };
        auto cmp_keys { ent.GetComponentIDs() };
        Base_t::template erase<RequiredEntity_t>(ent_key);

        return cmp_keys;
    }

    template<class EntSig_t> constexpr const auto&
    GetEntity(Size_t<EntSig_t> pos) const
    {
        return Base_t::template operator[]<entity_type<EntSig_t>>(pos);
    }

    template<class EntSig_t> constexpr auto&
    GetEntity(Size_t<EntSig_t> pos)
    {
        return Base_t::template operator[]<entity_type<EntSig_t>>(pos);
    }

    template<class It_t> constexpr auto
    GetKey(It_t&& it) const
    {
        auto& cont {
            Base_t::template
                GetRequiredContainer<typename std::remove_reference_t<It_t>::value_type>() };

        return cont.get_key(std::forward<It_t>(it));
    }

    template<class EntSig_t> constexpr auto 
    GetKey(Size_t<EntSig_t> pos) const
    {
        auto& cont {
            Base_t::template GetRequiredContainer<entity_type<EntSig_t>>() };

        return cont.get_key(pos);
    }

    template<class EntSig_t> constexpr auto
    size() const { return Base_t::template size<entity_type<EntSig_t>>(); }

private:
    using Base_t::operator[];
    using Base_t::at;
    using Base_t::push_back;
    using Base_t::emplace;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
