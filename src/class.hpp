#pragma once

#include <tmpl/type_list.hpp>

namespace ECS {

template<class... Ts> struct Class_t
{
  using types = TMPL::TypeList_t<Ts...>;
};

} // namespace ECS
