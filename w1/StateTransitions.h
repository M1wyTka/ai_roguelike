#pragma once
#include "flecs.h"
#include "ecsTypes.h"

#include <memory>
#include <utility>
#include <concepts>
#include <vector>

class StateTransition
{
public:
    template<std::derived_from<StateTransition> Transition, typename ... Args>
    static std::unique_ptr<StateTransition> make(Args&&... args) 
    { 
        return std::make_unique<Transition>(std::forward<Args>(args)...);
    }

	virtual ~StateTransition() {}
	virtual bool isAvailable(flecs::world& ecs, flecs::entity entity) const = 0;
};

class JumpPressedTransition : public StateTransition
{
public:
    JumpPressedTransition() {};
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override;
};

class ActPressedTransition : public StateTransition
{
public:
    ActPressedTransition() {};
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override;
};

class MehPressedTransition : public StateTransition
{
public:
    MehPressedTransition() {};
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override;
};

class EnemyAvailableTransition : public StateTransition
{
public:
    EnemyAvailableTransition(float in_dist) : triggerDist(in_dist) {}
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override;

private:
    float triggerDist;
};

class HitpointsLessThanTransition : public StateTransition
{
public:
    HitpointsLessThanTransition(float in_thres) : threshold(in_thres) {}
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override;
private:
    float threshold;
};

class YesTransition : public StateTransition
{
public:
    YesTransition() {}
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override { return true; }
};

class EnemyReachableTransition : public StateTransition
{
public:
    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override { return false; }
};

class OrTransition : public StateTransition
{
public:
    template<std::derived_from<StateTransition> ... Args>
    OrTransition(std::unique_ptr<Args>... args)
    {
        (transitions.push_back(std::move(args)), ...);
    };

    ~OrTransition() override {}

    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        for (auto& trasition : transitions)
        {
            if (trasition->isAvailable(ecs, entity))
                return true;
        }
        return false;
    }

private:
    std::vector<std::unique_ptr<StateTransition>> transitions;
};


class AndTransition : public StateTransition
{
public:
    template<std::derived_from<StateTransition> ... Args>
    AndTransition(std::unique_ptr<Args>... args) 
    {
        (transitions.push_back(std::move(args)), ...);
    };

    ~AndTransition() override {}

    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        for (auto& trasition : transitions)
        {
            if (!trasition->isAvailable(ecs, entity))
                return false;
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<StateTransition>> transitions;
};

class NegateTransition : public StateTransition
{
    std::unique_ptr<StateTransition> transition;
public:
    NegateTransition(std::unique_ptr<StateTransition> in_trans) : transition(std::move(in_trans)) {}
    ~NegateTransition() override {}

    bool isAvailable(flecs::world& ecs, flecs::entity entity) const override
    {
        return !transition->isAvailable(ecs, entity);
    }
};

std::unique_ptr<StateTransition> create_negate_transition(std::unique_ptr<StateTransition> in);


template<std::derived_from<StateTransition> ... Args> requires (sizeof...(Args) >= 2)
std::unique_ptr<StateTransition> create_or_transition(std::unique_ptr<Args>... transitions)
{
    return std::make_unique<OrTransition>(std::move(transitions)...);
}

template<std::derived_from<StateTransition> ... Args> requires (sizeof...(Args) >= 2)
std::unique_ptr<StateTransition> create_and_transition(std::unique_ptr<Args>... transitions)
{
    return std::make_unique<AndTransition>(std::move(transitions)...);
}