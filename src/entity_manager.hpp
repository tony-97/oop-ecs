#pragma once

#include "struct_of_arrays.hpp"
#include "helpers.hpp"
#include "ecs_map.hpp"
#include <type_traits>
#include <utility>

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

    template<class K> [[nodiscard]] constexpr auto 
    Destroy(K&& ent_id)
    {
        using RequiredEntity_t = typename std::remove_reference_t<K>::value_type;
        auto& ent  { Base_t::template operator[]<RequiredEntity_t>(std::forward<K>(ent_id)) };
        auto cmp_ids { ent.GetComponentIDs() };
        Base_t::template erase<RequiredEntity_t>(std::forward<K>(ent_id));

        return cmp_ids;
    }

    template<class It_t> constexpr auto
    GetKey(It_t&& it) const
    {
        auto& cont {
            Base_t::template
                GetRequiredContainer<typename std::remove_reference_t<It_t>::value_type>() };

        return cont.get_key(std::forward<It_t>(it));
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
