#include <raylib.h>
#include <raymath.h>

#define GRAVITY 32.0F
#define MAX_SPEED 20.0F
#define CROUCH_SPEED 5.0F
#define JUMP_FORCE 12.0F
#define MAX_ACCEL 150.0F

#define FRICTION 0.86F
#define AIR_DRAG 0.98F

#define CONTROL 15.0F
#define CROUCH_HEIGHT 0.0F
#define STAND_HEIGHT 1.0F
#define BOTTOM_HEIGHT 0.5F

#define NORMALIZE_INPUT 0

typedef struct {
  Vector3 position;
  Vector3 velocity;
  Vector3 direction;
  bool is_grounded;
} body_t;

static Vector2 sensivity = { 0.001F, 0.001F };

static body_t player = { 0 };
static Vector2 look_rotation = { 0 };
static float head_timer = 0.0F;
static float walk_lerp = 0.0F;
static float head_lerp = STAND_HEIGHT;
static Vector2 lean = { 0 };

static void draw_level(void);
static void update_camera_fps(Camera3D *camera);
static void update_body(body_t *body, float rot, char side, char forward, bool jump_pressed, bool crouch_hold);

int examples_core_simple_fps(void) {
  const int SCREEN_WIDTH = 800;
  const int SCREEN_HEIGHT = 450;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [core] example - simple FPS");

  Camera3D camera = { 0 };
  camera.fovy = 60.0F;
  camera.projection = CAMERA_PERSPECTIVE;
  camera.position = (Vector3){
    player.position.x,
    player.position.y + (BOTTOM_HEIGHT + head_lerp),
    player.position.z,
  };

  update_camera_fps(&camera);

  DisableCursor();

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    Vector2 mouse_delta = GetMouseDelta();
    look_rotation.x -= mouse_delta.x * sensivity.x;
    look_rotation.y += mouse_delta.y * sensivity.y;

    char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    bool crouching = IsKeyDown(KEY_LEFT_CONTROL);

    update_body(&player, look_rotation.x, sideway, forward, IsKeyPressed(KEY_SPACE), crouching);

    float delta = GetFrameTime();
    head_lerp = Lerp(head_lerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 20.0F * delta);
    camera.position = (Vector3){
      player.position.x,
      player.position.y + (BOTTOM_HEIGHT + head_lerp),
      player.position.z,
    };

    if (player.is_grounded && ((forward != 0) || (sideway != 0))) {
      head_timer += delta * 3.0F;
      walk_lerp = Lerp(walk_lerp, 1.0F, 10.0F * delta);
      camera.fovy = Lerp(camera.fovy, 55.0F, 10.0F * delta);
    }
    else {
      walk_lerp = Lerp(walk_lerp, 0.0F, 10.0F * delta);
      camera.fovy = Lerp(camera.fovy, 60.0F, 5.0F * delta);
    }

    lean.x = Lerp(lean.x, sideway * 0.02F, 10.0F * delta);
    lean.y = Lerp(lean.y, forward * 0.015F, 10.0F * delta);

    update_camera_fps(&camera);

    BeginDrawing();

      ClearBackground(RAYWHITE);

      BeginMode3D(camera);
        draw_level();
      EndMode3D();

      DrawRectangle(5, 5, 330, 75, Fade(SKYBLUE, 0.5f));
      DrawRectangleLines(5, 5, 330, 75, BLUE);

      DrawText("Camera controls:", 15, 15, 10, BLACK);
      DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
      DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
      DrawText(TextFormat("- Velocity Len: (%06.3f)", Vector2Length((Vector2){ player.velocity.x, player.velocity.z })), 15, 60, 10, BLACK);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}

void update_body(body_t *body, float rot, char side, char forward, bool jump_pressed, bool crouch_hold) {
  Vector2 input = (Vector2){ (float)side, (float)-forward };

  #ifdef NORMALIZE_INPUT
    // Slow down diagonal movements
    if ((side != 0) && (forward != 0)) {
      input = Vector2Normalize(input);
    }
  #endif

  float delta = GetFrameTime();

  if (!body->is_grounded) {
    body->velocity.y -= GRAVITY * delta;
  }

  if (body->is_grounded && jump_pressed) {
    body->velocity.y = JUMP_FORCE;
    body->is_grounded = false;
  }

  Vector3 front = (Vector3){ sinf(rot), 0.F, cosf(rot) };
  Vector3 right = (Vector3){ cosf(-rot), 0.F, sinf(-rot) };

  Vector3 desired_direction = (Vector3){ (input.x * right.x) + (input.y * front.x), 0.F, (input.x * right.z) + (input.y * front.z) };
  body->direction = Vector3Lerp(body->direction, desired_direction, CONTROL * delta);

  float decel = body->is_grounded ? FRICTION : AIR_DRAG;
  Vector3 horizontal_velocity = (Vector3){ body->velocity.x * decel, 0.F, body->velocity.z * decel };

  float horizontal_velocity_length = Vector3Length(horizontal_velocity);

  if (horizontal_velocity_length < (MAX_SPEED * 0.01F)) {
    horizontal_velocity = (Vector3){ 0 };
  }

  float speed = Vector3DotProduct(horizontal_velocity, body->direction);

  float max_speed = (crouch_hold ? CROUCH_SPEED : MAX_SPEED);
  float accel = Clamp(max_speed - speed, 0.F, MAX_ACCEL * delta);

  horizontal_velocity.x += body->direction.x * accel;
  horizontal_velocity.z += body->direction.z * accel;

  body->velocity.x = horizontal_velocity.x;
  body->velocity.z = horizontal_velocity.z;

  body->position.x += body->velocity.x * delta;
  body->position.y += body->velocity.y * delta;
  body->position.z += body->velocity.z * delta;

  if (body->position.y <= 0.F) {
    body->position.y = 0.F;
    body->velocity.y = 0.F;
    body->is_grounded = true;
  }
}

