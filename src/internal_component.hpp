#pragma once

#include <utility>

#include "type_aliases.hpp"

namespace ECS
{
    template<class Component_t>
    struct InternalComponent_t final
    {
        using Component_type = Component_t;

        template<class... Args_t> constexpr explicit
        InternalComponent_t(EntityID_t eid, Args_t&&... args)
            : mEntityID{ eid }, mSelf{ std::forward<Args_t>(args)... } {  }

        EntityID_t mEntityID {  };
        Component_t mSelf {  };
    };
} // namespace ECS
