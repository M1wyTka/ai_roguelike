#pragma once
#include <vector>
#include <flecs.h>
#include <memory>

class State
{
public:
	virtual ~State() = default;
	virtual void enter() const = 0;
	virtual void exit() const = 0;
	virtual void act(float dt, flecs::world &ecs, flecs::entity entity) = 0;
};

class StateTransition
{
public:
  virtual ~StateTransition() {}
  virtual bool isAvailable(flecs::world &ecs, flecs::entity entity) const = 0;
};

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

  int addState(std::unique_ptr<State> st);
  void addTransition(std::unique_ptr<StateTransition> trans, int from, int to);

private:
	int curStateIdx = 0;
	std::vector<std::unique_ptr<State>> states;
	std::vector<std::vector<std::pair<std::unique_ptr<StateTransition>, int>>> transitions{};
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