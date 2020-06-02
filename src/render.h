//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_RENDER_H
#define GLSL_RAYTRACING_RENDER_H

#include "bvh.h"
#include "camera.h"
#include "vkut.h"

class Renderer {
 public:
  explicit Renderer();
  ~Renderer();

  void reset();
  void render(BvhScene* bvh_scene, Camera* camera);

 private:
  BvhScene* bvh_scene_{nullptr};
  Camera* camera_{nullptr};
  int camera_version_{0};

  bool is_need_reset(BvhScene* bvh_scene, Camera* camera) {
    return true;  // TODO
  }
};

#endif  // GLSL_RAYTRACING_RENDER_H
