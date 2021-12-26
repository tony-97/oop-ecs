#pragma once

#include <memory>
#include <stack>

#include "helpers.hpp"
#include "type_aliases.hpp"
#include "entity.hpp"
#include "arguments.hpp"

#include "component_storage.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<class... SystemsSignatures_t>
using ComponentExtractor_t = Seq::UniqueTypes_t<Seq::SeqCat_t<SystemsSignatures_t...>>;

// SystemsSignatures_t is a variadic template argument that each one is a list of components
template<class... SystemsSignatures_t>
struct EntityManager_t
{
private:

    using Self_t = EntityManager_t;

    using OwnEntitity_t = Entity_t<Self_t>;

    using ComponentList_t = ComponentExtractor_t<SystemsSignatures_t...>;
    using ComponentWarehouse_t = ComponentStorage_t<ComponentList_t>;

    using EntityVector_t = Vector_t<std::unique_ptr<OwnEntitity_t>>;

    struct ComponentCreator_t
    {
        Self_t& ent_man {  };

        template<class RequieredComponent_t, class... Args_t> constexpr auto
        operator()(OwnEntitity_t& ent, Args_t&&... args) -> void
        {
            ent_man.CreateRequieredComponent<RequieredComponent_t>
                (ent, std::forward<Args_t>(args)...);
        }
    };

    struct ComponentTraversal_t
    {
        template<class Head_t, class... Tail_t, class Callable_t, class EntMan_t>
        constexpr auto
        operator()(Callable_t&& callback, EntMan_t&& ent_man) const -> void
        {
            auto it  { ent_man.mComponents.template begin<Head_t>() };
            auto end { ent_man.mComponents.template end<Head_t>() };
            for (; it != end; ++it) {
                auto& [ent_id, cmp] { *it };
                const auto& ent { ent_man.GetEntityByID(ent_id) };
                callback(cmp, ent_man.template GetRequiredComponent<Tail_t>(ent)..., ent);
            }
        }

        template<class Callable_t, class EntMan_t>
        constexpr auto
        operator()([[maybe_unused]]Callable_t&& callback,
                   [[maybe_unused]]EntMan_t&& ent_man) const -> void {  }
    };

    template<class RequieredComponent_t, class... Args_t> constexpr auto
    CreateRequieredComponent(OwnEntitity_t& ent, Args_t&&... args) -> void
    {
        constexpr auto cmp_tp_id
        {
            ComponentWarehouse_t::template
                GetRequiredComponentTypeIndex<RequieredComponent_t>()
        };
        const auto cmp_id
        {
            mComponents.template
                        CreateRequiredComponent<RequieredComponent_t>
                        (ent.GetEntityID(), std::forward<Args_t>(args)...)
        };

        ent.AttachComponentID(cmp_tp_id, cmp_id);
    }

    template<class ComponentArgs_t> constexpr auto
    CreateRequieredComponentFromArgs(OwnEntitity_t& ent, ComponentArgs_t&& args)
    -> void
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::For_type;

        auto cmp_creator
        {
            [&](auto&&... args)
            {
                CreateRequieredComponent<RequieredComponent_t>
                    (ent, std::forward<decltype(args)>(args)...);
            }
        };