void update_camera_fps(Camera3D *camera) {
  const Vector3 UP = (Vector3){ 0.0F, 1.0F, 0.0F };
  const Vector3 TARGET_OFFSET = (Vector3){ 0.0F, 0.0F, -1.0F };

  Vector3 yaw = Vector3RotateByAxisAngle(TARGET_OFFSET, UP, look_rotation.x);

  float max_angle_up = Vector3Angle(UP, yaw);
  max_angle_up -= 0.001F; // Avoid numerical errors
  if (-look_rotation.y > max_angle_up) {
    look_rotation.y = -max_angle_up;
  }

  float max_angle_down = Vector3Angle(Vector3Negate(UP), yaw);
  max_angle_down *= -1.0F; // downwards angle is negative
  max_angle_down += 0.001F; // Avoid numerical errors
  if (-look_rotation.y < max_angle_down) {
    look_rotation.y = -max_angle_down;
  }

  Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, UP));

  float pitch_angle = -look_rotation.y - lean.y;
  pitch_angle = Clamp(pitch_angle, (-PI / 2) + 0.0001F, (PI / 2) - 0.0001F);
  Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitch_angle);

  float head_sin = sinf(head_timer * PI);
  float head_cos = cosf(head_timer * PI);
  const float STEP_ROTATION = 0.01F;
  camera->up = Vector3RotateByAxisAngle(UP, pitch, (head_sin * STEP_ROTATION) + lean.x);

  const float BOB_SIDE = 0.1F;
  const float BOB_UP = 0.15F;
  Vector3 bobbing = Vector3Scale(right, head_sin * BOB_SIDE);
  bobbing.y = fabsf(head_cos * BOB_UP);

  camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walk_lerp));
  camera->target = Vector3Add(camera->position, pitch);
}

void draw_level(void) {
  const int FLOOR_EXTENT = 25;
  const float TILE_SIZE = 5.0F;
  const Color TILE_COLOR = (Color){ 150, 200, 200, 255 };

  for (int yy = -FLOOR_EXTENT; yy < FLOOR_EXTENT; yy++) {
    for (int xx = -FLOOR_EXTENT; xx < FLOOR_EXTENT; xx++) {
      if (yy & 1 && xx & 1) {
        DrawPlane((Vector3){ xx * TILE_SIZE, 0.0F, yy * TILE_SIZE }, (Vector2){ TILE_SIZE, TILE_SIZE }, TILE_COLOR);
      }
      else if (!(yy & 1) && !(xx & 1)) {
        DrawPlane((Vector3){ xx * TILE_SIZE, 0.0F, yy * TILE_SIZE }, (Vector2){ TILE_SIZE, TILE_SIZE }, LIGHTGRAY);
      }
    }
  }

  const Vector3 TOWER_SIZE = (Vector3){ 16.0F, 32.0F, 16.0F };
  const Color TOWER_COLOR = (Color){ 150, 200, 200, 255 };

  Vector3 tower_pos = (Vector3){ 16.0F, 16.0F, 16.0F };
  DrawCubeV(tower_pos, TOWER_SIZE, TOWER_COLOR);
  DrawCubeWiresV(tower_pos, TOWER_SIZE, TOWER_COLOR);

  tower_pos.x *= -1;
  DrawCubeV(tower_pos, TOWER_SIZE, TOWER_COLOR);
  DrawCubeWiresV(tower_pos, TOWER_SIZE, TOWER_COLOR);

  tower_pos.z *= -1;
  DrawCubeV(tower_pos, TOWER_SIZE, TOWER_COLOR);
  DrawCubeWiresV(tower_pos, TOWER_SIZE, TOWER_COLOR);

  tower_pos.x *= -1;
  DrawCubeV(tower_pos, TOWER_SIZE, TOWER_COLOR);
  DrawCubeWiresV(tower_pos, TOWER_SIZE, TOWER_COLOR);

  // Red sun
  DrawSphere((Vector3){ 300.0F, 300.0F, 0.0F }, 100.0F, (Color){ 255, 0, 0, 255 });
}