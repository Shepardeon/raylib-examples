#include <raylib.h>
#include <raymath.h>

#define G 400
#define PLAYER_JUMP_SPEED 350.0f
#define PLAYER_HORIZONTAL_SPEED 200.0f

typedef struct Player {
  Vector2 position;
  float speed;
  bool canJump;
} Player;

typedef struct EnvItem {
  Rectangle rect;
  bool blocking;
  Color color;
} EnvItem;

void update_player(Player *player, EnvItem *envItems, int envItemsLength, float delta);
void update_camera_center(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void update_camera_inside_map(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void update_camera_smooth_follow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void update_camera_even_out_on_landing(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);
void update_camera_player_bounds_push(Camera2D *camera, Player *player, EnvItem *envItemps, int envItemsLength, float delta, int width, int height);

int examples_core_simple_platformer() {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - Simple Platformer");

  Player player = { 0 };
  player.position = (Vector2){ 400, 280 };
  player.speed = 0;
  player.canJump = false;

  EnvItem envItems[] = {
      { { 0, 0, 1000, 400 }, false, LIGHTGRAY },
      { { 0, 400, 1000, 200 }, true, GRAY },
      { { 300, 200, 400, 10 }, true, GRAY },
      { { 250, 300, 100, 10 }, true, GRAY },
      { { 650, 300, 100, 10 }, true, GRAY },
  };

  int envItemsLength = sizeof(envItems)/sizeof(envItems[0]);

  Camera2D camera = { 0 };
  camera.target = player.position;
  camera.offset = (Vector2){ screenWidth/2.0F, screenHeight/2.0F };
  camera.rotation = 0.0F;
  camera.zoom = 1.0F;

  void (*cameraUpdaters[])(Camera2D*, Player*, EnvItem*, int, float, int, int) = {
    update_camera_center,
    update_camera_inside_map,
    update_camera_smooth_follow,
    update_camera_even_out_on_landing,
    update_camera_player_bounds_push,
  };

  int cameraOption = 0;
  int cameraUpdatersLength = sizeof(cameraUpdaters)/sizeof(cameraUpdaters[0]);

  char* cameraDescriptions[] = {
    "Follow player center",
    "Follow player center, but clamp to map edges",
    "Follow player center; smoothed",
    "Follow player center horizontally; update player center vertically after landing",
    "Player push camera on getting too close to screen edge",
  };

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    update_player(&player, envItems, envItemsLength, deltaTime);

    camera.zoom += GetMouseWheelMove()*0.05F;

    if (camera.zoom > 3.0F) camera.zoom = 3.0F;
    else if (camera.zoom < 0.25F) camera.zoom = 0.25F;

    if (IsKeyPressed(KEY_R)) {
      camera.zoom = 1.0F,
      player.position = (Vector2){ 400, 200 };
    }

    if (IsKeyPressed(KEY_C)) cameraOption = (cameraOption + 1) % cameraUpdatersLength;

    cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

    BeginDrawing();

      ClearBackground(LIGHTGRAY);

      BeginMode2D(camera);

        for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

        Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40.0F, 40.0F };
        DrawRectangleRec(playerRect, RED);

        DrawCircleV(player.position, 5.0F, GOLD);

      EndMode2D();


      DrawText("Controls:", 20, 20, 10, BLACK);
      DrawText("- A/D to move", 40, 40, 10, DARKGRAY);
      DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
      DrawText("- Mouse Wheel to Zoom in-out", 40, 80, 10, DARKGRAY);
      DrawText("- R to reset position + zoom", 40, 100, 10, DARKGRAY);
      DrawText("- C to change camera mode", 40, 120, 10, DARKGRAY);
      DrawText("Current camera mode:", 20, 140, 10, BLACK);
      DrawText(cameraDescriptions[cameraOption], 40, 160, 10, DARKGRAY);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

void update_player(Player *player, EnvItem *envItems, int envItemsLength, float delta) {
  if (IsKeyDown(KEY_A)) {
    player->position.x -= PLAYER_HORIZONTAL_SPEED*delta;
  }
  
  if (IsKeyDown(KEY_D)) {
    player->position.x += PLAYER_HORIZONTAL_SPEED*delta;
  }

  if (IsKeyDown(KEY_SPACE) && player->canJump) {
    player->speed = -PLAYER_JUMP_SPEED;
    player->canJump = false;
  }

  bool hitObstacle = false;
  for (int i = 0; i < envItemsLength; i++) {
    EnvItem *envI = envItems + i;
    Vector2 *pos = &player->position;
    if (envI->blocking &&
        envI->rect.x <= pos->x &&
        envI->rect.x + envI->rect.width >= pos->x &&
        envI->rect.y >= pos->y &&
        envI->rect.y <= pos->y + (player->speed*delta))
    {
      hitObstacle = true;
      player->speed = 0.0F;
      pos->y = envI->rect.y;
      break;
    }
  }

  if (!hitObstacle) {
    player->position.y += player->speed*delta;
    player->speed += G*delta;
    player->canJump = false;
  }
  else {
    player->canJump = true;
  }
}

void update_camera_center(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height) {
  camera->offset = (Vector2){ width/2.0F, height/2.0F };
  camera->target = player->position;
}

void update_camera_inside_map(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height) {
  camera->target = player->position;
  camera->offset = (Vector2){ width/2.0F, height/2.0F };
  float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

  for (int i = 0; i < envItemsLength; i++) {
    EnvItem *envI = envItems + i;
    minX = fminf(envI->rect.x, minX);
    maxX = fmaxf(envI->rect.x + envI->rect.width, maxX);
    minY = fminf(envI->rect.y, minY);
    maxY = fmaxf(envI->rect.y + envI->rect.height, maxY);
  }

  Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
  Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

  if (max.x < width) camera->offset.x = width - (max.x - ((float)width/2));
  if (max.y < height) camera->offset.y = height - (max.y - ((float)height/2));
  if (min.x > 0) camera->offset.x = ((float)width/2) - min.x;
  if (min.y > 0) camera->offset.y = ((float)height/2) - min.y;
}

void update_camera_smooth_follow(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height) {
  static float minSpeed = 30;
  static float minEffectLength = 10;
  static float fractionSpeed = 0.8F;

  camera->offset = (Vector2){ width/2.0F, height/2.0F };
  Vector2 diff = Vector2Subtract(player->position, camera->target);
  float length = Vector2Length(diff);

  if (length > minEffectLength) {
    float speed = fmaxf(fractionSpeed*length, minSpeed);
    camera->target = Vector2Add(camera->target, Vector2Scale(diff, speed*delta/length));
  }
}

void update_camera_even_out_on_landing(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height) {
  static float evenOutSpeed = 700;
  static bool eveningOut = false;
  static float evenOutTarget;

  camera->offset = (Vector2){width / 2.0F, height / 2.0F};
  camera->target.x = player->position.x;

  if (eveningOut)
  {
    if (evenOutTarget > camera->target.y)
    {
      camera->target.y += evenOutSpeed * delta;

      if (camera->target.y > evenOutTarget)
      {
        camera->target.y = evenOutTarget;
        eveningOut = false;
      }
    }
    else
    {
      camera->target.y -= evenOutSpeed * delta;

      if (camera->target.y < evenOutTarget)
      {
        camera->target.y = evenOutTarget;
        eveningOut = false;
      }
    }
  }
  else
  {
    if (player->canJump && (player->speed == 0) && (player->position.y != camera->target.y))
    {
      eveningOut = true;
      evenOutTarget = player->position.y;
    }
  }
}

void update_camera_player_bounds_push(Camera2D *camera, Player *player, EnvItem *envItemps, int envItemsLength, float delta, int width, int height) {
  static Vector2 bbox = { 0.2F, 0.2F };

  Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2) { (1 - bbox.x)*0.5F*width, (1 - bbox.y)*0.5F*height }, *camera);
  Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2) { (1 + bbox.x)*0.5F*width, (1 + bbox.y)*0.5F*height }, *camera);
  camera->offset = (Vector2){ (1 - bbox.x)*0.5F*width, (1 - bbox.y)*0.5F*height };

  if (player->position.x < bboxWorldMin.x) camera->target.x = player->position.x;
  if (player->position.y < bboxWorldMin.y) camera->target.y = player->position.y;
  if (player->position.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x);
  if (player->position.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}
