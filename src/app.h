//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef APP_H
#define APP_H

#include "bvh.h"
#include "camera.h"
#include "render.h"
#include "scene.h"
#include "vkut.h"

class App {
 public:
  void startup(int width, int height);
  void shutdown();

  void load_model(const char* path);

  void run_event_loop();

 private:
  GLFWwindow* window_{nullptr};

  InstancePtr instance_;
  SurfacePtr surface_;
  DevicePtr device_;
  SwapChainPtr swap_chain_;

  Scene* scene_{nullptr};
  BvhScene* bvh_scene_{nullptr};
  Camera* camera_{nullptr};
  Renderer* renderer_{nullptr};

  void on_window_size();
  void on_cursor_pos();
};

#endif  // APP_H
