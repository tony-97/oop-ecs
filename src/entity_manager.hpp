#pragma once

#include "struct_of_arrays.hpp"
#include "entity.hpp"
#include <tuple>

namespace ECS
{

// Entities is a variadic template argument that each one is a list of components
template<class... Entities>
struct EntityManager_t final : SoA_t<Entities...>, Uncopyable_t
{
public:

                      using Self_t           = EntityManager_t;
                      using Base_t           = SoA_t<Entities...>;
                      using ConstructorKey_t = Key_t<Self_t>;
    template<class T> using EntityID_t       = ID_t<T, IndexSize_t>;

    constexpr explicit EntityManager_t() : Base_t{  }
    {
        ((void)Entities{  }, ...) ;
    }

    template<class RequiredEntity_t, class... ComponentIDs_t> constexpr auto
    Create(ComponentIDs_t&&... ids)
    {
        EntityID_t<RequiredEntity_t> ent_id { Base_t::template size<RequiredEntity_t>() };

        Base_t::template emplace_back<RequiredEntity_t>(ids...);

        return ent_id;
    }

private:

    constexpr static inline ConstructorKey_t constructor_key {  };

    using Base_t::push_back;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS
