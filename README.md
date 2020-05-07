# glsl-raytracing

用 glsl 实现光线跟踪算法，此 repo 属于玩票性质，不可用于生产用途，项目中的代码也仅可用作参考。

requirements:

- Vulkan SDK
- CMake
- Visual Studio or other compilers

# 模块

一个 .h/.cpp 文件表示一个模块：

- camera 相机相关的内容，包括模型相机和第一人称相机
- scene 场景描述
- bvh 层次包围盒
- render 渲染
- app 交互逻辑
- main 程序的初始化和退出
- util 一些通用的工具类
