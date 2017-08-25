#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "../game_window/src/game_window.hpp"
#include "../game_window/src/game_window.cpp"


struct GameApp {
  gwin::GameWindow window;
  bool running;
  bool on_emscripten;
};

void game_loop(void* arg)
{
  GameApp* app = static_cast<GameApp*>(arg);

  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT) { app->running = false; }
  }

  if (!app->running) { exit(0); }

  app->window.flip();

  if (!app->on_emscripten) { SDL_Delay(20); }
}


int main(int argc, char** argv)
{
  gwin::GameWindowOptOpenGL opt;
  opt.fullscreen = false;
  opt.resizable = false;
  opt.set_pos = false;
  opt.x = 0;
  opt.y = 0;
  opt.w = 640;
  opt.h = 480;
#ifdef __EMSCRIPTEN__
  opt.gles = true;
  opt.major = 2;
  opt.minor = 0;
  opt.debug_ctx = false;
#else
  opt.gles = false;
  opt.major = 3;
  opt.minor = 3;
  opt.debug_ctx = true;
#endif
  opt.vsync = false;

  GameApp app;
  app.running = true;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_Init(SDL_INIT_VIDEO);

  if (!app.window.open("hello window!", opt)) { return 1; }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(game_loop, &app, 0, 1);
#else
  for (;;) { game_loop(&app); }
#endif

  return 0;
}
