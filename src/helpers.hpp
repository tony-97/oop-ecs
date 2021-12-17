#pragma once

#include "type_aliases.hpp"
#include <tuple>
#include <type_traits>

namespace ECS
{

template<class T>
struct IsVariadicTemplated
{
    inline static constexpr bool value { false };
};

template<template<class...> class T, class... Args_t>
struct IsVariadicTemplated<T<Args_t...>>
{
    inline static constexpr bool value { true };
};

///////////////////////////////////////////////////////////////////////////////
// contains
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename ...Ts>
struct IsOneOf;

template<typename T, typename ...Ts>
struct IsOneOf
{
    constexpr static inline bool value
    {
        std::disjunction_v<std::is_same<T, Ts>...>
    };
};

template<typename T, template<class...> class U, class... Us>
struct IsOneOf<T, U<Us...>> : IsOneOf<T, Us...> {  };

template<typename T, typename ...Ts>
inline constexpr bool IsOneOf_v = IsOneOf<T, Ts...>::value;

template<class T, class U>
struct IsSubsetOf;

template<template<class...> class T, template<class...> class U, class... Ts>
struct IsSubsetOf<T<>, U<Ts...>> final : public std::true_type {  };

template<template<class...> class T, class... Types, class U>
struct IsSubsetOf<T<Types...>, U>
{
    static inline constexpr bool value { std::disjunction_v<IsOneOf<Types, U>...> };
};

template<class T, class U>
inline constexpr bool IsSubsetOf_v = IsSubsetOf<T, U>::value;


///////////////////////////////////////////////////////////////////////////////
// SameAsConstMemFunc
///////////////////////////////////////////////////////////////////////////////

// https://stackoverflow.com/a/16780327
template <typename T> struct NonConst_t
{
    typedef T type;
};

template <typename T> struct NonConst_t<T const>
{
    typedef T type;
}; //by value

template <typename T> struct NonConst_t<T const&>
{
    typedef T& type;
}; //by reference

template <typename T> struct NonConst_t<T const*>
{
    typedef T* type;
}; //by pointer

template <typename T> struct NonConst_t<T const&&>
{
    typedef T&& type;
}; //by rvalue-reference

template<typename ConstReturn_t,
         class Obj_t,
         typename ...MemFnArgs_t,
         typename ...Args_t>
typename NonConst_t<ConstReturn_t>::type
SameAsConstMemFunc(
    Obj_t const* this_ptr,
    ConstReturn_t (Obj_t::* memFun)(MemFnArgs_t...) const,
    Args_t&& ...args)
{
    return const_cast<typename NonConst_t<ConstReturn_t>::type>
        ((this_ptr->*memFun)(std::forward<Args_t>(args)...));
}

///////////////////////////////////////////////////////////////////////////////
// Uncopyable_t
///////////////////////////////////////////////////////////////////////////////
struct Uncopyable_t
{
    Uncopyable_t() = default;

    Uncopyable_t(Uncopyable_t&&)                 = delete;
    Uncopyable_t(const Uncopyable_t&)            = delete;
    Uncopyable_t& operator=(Uncopyable_t&&)      = delete;
    Uncopyable_t& operator=(const Uncopyable_t&) = delete;
};

///////////////////////////////////////////////////////////////////////////////
// AreUnique_t
///////////////////////////////////////////////////////////////////////////////
template<class ...Args_t>
struct AreUnique;

template<class T0, class T1, class ...Tn>
struct AreUnique<T0, T1, Tn...>
{
    constexpr static inline bool value
    {
        !std::disjunction_v<std::is_same<T0, T1>,
                            std::is_same<T0, Tn>...,
                            std::is_same<T1, Tn>...>
    };
};

template<class T>
struct AreUnique<T> : public std::true_type {  };


template<>
struct AreUnique<> : public std::true_type {  };

template<class... Ts>
constexpr auto AreUnique_v { AreUnique<Ts...>::value };

///////////////////////////////////////////////////////////////////////////////
// IndexOfElement_t
///////////////////////////////////////////////////////////////////////////////

//https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type/60868425

template<class Element_t, class TElements_t>
struct IndexOfElement_t
{

private:
    template<std::size_t I, class T, class TElemns_t>
    static constexpr auto GetIndex() -> std::size_t
    {
        constexpr bool isOutOfRange = I >= std::tuple_size_v<TElemns_t>; 

        static_assert(!isOutOfRange, "The element does not exist.");

        if constexpr (!isOutOfRange) {
            using element_tp = std::tuple_element_t<I, TElemns_t>;
            if constexpr (std::is_same_v<T, element_tp>) {
                return I;
            } else {
                return GetIndex<I + 1, T, TElemns_t>();
            }
        }
    }

public:
    constexpr static inline std::size_t value
    {
        GetIndex<0, Element_t, TElements_t>()
    };

};


///////////////////////////////////////////////////////////////////////////////
// IsConstructible
///////////////////////////////////////////////////////////////////////////////

template <class...>
using void_t = void;

template <class, class T, class... Args>
struct IsConstructibleIMPL
: std::false_type {};

template <class T, class... Args>
struct IsConstructibleIMPL<void_t<decltype(T{std::declval<Args>()...})>,
                           T, Args...> : std::true_type {};

template <class T, class... Args>
using IsConstructible = IsConstructibleIMPL<void_t<>, T, Args...>;

template <class T, class... Args>
inline static constexpr auto IsConstructible_v
{
    IsConstructible<T, Args...>::value
};

///////////////////////////////////////////////////////////////////////////////
// Args_t
///////////////////////////////////////////////////////////////////////////////

template<class T, class... Args_t>
struct ArgsWrapper_t
{
    using type = T;
    Elements_t<Args_t...> mArgs {  };
};

template<class T,
         class... Args_t,
         std::enable_if_t<IsConstructible_v<T, Args_t...>, bool> = true>
ArgsWrapper_t<T, Args_t...> MakeArgs(Args_t&&... args)
{
    return ArgsWrapper_t<T, Args_t...>{ { std::forward<Args_t>(args)... } };
}

template<class... Args_t>
Elements_t<Args_t...> MakeForwadTuple(Args_t&&... args)
{
    return Elements_t<Args_t...>{ std::forward<Args_t>(args)... };
}

template<std::size_t... Is>
constexpr auto
MakeEmptyArgs(const std::index_sequence<Is...>&)
{
    return std::tuple{ ((void)Is, Elements_t<>{})... };
}

} // namespace ECS
