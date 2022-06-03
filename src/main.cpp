#include <iostream>
#include <vector>

#include "ecs_manager.hpp"
#include "arguments.hpp"

#include "struct_of_arrays.hpp"

struct RenderComponent_t
{
    char c;
    ~RenderComponent_t() { std::cout << "Dtor. called\n"; }
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
    std::cout << "[Found entity ID    ]: " << ent.GetIndex() << std::endl;
    std::cout << "[RenderComponent    ]: " << ren.c << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: " << pos.x << " Y: " << pos.y << std::endl << std::endl;
};

constexpr auto movable_printer [[maybe_unused]] = [](auto& phy, auto& pos, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetIndex() << std::endl;
    std::cout << "[PhysicsComponent_t ]: " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl << std::endl;
};

constexpr auto basic_character_printer [[maybe_unused]] = [](auto& ren, auto& pos, auto& phy, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetIndex() << std::endl;
    std::cout << "[RenderComponent    ]: " << ren.c << std::endl;
    std::cout << "[PhysicsComponent_t ]: " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl << std::endl;
};

int main()
{
    using Renderable_t     = ECS::Base_t<RenderComponent_t, PositionComponent_t>;
    using Movable_t        = ECS::Base_t<PhysicsComponent_t, PositionComponent_t>;
    using BasicCharacter_t = ECS::Derived_t<Renderable_t, Movable_t>;

    ECS::ECSManager_t<Renderable_t, Movable_t, BasicCharacter_t> ecs_man {  };

    Args::Arguments_t ren_args1 { Args::For_v<RenderComponent_t>, 'a' };
    Args::Arguments_t ren_args2 { Args::For_v<RenderComponent_t>, 'b' };
    Args::Arguments_t pos_args { Args::For_v<PositionComponent_t>, 1, 2 };
    Args::Arguments_t phy_args { Args::For_v<PhysicsComponent_t>, 3, 4 };

    auto ent1 [[maybe_unused]] = ecs_man.CreateEntity<Renderable_t>(ren_args1);
    auto ent2 [[maybe_unused]] = ecs_man.CreateEntity<Renderable_t>(ren_args2, pos_args);
    auto ent3 [[maybe_unused]] = ecs_man.CreateEntity<Movable_t>(pos_args, phy_args);
    auto ent4 [[maybe_unused]] = ecs_man.CreateEntity<BasicCharacter_t>(Args::Arguments_t{ Args::For_v<RenderComponent_t>, 'd' });

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);

    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>(movable_printer);

    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>(basic_character_printer);

    std::cout << "===Transforming ent4 aka basic_character to renderable===" << std::endl;
    ecs_man.TransformTo<Renderable_t>(ent4);

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);

    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>(movable_printer);

    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>(basic_character_printer);

    return 0;
}
