#pragma once

#include "stateMachine.h"
#include <memory>

// states
std::unique_ptr<State> create_attack_enemy_state();

std::unique_ptr<State> create_move_to_enemy_state();
std::unique_ptr<State> create_move_to_sleep_state();
std::unique_ptr<State> create_move_to_craft_state();
std::unique_ptr<State> create_move_to_market_state();
std::unique_ptr<State> create_move_to_eat_state();


std::unique_ptr<State> create_flee_from_enemy_state();
std::unique_ptr<State> create_patrol_state(float patrol_dist);

std::unique_ptr<State> create_selfheal_state(float heal_amt);
std::unique_ptr<StateMachine> create_inner_state_machine();

std::unique_ptr<State> create_sleep_state();
std::unique_ptr<State> create_craft_state();
std::unique_ptr<State> create_market_state();
std::unique_ptr<State> create_eat_state();


std::unique_ptr<State> create_nop_state();

// transitions
std::unique_ptr<StateTransition> create_enemy_available_transition(float dist);
std::unique_ptr<StateTransition> create_enemy_reachable_transition();
std::unique_ptr<StateTransition> create_hitpoints_less_than_transition(float thres);
std::unique_ptr<StateTransition> create_yes_transition();
std::unique_ptr<StateTransition> create_jump_pressed_transition();
std::unique_ptr<StateTransition> create_act_pressed_transition();

std::unique_ptr<StateTransition> create_negate_transition(std::unique_ptr<StateTransition> in);
std::unique_ptr<StateTransition> create_and_transition(std::unique_ptr<StateTransition> lhs, std::unique_ptr<StateTransition> rhs);
std::unique_ptr<StateTransition> create_or_transition(std::unique_ptr<StateTransition> lhs, std::unique_ptr<StateTransition> rhs);