        Args::apply(cmp_creator, std::forward<ComponentArgs_t>(args));
    }

    template<class RequieredComponent_t> constexpr auto
    GetRequiredComponent(const OwnEntitity_t& ent) const -> const RequieredComponent_t&
    {
        const auto cmp_tp_id
        {
            mComponents.template
                GetRequiredComponentTypeIndex<RequieredComponent_t>()
        };
        const auto cmp_id { ent.GetRequiredComponentID(cmp_tp_id) };
        return mComponents.template GetRequiredComponent<RequieredComponent_t>(cmp_id);
    }

    template<class RequieredComponent_t> constexpr auto
    GetRequiredComponent(const OwnEntitity_t& ent) -> RequieredComponent_t&
    {
        return SameAsConstMemFunc(this,
                                  &Self_t::GetRequiredComponent<RequieredComponent_t>,
                                  ent);
    }

    constexpr auto GetEntityByID(EntityID_t eid) const -> const OwnEntitity_t&
    {
        return *mEntitites[eid];
    }

    constexpr auto GetEntityByID(EntityID_t eid) -> OwnEntitity_t&
    {
        return *mEntitites[eid];
    }

    constexpr static auto CheckIfSignaturesAreUnique() -> void
    {
        static_assert(TMPL::AreUnique_v<SystemsSignatures_t...>,
                      "Entities signatures must be unique.");
        static_assert(TMPL::AreUnique_v<TMPL::TypeAt_t<0, SystemsSignatures_t>...>,
                      "Main components must be unique.");
    }

    constexpr auto RemoveAllComponentsFromEntity(OwnEntitity_t& ent) -> void
    {
        for (auto [cmp_tp_id, cmp_id] : ent) {
            auto eid { mComponents.RemoveRequiredComponent(cmp_tp_id, cmp_id) };
            auto& ent_upd { GetEntityByID(eid) };
            ent_upd.UpdateComponentID(cmp_tp_id, cmp_id);
        }       
    }

    constexpr auto
    UpdateComponentsEntityIDFromEntity(OwnEntitity_t& ent, EntityID_t new_ent_id)
    -> void
    {
        for (auto [cmp_tp_id, cmp_id] : ent) {
            mComponents.SetEntityID(cmp_tp_id, cmp_id, new_ent_id);
        }
    }

    constexpr auto DestroyEntity(OwnEntitity_t& ent) -> void
    {
        auto& last_ent { *mEntitites.back() };
        RemoveAllComponentsFromEntity(ent);
        UpdateComponentsEntityIDFromEntity(last_ent, ent.GetEntityID());
        ent = std::move(last_ent);
        mEntitites.pop_back();
    }

public:

    using EntityCreationKey_t = Key_t<Self_t>;

    explicit EntityManager_t()
    {
        CheckIfSignaturesAreUnique();
    }

    template<class... Signatures_t , class... ComponentsArgs_t> constexpr auto
    CreateEntityForSystems(ComponentsArgs_t&&... args) -> OwnEntitity_t&
    {
        static_assert(TMPL::AreUnique_v<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>,
                      "Component arguments must be unique.");
        using RequiredComponents_t = ComponentExtractor_t<Signatures_t...>;
        using ComponentsArgsFor_t  = TMPL::TypeList_t<typename std::remove_reference_t<ComponentsArgs_t>::For_type...>;
        using RemainingComponents_t [[maybe_unused]] = Seq::RemoveTypes_t<RequiredComponents_t, ComponentsArgsFor_t>;
        static_assert(TMPL::IsSubsetOf_v<ComponentsArgsFor_t, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");
        auto& ent { *mEntitites.emplace_back(std::make_unique<OwnEntitity_t>(constructor_key, mEntitites.size())) };
        (CreateRequieredComponentFromArgs(ent, std::forward<ComponentsArgs_t>(args)), ...);

        Seq::ForEach_t<RemainingComponents_t>::Do(ComponentCreator_t{ *this }, ent);
        return ent;
    }

    constexpr auto MarkEntityForDestroy(const OwnEntitity_t& ent) -> void
    {
        mDeadEntities.emplace(ent.GetEntityID());
    }

    constexpr auto GCEntities() -> void
    {
        while (not mDeadEntities.empty()) {
            DestroyEntity(GetEntityByID(mDeadEntities.top()));
            mDeadEntities.pop();
        }
    }

    template<class Signature_t, class Callable_t> constexpr auto
    ForEachEntityType(Callable_t&& callback) -> void
    {
        Seq::Unpacker_t<Signature_t>::Call(ComponentTraversal_t{  },
                                           std::forward<Callable_t>(callback),
                                           *this);
    }

    template<class Signature_t, class Callable_t> constexpr auto
    ForEachEntityType(Callable_t&& callback) const -> void
    {
        SameAsConstMemFunc(this, &Self_t::ForEachEntityType<Signature_t, decltype(callback)>, callback);
    }

private:

    static constexpr inline Key_t<Self_t> constructor_key {  };

    std::stack<EntityID_t> mDeadEntities {  };
    EntityVector_t mEntitites {  };
    ComponentWarehouse_t mComponents {  };
};

} // namespace ECS

