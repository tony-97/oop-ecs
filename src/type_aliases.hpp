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

    template<class CID_t, class = typename CID_t::value_type>
    constexpr Handle_t<CID_t::value_type>(CID_t cid) : mIndex{ cid } {  }

    //template<class EID_t, class = typename EID_t::value_type::Signature_t>
    //constexpr Handle_t<EID_t::value_type::Signature_t>(EID_t eid) : mIndex{ eid } {  }

    constexpr Handle_t(std::size_t index) : mIndex { index } {  }

    constexpr std::size_t GetIndex() const { return mIndex; }

    template<class U>
    constexpr operator U() { return static_cast<U>(mIndex); }
};

} // namespace ECS
