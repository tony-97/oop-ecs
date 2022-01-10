#pragma once

#include "struct_of_arrays.hpp"
#include "entity.hpp"
#include <tuple>

namespace ECS
{

// EntitySignatures_t is a variadic template argument that each one is a list of components
template<class... EntitySignatures_t>
struct EntityManager_t final : SoA_t<Entity_t<EntitySignatures_t>...>, Uncopyable_t
{
public:

                      using Self_t           = EntityManager_t;
                      using Base_t           = SoA_t<Entity_t<EntitySignatures_t>...>;
                      using ConstructorKey_t = Key_t<Self_t>;
    template<class T> using EntityID_t       = Identifier_t<Self_t, T>;

    explicit EntityManager_t() : Base_t{  }
    {
        ((void)EntitySignatures_t{  }, ...) ;
    }

    template<class RequiredEntity_t, class... ComponentIDs_t> constexpr auto
    CreateEntity(ComponentIDs_t&&... ids)
    {
        EntityID_t<RequiredEntity_t> id {
            constructor_key,
            Base_t::template size<RequiredEntity_t>() };
        Base_t::template emplace_back<RequiredEntity_t>(ids...);

        return id;
    }

private:

    constexpr static inline ConstructorKey_t constructor_key {  };

    using Base_t::data;
    using Base_t::push_back;
    using Base_t::emplace_back;
    using Base_t::reserve;
    using Base_t::shrink_to_fit;
    using Base_t::resize;
};

} // namespace ECS

