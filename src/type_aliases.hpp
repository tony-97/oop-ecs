#pragma once

#include <tuple>
#include <vector>

namespace ECS {

template<class T>
using Vector_t = std::vector<T>;

template<class... Ts>
using Elements_t = std::tuple<Ts...>;

using IndexSize_t = std::size_t;

}
