#include <iostream>

#include "entity_manager.hpp"
#include "type_list.hpp"

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

int main()
{
    using Rendereable_t = TMPL::TypeList_t<RenderComponent_t, PositionComponent_t>;
    using Movable_t     = TMPL::TypeList_t<PhysicsComponent_t, PositionComponent_t>;
    ECS::EntityManager_t<Rendereable_t, Movable_t> ent_man {  };

    Args::Arguments_t ren_args { Args::Wrapper_v<RenderComponent_t>, 'c' };
    Args::Arguments_t pos_args { Args::Wrapper_v<PositionComponent_t>, 1, 2 };

    ent_man.CreateEntityForSystems<Rendereable_t>(ren_args);
    ent_man.CreateEntityForSystems<Rendereable_t>(ren_args, pos_args);

    const auto& e_man = ent_man;
    e_man.ForEachEntityType<Rendereable_t>([](auto& ren, auto& pos, auto& ent){
        std::cout << "Found entity ID:     " << ent.GetEntityID() << std::endl;
        std::cout << "RenderComponent:     " << ren.c << std::endl;
        std::cout << "PositionComponent_t: " << "X: " << pos.x << " Y: " << pos.y << std::endl;
    });
    return 0;
}
