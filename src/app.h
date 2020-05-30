//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_APP_H
#define GLSL_RAYTRACING_APP_H

#include "bvh.h"
#include "camera.h"
#include "device.h"
#include "render.h"
#include "scene.h"

class App {
 public:
  void startup(int width, int height);
  void shutdown();

  void load_model(const char* path);

  void run_event_loop();

 private:
  GLFWwindow* window_{nullptr};

  Device* device_{nullptr};
  SwapChain* swap_chain_{nullptr};
  Scene* scene_{nullptr};
  BvhScene* bvh_scene_{nullptr};
  Camera* camera_{nullptr};
  Renderer* renderer_{nullptr};

  void on_window_size();
  void on_cursor_pos();
};

#endif  // GLSL_RAYTRACING_APP_H
