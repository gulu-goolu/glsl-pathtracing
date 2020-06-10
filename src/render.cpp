//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "render.h"

#include <cassert>

#include "bvh.h"

void Renderer::reset_trace_buffer() {
  // TODO: reset epoch, clear color image
}

void Renderer::dispatch_trace_unit(BvhScene* bvh_scene, Camera* camera) {}
