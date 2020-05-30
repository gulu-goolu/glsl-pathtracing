//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_CAMERA_H
#define GLSL_RAYTRACING_CAMERA_H

#include "util.h"

class CameraData {
  Vec3f look_from;
  Vec3f look_to;
  Vec3f up_dir;

  float fov_angle_y{0};
  float ratio_aspect{0};
};

class Camera {
 public:
  virtual void get_data(CameraData* out_data, uint32_t* out_version) = 0;
};

class FirstPersonCamera : public Camera {
 public:
 private:
};

class ModelViewCamera : public Camera {
 public:
 private:
};

#endif  // GLSL_RAYTRACING_CAMERA_H
