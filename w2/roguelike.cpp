#include "raylib.h"

#include "roguelike.h"
#include "ecsTypes.h"
#include "stateMachine.h"
#include "aiLibrary.h"
#include "blackboard.h"

#include <cstdlib>
#include <vector>
#include <memory>


static void create_minotaur_beh(flecs::entity e)
{
    e.set(Blackboard{});
    auto lol = BehNode::make<SequenceNode>(
        BehNode::make<IsLowHpNode>(50.f),
        BehNode::make<FindEnemyNode>(e, 4.f, "flee_enemy"),
        BehNode::make<FleeNode>(e, "flee_enemy")
        );

    auto kek = BehNode::make<SequenceNode>(
        BehNode::make<FindEnemyNode>(e, 3.f, "attack_enemy"),
        BehNode::make<MoveToEntityNode>(e, "attack_enemy")
        );

    auto cheburek = BehNode::make<SelectorNode>(std::move(lol), std::move(kek), BehNode::make<PatrolNode>(e, 2.f, "patrol_pos"));

  e.set(BehaviourTree{std::move(cheburek)});
}

static void create_collector_mob(flecs::entity e)
{
    e.set(Blackboard{});
    auto attack_all = BehNode::make<SequenceNode>(
        BehNode::make<FindEnemyNode>(e, 100.f, "attack_all_enemy"),
        BehNode::make<MoveToEntityNode>(e, "attack_all_enemy")
    );
    
    auto attack_close = BehNode::make<SequenceNode>(
        BehNode::make<FindEnemyNode>(e, 3.f, "attack_enemy_close"),
        BehNode::make<MoveToEntityNode>(e, "attack_enemy_close")
    );

    auto find_health = BehNode::make<SequenceNode>(
        BehNode::make<FindTagNode<HealAmount>>(e, 100.f, "health"),
        BehNode::make<MoveToEntityNode>(e, "health")
        );

    auto find_buffs = BehNode::make<SequenceNode>(
        BehNode::make<FindTagNode<PowerupAmount>>(e, 100.f, "buff"),
        BehNode::make<MoveToEntityNode>(e, "buff")
        );

    auto find_all = BehNode::make<SelectorNode>(
        std::move(find_health),
        std::move(find_buffs)
        );

    auto collector = BehNode::make<SelectorNode>(
        std::move(attack_close),
        std::move(find_all),
        std::move(attack_all)
        );

    e.set(BehaviourTree{ std::move(collector) });
}

static flecs::entity generate_waypoints(flecs::world& ecs, const std::vector<std::pair<int, int>>& waypoints) {
    flecs::entity first = ecs.entity()
        .set(Position{ waypoints[0].first, waypoints[0].second });
    
    flecs::entity prev = first;
    for (int i = 1; i < waypoints.size(); i++)
    {
        auto current = ecs.entity()
            .set(Position{ waypoints[i].first, waypoints[i].second });
        prev.set(NextWaypoint{ current });
        prev = current;
    }
    prev.set(NextWaypoint{ first });

    return first;
}

static void create_guard_mob(flecs::entity e)
{
    e.set(Blackboard{});
    auto attack_all = BehNode::make<SequenceNode>(
        BehNode::make<FindEnemyNode>(e, 3.f, "attack_enemy"),
        BehNode::make<MoveToEntityNode>(e, "attack_enemy")
        );
    
    auto patrol_waypoint = BehNode::make<SequenceNode>(
        BehNode::make<FindWaypointNode>(e, "waypoint"),
        BehNode::make<MoveToEntityNode>(e, "waypoint")
        );

    auto guard = BehNode::make<SelectorNode>(
        std::move(attack_all),
        std::move(patrol_waypoint)
        );

    e.set(BehaviourTree{ std::move(guard) });
}


static flecs::entity create_monster(flecs::world &ecs, int x, int y, Color col, const char *texture_src)
{
  flecs::entity textureSrc = ecs.entity(texture_src);
  return ecs.entity()
    .set(Position{x, y})
    .set(MovePos{x, y})
    .set(Hitpoints{100.f})
    .set(Action{Actions::NOP})
    .set(Color{col})
    .add<TextureSource>(textureSrc)
    .set(StateMachine{})
    .set(Team{1})
    .set(NumActions{1, 0})
    .set(MeleeDamage{20.f})
    .set(Blackboard{});
}

static void create_player(flecs::world &ecs, int x, int y, const char *texture_src)
{
  flecs::entity textureSrc = ecs.entity(texture_src);
  ecs.entity("player")
    .set(Position{x, y})
    .set(MovePos{x, y})
    .set(Hitpoints{100.f})
    //.set(Color{0xee, 0xee, 0xee, 0xff})
    .set(Action{Actions::NOP})
    .add<IsPlayer>()
    .add<MayPickUp>()
    .set(Team{0})
    .set(PlayerInput{})
    .set(NumActions{2, 0})
    .set(Color{255, 255, 255, 255})
    .add<TextureSource>(textureSrc)
    .set(MeleeDamage{50.f});
}

static void create_heal(flecs::world &ecs, int x, int y, float amount)
{
  ecs.entity()
    .set(Position{x, y})
    .set(HealAmount{amount})
    .set(Color{0xff, 0x44, 0x44, 0xff});
}

static void create_powerup(flecs::world &ecs, int x, int y, float amount)
{
  ecs.entity()
    .set(Position{x, y})
    .set(PowerupAmount{amount})
    .set(Color{0xff, 0xff, 0x00, 0xff});
}

