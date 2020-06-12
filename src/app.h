//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef APP_H
#define APP_H

#include <string>

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

  void run();

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

class App2 {
 public:
  App2(int width, int height, const char* title, const char* model_path);
  ~App2();

  void run();

 private:
  // creation arguments
  int width_{0};
  int height_{0};
  std::string title_;
  std::string model_path_;

  // window
  GLFWwindow* window_{nullptr};
};

#endif  // APP_H
