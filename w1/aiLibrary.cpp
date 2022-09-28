#include <flecs.h>
#include "ecsTypes.h"
#include "aiLibrary.h"
#include "FunUtilities.h"
#include <bx/rng.h>

#include <utility>
#include <type_traits>
#include <memory>
#include <cstdint>
#include <iostream>
#include <format>
#include <limits>

static bx::RngShr3 rng;

class AttackEnemyState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &/*ecs*/, flecs::entity /*entity*/) override {}
};

template<typename T>
T sqr(T a){ return a*a; }

template<typename T, typename U>
static float dist_sq(const T &lhs, const U &rhs) { return float(sqr(lhs.x - rhs.x) + sqr(lhs.y - rhs.y)); }

template<typename T, typename U>
static float dist(const T &lhs, const U &rhs) { return sqrtf(dist_sq(lhs, rhs)); }

template<typename T, typename U>
static Actions move_towards(const T &from, const U &to)
{
  int deltaX = to.x - from.x;
  int deltaY = to.y - from.y;
  if (abs(deltaX) > abs(deltaY))
    return deltaX > 0 ? Actions::MOVE_RIGHT : Actions::MOVE_LEFT;
  return deltaY > 0 ? Actions::MOVE_UP : Actions::MOVE_DOWN;
}

static Actions inverse_move(Actions move)
{
  return move == Actions::MOVE_LEFT  ? Actions::MOVE_RIGHT :
         move == Actions::MOVE_RIGHT ? Actions::MOVE_LEFT :
         move == Actions::MOVE_UP    ? Actions::MOVE_DOWN :
         move == Actions::MOVE_DOWN  ? Actions::MOVE_UP : move;
}


template<typename Callable>
static void on_closest_enemy_pos(flecs::world &ecs, flecs::entity entity, Callable c)
{
  static auto enemiesQuery = ecs.query<const Position, const Team>();
  entity.set([&](const Position &pos, const Team &t, Action &a)
  {
    flecs::entity closestEnemy;
    float closestDist = std::numeric_limits<float>::max();
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
    if (ecs.is_valid(closestEnemy))
      c(a, pos, closestPos);
  });
}

template<typename Callable>
static void on_closest_market_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto marketQuery = ecs.query<const Position, const Market>();
    entity.set([&](const Position& pos, Action& a)
        {
            flecs::entity closestMarket;
            Position closestPos;
            float closestDist = std::numeric_limits<float>::max();
            marketQuery.each([&](flecs::entity market_ent, const Position& mpos, const Market& market)
            {
                float curDist = dist(mpos, pos);
                if (curDist < closestDist)
                {
                    closestDist = curDist;
                    closestPos = mpos;
                    closestMarket = market_ent;
                }
            });

            if (ecs.is_valid(closestMarket))
                c(a, pos, closestPos);
        });
}

template<typename Callable>
static void on_closest_sleep_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto sleepQuery = ecs.query<const Position, const Sleep>();
    entity.set([&](const Position& pos, Action& a)
        {
            flecs::entity closestSleep;
            Position closestPos;
            float closestDist = std::numeric_limits<float>::max();
            sleepQuery.each([&](flecs::entity sleep_ent, const Position& mpos, const Sleep& sleep)
                {
                    float curDist = dist(mpos, pos);
                    if (curDist < closestDist)
                    {
                        closestDist = curDist;
                        closestPos = mpos;
                        closestSleep = sleep_ent;
                    }
                });

            if (ecs.is_valid(closestSleep))
                c(a, pos, closestPos);
        });
}

template<typename Callable>
static void on_closest_craft_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto craftQuery = ecs.query<const Position, const Craft>();
    entity.set([&](const Position& pos, Action& a)
        {
            flecs::entity closestCraft;
            Position closestPos;
            float closestDist = std::numeric_limits<float>::max();
            craftQuery.each([&](flecs::entity craft_ent, const Position& mpos, const Craft& craft)
                {
                    float curDist = dist(mpos, pos);
                    if (curDist < closestDist)
                    {
                        closestDist = curDist;
                        closestPos = mpos;
                        closestCraft = craft_ent;
                    }
                });

            if (ecs.is_valid(closestCraft))
                c(a, pos, closestPos);
        });
}

template<typename Callable>
static void on_closest_eat_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto eatQuery = ecs.query<const Position, const Eat>();
    entity.set([&](const Position& pos, Action& a)
        {
            flecs::entity closestEat;
            Position closestPos;
            float closestDist = std::numeric_limits<float>::max();
            eatQuery.each([&](flecs::entity eat_ent, const Position& mpos, const Eat& eat)
                {
                    float curDist = dist(mpos, pos);
                    if (curDist < closestDist)
                    {
                        closestDist = curDist;
                        closestPos = mpos;
                        closestEat = eat_ent;
                    }
                });

            if (ecs.is_valid(closestEat))
                c(a, pos, closestPos);
        });
}


