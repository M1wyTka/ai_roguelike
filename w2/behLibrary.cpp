#include "aiLibrary.h"
#include "ecsTypes.h"
#include "aiUtils.h"
#include "math.h"
#include "raylib.h"
#include "blackboard.h"


BehResult SequenceNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    for (auto& node : nodes)
    {
        BehResult res = node->update(ecs, entity, bb);
        if (res != BehResult::SUCCESS)
            return res;
    }
    return BehResult::SUCCESS;
};

BehResult SelectorNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    for (auto& node : nodes)
    {
        BehResult res = node->update(ecs, entity, bb);
        if (res != BehResult::FAIL)
            return res;
    }
    return BehResult::FAIL;
};

BehResult OrNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    BehResult res = BehResult::FAIL;
    for (auto& node : nodes)
    {
        auto temp = node->update(ecs, entity, bb);
        if (temp == BehResult::SUCCESS)
            return BehResult::SUCCESS;
        if (temp == BehResult::RUNNING)
            res = BehResult::RUNNING;
    }
    return res;
}

BehResult NotNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    switch (anti_node->update(ecs, entity, bb))
    {
    case BehResult::FAIL:
        return BehResult::SUCCESS;
    case BehResult::SUCCESS:
        return BehResult::FAIL;
    case BehResult::RUNNING:
    default:
        return BehResult::RUNNING;
    }
}

BehResult ParallelNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    bool failed = false;
    bool at_least_one_runs = false;
    for (auto& node : nodes)
    {
        auto temp = node->update(ecs, entity, bb);
        if (temp == BehResult::RUNNING)
            at_least_one_runs |= true;
        else if (temp == BehResult::FAIL)
            failed |= true; 
    }
    return failed ? BehResult::FAIL : at_least_one_runs ? BehResult::RUNNING : BehResult::SUCCESS;
}

BehResult MoveToEntityNode::update(flecs::world&, flecs::entity entity, Blackboard& bb)
{
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action& a, const Position& pos)
        {
            flecs::entity targetEntity = bb.get<flecs::entity>(entityBb);
            if (!targetEntity.is_alive())
            {
                res = BehResult::FAIL;
                return;
            }
            targetEntity.get([&](const Position& target_pos)
                {
                    if (pos != target_pos)
                    {
                        a.action = move_towards(pos, target_pos);
                        res = BehResult::RUNNING;
                    }
                    else
                        res = BehResult::SUCCESS;
                });
        });
    return res;
};

BehResult IsLowHpNode::update(flecs::world&, flecs::entity entity, Blackboard&)
{
    BehResult res = BehResult::SUCCESS;
    entity.get([&](const Hitpoints& hp)
        {
            res = hp.hitpoints < threshold ? BehResult::SUCCESS : BehResult::FAIL;
        });
    return res;
};

BehResult FindEnemyNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    BehResult res = BehResult::FAIL;
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    entity.set([&](const Position& pos, const Team& t)
        {
            flecs::entity closestEnemy;
            float closestDist = FLT_MAX;
            Position closestPos;
            enemiesQuery.each([&](flecs::entity enemy, const Position& epos, const Team& et)
                {
                    if (t.team == et.team)
                        return;
                    float curDist = dist(epos, pos);
                    if (curDist < closestDist)
                    {
                        closestDist = curDist;
                        closestPos = epos;
                        closestEnemy = enemy;
                    }
                });
            if (ecs.is_valid(closestEnemy) && closestDist <= distance)
            {
                bb.set<flecs::entity>(entityBb, closestEnemy);
                res = BehResult::SUCCESS;
            }
        });
    return res;
};


BehResult FleeNode::update(flecs::world&, flecs::entity entity, Blackboard& bb)
{
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action& a, const Position& pos)
        {
            flecs::entity targetEntity = bb.get<flecs::entity>(entityBb);
            if (!targetEntity.is_alive())
            {
                res = BehResult::FAIL;
                return;
            }
            targetEntity.get([&](const Position& target_pos)
                {
                    a.action = inverse_move(move_towards(pos, target_pos));
                });
        });
    return res;
};

BehResult PatrolNode::update(flecs::world&, flecs::entity entity, Blackboard& bb)
{
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action& a, const Position& pos)
        {
            Position patrolPos = bb.get<Position>(pposBb);
            if (dist(pos, patrolPos) > patrolDist)
                a.action = move_towards(pos, patrolPos);
            else
            {
                constexpr auto begin = to_underlying(Actions::MOVE_START);
                constexpr auto end = to_underlying(Actions::MOVE_END);
                a.action = Actions(begin + (std::rand() % (end - begin)));
            }
        });
    return res;
};

BehResult FindWaypointNode::update(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
{
    auto res = BehResult::FAIL;
    entity.set([&](const Position& pos, CurrentWaypoint& waypoint)
        {
            waypoint.ent.get([&](const Position& wpos, const NextWaypoint& next_waypoint) {
                if (pos == wpos) {
                    waypoint.ent = next_waypoint.ent;
                }
            });
            if (ecs.is_valid(waypoint.ent)) {
                res = BehResult::SUCCESS;
                bb.set<flecs::entity>(entityBb, waypoint.ent);
            }
        });
    return res;
}