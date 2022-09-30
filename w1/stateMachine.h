#pragma once
#include "States.h"
#include "StateTransitions.h"

#include <vector>
#include <flecs.h>
#include <memory>

class StateMachine
{
public:
  StateMachine() = default;
  StateMachine(const StateMachine &sm) = delete;
  StateMachine(StateMachine &&sm) = default;

  ~StateMachine();

  StateMachine& operator=(const StateMachine &sm) = delete;
  StateMachine& operator=(StateMachine &&sm) = default;

  void enter() const {};
  void exit() const {};

  void act(float dt, flecs::world &ecs, flecs::entity entity);

  size_t addState(std::unique_ptr<State> st);
  void addTransition(std::unique_ptr<StateTransition> trans, size_t from, size_t to);

private:
	size_t curStateIdx = 0;
	std::vector<std::unique_ptr<State>> states;
	std::vector<std::vector<std::pair<std::unique_ptr<StateTransition>, size_t>>> transitions{};
};

class StateMachineState : public State
{
	std::unique_ptr<StateMachine> m_state_machine;
public:
	StateMachineState(std::unique_ptr<StateMachine>&& sm) : m_state_machine(std::move(sm)) {}
	void enter() const override {}
	void exit() const override {}
	void act(float/* dt*/, flecs::world& ecs, flecs::entity entity) override {
		m_state_machine->act(0.f, ecs, entity);
	}
};