class MoveToEnemyState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) override
  {
    on_closest_enemy_pos(ecs, entity, [&](Action &a, const Position &pos, const Position &enemy_pos)
    {
      bgfx::dbgTextPrintf(0, 4, 0x0f, "MoveToEnemy");
      a.action = move_towards(pos, enemy_pos);
    });
  }
};

class MoveToEatState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        on_closest_eat_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
            {
                bgfx::dbgTextPrintf(0, 4, 0x0f, "MoveToEat");
                a.action = move_towards(pos, enemy_pos);
            });
    }
};

class MoveToMarketState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        on_closest_market_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
            {
                bgfx::dbgTextPrintf(0, 4, 0x0f, "MoveToMarket");
                a.action = move_towards(pos, enemy_pos);
            });
    }
};

class MoveToSleepState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        on_closest_sleep_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
            {
                bgfx::dbgTextPrintf(0, 4, 0x0f, "MoveToSleep");
                a.action = move_towards(pos, enemy_pos);
            });
    }
};

class MoveToCraftState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        on_closest_craft_pos(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
            {
                bgfx::dbgTextPrintf(0, 4, 0x0f, "MoveToCraft");
                a.action = move_towards(pos, enemy_pos);
            });
    }
};

class FleeFromEnemyState : public State
{
public:
  FleeFromEnemyState() {}
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) override
  {
    on_closest_enemy_pos(ecs, entity, [&](Action &a, const Position &pos, const Position &enemy_pos)
    {
      a.action = inverse_move(move_towards(pos, enemy_pos));
    });
  }
};

class PatrolState : public State
{
public:
  PatrolState(float dist) : patrolDist(dist) {}
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) override
  {
    entity.set([&](const Position &pos, const PatrolPos &ppos, Action &a)
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
  }
private:
    float patrolDist;
};

class SelfhealState : public State 
{
public:
    SelfhealState(float heal_amt) : healAmt(heal_amt) {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
            {
                bgfx::dbgTextPrintf(0, 4, 0x0f, "hp: %d", (int)hp.hitpoints);
                hp.hitpoints += heal.amount;
                std::cout << hp.hitpoints;
            });
    }

private:
    float healAmt{};
};

class CraftState : public State
{
public:
    CraftState(){}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
            {
                bgfx::dbgTextPrintf(0, 5, 0x0f, "Crafting\n");
            });
    }
};

class EatState : public State
{
public:
    EatState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
            {
                bgfx::dbgTextPrintf(0, 5, 0x0f, "Eating\n");
            });
    }
};

class MarketState : public State
{
public:
    MarketState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
            {
                bgfx::dbgTextPrintf(0, 5, 0x0f, "Marketing\n");
            });
    }
};

class SleepState : public State
{
public:
    SleepState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        entity.set([&](const SelfhealAmount& heal, Hitpoints& hp, Action& a)
            {
                bgfx::dbgTextPrintf(0, 5, 0x0f, "Sleeping\n");
            });
    }
};

class NopState : public State
{
public:
  void enter() const override {}
  void exit() const override {}
  void act(float/* dt*/, flecs::world &ecs, flecs::entity entity) override {}
};

class JumpPressedTransition : public StateTransition
{
public:
    JumpPressedTransition() {};
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        static auto inputQuery = ecs.query<const PlayerInput>();
        bool jump = false;
        inputQuery.each([&](flecs::entity enemy, PlayerInput input)
            {
                jump |= input.up;
            });
        return jump;
    };
};

class ActPressedTransition : public StateTransition
{
public:
    ActPressedTransition() {};
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        static auto inputQuery = ecs.query<const PlayerInput>();
        bool act = false;
        inputQuery.each([&](flecs::entity enemy, PlayerInput input)
            {
                act |= input.act;
            });
        return act;
    };
};


class EnemyAvailableTransition : public StateTransition
{
public:
  EnemyAvailableTransition(float in_dist) : triggerDist(in_dist) {}
  
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    bool enemiesFound = false;
    entity.get([&](const Position &pos, const Team &t)
    {
      enemiesQuery.each([&](flecs::entity enemy, const Position &epos, const Team &et)
      {
        if (t.team == et.team)
          return;
        float curDist = dist(epos, pos);
        enemiesFound |= curDist <= triggerDist;
      });
    });
    return enemiesFound;
  }

private:
    float triggerDist;
};

class HitpointsLessThanTransition : public StateTransition
{
public:
  HitpointsLessThanTransition(float in_thres) : threshold(in_thres) {}
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    bool hitpointsThresholdReached = false;
    entity.get([&](const Hitpoints &hp)
    {
      hitpointsThresholdReached |= hp.hitpoints < threshold;
    });
    return hitpointsThresholdReached;
  }
private:
    float threshold;
};

