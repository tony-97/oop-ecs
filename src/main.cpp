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
    Args::Arguments_t phy_args { Args::Wrapper_v<PhysicsComponent_t>, 3, 4 };

    ent_man.CreateEntityForSystems<Rendereable_t>(ren_args);
    ent_man.CreateEntityForSystems<Rendereable_t>(ren_args, pos_args);
    ent_man.CreateEntityForSystems<Movable_t>(pos_args, phy_args);

    std::cout << std::endl << "Iterating over rederables...." << std::endl;
    ent_man.ForEachEntityType<Rendereable_t>([](auto& ren, auto& pos, auto& ent){
        std::cout << "Found entity ID:     " << ent.GetEntityID() << std::endl;
        std::cout << "Entity address:      " << (void*)&ent << std::endl;
        std::cout << "RenderComponent:     " << ren.c << std::endl;
        std::cout << "PositionComponent_t: " << "X: " << pos.x << " Y: " << pos.y << std::endl;
    });

    std::cout << std::endl << "Iterating over movables...." << std::endl;
    ent_man.ForEachEntityType<Movable_t>([](auto& phy, auto& pos, auto& ent){
        std::cout << "Found entity ID:     " << ent.GetEntityID() << std::endl;
        std::cout << "Entity address:      " << (void*)&ent << std::endl;
        std::cout << "PhysicsComponent_t:  " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
        std::cout << "PositionComponent_t: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl;
    });
    return 0;
}
