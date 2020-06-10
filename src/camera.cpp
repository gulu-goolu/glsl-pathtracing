//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "camera.h"

FirstPersonCamera::FirstPersonCamera(const Vec3f &look_from,
                                     const Vec3f &look_to, const Vec3f &up_dir,
                                     float fov_angle, float aspect)
    : look_from_(look_from),
      look_to_(look_to),
      up_dir_(up_dir),
      fov_angle_(fov_angle),
      aspect_(aspect) {}

void FirstPersonCamera::get_data(CameraData *out_data) {
  out_data->look_from = look_from_;
  out_data->up_dir = up_dir_;
  out_data->look_to = look_to_;
  out_data->fov_angle_y = fov_angle_;
  out_data->ratio_aspect = aspect_;
}
