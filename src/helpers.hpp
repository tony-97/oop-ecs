#pragma once

#include <type_traits>
#include <utility>

namespace ECS
{

///////////////////////////////////////////////////////////////////////////////
// SameAsConstMemFunc
///////////////////////////////////////////////////////////////////////////////

template<class T>
struct TypeIdentity { using type = T; };

template<class T>
using TypeIdentity_t = typename TypeIdentity<T>::type;

template <class T> struct NonConst            : TypeIdentity<T>   {  };
template <class T> struct NonConst<T const>   : TypeIdentity<T>   {  };
template <class T> struct NonConst<T const&>  : TypeIdentity<T&>  {  };
template <class T> struct NonConst<T const*>  : TypeIdentity<T*>  {  };
template <class T> struct NonConst<T const&&> : TypeIdentity<T&&> {  };

template<class T>
using NonConst_t = typename NonConst<T>::type;

template<class CReturn_t,
         class This_t,
         class ...MemFnArgs_t,
         class ...Args_t> constexpr auto
SameAsConstMemFunc(This_t& obj,
                   CReturn_t (This_t::* mem_fn)(MemFnArgs_t...) const,
                   Args_t&& ...args)
-> NonConst_t<CReturn_t> 
{
    using Return_t = NonConst_t<CReturn_t>;
    using CThis_t = const This_t;
    return const_cast<Return_t>((const_cast<CThis_t&>(obj).*mem_fn)(std::forward<Args_t>(args)...));
}

template<class CReturn_t,
         class This_t,
         class ...MemFnArgs_t,
         class ...Args_t> constexpr auto
SameAsConstMemFunc(This_t* obj,
                   CReturn_t (This_t::* mem_fn)(MemFnArgs_t...) const,
                   Args_t&& ...args)
-> NonConst_t<CReturn_t> 
{
    return SameAsConstMemFunc(*obj, mem_fn, std::forward<Args_t>(args)...);
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
// key for managers
///////////////////////////////////////////////////////////////////////////////

template<class Owner_t>
struct Key_t
{
private:
friend Owner_t;
    explicit constexpr Key_t() = default;
};

///////////////////////////////////////////////////////////////////////////////
// IsConstructible
///////////////////////////////////////////////////////////////////////////////

template <class...>
using void_t = void;

template <class, class T, class... Args>
struct IsConstructibleIMPL : std::false_type {  };

template <class T, class... Args>
struct IsConstructibleIMPL<void_t<decltype(T{std::declval<Args>()...})>,
                           T, Args...> : std::true_type {};

template <class T, class... Args>
using IsConstructible = IsConstructibleIMPL<void_t<>, T, Args...>;

template <class T, class... Args>
constexpr static inline auto IsConstructible_v { IsConstructible<T, Args...>::value };

} // namespace ECS
