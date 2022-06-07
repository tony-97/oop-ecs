#pragma once

#include <tmpl/sequence.hpp>
#include <tmpl/tmpl.hpp>

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<bool b, class... Ts>
struct RawBase
{
    static_assert(b, "Types are not unique.");
};

template<class... Ts>
struct RawBase<true, Ts...>
{
    using type = Seq::UniqueTypes_t<TMPL::TypeList_t<Ts...>>;
};

template<class... Ts>
struct Base_t : RawBase<TMPL::AreUnique_v<Ts...>, Ts...> {  };

template<bool b, class... Ts>
struct RawDerived
{
    static_assert(b, "Derived not derived from Base.");
};

template<class... Ts>
struct RawDerived<true, Ts...>
{
    using type = Seq::UniqueTypes_t<Seq::SeqCat_t<Ts...>>;
};

template<class T>
struct IsBase : std::false_type {  };

template<template<class...> class T, class... Ts>
struct IsBase<T<Ts...>> : std::bool_constant<std::is_same_v<T<Ts...>, Base_t<Ts...>>> {  };

template<class T>
constexpr static inline auto IsBase_v { IsBase<T>::value };

template<class... Ts>
struct Derived_t : RawDerived<std::conjunction_v<IsBase<Ts>...>, Ts...> {  };

} // namespace ECS
