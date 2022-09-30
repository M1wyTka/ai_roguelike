#include "StateTransitions.h"
#include "MathUtilities.h"


bool JumpPressedTransition::isAvailable(flecs::world& ecs, flecs::entity entity) const
{
    static auto inputQuery = ecs.query<const PlayerInput>();
    bool jump = false;
    inputQuery.each([&](flecs::entity enemy, PlayerInput input)
        {
            jump |= input.jump;
        });
    return jump;
};

bool ActPressedTransition::isAvailable(flecs::world& ecs, flecs::entity entity) const
{
    static auto inputQuery = ecs.query<const PlayerInput>();
    bool act = false;
    inputQuery.each([&](flecs::entity enemy, PlayerInput input)
        {
            act |= input.act;
        });
    return act;
};

bool MehPressedTransition::isAvailable(flecs::world& ecs, flecs::entity entity) const
{
    static auto inputQuery = ecs.query<const PlayerInput>();
    bool meh = false;
    inputQuery.each([&](flecs::entity enemy, PlayerInput input)
        {
            meh |= input.meh;
        });
    return meh;
};

bool EnemyAvailableTransition::isAvailable(flecs::world& ecs, flecs::entity entity) const
{
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    bool enemiesFound = false;
    entity.get([&](const Position& pos, const Team& t)
        {
            enemiesQuery.each([&](flecs::entity enemy, const Position& epos, const Team& et)
                {
                    if (t.team == et.team)
                        return;
                    float curDist = dist(epos, pos);
                    enemiesFound |= curDist <= triggerDist;
                });
        });
    return enemiesFound;
};

bool HitpointsLessThanTransition::isAvailable(flecs::world& ecs, flecs::entity entity) const
{
    bool hitpointsThresholdReached = false;
    entity.get([&](const Hitpoints& hp)
        {
            hitpointsThresholdReached |= hp.hitpoints < threshold;
        });
    return hitpointsThresholdReached;
};

std::unique_ptr<StateTransition> create_negate_transition(std::unique_ptr<StateTransition> in)
{
    return std::make_unique<NegateTransition>(std::move(in));
}
