#include "entity_manager.hpp"

struct RenderComponent_t
{
    char c;
};

struct PositionComponent_t
{
    int x, y;
};

struct PhysicsComponent_t
{
    int vx, vy;
};

#include <iostream>
int main()
{
    using Rendereable_t = TMPL::TypeList_t<RenderComponent_t>;
    using Movable_t     = TMPL::TypeList_t<PositionComponent_t>;
    using Physicable_t  = TMPL::TypeList_t<PhysicsComponent_t>;
    using Character_t   = TMPL::TypeListCat_t<Rendereable_t, Movable_t, Physicable_t>;
    ECS::EntityManager_t<Rendereable_t,
                         Movable_t,
                         Physicable_t,
                         Character_t> ent_man {  };

    Args::Arguments_t ren_args { Args::Wrapper_v<RenderComponent_t>, 'c' };

    const auto& ent1 = ent_man.CreateRequieredEntity<Rendereable_t>(ren_args);
    const auto& ent2 = ent_man.CreateRequieredEntity<Character_t>(ren_args);
    return 0;
}
