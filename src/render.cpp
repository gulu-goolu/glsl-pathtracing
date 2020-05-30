//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "render.h"

#include <cassert>

#include "bvh.h"

void Renderer::reset() {
  // TODO: reset epoch, clear color image
}

void Renderer::render(BvhScene* bvh_scene, Camera* camera) {
  if (is_need_reset(bvh_scene, camera)) {
    reset();
  }

  // TODO: execute render pass
}
