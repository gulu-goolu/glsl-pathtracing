//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "app.h"

void App::startup(int width, int height) {
  if (!glfwInit()) {
    return;
  }

  // create window
  const char* title = "glfw-raytracing";
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);

  // create camera
}

void App::shutdown() {
  // TODO
  // destroy scene
  // destroy render and swap chain
  // destroy device
  glfwTerminate();
}

void App::load_model(const char* path) {
  // TODO
  // scene = new Scene(path);
  // bvh_scene_ = new BvhScene(scene);
}

void App::run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void App::on_window_size() {
  // TODO destroy swap chain, destroy renderer
  // TODO create swap chain, create renderer
}

void App::on_cursor_pos() {
  // TODO update camera
}

App2::App2(int width, int height, const char* title, const char* model_path) {
  if (!glfwInit()) {
    std::abort();
  }
  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

App2::~App2() { glfwTerminate(); }

void App2::run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}