#include "States.h"
#include "FunUtilities.h"
#include "MathUtilities.h"

#include <bx/rng.h>

static bx::RngShr3 rng;

void MoveToEnemyState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    on_closest_enemy_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
        {
            std::cout << "MoveToEnemy\n";
            a.action = move_towards(pos, enemy_pos);
        });
}

void FleeFromEnemyState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    on_closest_enemy_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
        {
            a.action = inverse_move(move_towards(pos, enemy_pos));
        });
};

 
void PatrolState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const Position& pos, const PatrolPos& ppos, Action& a)
        {
            if (dist(pos, ppos) > patrolDist) {
                // do a recovery walk
                a.action = move_towards(pos, ppos);
            }
            else
            {
                // do a random walk
                constexpr auto begin = to_underlying(Actions::MOVE_START);
                constexpr auto end = to_underlying(Actions::MOVE_END);
                a.action = Actions(begin + (rng.gen() % (end - begin)));
            }
        });
};

void SelfhealState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
        {
            hp.hitpoints += heal.amount;
            std::cout << hp.hitpoints;
        });
};
 
void CraftState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
        {
            std::cout << "Crafting\n";
        });
};

void EatState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
        {
            std::cout << "Eating\n";
        });
};


void MarketState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
    {
            std::cout << "Marketing\n";
    });
};

void SleepState::act(float/* dt*/, flecs::world& ecs, flecs::entity entity)
{
    entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
        {
            std::cout << "Sleeping\n";
        });
};
