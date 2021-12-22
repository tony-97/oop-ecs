#pragma once

#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <iostream>

#include "type_aliases.hpp"
#include "helpers.hpp"

namespace ECS
{

struct EntityBase_t
{
protected:

    void DetachComponentID(ComponentTypeID_t cmp_tp_id)
    {
        mComps.erase(cmp_tp_id);
    }

    void AttachComponentID(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id)
    {
        mComps[cmp_tp_id] = cmp_id;
    }

    auto FindRequiredComponentID(ComponentTypeID_t cmp_tp_id) const
    -> Optional_t<ComponentID_t>
    {
        Optional_t<ComponentID_t> cmp_id {  };

        auto ite { mComps.find(cmp_tp_id) };
        if (ite != mComps.cend()) {
            cmp_id.emplace(ite->second);
        }

        return cmp_id;
    }

    auto FindRequiredComponentID(ComponentTypeID_t cmp_tp_id)
    -> Optional_t<ComponentID_t>
    {
        return const_cast<const EntityBase_t*>
               (this)->FindRequiredComponentID(cmp_tp_id);
    }

    auto GetRequiredComponentID(ComponentTypeID_t cmp_tp_id) const
    -> ComponentID_t
    {
        if (!FindRequiredComponentID(cmp_tp_id)) {
            std::cerr << "[ERROR]: Required component ID not found." << std::endl
                      << "ComponentTypeID_t: " << cmp_tp_id << std::endl;
            std::exit(EXIT_FAILURE);
        }

        return const_cast<EntityBase_t*>(this)->mComps[cmp_tp_id];
    }

    auto GetRequiredComponentID(ComponentTypeID_t cmp_tp_id)
    -> ComponentID_t
    {
        return const_cast<const EntityBase_t*>
               (this)->GetRequiredComponentID(cmp_tp_id);
    }

    void UpdateComponentID(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id)
    {
        mComps[cmp_tp_id] = cmp_id;
    }

    auto begin()        { return mComps.begin();  }
    auto begin()  const { return mComps.begin();  }
    auto cbegin() const { return mComps.cbegin(); }
    auto end()          { return mComps.end();    }
    auto end()    const { return mComps.end();    }
    auto cend()   const { return mComps.cend();   }

public:

    virtual ~EntityBase_t() = default;
    constexpr explicit EntityBase_t() = default;

private:

    std::unordered_map<ComponentTypeID_t, ComponentID_t> mComps {  };
};

template<class EntMan_t>
struct Entity_t final : public EntityBase_t,
                        public Uncopyable_t
{
    friend EntMan_t;
    using ConstructorKey_t = typename EntMan_t::EntityCreationKey_t;

    explicit constexpr Entity_t(ConstructorKey_t, EntityID_t ent_id)
        : EntityBase_t {  }, mID { ent_id } {  };

    constexpr auto GetEntityID() const -> EntityID_t { return mID; }

private:

    constexpr auto operator=(Entity_t&& other)
    -> Entity_t&
    {
        EntityBase_t::operator=(std::move(other));
        return *this;
    }

    EntityID_t mID {  };
};

} // namespace ECS
