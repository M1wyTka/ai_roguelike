#include "stateMachine.h"

StateMachine::~StateMachine()
{
}

void StateMachine::act(float dt, flecs::world &ecs, flecs::entity entity)
{
  if (curStateIdx < states.size())
  {
      for (const auto& [transition, ind] : transitions[curStateIdx])
      {
          if (transition->isAvailable(ecs, entity))
          {
              states[curStateIdx]->exit();
              curStateIdx = ind;
              states[curStateIdx]->enter();
              break;
          }
      }
    states[curStateIdx]->act(dt, ecs, entity);
  }
  else
    curStateIdx = 0;
}

size_t StateMachine::addState(std::unique_ptr<State> st)
{
  size_t idx = states.size();
  states.push_back(std::move(st));
  transitions.push_back(std::vector<std::pair<std::unique_ptr<StateTransition>, size_t>>());
  return idx;
}

void StateMachine::addTransition(std::unique_ptr<StateTransition> trans, size_t from, size_t to)
{
  transitions[from].push_back(std::make_pair(std::move(trans), to));
}

