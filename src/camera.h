//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "util.h"

class CameraData {
  Vec3f look_from;
  Vec3f look_to;
  Vec3f up_dir;

  float fov_angle_y{0};
  float ratio_aspect{0};
};

const uint32_t CAMERA_FLAG_UPDATED = 1;

class Camera {
 public:
  bool is_flag(uint32_t flags) const { return (flags & flags_) == flags; }
  void remove_flag(uint32_t flags) { flags_ = flags_ & (~flags); }
  void set_flag(uint32_t flags) { flags_ = flags_ | flags; }

  virtual void get_data(CameraData* out_data) = 0;

 private:
  uint32_t flags_{0};
};

class FirstPersonCamera : public Camera {
 public:
 private:
};

class ModelViewCamera : public Camera {
 public:
 private:
};

#endif  // CAMERA_H
