#pragma once

#include "entity.hpp"
#include "struct_of_arrays.hpp"
#include "type_aliases.hpp"

namespace ECS
{

// Entities is a variadic template argument that each one is a list of components
template<template<class> class CmptKey_t, class... EntSigs_t>
struct EntityManager_t final : SoA_t<Vector_t, Entity_t<EntSigs_t, CmptKey_t>...>, Uncopyable_t
{
public:
                      using Self_t      = EntityManager_t;
                      using Base_t      = SoA_t<Vector_t, Entity_t<EntSigs_t, CmptKey_t>...>;
    template<class T> using EntityID_t  = typename Vector_t<T>::size_type;
    template<class T> using entity_type = Entity_t<T, CmptKey_t>;

    constexpr explicit EntityManager_t() : Base_t{  } {  }

    template<class EntSig_t, class... CmptKeys_t> constexpr auto
    Create(CmptKeys_t... keys) -> const auto&
    {
        return Base_t::template emplace_back<entity_type<EntSig_t>>(keys...);
    }

    template<class EntSig_t> [[nodiscard]] constexpr auto 
    Destroy(EntityID_t<EntSig_t> ent_key)
    {
        using RequieredEntity = entity_type<EntSig_t>;
        auto& ent  { Base_t::template operator[]<RequieredEntity>(ent_key) };
        auto cmp_keys { ent.GetComponentIDs() };
        auto& last_ent { Base_t::template back<RequieredEntity>() };

        ent = std::move(last_ent);

        Base_t::template pop_back<RequieredEntity>();

        return cmp_keys;
    }

    template<class EntSig_t> constexpr const auto&
    GetEntity(EntityID_t<EntSig_t> pos) const
    {
        return Base_t::template operator[]<entity_type<EntSig_t>>(pos);
    }

    template<class EntSig_t> constexpr auto&
    GetEntity(EntityID_t<EntSig_t> pos)
    {
        return Base_t::template operator[]<entity_type<EntSig_t>>(pos);
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
