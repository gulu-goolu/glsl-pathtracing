//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef RENDER_H
#define RENDER_H

#include "bvh.h"
#include "camera.h"
#include "vkut.h"

class Renderer {
 public:
  explicit Renderer();
  ~Renderer();

  void reset_trace_buffer();
  virtual void dispatch_trace_unit(BvhScene* bvh_scene, Camera* camera);

 private:
  BvhScene* bvh_scene_{nullptr};
  Camera* camera_{nullptr};
};

class ClearScreen : public Renderer {
 public:
  explicit ClearScreen(SwapChain* swap_chain, Vec3f color);
  ~ClearScreen();

  void reset_trace_buffer();
  void dispatch_trace_unit(BvhScene* bvh_scene, Camera* camera) override;

 private:
  VkRenderPass vk_render_pass_{VK_NULL_HANDLE};
  SwapChain* swap_chain_{nullptr};
  std::vector<VkFramebuffer> framebuffers_;
};

#endif  // RENDER_H
