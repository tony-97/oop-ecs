#include <iostream>

#include "class.hpp"
#include "ecs_manager.hpp"
#include "traits.hpp"
#include "type_aliases.hpp"

struct RenderComponent_t
{
    char c {  };
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

struct Renderable_t     : ECS::Class_t<RenderComponent_t, PositionComponent_t>{};
struct Movable_t        : ECS::Class_t<PhysicsComponent_t, PositionComponent_t>{};
struct BasicCharacter_t : ECS::Class_t<Renderable_t, Movable_t>{};

struct ECSConfig_t
{
    using Signatures_t = TMPL::TypeList_t<Renderable_t, Movable_t, BasicCharacter_t>;
};

template<class T>
void fn(ECS::Handle_t<T> e) {  }

auto main() -> int
{
    ECS::ECSManager_t<ECSConfig_t> ecs_man {  };
    ecs_man.CreateEntity<Renderable_t>(RenderComponent_t{ 'a' });
    ecs_man.CreateEntity<Renderable_t>(RenderComponent_t{ 'b' }, PositionComponent_t{ 1, 2 });
    ecs_man.CreateEntity<Movable_t>(PositionComponent_t{ 2, 2 }, PhysicsComponent_t{ 3, 4 });
    auto basic_e = ecs_man.CreateEntity<BasicCharacter_t>(RenderComponent_t{ 'd' }, PositionComponent_t{ 6, 2 }, PhysicsComponent_t{ 1, 1 });
    auto ren_e = ecs_man.GetBaseID<Renderable_t>(basic_e);
    ECS::ECSManager_t<ECSConfig_t>::EntityID_t<Renderable_t> eid { 3 };
    ECS::ID_t<RenderComponent_t> cid { 4 };
    ECS::Handle_t h1 { cid };
    ECS::Handle_t h2 { eid };

    //fn(cid);

    using v = ECS::ECSManager_t<ECSConfig_t>::entity_type<Renderable_t>::ParentVariant_t;
    using v1 = ECS::Traits::Bases_t<ECS::Handle_t<Renderable_t>>;


    ecs_man.Match(ren_e,
            [&](ECS::Handle_t<BasicCharacter_t> e) {
                
            });

    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEach<Renderable_t>(rendereable_printer);
    
    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEach<Movable_t>(movable_printer);
    
    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEach<BasicCharacter_t>(basic_character_printer);
    
    std::cout << "Converting basic character to movable..." << std::endl;
    ecs_man.ForEach<BasicCharacter_t>([&](auto&, auto&, auto&, auto&& ent){
               ecs_man.TransformTo<Movable_t>(ent);
            });
    
    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEach<Renderable_t>(rendereable_printer);
    
    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEach<Movable_t>(movable_printer);
    
    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEach<BasicCharacter_t>(basic_character_printer);
    
    std::cout << "Converting back basic character..." << std::endl;
    ecs_man.ForEach<Movable_t>([&](auto& phy, auto&, auto e){
                if (phy.vx == 5 && phy.vy == 4) {
                    ecs_man.TransformTo<BasicCharacter_t>(e, RenderComponent_t{ 'd' });
                }
            });
    
    std::cout << "Iterating over renderables..." << std::endl;
    ecs_man.ForEach<Renderable_t>(rendereable_printer);
    
    std::cout << "Iterating over movables..." << std::endl;
    ecs_man.ForEach<Movable_t>(movable_printer);
    
    std::cout << "Iterating over basic characters..." << std::endl;
    ecs_man.ForEach<BasicCharacter_t>(basic_character_printer);
    ecs_man.Destroy(ren_e);
    return 0;
}
