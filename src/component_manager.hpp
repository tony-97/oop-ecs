#pragma once

#include "helpers.hpp"
#include "type_aliases.hpp"

namespace ECS {

template<class Config_t>
struct ComponentManager_t final
  : Config_t::base
  , Uncopyable_t
{
public:
  using Self_t = ComponentManager_t;
  using Base_t = typename Config_t::base;

  constexpr explicit ComponentManager_t()
    : Base_t{}
  {
  }

  template<class Cmp_t> constexpr auto Create(Cmp_t&& cmp) -> auto
  {
    return Handle_t{ Base_t::template emplace_back<Cmp_t>(std::forward<Cmp_t>(cmp)).key() };
  }

  template<class Cmp_t> constexpr auto Destroy(Handle_t<Cmp_t> cmp) -> void
  {
    Base_t::template erase<Cmp_t>(ID_t<Cmp_t>{ cmp });
  }

  template<class Cmp_t> constexpr auto GetComponent(Handle_t<Cmp_t> cmp) const -> const auto&
  {
    return Base_t::template operator[]<Cmp_t>(ID_t<Cmp_t>{ cmp });
  }

  template<class Cmp_t> constexpr auto GetComponent(Handle_t<Cmp_t> cmp) -> auto&
  {
    return Base_t::template operator[]<Cmp_t>(ID_t<Cmp_t>{ cmp });
  }

private:
  using Base_t::operator[];
  using Base_t::at;
  using Base_t::data;
  using Base_t::emplace_back;
  using Base_t::push_back;
  using Base_t::reserve;
  using Base_t::resize;
  using Base_t::shrink_to_fit;
};

} // namespace ECS
