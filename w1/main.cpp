// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/timer.h>
#include <debugdraw/debugdraw.h>
#include <flecs.h>

#include "app.h"
#include "ecsTypes.h"
#include "roguelike.h"
#include "GameSettings.h"

#include <array>


void DrawGrid(DebugDrawEncoder& dde)
{
    // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
    const bgfx::ViewId kClearView = 0;
    bgfx::touch(kClearView);

    dde.begin(0);
    dde.drawGrid(bx::Vec3(0, 0, 1), bx::Vec3(0.5, 0.5, 0), GridWidth, QuadSize);
    dde.end();
}

int main(int argc, const char **argv)
{
  int width = 960;
  int height = 540;
  if (!app_init(width, height))
    return 1;
  ddInit();

  bgfx::setDebug(BGFX_DEBUG_TEXT);

  bx::Vec3 eye(0.f, 0.f, -16.f);
  bx::Vec3 at(0.f, 0.f, 0.f);
  bx::Vec3 up(0.f, 1.f, 0.f);

  std::array<float, 16> view{};
  std::array<float, 16> proj{};
  bx::mtxLookAt(view.data(), bx::load<bx::Vec3>(&eye.x), bx::load<bx::Vec3>(&at.x), bx::load<bx::Vec3>(&up.x) );

  flecs::world ecs;

  init_roguelike(ecs);

  DebugDrawEncoder dde;
  while (!app_should_close())
  {
    process_turn(ecs);

    app_poll_events();

    // Handle window resize.
    app_handle_resize(width, height);

    //bx::mtxOrtho(proj, 0.f, width, 0.f, height, 0.f, 100.f, 0.f, bgfx::getCaps()->homogeneousDepth);
    bx::mtxProj(proj.data(), 60.0f, float(width)/float(height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view.data(), proj.data());

    DrawGrid(dde);
    
    print_stats(ecs);

    ecs.progress();

    // Advance to next frame. Process submitted rendering primitives.
    bgfx::frame();
  }
  ddShutdown();
  bgfx::shutdown();
  app_terminate();
  return 0;
}
