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

constexpr auto rendereable_printer = [](auto& ren, auto& pos, auto& ent) {
    std::cout << "Found entity ID:     " << ent.GetEntityID() << std::endl;
    std::cout << "Entity address:      " << (void*)&ent << std::endl;
    std::cout << "RenderComponent:     " << ren.c << std::endl;
    std::cout << "PositionComponent_t: " << "X: " << pos.x << " Y: " << pos.y << std::endl;
};

constexpr auto movable_printer = [](auto& phy, auto& pos, auto& ent) {
    std::cout << "Found entity ID:     " << ent.GetEntityID() << std::endl;
    std::cout << "Entity address:      " << (void*)&ent << std::endl;
    std::cout << "PhysicsComponent_t:  " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "PositionComponent_t: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl;
};

int main()
{
    using Rendereable_t = TMPL::TypeList_t<RenderComponent_t, PositionComponent_t>;
    using Movable_t     = TMPL::TypeList_t<PhysicsComponent_t, PositionComponent_t>;
    ECS::EntityManager_t<Rendereable_t, Movable_t> ent_man {  };
    Args::Arguments_t ren_args { Args::Wrapper_v<RenderComponent_t>, 'c' };
    Args::Arguments_t pos_args { Args::Wrapper_v<PositionComponent_t>, 1, 2 };
    Args::Arguments_t phy_args { Args::Wrapper_v<PhysicsComponent_t>, 3, 4 };

    auto& e1 = ent_man.CreateEntityForSystems<Rendereable_t>(ren_args);
    auto& e2 = ent_man.CreateEntityForSystems<Rendereable_t>(ren_args, pos_args);
    auto& e3 = ent_man.CreateEntityForSystems<Movable_t>(pos_args, phy_args);

    std::cout << std::endl << "Iterating over rederables...." << std::endl;
    ent_man.ForEachEntityType<Rendereable_t>(rendereable_printer);

    ent_man.MarkEntityForDestroy(e1);
    ent_man.MarkEntityForDestroy(e2);
    ent_man.MarkEntityForDestroy(e3);

    ent_man.GCEntities();

    std::cout << std::endl << "Iterating again over rederables...." << std::endl;
    ent_man.ForEachEntityType<Rendereable_t>(rendereable_printer);

    std::cout << std::endl << "Iterating over movables...." << std::endl;
    ent_man.ForEachEntityType<Movable_t>(movable_printer);

    std::cout << std::endl << "Iterating again over movables...." << std::endl;
    ent_man.ForEachEntityType<Movable_t>(movable_printer);

    return 0;
}
