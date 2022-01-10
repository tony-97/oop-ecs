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
        InternalComponent_t(EntityTypeID_t e_tp_id, EntityID_t eid, Args_t&&... args)
            : mEntityTypeID { e_tp_id },
              mEntityID { eid },
              mSelf { std::forward<Args_t>(args)... } {  }

        EntityTypeID_t mEntityTypeID {  };
        EntityID_t mEntityID {  };
        Component_t mSelf {  };
    };
} // namespace ECS
