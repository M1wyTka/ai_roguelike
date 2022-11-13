#include "dijkstraMapGen.h"
#include "ecsTypes.h"
#include "dungeonUtils.h"
#include <cmath>

template<typename Callable>
static void query_dungeon_data(flecs::world &ecs, Callable c)
{
  static auto dungeonDataQuery = ecs.query<const DungeonData>();

  dungeonDataQuery.each(c);
}

template<typename Callable>
static void query_characters_positions(flecs::world &ecs, Callable c)
{
  static auto characterPositionQuery = ecs.query<const Position, const Team>();

  characterPositionQuery.each(c);
}

constexpr float invalid_tile_value = 1e5f;

static void init_tiles(std::vector<float> &map, const DungeonData &dd)
{
  map.resize(dd.width * dd.height);
  for (float &v : map)
    v = invalid_tile_value;
}

// scan version, could be implemented as Dijkstra version as well
static void process_dmap(std::vector<float> &map, const DungeonData &dd)
{
  bool done = false;
  auto getMapAt = [&](size_t x, size_t y, float def)
  {
    if (x < dd.width && y < dd.width && dd.tiles[y * dd.width + x] == dungeon::floor)
      return map[y * dd.width + x];
    return def;
  };
  auto getMinNei = [&](size_t x, size_t y)
  {
    float val = map[y * dd.width + x];
    val = std::min(val, getMapAt(x - 1, y + 0, val));
    val = std::min(val, getMapAt(x + 1, y + 0, val));
    val = std::min(val, getMapAt(x + 0, y - 1, val));
    val = std::min(val, getMapAt(x + 0, y + 1, val));
    return val;
  };
  while (!done)
  {
    done = true;
    for (size_t y = 0; y < dd.height; ++y)
      for (size_t x = 0; x < dd.width; ++x)
      {
        const size_t i = y * dd.width + x;
        if (dd.tiles[i] != dungeon::floor)
          continue;
        const float myVal = getMapAt(x, y, invalid_tile_value);
        const float minVal = getMinNei(x, y);
        if (minVal < myVal - 1.f)
        {
          map[i] = minVal + 1.f;
          done = false;
        }
      }
  }
}

void dmaps::gen_player_approach_map(flecs::world &ecs, std::vector<float> &map)
{
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    query_characters_positions(ecs, [&](const Position &pos, const Team &t)
    {
      if (t.team == 0) // player team hardcode
        map[pos.y * dd.width + pos.x] = 0.f;
    });
    process_dmap(map, dd);
  });
}

void dmaps::gen_player_flee_map(flecs::world &ecs, std::vector<float>& ready_approach_map, std::vector<float> &map)
{
    map.resize(ready_approach_map.size());
    for (size_t i = 0; i < map.size(); i++)
        if(ready_approach_map[i] < invalid_tile_value)
            map[i] = -1.2f * ready_approach_map[i];
}

void dmaps::gen_hive_pack_map(flecs::world &ecs, std::vector<float> &map)
{
  static auto hiveQuery = ecs.query<const Position, const Hive>();
  query_dungeon_data(ecs, [&](const DungeonData &dd)
  {
    init_tiles(map, dd);
    hiveQuery.each([&](const Position &pos, const Hive &)
    {
      map[pos.y * dd.width + pos.x] = 0.f;
    });
    process_dmap(map, dd);
  });
}

bool is_view_obstucted(const DungeonData& dd, const Position& center, const Position& tile)
{
    constexpr uint32_t step_amount = 5;
    const float step_x = float(center.x - tile.x) / step_amount;
    const float step_y = float(center.y - tile.y) / step_amount;
    float x = tile.x + step_x;
    float y = tile.y + step_y;
    for(int i = 1; i < step_amount; i++)
    {
        if (dd.tiles[std::round(y) * dd.width + std::round(x)] == dungeon::wall)
            return true;
        x += step_x;
        y += step_y;
    }
    return false;
}

void dmaps::gen_archer_map(flecs::world& ecs, std::vector<float>& map)
{
    constexpr int radius = 2;
    query_dungeon_data(ecs, [&](const DungeonData& dd)
    {
        init_tiles(map, dd);
        query_characters_positions(ecs, [&](const Position& pos, const Team& t)
            {    
                if (t.team == 0) // player team hardcode
                {  
                    for (int i = -radius; i <= radius; i++) 
                    {
                        for (int j = -radius; j <= radius; j++)
                        {
                            int newX = pos.x + i;
                            int newY = pos.y + j;
                            if (newX < 0 || newX > dd.width || newY < 0 || newY > dd.height)
                                continue;
                            if ((i != -radius) && (i != radius) && (j != -radius) && (j != radius))
                                continue;
                            if (dd.tiles[newY * dd.width + newX] == dungeon::wall)
                                continue;
                            if (is_view_obstucted(dd, pos, { newX, newY }))
                                continue;
                            map[newY * dd.width + newX] = 0.f;
                        }
                    }
                } 
            });
        process_dmap(map, dd);
    });
}