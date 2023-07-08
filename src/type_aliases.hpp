#pragma once

#include "ecs_map.hpp"
#include <cstddef>

namespace ECS
{

template<class T> using ID_t = typename ECSMap_t<T>::Key_t;

template<class T>
struct Handle_t
{
private:
    std::size_t mIndex {  };
public:
    using type = T;

    constexpr Handle_t(std::size_t index) : mIndex { index } {  }

    constexpr std::size_t GetIndex() const { return mIndex; }

    template<class U>
    constexpr operator U() { return static_cast<U>(mIndex); }
};

} // namespace ECS
