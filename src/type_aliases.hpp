#pragma once

#include "ecs_map.hpp"

#include <cstddef>
#include <type_traits>

namespace ECS
{

template<class T> using ID_t = typename ECSMap_t<T>::Key_t;

template<typename T>
concept ComponentID = requires { typename T::value_type; } && !requires { typename T::value_type::Signature_t; };

template<typename T>
concept EntityID = requires { typename T::value_type::Signature_t; };

template<class T>
struct Handle_t
{
private:
    std::size_t mIndex {  };
public:
    using type = T;

    template<class U, class = std::enable_if_t<!std::is_same_v<T, U>>>
    constexpr Handle_t(Handle_t<U>) = delete;

    constexpr Handle_t() : mIndex {  } {  }

    constexpr Handle_t(std::size_t index) : mIndex { index } {  }

    constexpr std::size_t GetIndex() const { return mIndex; }

    template<class U>
    constexpr operator U() { return static_cast<U>(mIndex); }
};

template<ComponentID CID>
Handle_t(CID) -> Handle_t<typename CID::value_type>;

template<EntityID EID>
Handle_t(EID) -> Handle_t<typename EID::value_type::Signature_t>;

} // namespace ECS
