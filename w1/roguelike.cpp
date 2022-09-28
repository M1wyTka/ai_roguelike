#include <debugdraw/debugdraw.h>
//for scancodes
#include <GLFW/glfw3.h>

#include "app.h"
#include "ecsTypes.h"
#include "roguelike.h"
#include "stateMachine.h"
#include "aiLibrary.h"
#include "GameSettings.h"
#include "FunUtilities.h"


static void add_patrol_attack_flee_sm(flecs::entity entity)
{
  entity.get([](StateMachine &sm)
  {
    int patrol = sm.addState(create_patrol_state(3.f));
    int moveToEnemy = sm.addState(create_move_to_enemy_state());
    int fleeFromEnemy = sm.addState(create_flee_from_enemy_state());

    sm.addTransition(create_enemy_available_transition(3.f), patrol, moveToEnemy);
    sm.addTransition(create_negate_transition(create_enemy_available_transition(5.f)), moveToEnemy, patrol);

    sm.addTransition(create_and_transition(create_hitpoints_less_than_transition(60.f), create_enemy_available_transition(5.f)),
                     moveToEnemy, fleeFromEnemy);
    sm.addTransition(create_and_transition(create_hitpoints_less_than_transition(60.f), create_enemy_available_transition(3.f)),
                     patrol, fleeFromEnemy);

    sm.addTransition(create_negate_transition(create_enemy_available_transition(7.f)), fleeFromEnemy, patrol);
  });
}

static void add_berserker_sm(flecs::entity entity) 
{
    entity.get([](StateMachine& sm)
    {
        int patrol = sm.addState(create_patrol_state(3.f));
        int moveToEnemy = sm.addState(create_move_to_enemy_state());

        sm.addTransition(
            create_or_transition(create_enemy_available_transition(3.f), create_hitpoints_less_than_transition(60.f)),
            patrol, moveToEnemy);
        sm.addTransition(create_negate_transition(
                        create_or_transition(create_enemy_available_transition(5.f), create_hitpoints_less_than_transition(60.f)))
                        , moveToEnemy, patrol);
    });
}

std::unique_ptr<State> create_crafter_main() 
{
    auto main_sm = create_inner_state_machine();
    
    int buy = main_sm->addState(create_nop_state());
    int sell = main_sm->addState(create_nop_state());
    int craft = main_sm->addState(create_nop_state());

    int go_buy = main_sm->addState(create_move_to_market_state());
    int go_sell = main_sm->addState(create_move_to_market_state());
    int go_craft = main_sm->addState(create_move_to_craft_state());

    main_sm->addTransition(create_hitpoints_less_than_transition(500.f), buy, go_craft);
    main_sm->addTransition(create_hitpoints_less_than_transition(450.f), go_craft, craft);
    main_sm->addTransition(create_hitpoints_less_than_transition(400.f), craft, go_sell);
    main_sm->addTransition(create_hitpoints_less_than_transition(350.f), go_sell, sell);
    main_sm->addTransition(create_hitpoints_less_than_transition(300.f), sell, go_craft);
    main_sm->addTransition(create_hitpoints_less_than_transition(250.f), sell, go_buy);
    main_sm->addTransition(create_hitpoints_less_than_transition(200.f), go_buy, buy);

    return main_sm;
}

std::unique_ptr<State> create_crafter_eat()
{
    auto eat_sm = create_inner_state_machine();
    int go_to_eat = eat_sm->addState(create_eat_state());
    int eat = eat_sm->addState(create_nop_state());

    eat_sm->addTransition(create_hitpoints_less_than_transition(100.f), go_to_eat, eat);

    return eat_sm;
}

std::unique_ptr<State> create_crafter_sleep()
{
    auto sleep_sm = create_inner_state_machine();
    int go_to_sleep = sleep_sm->addState(create_move_to_sleep_state());
    int sleep = sleep_sm->addState(create_nop_state());

    sleep_sm->addTransition(create_hitpoints_less_than_transition(50.f), go_to_sleep, sleep);

    return sleep_sm;
}

