# glsl-raytracing

此 repo 属于玩票性质，不可用于生产用途，项目中的代码仅可用作参考。

# 快速开始

首先需要在你的设备上安装如下软件：

- Vulkan SDK
- CMake
- Visual Studio or other compilers

然后使用 git 下载项目代码：

```bash
git clone --recursive https://github.com/murmur-wheel/glsl-raytracing.git
```

然后使用 cmake 生产解决方案

```bash
cd glsl-raytracing
mkdir cmake-build
cd cmake-build
cmake ..
```

然后用 vs 打开 glsl-raytracing.sln 编译项目，并执行
