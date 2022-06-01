#pragma once

#include "struct_of_arrays.hpp"
#include "helpers.hpp"
#include "ecs_map.hpp"

namespace ECS
{

// Entities is a variadic template argument that each one is a list of components
template<class... Entities>
struct EntityManager_t final : SoA_t<ECSMap_t, Entities...>, Uncopyable_t
{
public:
                      using Self_t           = EntityManager_t;
                      using Base_t           = SoA_t<ECSMap_t, Entities...>;
                      using ConstructorKey_t = Key_t<Self_t>;
    template<class T> using EntityID_t       = typename ECSMap_t<T>::Key_t;

    constexpr explicit EntityManager_t() : Base_t{  } {  }

    template<class RequiredEntity_t, class... ComponentIDs_t> constexpr auto
    Create(ComponentIDs_t&&... ids)
    {
        return Base_t::template emplace_back<RequiredEntity_t>(ids...);
    }

    template<class K> constexpr auto
    Destroy(K&& ent_id)
    {
        using RequiredEntity_t = typename K::value_type;
        auto& ent  { Base_t::template operator[]<RequiredEntity_t>(ent_id) };
        auto cmp_ids { ent.GetComponentIDs() };
        Base_t::template erase<RequiredEntity_t>(ent_id);

        return cmp_ids;
    }

private:

    using Base_t::push_back;
    using Base_t::emplace;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
