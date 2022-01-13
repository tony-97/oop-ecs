#include <tmpl.hpp>
#include <tuple>
#include <utility>

#include "arguments.hpp"
#include "component_manager.hpp"
#include "entity.hpp"
#include "entity_manager.hpp"
#include "helpers.hpp"

namespace ECS
{

namespace Seq = TMPL::Sequence;

template<bool b, class... Ts>
struct RawBase
{
    static_assert(b, "Types are not unique.");
};

template<class... Ts>
struct RawBase<true, Ts...>
{
    using type = Seq::UniqueTypes_t<TMPL::TypeList_t<Ts...>>;
};

template<class... Ts>
struct Base_t : RawBase<TMPL::AreUnique_v<Ts...>, Ts...> {  };

template<bool b, class... Ts>
struct RawDerived
{
    static_assert(b, "Derived not derived from Base.");
};

template<class... Ts>
struct RawDerived<true, Ts...>
{
    using type = Seq::UniqueTypes_t<Seq::SeqCat_t<Ts...>>;
};

template<class T>
struct IsBase : std::false_type {  };

template<template<class...> class T, class... Ts>
struct IsBase<T<Ts...>> : std::bool_constant<std::is_same_v<T<Ts...>, Base_t<Ts...>>> {  };

template<class T>
constexpr static inline auto IsBase_v { IsBase<T>::value };

template<class... Ts>
struct Derived_t : RawDerived<std::conjunction_v<IsBase<Ts>...>, Ts...> {  };

template<class... EntitySignatures_t>
using ComponentsFrom_t = Seq::UniqueTypes_t<Seq::SeqCat_t<typename EntitySignatures_t::type...>>;

template<class... EntitySignatures_t>
struct ECSManager_t
{
private:

    template<class SystemSignature_t>
    struct EntityTraversal_t
    {
        template<class EntitySignature_t, class Callback_t, class ECSMan_t>
        constexpr auto operator()(Callback_t&& cb, ECSMan_t&& ecs_man) const -> void
        {
            if constexpr (TMPL::IsSubsetOf_v<typename SystemSignature_t::type,
                                             typename EntitySignature_t::type>) {
                auto it { ecs_man.mEntities.template begin<Entity_t<EntitySignature_t>>() };
                auto end { ecs_man.mEntities.template end<Entity_t<EntitySignature_t>>() };
                // TODO: iterate with rbegin and when delete a entity just swap with the end
                for (; it != end; ++it) {
                    std::apply(cb, ecs_man.template GetArgumentsFor<SystemSignature_t>(*it));
                }
            }
        }
    };

    struct GetArguments_t
    {
        template<class... Args_t, class Ent_t, class ECSMan_t>
        constexpr auto operator()(Ent_t&& ent, ECSMan_t&& ecs_man) const
        {
            return std::forward_as_tuple(ecs_man.GetComponent(ent.template GetComponentID<Args_t>())...);
        }
    };

    struct ComponentCreator_t
    {
        template<class... Components_t, class ECSMan_t> constexpr auto
        operator()(ECSMan_t& ecs_man) const
        {
            return std::tuple {
                ecs_man.mComponents.template Create<Components_t>()... };
        }
    };

    template<class ComponentArgs_t> constexpr auto
    CreateComponent(ComponentArgs_t&& arg)
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::type;

        auto cmp_creator {
            [&](auto&&... args) {
                return mComponents.template Create<RequieredComponent_t>
                    (std::forward<decltype(args)>(args)...);
            }
        };

        return Args::apply(cmp_creator, std::forward<ComponentArgs_t>(arg));
    }

    template<class CmpID_t, class Cmp = typename std::remove_reference_t<CmpID_t>::type>
    constexpr auto GetComponent(CmpID_t&& cmp_id) const -> const Cmp&
    {
        return mComponents.template operator[]<ComponentWrapper<Cmp>>(cmp_id.mID).mSelf;
    }

    template<class CmpID_t, class Cmp = typename std::remove_reference_t<CmpID_t>::type>
    constexpr auto GetComponent(CmpID_t&& cmp_id) -> Cmp&
    {
        return SameAsConstMemFunc(this, &ECSManager_t::GetComponent<CmpID_t, Cmp>, std::forward<CmpID_t>(cmp_id));
    }

    template<class Signature_t, class Ent_t>
    constexpr auto GetArgumentsFor(Ent_t&& ent)
    {
        return Seq::Unpacker_t<Signature_t>::Call(GetArguments_t{  }, std::forward<Ent_t>(ent), *this);
    }

    template<class Ent_t>
    constexpr auto GetComponents(Ent_t&& ent) const
    {
        return std::apply([&](auto&&... cmp_ids) {
                    return std::forward_as_tuple
                    (GetComponent(std::forward<decltype(cmp_ids)>(cmp_ids))...);
                }, ent.GetComponentIDs());
    }

    template<class Ent_t>
    constexpr auto GetComponents(Ent_t&& ent)
    {
        return std::apply([&](auto&&... cmp_ids) {
                    return std::forward_as_tuple
                    (GetComponent(std::forward<decltype(cmp_ids)>(cmp_ids))...);
                }, ent.GetComponentIDs());
    }

public:

    using ComponentList_t = ComponentsFrom_t<EntitySignatures_t...>;
    using EntityList_t    = TMPL::TypeList_t<EntitySignatures_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;
    using EntityMan_t     = EntityManager_t<Entity_t<EntitySignatures_t>...>;

    template<class T>
    using ComponentID_t = typename ComponentMan_t::template ComponentID_t<T>;

    template<class T>
    using EntityID_t    = typename EntityMan_t::template EntityID_t<T>;

    template<class EntitySignature_t, class... Arguments_t> constexpr auto
    CreateEntity(Arguments_t&&... args)
    {
        using RequiredEntity_t = Entity_t<EntitySignature_t>;
        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Arguments_t>::type...>;
        using RequiredComponents_t  = ComponentsFrom_t<EntitySignature_t>;
        using RemainingComponents_t = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        auto entity_creator {
            [&](auto&&... ids) {
                return mEntities.template
                    Create<RequiredEntity_t>(std::forward<decltype(ids)>(ids)...);
            }
        };

        return std::apply(entity_creator,
                ConvertTo<typename RequiredEntity_t::ComponentIDs_t>(std::tuple_cat(std::tuple{ CreateComponent(std::forward<Arguments_t>(args))... },
                                         Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{  }, *this)
                                         ))
                          );
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) const -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntitySignature_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) -> void
    {
        Seq::ForEach_t<EntityList_t>::Do(EntityTraversal_t<EntitySignature_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

private:

    ComponentMan_t mComponents {  };
    EntityMan_t mEntities {  };
};

} // namespace ECS
