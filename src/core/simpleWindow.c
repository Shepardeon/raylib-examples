#include <raylib.h>

int examples_core_simple_window(void) {
  const int screen_width = 800;
  const int screen_height = 450;
  const int target_fps = 60;
  const int text_x = 190;
  const int text_y = 200;
  const int text_size = 20;

  InitWindow(screen_width, screen_height, "raylib [core] example - Simple Window");

  SetTargetFPS(target_fps);

  while (!WindowShouldClose()) {
    BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("Congrats! You created your first window!", text_x, text_y, text_size, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}