#pragma once

#include <flecs.h>
#include <span>

constexpr float tile_size = 512.f;

void init_roguelike(flecs::world &ecs);
void init_dungeon(flecs::world &ecs, std::span<char> tiles, size_t w, size_t h);
void process_turn(flecs::world &ecs);
void print_stats(flecs::world &ecs);
