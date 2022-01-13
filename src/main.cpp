#include <iostream>

#include "ecs_manager.hpp"
#include "arguments.hpp"

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

constexpr auto rendereable_printer [[maybe_unused]] = [](auto& ren, auto& pos/*, auto& ent*/) {
    //std::cout << "Found entity ID:     " << ent.GetID() << std::endl;
    //std::cout << "Entity address:      " << (void*)&ent << std::endl;
    std::cout << "RenderComponent:     " << ren.c << std::endl;
    std::cout << "PositionComponent_t: " << "X: " << pos.x << " Y: " << pos.y << std::endl;
};

constexpr auto movable_printer [[maybe_unused]] = [](auto& phy, auto& pos, auto& ent) {
    std::cout << "Found entity ID:     " << ent.GetID() << std::endl;
    std::cout << "Entity address:      " << (void*)&ent << std::endl;
    std::cout << "PhysicsComponent_t:  " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "PositionComponent_t: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl;
};

int main()
{
    using Rendereable_t    = ECS::Base_t<RenderComponent_t, PositionComponent_t>;
    using Movable_t        = ECS::Base_t<PhysicsComponent_t, PositionComponent_t>;
    using BasicCharacter_t = ECS::Derived_t<Rendereable_t, Movable_t>;

    ECS::ECSManager_t<Rendereable_t, Movable_t, BasicCharacter_t> ecs_man {  };

    Args::Arguments_t ren_args { Args::Wrapper_v<RenderComponent_t>, 'c' };
    Args::Arguments_t pos_args { Args::Wrapper_v<PositionComponent_t>, 1, 2 };
    Args::Arguments_t phy_args { Args::Wrapper_v<PhysicsComponent_t>, 3, 4 };

    ecs_man.CreateEntity<Rendereable_t>(ren_args);
    ecs_man.CreateEntity<Rendereable_t>(ren_args, pos_args);
    ecs_man.CreateEntity<Movable_t>(pos_args, phy_args);

    ecs_man.ForEachEntity<Rendereable_t>(rendereable_printer);

    return 0;
}
