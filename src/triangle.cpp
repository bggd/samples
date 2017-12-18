#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <vector>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#define GAME_WINDOW_GLFW3

#else

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#define GAME_WINDOW_SDL2

#define MYGLLOADER_DEBUG
#include "../my_gl_loader/src/my_gl_loader.hpp"
#include "../my_gl_loader/src/my_gl_loader.cpp"

#endif


#define GAME_WINDOW_IMPLEMENTATION
#include "../game_window/src/gwin.hpp"

#define GAME_MATH_IMPLEMENTATION
#include "../game_math/src/gmath.hpp"

#define GAME_RENDERER_IMPLEMENTATION
#include "../game_renderer/src/grndr.hpp"


static GLuint VAO;

struct GameApp {
  gwin::GameWindow window;
  grndr::Renderer renderer;
  grndr::PipelineSpec* spec;
  grndr::ogl::Texture2D tex;
  grndr::ogl::VBO vbo;
  bool running;
  bool on_emscripten;
};

void game_loop(void* arg)
{
  GameApp* app = static_cast<GameApp*>(arg);

#ifdef GAME_WINDOW_SDL2
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT) { app->running = false; }
  }

  if (!app->running) { exit(0); }
#endif

  glClear(GL_COLOR_BUFFER_BIT);

  app->renderer.draw_triangles(0, 3);

  app->window.flip();

#ifdef GAME_WINDOW_SDL2
  if (!app->on_emscripten) { SDL_Delay(20); }
#endif
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

#ifdef GAME_WINDOW_SDL2
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_Init(SDL_INIT_VIDEO);
#endif

  if (!app.window.open("hello window!", opt)) { return 1; }
  opt = app.window.get_info();
  grndr::ogl::make_current(opt.gles, opt.major, opt.minor);

#ifdef GAME_WINDOW_SDL2
  my_gl_loader();
  my_gl_loader_enable_debug_output();
#endif

  if (!grndr::ogl::Info::gles) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
  }

  glViewport(0, 0, 640, 480);
  glClearColor(32.0f/255.0f, 64.0f/255.0f, 128.0f/255.0f, 1.0f);

  unsigned char white[32*32*3];
  for (int i=0; i<32*32*3; ++i) { white[i] = 255; }
  app.tex.create();
  app.tex.bind();
  app.tex.tex_parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  app.tex.tex_parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  app.tex.upload_rgb8(32, 32, white);
  grndr::ogl::Texture2D::bind_zero();

  GLfloat vertices[] = {
    0.0f, 1.0f, 0.5f, 1.0,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f
  };
  app.vbo.create();
  app.vbo.bind();
  app.vbo.buffer_data(sizeof(GLfloat)*12, vertices, GL_STATIC_DRAW);
  grndr::ogl::VBO::bind_zero();

  app.spec = grndr::PipelineSpec::get_default_2d();
  app.spec->compile_shader();

  app.renderer.set_pipeline(*app.spec);
  app.renderer.set_uniform_matrix4(app.spec->uniforms[0], gmath::mat4::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f).elements);
  app.tex.bind();
  app.renderer.set_vbo(0, app.vbo);
 
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(game_loop, &app, 0, 1);
#else
  for (;;) { game_loop(&app); }
#endif

  return 0;
}