static void add_crafter_sm(flecs::entity entity)
{
    entity.get([](StateMachine& sm)
        {
            int patrol = sm.addState(create_patrol_state(3.f));
            int main_sm = sm.addState(create_crafter_main());
            int sleep_sm = sm.addState(create_crafter_sleep());
            int eat_sm = sm.addState(create_crafter_eat());
            
            sm.addTransition(create_hitpoints_less_than_transition(500.f), main_sm, sleep_sm);
            sm.addTransition(
                create_and_transition(create_enemy_available_transition(1.f), create_jump_pressed_transition())
                , sleep_sm, main_sm);
            
            sm.addTransition(create_jump_pressed_transition(), main_sm, eat_sm);
            sm.addTransition(create_act_pressed_transition(), eat_sm, main_sm);
            sm.addTransition(
                create_and_transition(create_jump_pressed_transition(), create_act_pressed_transition()),
                sleep_sm, eat_sm);

            sm.addTransition(create_negate_transition(create_enemy_available_transition(5.f)), main_sm, patrol);
            sm.addTransition(create_enemy_available_transition(3.f), patrol, main_sm);
        });
}

static void add_selfhealing_sm(flecs::entity entity)
{
    entity.get([](StateMachine& sm)
        {
            int patrol = sm.addState(create_patrol_state(3.f));
            int moveToEnemy = sm.addState(create_move_to_enemy_state());
            int selfheal = sm.addState(create_selfheal_state(5.f));

            sm.addTransition(create_enemy_available_transition(3.f), patrol, moveToEnemy);
            sm.addTransition(create_negate_transition(create_enemy_available_transition(5.f)), moveToEnemy, patrol);

            sm.addTransition(create_hitpoints_less_than_transition(60.0f), moveToEnemy, selfheal);
            sm.addTransition(create_hitpoints_less_than_transition(60.0f), patrol, selfheal);

            sm.addTransition(create_negate_transition(create_hitpoints_less_than_transition(60.0f)), selfheal, patrol);
        });
}

static void add_patrol_flee_sm(flecs::entity entity)
{
  entity.get([](StateMachine &sm)
  {
    int patrol = sm.addState(create_patrol_state(3.f));
    int fleeFromEnemy = sm.addState(create_flee_from_enemy_state());

    sm.addTransition(create_enemy_available_transition(3.f), patrol, fleeFromEnemy);
    sm.addTransition(create_negate_transition(create_enemy_available_transition(5.f)), fleeFromEnemy, patrol);
  });
}

static void add_attack_sm(flecs::entity entity)
{
  entity.get([](StateMachine &sm)
  {
    sm.addState(create_move_to_enemy_state());
  });
}

static flecs::entity create_monster(flecs::world &ecs, int x, int y, Colors color)
{
  return ecs.entity()
    .set(Position{x, y})
    .set(MovePos{x, y})
    .set(PatrolPos{x, y})
    .set(Hitpoints{100.f})
    .set(Action{Actions::NOP})
    .set(Color{color})
    .set(StateMachine{})
    .set(Team{1})
    .set(NumActions{1, 0})
    .set(MeleeDamage{20.f});
}

static void create_player(flecs::world &ecs, int x, int y)
{
  ecs.entity("player")
    .set(Position{x, y})
    .set(MovePos{x, y})
    .set(Hitpoints{100.f})
    .set(Color{Colors::WHITE})
    .set(Action{Actions::NOP})
    .add<IsPlayer>()
    .set(Team{0})
    .set(PlayerInput{})
    .set(NumActions{2, 0})
    .set(MeleeDamage{50.f});
}

static void create_heal(flecs::world &ecs, int x, int y, float amount)
{
  ecs.entity()
    .set(Position{x, y})
    .set(HealAmount{amount})
    .set(Color{Colors::GREEN});
}

static void create_powerup(flecs::world &ecs, int x, int y, float amount)
{
  ecs.entity()
    .set(Position{x, y})
    .set(PowerupAmount{amount})
    .set(Color{Colors::RED});
}

static void create_craft_spot(flecs::world& ecs, int x, int y) 
{
    ecs.entity()
        .set(Position{ x, y })
        .add<Craft>()
        .set(Color{ Colors::INDIGO });
}

