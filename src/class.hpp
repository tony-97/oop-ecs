#pragma once

#include "traits.hpp"

namespace ECS
{

template<class... Ts> struct Class_t
{
    using type = Seq::UniqueTypes_t<Seq::Cat_t<GetComponents_t<Ts>...>>;
};

} // namespace ECS
