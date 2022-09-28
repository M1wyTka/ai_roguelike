#pragma once
#include <cstdint>
#include "GameSettings.h"

struct Position;
struct MovePos;

struct MovePos
{
  int x = 0;
  int y = 0;

  MovePos &operator=(const Position &rhs);
};

struct Position
{
  int x = 0;
  int y = 0;

  Position &operator=(const MovePos &rhs);
};

inline Position &Position::operator=(const MovePos &rhs)
{
  x = rhs.x;
  y = rhs.y;
  return *this;
}

inline MovePos &MovePos::operator=(const Position &rhs)
{
  x = rhs.x;
  y = rhs.y;
  return *this;
}

inline bool operator==(const Position &lhs, const Position &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const Position &lhs, const MovePos &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const MovePos &lhs, const MovePos &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator==(const MovePos &lhs, const Position &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }


struct PatrolPos
{
  int x = 0;
  int y = 0;
};

struct Sleep {};

struct Eat{};

struct Craft {};

struct Market {};

struct Hitpoints
{
  float hitpoints = 10.f;
};

enum class Actions : uint32_t
{
  NOP = 0,
  JUMP,
  ACT,
  MOVE_START,
  MOVE_LEFT = MOVE_START,
  MOVE_RIGHT,
  MOVE_DOWN,
  MOVE_UP,
  MOVE_END,
  ATTACK = MOVE_END,
  NUM
};

struct Action
{
  Actions action = Actions::NOP;
};

struct NumActions
{
  int numActions = 1;
  int curActions = 0;
};

struct MeleeDamage
{
  float damage = 2.f;
};

struct HealAmount
{
  float amount = 0.f;
};

struct SelfhealAmount
{
  float amount = 5.f;
};


struct PowerupAmount
{
  float amount = 0.f;
};

struct PlayerInput
{
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  bool jump = false;
  bool act = false;
};

struct Symbol
{
  char symb;
};

struct Color
{
  Colors color;
};

struct IsPlayer {};

struct Team
{
  int team = 0;
};

