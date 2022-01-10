#pragma once

#include <tuple>
#include <vector>
#include <optional>

template<class T>
using Vector_t = std::vector<T>;

template<class T>
using Optional_t = std::optional<T>;

template<class... Ts>
using Elements_t = std::tuple<Ts...>;
