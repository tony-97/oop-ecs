#include <iostream>
#include <vector>

#include "ecs_manager.hpp"
#include "class.hpp"

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
    std::cout << "[Found entity ID    ]: " << ent.GetID() << std::endl;
    std::cout << "[RenderComponent    ]: " << ren.c << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: " << pos.x << " Y: " << pos.y << std::endl << std::endl;
};

constexpr auto movable_printer [[maybe_unused]] = [](auto& phy, auto& pos, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetID() << std::endl;
    std::cout << "[PhysicsComponent_t ]: " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl << std::endl;
};

constexpr auto basic_character_printer [[maybe_unused]] = [](auto& ren, auto& pos, auto& phy, auto ent) {
    std::cout << "[Found entity ID    ]: " << ent.GetID() << std::endl;
    std::cout << "[RenderComponent    ]: " << ren.c << std::endl;
    std::cout << "[PhysicsComponent_t ]: " << "VX: " << phy.vx  << " VY: " << phy.vy << std::endl;
    std::cout << "[PositionComponent_t]: " << "X: "  << pos.x   << " Y: "  << pos.y  << std::endl << std::endl;
};

int main()
{
    using Renderable_t     = ECS::Class_t<RenderComponent_t, PositionComponent_t>;
    using Movable_t        = ECS::Class_t<PhysicsComponent_t, PositionComponent_t>;
    using BasicCharacter_t = ECS::Class_t<Renderable_t, Movable_t>;

    ECS::ECSManager_t<Renderable_t, Movable_t, BasicCharacter_t> ecs_man {  };

    ecs_man.CreateEntity<Renderable_t>(RenderComponent_t{ 'a' });
    ecs_man.CreateEntity<Renderable_t>(RenderComponent_t{ 'b' }, PositionComponent_t{ 1, 2 });
    ecs_man.CreateEntity<Movable_t>(PositionComponent_t{ 2, 2 }, PhysicsComponent_t{ 3, 4 });
    ecs_man.CreateEntity<BasicCharacter_t>(RenderComponent_t{ 'd' }, PositionComponent_t{ 6, 2 }, PhysicsComponent_t{ 1, 1 });

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);

    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>(movable_printer);

    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>(basic_character_printer);

    std::cout << "Converting basic character to movable..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>([&](auto&, auto&, auto&, auto&& ent){
               ecs_man.TransformTo<Movable_t>(ent);
            });

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);

    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>(movable_printer);

    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>(basic_character_printer);

    std::cout << "Converting back basic character..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>([&](auto& phy, auto&, auto&& ent){
                if (phy.vx == 5 && phy.vy == 4) {
                    ecs_man.TransformTo<BasicCharacter_t>(ent, RenderComponent_t{ 'd' });
                }
            });

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEachEntity<Renderable_t>(rendereable_printer);

    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEachEntity<Movable_t>(movable_printer);

    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEachEntity<BasicCharacter_t>(basic_character_printer);
    return 0;
}
