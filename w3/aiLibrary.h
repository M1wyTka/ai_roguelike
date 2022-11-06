#pragma once

#include <functional>
#include "stateMachine.h"
#include "behaviourTree.h"
#include "math.h"
#include "aiUtils.h"
#include <iostream>
#include <format>

// states
State *create_attack_enemy_state();
State *create_move_to_enemy_state();
State *create_flee_from_enemy_state();
State *create_patrol_state(float patrol_dist);
State *create_nop_state();

// transitions
StateTransition *create_enemy_available_transition(float dist);
StateTransition *create_enemy_reachable_transition();
StateTransition *create_hitpoints_less_than_transition(float thres);
StateTransition *create_negate_transition(StateTransition *in);
StateTransition *create_and_transition(StateTransition *lhs, StateTransition *rhs);

using utility_function = std::function<float(Blackboard&)>;

BehNode *sequence(const std::vector<BehNode*> &nodes);
BehNode *selector(const std::vector<BehNode*> &nodes);
BehNode *utility_selector(std::vector<std::pair<BehNode*, utility_function>>&& nodes);
BehNode *weighted_selector(std::vector<std::pair<BehNode*, utility_function>>&& nodes);

BehNode *move_to_entity(flecs::entity entity, const char *bb_name);
BehNode *is_low_hp(float thres);
BehNode *find_enemy(flecs::entity entity, float dist, const char *bb_name);
BehNode *flee(flecs::entity entity, const char *bb_name);
BehNode *patrol(flecs::entity entity, float patrol_dist, const char *bb_name);
BehNode *patch_up(float thres);

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
        std::cout << std::format("Looking for {0}\n", typeid(Tag).name());
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

template<typename T>
BehNode* find_tag(flecs::entity entity, float dist, const char* bb_name)
{ 
    return new FindTagNode<T>(entity, dist, bb_name);
};
