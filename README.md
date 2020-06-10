# glsl-raytracing

玩票性质的项目，使用 glsl 来实现光线跟踪算法。

## 快速开始

首先在你的计算机上安装下列软件：

- Vulkan SDK
- CMake
- Git
- Visual Studio

```bash
# 拉取代码
git clone -b master https://github.com/murmur-wheel/glsl-raytracing.git
cd glsl-raytracing
mkdir cmake-build && cd cmake-build
cmake ..
```

然后使用 visual studio 打开你的项目

## 流程

```cpp
// 初始化资源，创建窗口等
App::startup(); 

// 使用用户指定的模型替代默认的模型
// TODO 在后台执行模型加载操作
App::load_model();

// 处理调整窗口大小，更新相机等操作
// 如果收到退出信号，终止应用程序
while (App::dispatch_events()) { 

  // 渲染
  // 渲染开始前应当检查是否需要更新 ColorBuffer
  App::render(); 
}

// 销毁资源
App::shutdown(); 
```

## 项目结构

- src 源码
  - render 调用着色器渲染
  - scene 场景
  - bvh 将场景处理成可供 shader 访问的格式
  - device 创建各种资源
  - app 与用户交互
  - camera 相机
- shader 着色器
  - rt.vert.glsl 和 rt.frag.glsl 显示内容

## 架构

实现基于渐进式的光追，我们使用一个 ColorBuffer 来保存累加和。

以下两种情况，ColorBuffer 应当被重置

- 当场景变化时
- 当相机改变时

## 单例

- Device
- Renderer

## 随系统运行时变更对象

- SwapChain
- Camera

## 交互

用户可以通过拖拽等方式来调整相机的位置，以便能通过不同的角度来观察模型。
