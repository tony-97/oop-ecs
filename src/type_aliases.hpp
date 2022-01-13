#pragma once

#include <tuple>
#include <vector>
#include <optional>

namespace ECS {

template<class T>
using Vector_t = std::vector<T>;

template<class T>
using Optional_t = std::optional<T>;

template<class... Ts>
using Elements_t = std::tuple<Ts...>;

using IndexSize_t = std::size_t;

}
