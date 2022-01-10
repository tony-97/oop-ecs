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
struct Derived : RawDerived<std::conjunction_v<IsBase<Ts>...>, Ts...> {  };

template<class... Ts>
using Derived_t = Derived<Ts...>;

template<class... EntitySignatures_t>
using ComponentExtractor_t = Seq::UniqueTypes_t<Seq::SeqCat_t<typename EntitySignatures_t::type...>>;

template<class... EntitySignatures_t>
struct ECSManager_t
{
private:

    template<class SignatureBase_t>
    struct EntityTraversal_t
    {
        template<class EntitySignature_t, class Callback_t, class ECSMan_t>
        constexpr auto operator()(Callback_t&& cb, ECSMan_t&& ecs_man) const -> void
        {
            if constexpr (TMPL::IsSubsetOf_v<typename SignatureBase_t::type,
                                             typename EntitySignature_t::type>) {
                auto it { ecs_man.mEntities.template begin<Entity_t<EntitySignature_t>>() };
                auto end { ecs_man.mEntities.template end<Entity_t<EntitySignature_t>>() };
                for (; it != end; ++it) {
                    
                }
            }
        }
    };

    struct ComponentCreator_t
    {
        template<class... Components_t, class ECSMan_t> constexpr auto
        operator()(ECSMan_t& ecs_man) const
         {
            return std::tuple { ecs_man.mComponents.template CreateComponent<Components_t>()... };
        }
    };

    template<class ComponentArgs_t> constexpr auto
    CreateComponentFromArg(ComponentArgs_t&& args)
    {
        using RequieredComponent_t =
            typename std::remove_reference_t<ComponentArgs_t>::For_type;

        auto cmp_creator
        {
            [&](auto&&... args)
            {
                return mComponents.template CreateComponent<RequieredComponent_t>
                    (std::forward<decltype(args)>(args)...);
            }
        };

        return Args::apply(cmp_creator, std::forward<ComponentArgs_t>(args));
    }

public:

    using ComponentList_t = ComponentExtractor_t<EntitySignatures_t...>;
    using ComponentMan_t  = ComponentManager_t<ComponentList_t>;
    using EntityMan_t     = EntityManager_t<EntitySignatures_t...>;

    template<class T>
    using ComponentID_t = typename ComponentMan_t::template ComponentID_t<T>;

    template<class T>
    using EntityID_t    = typename EntityMan_t::template EntityID_t<T>;

    template<class EntitySignature_t, class... Arguments_t> constexpr auto
    CreateEntity(Arguments_t&&... args)
    {
        using ArgsTypes = TMPL::TypeList_t<typename std::remove_reference_t<Arguments_t>::For_type...>;
        using RequiredComponents_t = ComponentExtractor_t<EntitySignature_t>;
        using RemainingComponents_t [[maybe_unused]] = Seq::RemoveTypes_t<RequiredComponents_t, ArgsTypes>;
        static_assert(Seq::IsUnique_v<ArgsTypes>, "Component arguments must be unique.");
        static_assert(TMPL::IsSubsetOf_v<ArgsTypes, RequiredComponents_t>,
                      "Components arguments does not match the requiered components");

        //std::tuple ids1 { CreateComponentFromArg(std::forward<Arguments_t>(args))... };
        //auto ids2 { Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{  }, *this) };

        
        return std::apply([&](auto&&... ids) {
                          return mEntities.template CreateEntity<Entity_t<EntitySignature_t>>(ids.GetID()...);
                      }, 
                          std::tuple_cat(std::tuple{ CreateComponentFromArg(std::forward<Arguments_t>(args))... },
                                         Seq::Unpacker_t<RemainingComponents_t>::Call(ComponentCreator_t{  }, *this)
                                         )
                          );
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) const -> void
    {
        using Signatures_t = TMPL::TypeList_t<EntitySignatures_t...>;

        Seq::ForEach_t<Signatures_t>::Do(EntityTraversal_t<EntitySignature_t>{  },
                                         std::forward<Callable_t>(cb),
                                         *this);
    }

    template<class EntitySignature_t, class Callable_t>
    constexpr auto ForEachEntity(Callable_t&& cb) -> void
    {
        SameAsConstMemFunc
            (this, &ECSManager_t::ForEachEntity<EntitySignature_t, decltype(cb)>, std::forward<Callable_t>(cb));
    }

private:

    ComponentMan_t mComponents {  };
    EntityMan_t mEntities {  };
};

} // namespace ECS
