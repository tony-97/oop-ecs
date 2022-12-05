#pragma once

#include <functional>
#include <type_traits>

#include "helpers.hpp"

namespace Args
{

template <class T, class... Args_t>
struct Arguments_t;

template<std::size_t N, class Args_t>
struct ArgumentsElemnt;

template<std::size_t N, class Args_t>
using ArgumentsElemnt_t = typename ArgumentsElemnt<N, Args_t>::type;

// TODO support for lvalue xvalue
template<std::size_t N, class T, class... Args_t> constexpr auto
get(const Arguments_t<T, Args_t...>& args)
-> decltype(auto);

namespace IMPL
{

template <std::size_t I, class Arg_t>
struct SingleArgument_t
{ Arg_t value {  }; };

template <class IndexSq_t, class... Args_t>
struct Arguments_t;

template <std::size_t... Indices, class... Args_t>
struct Arguments_t<std::index_sequence<Indices...>, Args_t...>
    : SingleArgument_t<Indices, Args_t>... {  };

template<class Callable_t, class Args_t, std::size_t... Indexs> constexpr auto
apply(Callable_t&& cb, Args_t&& args, std::index_sequence<Indexs...>)
-> decltype(auto)
{
    return std::invoke(std::forward<Callable_t>(cb),
                       get<Indexs>(std::forward<Args_t>(args))...);
}

} // namespace IMPL

template<class T>
struct For_t
{
    using type = T;
};

template<class T>
static inline constexpr For_t<T> For_v {  };

template<class T, class... Args_t>
struct Arguments_t
    : For_t<T>, IMPL::Arguments_t<std::index_sequence_for<Args_t...>, Args_t...>
{  };

template<class T, class... Args_t, std::enable_if_t<ECS::IsConstructible_v<T, Args_t...>, bool> = true>
Arguments_t(For_t<T>, Args_t...) -> Arguments_t<T, Args_t...>;

template<class Args_t>
struct ArgumentsSize;

template<class T, class... Args_t>
struct ArgumentsSize<Arguments_t<T, Args_t...>>
    : std::integral_constant<std::size_t, sizeof...(Args_t)> {  };

template<class Args_t>
static inline constexpr auto ArgumentsSize_v { ArgumentsSize<std::remove_const_t<Args_t>>::value };

template<std::size_t N, class Args_t>
struct ArgumentsElemnt;

template<std::size_t N, class T, class FirstArg_t, class... Rest_t>
struct ArgumentsElemnt<N, Arguments_t<T, FirstArg_t, Rest_t...>>
    : ArgumentsElemnt<N - 1, Arguments_t<T, Rest_t...>>
{
    static_assert(ArgumentsSize_v<Arguments_t<T, FirstArg_t, Rest_t...>> > N,
                  "Index out of bounds.");
};

template<class T, class FirstArg_t, class... Rest_t>
struct ArgumentsElemnt<0, Arguments_t<T, FirstArg_t, Rest_t...>>
{
    using type = FirstArg_t;
};

template<std::size_t N, class Args_t>
using ArgumentsElemnt_t = typename ArgumentsElemnt<N, Args_t>::type;

template<std::size_t N, class T, class... Args_t> constexpr auto 
get(const Arguments_t<T, Args_t...>& args)
-> decltype(auto)
{
    using SingleArgument_t
        = IMPL::SingleArgument_t<N, ArgumentsElemnt_t<N, Arguments_t<T, Args_t...>>>;
    return static_cast<const SingleArgument_t&>(args).value;
}

template<class Callable_t, class Args_t> constexpr auto
apply(Callable_t&& cb, Args_t&& args)
-> decltype(auto)
{
    return IMPL::apply(std::forward<Callable_t>(cb),
                       std::forward<Args_t>(args),
                       std::make_index_sequence<ArgumentsSize_v<std::remove_reference_t<Args_t>>>{});
}

} // namespace Args
