#pragma once
#include <cstdint>
#include <debugdraw/debugdraw.h>

constexpr uint32_t GridWidth = 20;
constexpr uint32_t GridHeight = GridWidth;
constexpr float QuadSize = 0.9f;

enum class Colors : uint32_t
{
	RED = 0xff0000ff,
	PINK = 0xff6600ff,
	PURPLE = 0xff660066,
	YELLOW = 0xff00ffff,
	ORANGE = 0xff0066ff,
	GREEN = 0xff00ff00,
	SWAMP = 0xff336666,
	BLUE = 0xffff0000,
	INDIGO = 0xff660000, // indigo ??
	TURQUOISE = 0xffffff66,
	LIGHTBLUE = 0xffcc6633,
	BROWN = 0xff003399,
	BLACK = 0xff000000,
	WHITE = 0xffffffff
};