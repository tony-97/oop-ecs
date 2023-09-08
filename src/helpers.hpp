#pragma once

#include <type_traits>
#include <utility>

namespace ECS {
///////////////////////////////////////////////////////////////////////////////
// SameAsConstMemFunc
///////////////////////////////////////////////////////////////////////////////

template<class T> struct NonConst : std::type_identity<T>
{};

template<class T> struct NonConst<T const> : std::type_identity<T>
{};

template<class T> struct NonConst<T const&> : std::type_identity<T&>
{};

template<class T> struct NonConst<T const*> : std::type_identity<T*>
{};

template<class T> struct NonConst<T const&&> : std::type_identity<T&&>
{};

template<class T> using NonConst_t = typename NonConst<T>::type;

template<class CReturn_t, class This_t, class... MemFnArgs_t, class... Args_t>
constexpr auto
SameAsConstMemFunc(This_t& obj, CReturn_t (This_t::*mem_fn)(MemFnArgs_t...) const, Args_t&&... args)
  -> NonConst_t<CReturn_t>
{
  using Return_t = NonConst_t<CReturn_t>;
  using CThis_t  = const This_t;
  if constexpr (not std::is_void_v<Return_t>) {
    return const_cast<Return_t>((const_cast<CThis_t&>(obj).*mem_fn)(std::forward<Args_t>(args)...));
  } else {
    (const_cast<CThis_t&>(obj).*mem_fn)(std::forward<Args_t>(args)...);
  }
}

template<class CReturn_t, class This_t, class... MemFnArgs_t, class... Args_t>
constexpr auto
SameAsConstMemFunc(This_t* obj, CReturn_t (This_t::*mem_fn)(MemFnArgs_t...) const, Args_t&&... args)
  -> NonConst_t<CReturn_t>
{
  return SameAsConstMemFunc(*obj, mem_fn, std::forward<Args_t>(args)...);
}

///////////////////////////////////////////////////////////////////////////////
// Uncopyable_t
///////////////////////////////////////////////////////////////////////////////

struct Uncopyable_t
{
  constexpr Uncopyable_t() = default;
  ~Uncopyable_t()          = default;

  constexpr Uncopyable_t(Uncopyable_t&&)                         = delete;
  constexpr Uncopyable_t(const Uncopyable_t&)                    = delete;
  constexpr auto operator=(Uncopyable_t&&) -> Uncopyable_t&      = delete;
  constexpr auto operator=(const Uncopyable_t&) -> Uncopyable_t& = delete;
};

///////////////////////////////////////////////////////////////////////////////
// lambda overloaded
///////////////////////////////////////////////////////////////////////////////

template<class... Ts> struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace ECS
