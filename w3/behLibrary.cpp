#include "aiLibrary.h"
#include "ecsTypes.h"
#include "aiUtils.h"
#include "math.h"
#include "raylib.h"
#include "blackboard.h"
#include <algorithm>
#include <iostream>
#include <format>
#include <random>

struct CompoundNode : public BehNode
{
  std::vector<BehNode*> nodes;

  virtual ~CompoundNode()
  {
    for (BehNode *node : nodes)
      delete node;
    nodes.clear();
  }

  CompoundNode &pushNode(BehNode *node)
  {
    nodes.push_back(node);
    return *this;
  }
};

struct Sequence : public CompoundNode
{
  BehResult update(flecs::world &ecs, flecs::entity entity, Blackboard &bb) override
  {
    for (BehNode *node : nodes)
    {
      BehResult res = node->update(ecs, entity, bb);
      if (res != BehResult::SUCCESS)
        return res;
    }
    return BehResult::SUCCESS;
  }
};

struct Selector : public CompoundNode
{
  BehResult update(flecs::world &ecs, flecs::entity entity, Blackboard &bb) override
  {
    for (BehNode *node : nodes)
    {
      BehResult res = node->update(ecs, entity, bb);
      if (res != BehResult::FAIL)
        return res;
    }
    return BehResult::FAIL;
  }
};


struct WeightedUtilitySelector : public BehNode
{
    WeightedUtilitySelector(std::vector<std::pair<BehNode*, utility_function>>&& nodes)
        : utilityNodes(std::move(nodes)) {
        heats.resize(utilityNodes.size());
    };

    std::vector<std::pair<BehNode*, utility_function>> utilityNodes;
    std::vector<float> heats;
    static constexpr float max_heat = 200.f;
    static constexpr float heat_step = 25.f;
    size_t prev_heat = -1;

    BehResult update(flecs::world& ecs, flecs::entity entity, Blackboard& bb) override
    {
        std::vector<float> sub_scores;
        
        for(size_t i = 0; i < utilityNodes.size(); i++)
        {
            float total_val = (utilityNodes[i].second(bb) + heats[i]);
            std::cout << std::format("sub_score = {0} ; heat = {1}\n", total_val, heats[i]);

            sub_scores.emplace_back(total_val);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::discrete_distribution<> d(sub_scores.begin(), sub_scores.end());
        
        for (size_t rand_ind = d(gen);; rand_ind = d(gen))
        {
            std::cout << std::format("Rand ind = {0}\n", rand_ind);
            auto res = utilityNodes[rand_ind].first->update(ecs, entity, bb);
            if (res == BehResult::FAIL)
            {
                std::cout << "Failed\n";
                continue;
            }    

            if (prev_heat != rand_ind)
            {
                if (prev_heat != -1)
                    heats[prev_heat] = 0.f;
                prev_heat = rand_ind;
                heats[rand_ind] = max_heat;
            }
            else
                heats[prev_heat] -= heat_step;
            return res;
        }
        return BehResult::FAIL;
    }
};


struct UtilitySelector : public BehNode
{
  std::vector<std::pair<BehNode*, utility_function>> utilityNodes;

  BehResult update(flecs::world &ecs, flecs::entity entity, Blackboard &bb) override
  {
    std::vector<std::pair<float, BehNode*>> utilityScores;
    for (const auto&[node, func] : utilityNodes)
    {
        utilityScores.emplace_back(func(bb), node);
    }
    std::sort(utilityScores.begin(), utilityScores.end(), [](auto &lhs, auto &rhs)
    {
      return lhs.first > rhs.first;
    });
    for (const auto& [result, node] : utilityScores)
    {
        std::cout << std::format("Executing {0}\n", typeid(*node).name());
      BehResult res = node->update(ecs, entity, bb);
      if (res != BehResult::FAIL)
      {
          return res;
      } 
    }
    return BehResult::FAIL;
  }
};

struct MoveToEntity : public BehNode
{
  size_t entityBb = size_t(-1); // wraps to 0xff...
  MoveToEntity(flecs::entity entity, const char *bb_name)
  {
    entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
  }

  BehResult update(flecs::world &, flecs::entity entity, Blackboard &bb) override
  {
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action &a, const Position &pos)
    {
      flecs::entity targetEntity = bb.get<flecs::entity>(entityBb);
      if (!targetEntity.is_alive())
      {
        res = BehResult::FAIL;
        return;
      }
      targetEntity.get([&](const Position &target_pos)
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
  }
};

struct IsLowHp : public BehNode
{
  float threshold = 0.f;
  IsLowHp(float thres) : threshold(thres) {}

