#include <iostream>

//#include "ecs_manager.hpp"
//#include "arguments.hpp"

#include "ecs_map.hpp"

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

constexpr auto rendereable_printer [[maybe_unused]] = [](const auto& ren, const auto& pos, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetID() << std::endl;
    std::cout << "[RenderComponent    ]: " << ren.c << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: " << pos.x << " Y: " << pos.y << std::endl << std::endl;
};

constexpr auto movable_printer [[maybe_unused]] = [](auto& phy, auto& pos, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetID() << std::endl;
    std::cout << "[PhysicsComponent_t ]: " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl << std::endl;
};

int main()
{
    //using Renderable_t     = ECS::Base_t<RenderComponent_t, PositionComponent_t>;
    //using Movable_t        = ECS::Base_t<PhysicsComponent_t, PositionComponent_t>;
    //using BasicCharacter_t = ECS::Derived_t<Renderable_t, Movable_t>;
    //
    //ECS::ECSManager_t<Renderable_t, Movable_t, BasicCharacter_t> ecs_man {  };
    //
    //Args::Arguments_t ren_args1 { Args::For_v<RenderComponent_t>, 'a' };
    //Args::Arguments_t ren_args2 { Args::For_v<RenderComponent_t>, 'b' };
    //Args::Arguments_t pos_args { Args::For_v<PositionComponent_t>, 1, 2 };
    //Args::Arguments_t phy_args { Args::For_v<PhysicsComponent_t>, 3, 4 };
    //
    //auto ent1 = ecs_man.CreateEntity<Renderable_t>(ren_args1);
    //auto ent2 = ecs_man.CreateEntity<Renderable_t>(ren_args2, pos_args);
    //auto ent3 = ecs_man.CreateEntity<Movable_t>(pos_args, phy_args);
    //auto ent4 = ecs_man.CreateEntity<BasicCharacter_t>(Args::Arguments_t{ Args::For_v<RenderComponent_t>, 'd' });
    //
    //std::cout << "Iterating over renderables..." << std::endl;
    //ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);
    //
    //std::cout << "Iterating over movables..." << std::endl;
    //ecs_man.ForEachEntity<Movable_t>(movable_printer);
    //
    ////std::cout << "==Destroying ent4===" << std::endl;
    ////ecs_man.Destroy(ent4);
    //
    //std::cout << "==Destroying ent1===" << std::endl;
    //ecs_man.Destroy(ent1);
    //
    ////std::cout << "==Destroying ent3===" << std::endl;
    ////ecs_man.Destroy(ent3);
    ////
    ////std::cout << "==Destroying ent2===" << std::endl;
    ////ecs_man.Destroy(ent2);
    //
    //std::cout << "Iterating over renderables..." << std::endl;
    //ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);
    //
    //std::cout << "Iterating over movables..." << std::endl;
    //ecs_man.ForEachEntity<Movable_t>(movable_printer);

    ECS::ECSMap_t<int> slot_map {  };
    auto k1 = slot_map.emplace_back(1);
    auto k2 = slot_map.emplace_back(2);
    auto k3 = slot_map.emplace_back(3);
    auto k4 = slot_map.emplace_back(4);

    std::cout << slot_map[k1] << std::endl;

    slot_map.erase(std::move(k1));

    std::cout << slot_map[k2] << std::endl;
    std::cout << slot_map[k3] << std::endl;
    std::cout << slot_map[k4] << std::endl;

    slot_map.erase(std::move(k2));

    std::cout << slot_map[k3] << std::endl;
    std::cout << slot_map[k4] << std::endl;

    return 0;
}
