#pragma once

#include <tuple>
#include <vector>
#include <optional>

using ComponentID_t     = std::size_t;
using ComponentTypeID_t = std::size_t;
using EntityID_t        = std::size_t;

template<class T>
using Vector_t = std::vector<T>;

template<class T>
using Optional_t = std::optional<T>;

template<class... Ts>
using Elements_t = std::tuple<Ts...>;
