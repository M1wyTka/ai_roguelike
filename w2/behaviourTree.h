#pragma once

#include <memory>
#include <cstdint>

#include <flecs.h>

#include "blackboard.h"
#include "EnumUtility.h"

enum class BehResult : uint32_t
{
  FAIL = 0,
  SUCCESS,
  RUNNING
};

struct BehNode
{
    template<std::derived_from<BehNode> DerivedBehNode, typename ... Args>
    static std::unique_ptr<BehNode> make(Args&&... args)
    {
        return std::make_unique<DerivedBehNode>(std::forward<Args>(args)...);
    }
  virtual ~BehNode() {}
  virtual BehResult update(flecs::world &ecs, flecs::entity entity, Blackboard &bb) = 0;
};

struct BehaviourTree
{
  std::unique_ptr<BehNode> root = nullptr;

  BehaviourTree() = default;
  BehaviourTree(BehNode *r) : root(r) {}
  BehaviourTree(std::unique_ptr<BehNode>&& r) : root(std::move(r)) {}


  BehaviourTree(const BehaviourTree &bt) = delete;
  BehaviourTree(BehaviourTree &&bt) = default;

  BehaviourTree &operator=(const BehaviourTree &bt) = delete;
  BehaviourTree &operator=(BehaviourTree &&bt) = default;

  ~BehaviourTree() = default;

  void update(flecs::world &ecs, flecs::entity entity, Blackboard &bb)
  {
    root->update(ecs, entity, bb);
  }
};

