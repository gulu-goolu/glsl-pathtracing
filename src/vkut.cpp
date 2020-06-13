//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "vkut.h"

#include <gflags/gflags.h>

VKUT *g_vkut = nullptr;

void VKUT::startup(GLFWwindow *window, SwapchainNotifier *notifier) {
  g_vkut = new VKUT(window, notifier);
}

void VKUT::shutdown() { delete g_vkut; }

VKUT *VKUT::get() { return g_vkut; }

VKUT::VKUT(GLFWwindow *window, SwapchainNotifier *notifier)
    : window_(window), swapchain_notifier_(notifier) {}

VKUT::~VKUT() {}

void VKUT::render() {}

void VKUT::create_instance() {}

void VKUT::select_physical_device() {}

void VKUT::create_logic_device() {}