  BehResult update(flecs::world &, flecs::entity entity, Blackboard &) override
  {
    BehResult res = BehResult::SUCCESS;
    entity.get([&](const Hitpoints &hp)
    {
      res = hp.hitpoints < threshold ? BehResult::SUCCESS : BehResult::FAIL;
    });
    return res;
  }
};

struct FindEnemy : public BehNode
{
  size_t entityBb = size_t(-1);
  float distance = 0;
  FindEnemy(flecs::entity entity, float in_dist, const char *bb_name) : distance(in_dist)
  {
    entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
  }
  BehResult update(flecs::world &ecs, flecs::entity entity, Blackboard &bb) override
  {
    std::cout << std::format("Looking for enemy\n");
    BehResult res = BehResult::FAIL;
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    entity.set([&](const Position &pos, const Team &t)
    {
      flecs::entity closestEnemy;
      float closestDist = FLT_MAX;
      Position closestPos;
      enemiesQuery.each([&](flecs::entity enemy, const Position &epos, const Team &et)
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
  }
};

struct Flee : public BehNode
{
  size_t entityBb = size_t(-1);
  Flee(flecs::entity entity, const char *bb_name)
  {
    entityBb = reg_entity_blackboard_var<flecs::entity>(entity, bb_name);
  }

  BehResult update(flecs::world &, flecs::entity entity, Blackboard &bb) override
  {
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action &a, const Position &pos)
    {
      flecs::entity targetEntity = bb.get<flecs::entity>(entityBb);
      if (!targetEntity.is_alive())
      {
        res = BehResult::FAIL;
        return;
      }
      targetEntity.get([&](const Position &target_pos)
      {
        a.action = inverse_move(move_towards(pos, target_pos));
      });
    });
    return res;
  }
};

template<typename E>
constexpr auto to_underlying(E e) -> std::underlying_type<E>::type
{
    return static_cast<std::underlying_type<E>::type>(e);
}

struct Patrol : public BehNode
{
  size_t pposBb = size_t(-1);
  float patrolDist = 1.f;
  Patrol(flecs::entity entity, float patrol_dist, const char *bb_name)
    : patrolDist(patrol_dist)
  {
    pposBb = reg_entity_blackboard_var<Position>(entity, bb_name);
    entity.set([&](Blackboard &bb, const Position &pos)
    {
      bb.set<Position>(pposBb, pos);
    });
  }

  BehResult update(flecs::world &, flecs::entity entity, Blackboard &bb) override
  {
    BehResult res = BehResult::RUNNING;
    entity.set([&](Action &a, const Position &pos)
    {
            std::cout << "Patroling\n";
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
  }
};

struct PatchUp : public BehNode
{
  float hpThreshold = 100.f;
  PatchUp(float threshold) : hpThreshold(threshold) {}

  BehResult update(flecs::world &, flecs::entity entity, Blackboard &) override
  {
    BehResult res = BehResult::SUCCESS;
    entity.set([&](Action &a, Hitpoints &hp)
    {
      if (hp.hitpoints >= hpThreshold)
        return;
      res = BehResult::RUNNING;
      a.action = Actions::HEAL_SELF;
    });
    return res;
  }
};



BehNode *sequence(const std::vector<BehNode*> &nodes)
{
  Sequence *seq = new Sequence;
  for (BehNode *node : nodes)
    seq->pushNode(node);
  return seq;
}

BehNode *selector(const std::vector<BehNode*> &nodes)
{
  Selector *sel = new Selector;
  for (BehNode *node : nodes)
    sel->pushNode(node);
  return sel;
}

BehNode *utility_selector(std::vector<std::pair<BehNode*, utility_function>>&& nodes)
{
  UtilitySelector *usel = new UtilitySelector;
  usel->utilityNodes = std::move(nodes);
  return usel;
}

BehNode* weighted_selector(std::vector<std::pair<BehNode*, utility_function>>&& nodes) 
{
    WeightedUtilitySelector* wusel = new WeightedUtilitySelector(std::move(nodes));
    return wusel;
}

BehNode *move_to_entity(flecs::entity entity, const char *bb_name)
{
  return new MoveToEntity(entity, bb_name);
}

BehNode *is_low_hp(float thres)
{
  return new IsLowHp(thres);
}

BehNode *find_enemy(flecs::entity entity, float dist, const char *bb_name)
{
  return new FindEnemy(entity, dist, bb_name);
}

BehNode *flee(flecs::entity entity, const char *bb_name)
{
  return new Flee(entity, bb_name);
}

BehNode *patrol(flecs::entity entity, float patrol_dist, const char *bb_name)
{
  return new Patrol(entity, patrol_dist, bb_name);
}

BehNode *patch_up(float thres)
{
  return new PatchUp(thres);
}


