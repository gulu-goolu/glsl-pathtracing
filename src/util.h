//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <cstdint>
#include <vector>

// macros
#define NOCOPYABLE(N)               \
  N(const N &) = delete;            \
  N(N &&) = delete;                 \
  N &operator=(const N &) = delete; \
  N &operator=(N &&) noexcept = delete;

//---
// math
struct Vec3f {
  float x, y, z;

  Vec3f() : x(0.f), y(0.0f), z(0.0f) {}
  explicit Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

  float &operator[](int i) { return (&x)[i]; }
  const float &operator[](int i) const { return (&x)[i]; }

  [[nodiscard]] Vec3f operator+(const Vec3f &rhs) const {
    return Vec3f(x + rhs.x, y + rhs.y, z + rhs.z);
  }
  Vec3f &operator+=(const Vec3f &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  [[nodiscard]] Vec3f operator-(const Vec3f &rhs) const {
    return Vec3f(x - rhs.x, y - rhs.y, z - rhs.z);
  }
  Vec3f &operator-=(const Vec3f &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  [[nodiscard]] Vec3f operator*(float a) const {
    return Vec3f(x * a, y * a, z * a);
  }

  [[nodiscard]] static Vec3f cross(const Vec3f &v1, const Vec3f &v2) {
    float x = v1.y * v2.z - v1.z * v2.y;
    float y = v1.z * v2.x - v1.x * v2.z;
    float z = v1.x * v2.y - v1.y * v2.x;

    return Vec3f(x, y, z);
  }

  [[nodiscard]] static float dot(const Vec3f &v1, const Vec3f &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  [[nodiscard]] static Vec3f normalize(const Vec3f &v) {
    float len = std::sqrtf(dot(v, v));
    float a = 1.0f / len;
    return v * a;
  }
};

// refcounted
class RefCounted {
 public:
  NOCOPYABLE(RefCounted)

  RefCounted() = default;
  virtual ~RefCounted() = default;

  int add_ref() { return ++ref_count_; }
  int sub_ref() { return --ref_count_; }

 private:
  int ref_count_{1};
};

//----
// io
using Blob = std::vector<uint8_t>;

bool read_file(const char *path, Blob &out_blob);

#endif  // UTIL_H
