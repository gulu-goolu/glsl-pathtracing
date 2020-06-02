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

  vkut_createSurfaceAndDevice(window_, &surface_, &device_);

  swap_chain_ = std::make_shared<SwapChain>(device_, surface_);

  // TODO
  // create device
  // create swap chain and renderer
  // create default scene
}

void App::shutdown() {
  swap_chain_.reset();
  device_.reset();
  surface_.reset();
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

void App::run_event_loop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

    swap_chain_->acquire();
    renderer_->render(bvh_scene_, camera_);
    swap_chain_->present();
  }
}

void App::on_window_size() {
  // TODO destroy swap chain, destroy renderer
  // TODO create swap chain, create renderer
}

void App::on_cursor_pos() {
  // TODO update camera
}