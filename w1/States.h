#pragma once
#include "flecs.h"
#include "ecsTypes.h"

#include <memory>
#include <iostream>
#include <format>

class State
{
public:
    template<std::derived_from<State> DerivedState, typename ... Args>
    static std::unique_ptr<State> make(Args&&... args)
    {
        return std::make_unique<DerivedState>(std::forward<Args>(args)...);
    }

	virtual ~State() = default;
	virtual void enter() const = 0;
	virtual void exit() const = 0;
	virtual void act(float dt, flecs::world& ecs, flecs::entity entity) = 0;
};

template<typename T, typename U>
static Actions move_towards(const T& from, const U& to)
{
    int deltaX = to.x - from.x;
    int deltaY = to.y - from.y;
    if (abs(deltaX) > abs(deltaY))
        return deltaX > 0 ? Actions::MOVE_RIGHT : Actions::MOVE_LEFT;
    return deltaY > 0 ? Actions::MOVE_UP : Actions::MOVE_DOWN;
}

static Actions inverse_move(Actions move)
{
    return move == Actions::MOVE_LEFT ? Actions::MOVE_RIGHT :
        move == Actions::MOVE_RIGHT ? Actions::MOVE_LEFT :
        move == Actions::MOVE_UP ? Actions::MOVE_DOWN :
        move == Actions::MOVE_DOWN ? Actions::MOVE_UP : move;
}

template<typename TargetTag, std::invocable<Action&, Position, Position> Callable>
static void on_closest_tag_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto enemiesQuery = ecs.query<const Position, const TargetTag>();
    entity.set([&](const Position& pos, Action& a)
        {
            flecs::entity closestEntity;
            Position closestPos;
            float closestDist = std::numeric_limits<float>::max();
            enemiesQuery.each([&](flecs::entity ent, const Position& tr_pos, const TargetTag& tt)
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
                c(a, pos, closestPos);
        });
};

template<typename Callable>
static void on_closest_enemy_pos(flecs::world& ecs, flecs::entity entity, Callable c)
{
    static auto enemiesQuery = ecs.query<const Position, const Team>();
    entity.set([&](const Position& pos, const Team& t, Action& a)
        {
            flecs::entity closestEnemy;
            float closestDist = std::numeric_limits<float>::max();
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
            if (ecs.is_valid(closestEnemy))
                c(a, pos, closestPos);
        });
}


template<typename TargetTag>
class MoveToTargetState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override
    {
        on_closest_tag_pos<TargetTag>(ecs, entity, [&](Action& a, const Position& pos, const Position& enemy_pos)
            {
                std::cout << std::format("Moving to {}\n", typeid(TargetTag).name());
                a.action = move_towards(pos, enemy_pos);
            });
    }
};

template<typename TargetTag>
std::unique_ptr<State> create_move_to_target_state()
{
	return std::make_unique<MoveToTargetState<TargetTag>>();
};


class AttackEnemyState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world&/*ecs*/, flecs::entity /*entity*/) override {}
};


class MoveToEnemyState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class FleeFromEnemyState : public State
{
public:
    FleeFromEnemyState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class PatrolState : public State
{
public:
    PatrolState(float dist) : patrolDist(dist) {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
private:
    float patrolDist;
};

class SelfhealState : public State
{
public:
    SelfhealState(float heal_amt) : healAmt(heal_amt) {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;

private:
    float healAmt{};
};

class CraftState : public State
{
public:
    CraftState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class EatState : public State
{
public:
    EatState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class MarketState : public State
{
public:
    MarketState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class SleepState : public State
{
public:
    SleepState() {}
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override;
};

class NopState : public State
{
public:
    void enter() const override {}
    void exit() const override {}
    void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override {}
};

