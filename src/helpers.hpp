#pragma once

#include <type_traits>
#include <utility>

#include "sequence.hpp"

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
    if constexpr (not std::is_void_v<Return_t>) {
        return const_cast<Return_t>((const_cast<CThis_t&>(obj).*mem_fn)(std::forward<Args_t>(args)...));
    } else {
        (const_cast<CThis_t&>(obj).*mem_fn)(std::forward<Args_t>(args)...);
    }
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
// Convert tuple
///////////////////////////////////////////////////////////////////////////////

struct TupleConverter_t
{
    template<class... Ts, class Tuple_t>
    constexpr auto operator()(Tuple_t&& tup)
    {
        return std::tuple { std::get<Ts>(std::forward<Tuple_t>(tup))... };
    }
};

template<class DestTuple_t, class SrcTuple_t>
constexpr auto ConvertTo(SrcTuple_t&& tup)
{
    return TMPL::Sequence::Unpacker_t<DestTuple_t>::Call(TupleConverter_t{  }, std::forward<SrcTuple_t>(tup));
}

///////////////////////////////////////////////////////////////////////////////
// ID
///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
struct ID_t
{
    using type    = T;
    using id_type = U;
    U mID {  };
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
// identifier for managers
///////////////////////////////////////////////////////////////////////////////

template<class Manager_t, class T, class ID_t = std::size_t>
class Identifier_t
{
    friend Manager_t;
public:
    using type = T;
    using id_type = ID_t;

    constexpr ID_t GetID() const { return mID; }
private:
    constexpr explicit Identifier_t(Key_t<Manager_t>, ID_t id)
        : mID { id } {  }

    const ID_t mID {  };
};

///////////////////////////////////////////////////////////////////////////////
// IsConstructible
///////////////////////////////////////////////////////////////////////////////

template <class, class T, class... Args>
struct IsConstructibleIMPL : std::false_type {  };

template <class T, class... Args>
struct IsConstructibleIMPL<std::void_t<decltype(T{std::declval<Args>()...})>,
                           T, Args...> : std::true_type {};

template <class T, class... Args>
using IsConstructible = IsConstructibleIMPL<std::void_t<>, T, Args...>;

template <class T, class... Args>
constexpr static inline auto IsConstructible_v { IsConstructible<T, Args...>::value };

} // namespace ECS
