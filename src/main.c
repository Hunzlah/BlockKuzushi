#include "raylib.h"
#include "raymath.h"

#define MAX_BULLETS 128

typedef Vector2 vec2_t;
typedef Color color_t;

typedef struct {
  vec2_t position;
  vec2_t velocity;
  vec2_t facing;
  vec2_t size;
  color_t color;
  float max_speed; // pixels / second
} player_t;

int main() {

  const int playerWidth = 120;
  const int playerHeight = 20;
  const int screenWidth = 800;
  const int screenHeight = 600;
  player_t player = {.position = (vec2_t){400, screenHeight - playerHeight},
                     .size = (vec2_t){100, 20},
                     .color = GREEN,
                     .max_speed = 300};

  InitWindow(screenWidth, screenHeight, "Block Kuzushi");
  while (!WindowShouldClose()) {

    float delta_time = GetFrameTime();
    vec2_t mouse = GetMousePosition();

    player.velocity = (vec2_t){0.0f, 0.0f};
/*
    if (IsKeyDown(KEY_W)) {
      player.velocity.y = -1;
    } else if (IsKeyDown(KEY_S)) {
      player.velocity.y = 1;
    }
*/
    if (IsKeyDown(KEY_A) && player.position.x > 0) {
      player.velocity.x = -1;
    } else if (IsKeyDown(KEY_D) && (player.position.x + playerWidth) < screenWidth) {
      player.velocity.x = 1;
    }

    BeginDrawing();

    ClearBackground(BLACK);

    player.velocity = Vector2Scale(Vector2Normalize(player.velocity),
                                   player.max_speed * delta_time);

    player.position = Vector2Add(player.position, player.velocity);

    DrawRectangle(player.position.x, player.position.y, playerWidth, playerHeight, player.color);

    EndDrawing();
  }
  return 0;
}