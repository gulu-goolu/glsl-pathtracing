//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "app.h"

int main() {
  App app;
  app.startup(1920, 1024);

  app.load_model("my_model.glb");

  app.run_event_loop();

  app.shutdown();
}