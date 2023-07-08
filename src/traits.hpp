#pragma once

#include <tmpl/tmpl.hpp>
#include <tmpl/sequence.hpp>
#include <tmpl/type_list.hpp>

#include <type_traits>

namespace ECS
{

namespace Seq = TMPL::Sequence;

template <typename T, typename = void> struct IsClass : std::false_type {};

template <typename T>                  struct IsClass<T, std::void_t<typename T::Class_t>> : std::true_type {};

template<class T> constexpr static inline auto IsClass_v { IsClass<T>::value };

template <typename T, typename = void> struct GetClass : std::type_identity<T> {};

template <typename T>                  struct GetClass<T, std::void_t<typename T::Class_t>> : std::type_identity<typename T::Class_t> {};

template <class Derived> using GetClass_t = typename GetClass<Derived>::type;

template <typename T, typename = void>
struct GetComponents
{
    using type = TMPL::TypeList_t<T>;
};

template <typename T>
struct GetComponents<T, std::void_t<typename T::Class_t>>
{
    using type = typename T::Class_t::type;
};

template <typename T>
using GetComponents_t = typename GetComponents<T>::type;

template<class T>
struct IsBase : std::false_type {  };

template<template<class...> class Bases_t, class... Ts>
struct IsBase<Bases_t<Ts...>> : std::bool_constant<!std::disjunction_v<IsClass<Ts>...>> {  };

template<class T>
static inline constexpr auto IsBase_v { IsBase<GetClass_t<T>>::value };

template<class T>
struct GetBases
{
    using type = TMPL::TypeList_t<>;
};

template<template<class...> class Bases_t, class... Ts>
struct GetBases<Bases_t<Ts...>>
{
    using type =
        Seq::Cat_t<std::conditional_t<IsClass_v<Ts>,
        Seq::Cat_t<TMPL::TypeList_t<Ts>,
        typename GetBases<GetClass_t<Ts>>::type>, TMPL::TypeList_t<>>...>;
};

template<class T>
using GetBases_t = Seq::UniqueTypes_t<typename GetBases<GetClass_t<T>>::type>;

template<class... EntitySignatures_t>
using ComponentsFrom_t = Seq::UniqueTypes_t<Seq::Cat_t<typename EntitySignatures_t::type...>>;

template<class Fn_t, class... Args_t>
struct IsSystemCallable;

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t, class EntIdx_t>
struct IsSystemCallable<Fn_t, Sig_t<Sigs_t...>, EntIdx_t>
: std::bool_constant<std::is_invocable_v<Fn_t, Sigs_t&..., EntIdx_t>> {  };

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t>
struct IsSystemCallable<Fn_t, Sig_t<Sigs_t...>>
: std::bool_constant<std::is_invocable_v<Fn_t, Sigs_t&...>> {  };

template<class Fn_t, class... Args_t>
static inline constexpr auto IsSystemCallable_v { IsSystemCallable<Fn_t, Args_t...>::value };

template <bool Enable, class Fn_t, class... Args_t>
struct ConditionalIsSystemCallable;

template <class Fn_t, class... Args_t>
struct ConditionalIsSystemCallable<true, Fn_t, Args_t...> : std::bool_constant<IsSystemCallable_v<Fn_t, Args_t...>> {  };

template <class Fn_t, class... Args_t>
struct ConditionalIsSystemCallable<false, Fn_t, Args_t...> : std::false_type {  };

template <bool Enable, class Fn_t, class... Args_t>
constexpr bool ConditionalIsSystemCallable_v = ConditionalIsSystemCallable<Enable, Fn_t, Args_t...>::value;

template<class Sign1_t, class Sign2_t>
struct IsInstanceOf : std::bool_constant<std::is_same_v<Sign1_t, Sign2_t>> {  };

template<class Sign1_t, template<class...> class Sign2_t, class... Ts>
struct IsInstanceOf<Sign1_t, Sign2_t<Ts...>>
    : std::bool_constant<
        std::disjunction_v<
            std::is_same<Sign1_t, Sign2_t<Ts...>>, IsInstanceOf<Sign1_t, GetClass_t<Ts>>...
                          >
                        >{  };

template<class Sign1_t, class Sign2_t>
static inline constexpr auto IsInstanceOf_v { IsInstanceOf<typename Sign1_t::Class_t, typename Sign2_t::Class_t>::value };

} // namespace ECS