static void create_market_spot(flecs::world& ecs, int x, int y)
{
    ecs.entity()
        .set(Position{ x, y })
        .add<Market>()
        .set(Color{ Colors::BLACK });
}

static void create_eat_spot(flecs::world& ecs, int x, int y) 
{
    ecs.entity()
        .set(Position{ x, y })
        .add<Eat>()
        .set(Color{ Colors::PURPLE });
}

static void create_sleep_spot(flecs::world& ecs, int x, int y)
{
    ecs.entity()
        .set(Position{ x, y })
        .add<Sleep>()
        .set(Color{ Colors::ORANGE });
}


static void register_input_system(flecs::world& ecs) 
{
    ecs.system<PlayerInput, Action, const IsPlayer>()
        .each([&](PlayerInput& inp, Action& a, const IsPlayer)
            {
                bool left = app_keypressed(GLFW_KEY_LEFT);
                bool right = app_keypressed(GLFW_KEY_RIGHT);
                bool up = app_keypressed(GLFW_KEY_UP);
                bool down = app_keypressed(GLFW_KEY_DOWN);
                bool jump = app_keypressed(GLFW_KEY_SPACE);

                if (left && !inp.left)
                    a.action = Actions::MOVE_LEFT;
                if (right && !inp.right)
                    a.action = Actions::MOVE_RIGHT;
                if (up && !inp.up)
                    a.action = Actions::MOVE_UP;
                if (down && !inp.down)
                    a.action = Actions::MOVE_DOWN;
                if (jump && !inp.jump)
                    a.action = Actions::JUMP;


                inp.left = left;
                inp.right = right;
                inp.up = up;
                inp.down = down;
                inp.jump = jump;
            });
}

static void register_display_system(flecs::world& ecs)
{
    ecs.system<const Position, const Color>()
        .each([&](const Position& pos, const Color color)
            {
                DebugDrawEncoder dde;
                dde.begin(0);
                dde.push();
                dde.setColor(to_underlying(color.color));
                dde.drawQuad(bx::Vec3(0, 0, 1), bx::Vec3(pos.x* QuadSize, pos.y*QuadSize, 0.f), QuadSize);
                dde.pop();
                dde.end();
            });
}

static void register_roguelike_systems(flecs::world &ecs)
{
    register_input_system(ecs);
    register_display_system(ecs);
}


void init_roguelike(flecs::world &ecs)
{
  register_roguelike_systems(ecs);

  // TODO: Parse json for initial position

  //add_patrol_attack_flee_sm(create_monster(ecs, 5, 5, Colors::YELLOW));
  //add_patrol_attack_flee_sm(create_monster(ecs, 10, -5, Colors::YELLOW));
  //add_patrol_flee_sm(create_monster(ecs, -5, -5, Colors::PINK));
  //add_attack_sm(create_monster(ecs, -5, 5, Colors::BLUE));
  
  add_berserker_sm(create_monster(ecs, 3, 3, Colors::SWAMP));
  add_selfhealing_sm(create_monster(ecs, 4, 4, Colors::TURQUOISE));

  create_player(ecs, 0, 0);
  //add_crafter_sm(create_monster(ecs, 4, 4, Colors::TURQUOISE));

  //create_powerup(ecs, 7, 7, 10.f);
  //create_powerup(ecs, 10, -6, 10.f);
  //create_powerup(ecs, 10, -4, 10.f);
  //
  //create_heal(ecs, -5, -5, 50.f);
  //create_heal(ecs, -5, 5, 50.f);
  //
  //create_eat_spot(ecs, -9, 9);
  //create_sleep_spot(ecs, 9, 9);
  //create_craft_spot(ecs, 9, -9);
  //create_market_spot(ecs, -9, -9);
}

static bool is_player_acted(flecs::world &ecs)
{
  static auto processPlayer = ecs.query<const IsPlayer, const Action>();
  bool playerActed = false;
  processPlayer.each([&](const IsPlayer, const Action &a)
  {
    playerActed = a.action != Actions::NOP;
  });
  return playerActed;
}

