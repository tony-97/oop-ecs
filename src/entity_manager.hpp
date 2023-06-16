#pragma once

#include "entity.hpp"
#include "struct_of_arrays.hpp"
#include "ecs_map.hpp"
#include <type_traits>

namespace ECS
{

template<class... EntSigs_t>
struct EntityManager_t final : SoA_t<ECSMap_t, Entity_t<EntSigs_t>...>, Uncopyable_t
{
public:
                      using Self_t      = EntityManager_t;
                      using Base_t      = SoA_t<ECSMap_t, Entity_t<EntSigs_t>...>;
    template<class T> using entity_type = Entity_t<T>;

    constexpr explicit EntityManager_t() : Base_t{  } {  }

    template<class EntSig_t, class... CmpID_t> constexpr auto
    Create(CmpID_t... ids) -> auto
    {
        return Base_t::template emplace_back<entity_type<EntSig_t>>(ids...);
    }

    template<class EntID_t> [[nodiscard]] constexpr auto 
    Destroy(EntID_t ent_id) -> auto
    {
        auto& cont {
            Base_t::template GetRequiredContainer<typename EntID_t::value_type>()
        };

        auto& ent    { Base_t::template operator[]<typename EntID_t::value_type>(ent_id) };
        auto cmp_ids { ent.GetComponentIDs() };

        cont.erase(ent_id);

        return cmp_ids;
    }

    template<class EntID_t> constexpr auto
    GetEntity(EntID_t pos) const -> const auto&
    {
        return Base_t::template operator[]<typename EntID_t::value_type>(pos);
    }

    template<class EntID_t> constexpr auto&
    GetEntity(EntID_t pos)
    {
        return Base_t::template operator[]<typename EntID_t::value_type>(pos);
    }

    template<class It_t> constexpr auto
    GetEntityID(It_t it) const
    {
        return Base_t::template GetRequiredContainer<std::remove_const_t<typename It_t::value_type>>().get_key(it);
    }

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
