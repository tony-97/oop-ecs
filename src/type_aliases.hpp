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

template<class CID_t>
Handle_t(CID_t) -> Handle_t<typename CID_t::value_type>;

template<class EID_t>
Handle_t(EID_t) -> Handle_t<typename EID_t::value_type::Signature_t>;

} // namespace ECS