static bool upd_player_actions_count(flecs::world &ecs)
{
  static auto updPlayerActions = ecs.query<const IsPlayer, NumActions>();
  bool actionsReached = false;
  updPlayerActions.each([&](const IsPlayer, NumActions &na)
  {
    na.curActions = (na.curActions + 1) % na.numActions;
    actionsReached |= na.curActions == 0;
  });
  return actionsReached;
}

static Position move_pos(Position pos, Actions action)
{
  if (action == Actions::MOVE_LEFT)
    pos.x--;
  else if (action == Actions::MOVE_RIGHT)
    pos.x++;
  else if (action == Actions::MOVE_UP)
    pos.y++;
  else if (action == Actions::MOVE_DOWN)
    pos.y--;
  return pos;
}

static void process_actions(flecs::world &ecs)
{
  static auto processActions = ecs.query<Action, Position, MovePos, const MeleeDamage, const Team>();
  static auto checkAttacks = ecs.query<const MovePos, Hitpoints, const Team>();
  // Process all actions
  ecs.defer([&]
  {
    processActions.each([&](flecs::entity entity, Action &a, Position &pos, MovePos &mpos, const MeleeDamage &dmg, const Team &team)
    {
      Position nextPos = move_pos(pos, a.action);
      bool blocked = false;
      checkAttacks.each([&](flecs::entity enemy, const MovePos &epos, Hitpoints &hp, const Team &enemy_team)
      {
        if (entity != enemy && epos == nextPos)
        {
          blocked = true;
          if (team.team != enemy_team.team)
            hp.hitpoints -= dmg.damage;
        }
      });
      if (blocked)
        a.action = Actions::NOP;
      else
        mpos = nextPos;
    });
    // now move
    processActions.each([&](flecs::entity entity, Action &a, Position &pos, MovePos &mpos, const MeleeDamage &, const Team&)
    {
      pos = mpos;
      a.action = Actions::NOP;
    });
  });

  static auto deleteAllDead = ecs.query<const Hitpoints>();
  ecs.defer([&]
  {
    deleteAllDead.each([&](flecs::entity entity, const Hitpoints &hp)
    {
      if (hp.hitpoints <= 0.f)
        entity.destruct();
    });
  });

  static auto playerPickup = ecs.query<const IsPlayer, const Position, Hitpoints, MeleeDamage>();
  static auto healPickup = ecs.query<const Position, const HealAmount>();
  static auto powerupPickup = ecs.query<const Position, const PowerupAmount>();
  ecs.defer([&]
  {
    playerPickup.each([&](const IsPlayer&, const Position &pos, Hitpoints &hp, MeleeDamage &dmg)
    {
      healPickup.each([&](flecs::entity entity, const Position &ppos, const HealAmount &amt)
      {
        if (pos == ppos)
        {
          hp.hitpoints += amt.amount;
          entity.destruct();
        }
      });
      powerupPickup.each([&](flecs::entity entity, const Position &ppos, const PowerupAmount &amt)
      {
        if (pos == ppos)
        {
          dmg.damage += amt.amount;
          entity.destruct();
        }
      });
    });
  });
}

void process_turn(flecs::world &ecs)
{
  static auto stateMachineAct = ecs.query<StateMachine>();
  if (is_player_acted(ecs))
  {
    if (upd_player_actions_count(ecs))
    {
      // Plan action for NPCs
      ecs.defer([&]
      {
        stateMachineAct.each([&](flecs::entity e, StateMachine &sm)
        {
          sm.act(0.f, ecs, e);
        });
      });
    }
    process_actions(ecs);
  }
}

void print_stats(flecs::world &ecs)
{
  bgfx::dbgTextClear();
  static auto playerStatsQuery = ecs.query<const IsPlayer, const Hitpoints, const MeleeDamage>();
  playerStatsQuery.each([&](const IsPlayer &, const Hitpoints &hp, const MeleeDamage &dmg)
  {
    bgfx::dbgTextPrintf(0, 1, 0x0f, "hp: %d", (int)hp.hitpoints);
    bgfx::dbgTextPrintf(0, 2, 0x0f, "power: %d", (int)dmg.damage);
  });
}