static void register_input_system(flecs::world& ecs) 
{
    ecs.system<PlayerInput, Action, const IsPlayer>()
        .each([&](PlayerInput& inp, Action& a, const IsPlayer)
            {
                bool left = IsKeyDown(KEY_LEFT);
                bool right = IsKeyDown(KEY_RIGHT);
                bool up = IsKeyDown(KEY_UP);
                bool down = IsKeyDown(KEY_DOWN);
                bool jump = IsKeyDown(KEY_SPACE);
                bool act = IsKeyDown(KEY_Q);
                bool meh = IsKeyDown(KEY_E);

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
                if (act && !inp.act)
                    a.action = Actions::ACT;
                if (meh && !inp.meh)
                    a.action = Actions::MEH;
                                  
                inp.left = left;
                inp.right = right;
                inp.up = up;
                inp.down = down;
                inp.jump = jump;
                inp.act = act;
                inp.meh = meh;
            });
}

static void register_display_systems(flecs::world& ecs)
{
    ecs.system<const Position, const Color>()
        .term<TextureSource>(flecs::Wildcard).not_()
        .each([&](const Position& pos, const Color color)
            {
                const Rectangle rect = { float(pos.x), float(pos.y), 1, 1 };
                DrawRectangleRec(rect, color);
            });
    ecs.system<const Position, const Color>()
        .term<TextureSource>(flecs::Wildcard)
        .each([&](flecs::entity e, const Position& pos, const Color color)
            {
                const auto textureSrc = e.target<TextureSource>();
                DrawTextureQuad(*textureSrc.get<Texture2D>(),
                    Vector2{ 1, 1 }, Vector2{ 0, 0 },
                    Rectangle{ float(pos.x), float(pos.y), 1, 1 }, color);
            });
}

static void register_roguelike_systems(flecs::world &ecs)
{
    register_input_system(ecs);
    register_display_systems(ecs);
}


void init_roguelike(flecs::world &ecs)
{
  std::srand(std::time(nullptr));
  register_roguelike_systems(ecs);

  ecs.entity("swordsman_tex")
    .set(Texture2D{LoadTexture("assets/swordsman.png")});
  ecs.entity("minotaur_tex")
    .set(Texture2D{LoadTexture("assets/minotaur.png")});

  ecs.observer<Texture2D>()
    .event(flecs::OnRemove)
    .each([](Texture2D texture)
      {
        UnloadTexture(texture);
      });

  //create_minotaur_beh(create_monster(ecs, 5, 5, Color{0xee, 0x00, 0xee, 0xff}, "minotaur_tex"));
  //create_minotaur_beh(create_monster(ecs, 10, -5, Color{0xee, 0x00, 0xee, 0xff}, "minotaur_tex"));
  //create_minotaur_beh(create_monster(ecs, -5, -5, Color{0x11, 0x11, 0x11, 0xff}, "minotaur_tex"));
  //create_minotaur_beh(create_monster(ecs, -5, 5, Color{0, 255, 0, 255}, "minotaur_tex"));
  //
  create_collector_mob(create_monster(ecs, 10, 5, Color{ 0, 255, 255, 255 }, "swordsman_tex").add<MayPickUp>());

  create_guard_mob(
      create_monster(ecs, 3, 3, Color{ 0, 0, 255, 255 }, "swordsman_tex")
      .set(CurrentWaypoint{ generate_waypoints(ecs, {{6,6},{6,-6},{-6,-6},{-6,6}}) })
  );

  create_player(ecs, 0, 0, "swordsman_tex");

  create_powerup(ecs, 7, 7, 10.f);
  create_powerup(ecs, 10, -6, 10.f);
  create_powerup(ecs, 10, -4, 10.f);

  create_heal(ecs, -5, -5, 50.f);
  create_heal(ecs, -5, 5, 50.f);
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
    pos.y--;
  else if (action == Actions::MOVE_DOWN)
    pos.y++;
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
    processActions.each([&](Action &a, Position &pos, MovePos &mpos, const MeleeDamage &, const Team&)
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

  static auto entityPickup = ecs.query<const MayPickUp, const Position, Hitpoints, MeleeDamage>();
  static auto healPickup = ecs.query<const Position, const HealAmount>();
  static auto powerupPickup = ecs.query<const Position, const PowerupAmount>();
  ecs.defer([&]
  {
    entityPickup.each([&](const MayPickUp&, const Position &pos, Hitpoints &hp, MeleeDamage &dmg)
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
  static auto behTreeUpdate = ecs.query<BehaviourTree, Blackboard>();
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
        behTreeUpdate.each([&](flecs::entity e, BehaviourTree &bt, Blackboard &bb)
        {
          bt.update(ecs, e, bb);
        });
      });
    }
    process_actions(ecs);
  }
}

void print_stats(flecs::world &ecs)
{
  static auto playerStatsQuery = ecs.query<const IsPlayer, const Hitpoints, const MeleeDamage>();
  playerStatsQuery.each([&](const IsPlayer &, const Hitpoints &hp, const MeleeDamage &dmg)
  {
    DrawText(TextFormat("hp: %d", int(hp.hitpoints)), 20, 20, 20, WHITE);
    DrawText(TextFormat("power: %d", int(dmg.damage)), 20, 40, 20, WHITE);
  });
}