class YesTransition : public StateTransition
{
public:
    YesTransition() {}
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        return true;
    }
};

class EnemyReachableTransition : public StateTransition
{
public:
  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return false;
  }
};

class NegateTransition : public StateTransition
{
    std::unique_ptr<StateTransition> transition;
public:
  NegateTransition(std::unique_ptr<StateTransition> in_trans) : transition(std::move(in_trans)) {}
  ~NegateTransition() override {}

  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return !transition->isAvailable(ecs, entity);
  }
};

class AndTransition : public StateTransition
{
  std::unique_ptr<StateTransition> lhs;
  std::unique_ptr<StateTransition> rhs;
public:
  AndTransition(std::unique_ptr<StateTransition> in_lhs, std::unique_ptr<StateTransition> in_rhs) : lhs(std::move(in_lhs)), rhs(std::move(in_rhs)) {}
  ~AndTransition() override {}

  bool isAvailable(flecs::world &ecs, flecs::entity entity) const override
  {
    return lhs->isAvailable(ecs, entity) && rhs->isAvailable(ecs, entity);
  }
};

class OrTransition : public StateTransition
{
    std::unique_ptr<StateTransition> lhs;
    std::unique_ptr<StateTransition> rhs;
public:
    OrTransition(std::unique_ptr<StateTransition> in_lhs, std::unique_ptr<StateTransition> in_rhs) : lhs(std::move(in_lhs)), rhs(std::move(in_rhs)) {}
    ~OrTransition() override {}

    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        return lhs->isAvailable(ecs, entity) || rhs->isAvailable(ecs, entity);
    }
};



// states
std::unique_ptr<State> create_attack_enemy_state()
{
  return std::make_unique<AttackEnemyState>();
}
std::unique_ptr<State> create_move_to_enemy_state()
{
  return std::make_unique<MoveToEnemyState>();
}

std::unique_ptr<State> create_move_to_sleep_state()
{
    return std::make_unique<MoveToSleepState>();
}

std::unique_ptr<State> create_move_to_craft_state()
{
    return std::make_unique<MoveToCraftState>();
}

std::unique_ptr<State> create_move_to_market_state()
{
    return std::make_unique<MoveToMarketState>();
}

std::unique_ptr<State> create_move_to_eat_state()
{
    return std::make_unique<MoveToEatState>();
}

std::unique_ptr<State> create_eat_state()
{
    return std::make_unique<EatState>();
}

std::unique_ptr<State> create_sleep_state()
{
    return std::make_unique<SleepState>();
}

std::unique_ptr<State> create_craft_state()
{
    return std::make_unique<CraftState>();
}

std::unique_ptr<State> create_market_state()
{
    return std::make_unique<MarketState>();
}



std::unique_ptr<State> create_flee_from_enemy_state()
{
  return std::make_unique<FleeFromEnemyState>();
}

std::unique_ptr<State> create_selfheal_state(float heal_amt) 
{
    return std::make_unique<SelfhealState>(heal_amt);
}

std::unique_ptr<State> create_patrol_state(float patrol_dist)
{
  return std::make_unique<PatrolState>(patrol_dist);
}

std::unique_ptr<State> create_nop_state()
{
  return std::make_unique<NopState>();
}

std::unique_ptr<StateMachine> create_inner_state_machine()
{
  return std::make_unique<StateMachine>();
}

// transitions
std::unique_ptr<StateTransition> create_enemy_available_transition(float dist)
{
  return std::make_unique<EnemyAvailableTransition>(dist);
}

std::unique_ptr<StateTransition> create_enemy_reachable_transition()
{
  return std::make_unique<EnemyReachableTransition>();
}

std::unique_ptr<StateTransition> create_hitpoints_less_than_transition(float thres)
{
  return std::make_unique<HitpointsLessThanTransition>(thres);
}

std::unique_ptr<StateTransition> create_yes_transition(float thres)
{
    return std::make_unique<YesTransition>();
}

std::unique_ptr<StateTransition> create_jump_pressed_transition()
{
    return std::make_unique<JumpPressedTransition>();
}

std::unique_ptr<StateTransition> create_act_pressed_transition()
{
    return std::make_unique<ActPressedTransition>();
}

 
std::unique_ptr<StateTransition> create_negate_transition(std::unique_ptr<StateTransition> in)
{
  return std::make_unique<NegateTransition>(std::move(in));
}
std::unique_ptr<StateTransition> create_and_transition(std::unique_ptr<StateTransition> lhs, std::unique_ptr<StateTransition> rhs)
{
  return std::make_unique<AndTransition>(std::move(lhs), std::move(rhs));
}
std::unique_ptr<StateTransition> create_or_transition(std::unique_ptr<StateTransition> lhs, std::unique_ptr<StateTransition> rhs)
{
    return std::make_unique<OrTransition>(std::move(lhs), std::move(rhs));
}
