#pragma once

#include <tmpl/sequence.hpp>
#include <tmpl/type_list.hpp>

#include <type_traits>

#include "type_aliases.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<class T> struct Handle_t;

namespace Traits {

template <class T, class = void> struct IsClass                                    : std::false_type {};
template <class T>               struct IsClass<T, std::void_t<typename T::types>> : std::true_type  {};

template<class T> constexpr static inline auto IsClass_v { IsClass<T>::value };

template <class T, class = void> struct Class                                    : std::type_identity<T> {};
template <class T>               struct Class<T, std::void_t<typename T::types>> : std::type_identity<typename T::types> {};

template <class T> using Class_t = typename Class<T>::type;

template<class T> struct IsBaseIMPL : std::false_type {  };

template<template<class...> class Bases_t, class... Ts>
struct IsBaseIMPL<Bases_t<Ts...>> : std::bool_constant<!std::disjunction_v<IsClass<Ts>...>> {  };

template<class T> struct IsBase : IsBaseIMPL<T> {  };

template<class T> static inline constexpr auto IsBase_v { IsBase<Class_t<T>>::value };

template<class T>
struct BasesIMPL
{
    using type = TMPL::TypeList_t<>;
};

template<template<class...> class Bases_t, class... Ts>
struct BasesIMPL<Bases_t<Ts...>>
{
    using type =
        Seq::Cat_t<std::conditional_t<IsClass_v<Ts>,
            Seq::Cat_t<TMPL::TypeList_t<Ts>,
            typename BasesIMPL<Class_t<Ts>>::type>, TMPL::TypeList_t<>>...>;
};

template<class T> struct Bases : BasesIMPL<T> {  };

template<class T> using Bases_t = Seq::MakeSet_t<typename Bases<Class_t<T>>::type>;

template<class T>
struct ComponentsIMPL
{
    using type = TMPL::TypeList_t<>;
};

template<template<class...> class Types_t, class... Ts>
struct ComponentsIMPL<Types_t<Ts...>>
{
    using type =
        Seq::Cat_t<std::conditional_t<IsClass_v<Ts>,
            Seq::Cat_t<TMPL::TypeList_t<>,
            typename ComponentsIMPL<Class_t<Ts>>::type>, TMPL::TypeList_t<Ts>>...>;
};

template<class... Ts>
struct Components
{
    using type = Seq::MakeSet_t<Seq::Cat_t<typename ComponentsIMPL<Class_t<Ts>>::type...>>;
};

template<class... Ts> using Components_t = typename Components<Ts...>::type;

template<class Fn_t, class... Args_t> struct IsInvocable;

template<class Fn_t, class Sig_t>
struct IsInvocable<Fn_t, Handle_t<Sig_t>>
    : std::is_invocable<Fn_t, Handle_t<Sig_t>> {  };

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t, class EntIdx_t>
struct IsInvocable<Fn_t, Sig_t<Sigs_t...>, EntIdx_t>
    : std::is_invocable<Fn_t, Sigs_t&..., EntIdx_t> {  };

template<class Fn_t, template<class...> class Sig_t, class...Sigs_t>
struct IsInvocable<Fn_t, Sig_t<Sigs_t...>>
    : std::is_invocable<Fn_t, Sigs_t&...> {  };

template<class Fn_t, class... Args_t>
static inline constexpr auto IsInvocable_v { IsInvocable<Fn_t, Args_t...>::value };

template <bool Enable, class Fn_t, class... Args_t>
struct ConditionalIsInvocable;

template <class Fn_t, class... Args_t>
struct ConditionalIsInvocable<true, Fn_t, Args_t...> : IsInvocable<Fn_t, Args_t...> {  };

template <class Fn_t, class... Args_t>
struct ConditionalIsInvocable<false, Fn_t, Args_t...> : std::false_type {  };

template <bool Enable, class Fn_t, class... Args_t>
constexpr bool ConditionalIsInvocable_v = ConditionalIsInvocable<Enable, Fn_t, Args_t...>::value;

template <class Sign1_t, class Sign2_t>
struct IsInstanceOf;

template <class Sign1_t, class Sign2_t, class... Ts>
struct IsInstanceOfIMPL;

template <class Sign1_t, template <class...> class Sig2_t, class... Ts>
struct IsInstanceOfIMPL<Sign1_t, Sig2_t<Ts...>>
    : std::disjunction<
        std::is_same<Sign1_t, Ts>..., IsInstanceOf<Sign1_t, Ts>...
      > {};

template <class Sign1_t, class Sign2_t>
struct IsInstanceOf
    : std::disjunction<
        std::is_same<Sign1_t, Sign2_t>,
        std::conditional_t<IsClass_v<Sign2_t>,
            IsInstanceOfIMPL<Sign1_t, Class_t<Sign2_t>>, std::false_type>
      > {};

template<class Sign1_t, class Sign2_t>
static inline constexpr auto IsInstanceOf_v { IsInstanceOf<Sign1_t, Sign2_t>::value };

template<class ID>
struct Entity
{
    using type = typename ID::value_type;
};

template<class ID> using Entity_t = typename Entity<ID>::type;

template<class ID>
struct Signature
{
    using type = typename Entity_t<ID>::Signature_t;
};

template<class ID> using Signature_t = typename Signature<ID>::type;

} // namespace Traits

} // namespace ECS
