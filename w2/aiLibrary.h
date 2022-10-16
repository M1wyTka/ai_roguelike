#pragma once

#include "stateMachine.h"
#include "behaviourTree.h"
#include "aiUtils.h"

struct CompoundNode : public BehNode
{
    std::vector<std::unique_ptr<BehNode>> nodes;

    template<std::derived_from<BehNode> ... Args>
    CompoundNode(std::unique_ptr<Args>... args)
    {
        (nodes.push_back(std::move(args)), ...);
    };

    virtual ~CompoundNode() {}

    CompoundNode& pushNode(BehNode* node)
    { 
        nodes.emplace_back(node);
        return *this;
    }
};

struct SequenceNode : public CompoundNode
{
    using CompoundNode::CompoundNode;
    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};

struct SelectorNode : public CompoundNode
{
    using CompoundNode::CompoundNode;
    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};

struct OrNode : public CompoundNode
{
    using CompoundNode::CompoundNode;
    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};

struct NotNode : public BehNode
{
    std::unique_ptr<BehNode> anti_node;

    template<std::derived_from<BehNode> DerivedNode>
    NotNode(std::unique_ptr<DerivedNode>&& node) : anti_node(std::move(node)) {};

    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};

struct ParallelNode: public CompoundNode
{
    using CompoundNode::CompoundNode;
    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};


struct MoveToEntityNode : public BehNode
{
    size_t entityBb = size_t(-1); // wraps to 0xff...
    MoveToEntityNode(flecs::entity entity, const char* bb_name)
    {
        entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
    }

    BehResult update(flecs::world&, flecs::entity entity, Blackboard& bb) override;
};

struct IsLowHpNode : public BehNode
{
    float threshold = 0.f;
    IsLowHpNode(float thres) : threshold(thres) {}

    BehResult update(flecs::world&, flecs::entity entity, Blackboard&) override;
};

struct FindEnemyNode : public BehNode
{
    size_t entityBb = size_t(-1);
    float distance = 0;
    FindEnemyNode(flecs::entity entity, float in_dist, const char* bb_name) : distance(in_dist)
    {
        entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
    }
    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};

struct FleeNode : public BehNode
{
    size_t entityBb = size_t(-1);
    FleeNode(flecs::entity entity, const char* bb_name)
    {
        entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
    }

    BehResult update(flecs::world&, flecs::entity entity, Blackboard& bb) override;
};

struct PatrolNode : public BehNode
{
    size_t pposBb = size_t(-1);
    float patrolDist = 1.f;
    PatrolNode(flecs::entity entity, float patrol_dist, const char* bb_name)
        : patrolDist(patrol_dist)
    {
        pposBb = reg_entity_blackboard_var<Position>(entity, bb_name);
        entity.set([&](Blackboard& bb, const Position& pos)
            {
                bb.set<Position>(pposBb, pos);
            });
    }

    BehResult update(flecs::world&, flecs::entity entity, Blackboard& bb) override;
};

template<typename Tag>
struct FindTagNode : public BehNode
{
    size_t entityBb = size_t(-1);
    float distance = 0;

    FindTagNode(flecs::entity entity, float in_dist, const char* bb_name) : distance(in_dist)
    {
        entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
    }
    
    BehResult on_closest_tag_pos(flecs::world& ecs, flecs::entity entity, Blackboard& bb)
    {
        BehResult res = BehResult::FAIL;
        static auto enemiesQuery = ecs.query<const Position, const Tag>();
        entity.set([&](const Position& pos, Action& a)
            {
                flecs::entity closestEntity;
                Position closestPos;
                float closestDist = std::numeric_limits<float>::max();
                enemiesQuery.each([&](flecs::entity ent, const Position& tr_pos, const Tag& tt)
                    {
                        float curDist = dist(tr_pos, pos);
                        if (curDist < closestDist)
                        {
                            closestDist = curDist;
                            closestPos = tr_pos;
                            closestEntity = ent;
                        }
                    });
                if (ecs.is_valid(closestEntity))
                {
                    bb.set<flecs::entity>(entityBb, closestEntity);
                    res = BehResult::SUCCESS;
                }
            });
        return res;
    };

    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override 
    {
        return on_closest_tag_pos(ecs, entity, bb);
    }
};


struct FindWaypointNode : public BehNode
{
    size_t entityBb = size_t(-1);
    FindWaypointNode(flecs::entity entity, const char* bb_name)
    {
        entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
    }

    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override;
};