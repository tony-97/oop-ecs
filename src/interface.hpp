#pragma once

#include <tmpl/sequence.hpp>
#include <tmpl/tmpl.hpp>
#include <type_traits>

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
    using type = Seq::UniqueTypes_t<Seq::SeqCat_t<typename Ts::type...>>;
};

template<class T>
struct IsBase : std::false_type {  };

template<template<class...> class T, class... Ts>
struct IsBase<T<Ts...>> : std::bool_constant<std::is_same_v<T<Ts...>, Base_t<Ts...>>> {  };

template<class T>
constexpr static inline auto IsBase_v { IsBase<T>::value };

template<class... Ts>
struct Derived_t : RawDerived<std::conjunction_v<IsBase<Ts>...>, Ts...> {  };

template<class Sig_t, class EntSig_t>
struct IsInstanceOf;

template<template<class...> class Sig_t, template<class...> class EntSig_t, class... Sigs_t, class... EntSigs_t>
struct IsInstanceOf<Sig_t<Sigs_t...>, EntSig_t<EntSigs_t...>> 
    : std::bool_constant<std::disjunction_v<std::is_same<Sig_t<Sigs_t...>, EntSig_t<EntSigs_t...>>,
                                            TMPL::IsOneOf<EntSig_t<EntSigs_t...>, Sigs_t...>>> {  };

template<class Sig_t, class EntSig_t>
constexpr static inline bool IsInstanceOf_v { IsInstanceOf<Sig_t, typename std::remove_reference_t<EntSig_t>::type>::value };

} // namespace ECS
