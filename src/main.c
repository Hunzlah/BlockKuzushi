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

typedef struct
{
  vec2_t position;
  vec2_t velocity;
  int size;
  float maxSpeed;
  color_t color;
}ball_t;

typedef struct 
{
  int health;
  vec2_t position;
  vec2_t size;
}BlockStats;

typedef struct
{
  vec2_t gridSize;
  vec2_t blockSize;
}GridBlocks;

typedef struct
{
  vec2_t screenSize;
  float boundsSize;
  color_t color;
}ScreenBounds;

color_t GetColorByHp(int hp){
  color_t result;
  switch (hp)
  {
  case 1:
    result = GREEN;
    break;
  
  default:
  result = BLUE;
    break;
  }
  return result;
}



int main() {

  vec2_t playerSize = {100, 20};
  ScreenBounds screenBounds = {
    .screenSize = {800, 600},
    .boundsSize = 10,
    .color = WHITE
  };
  player_t player = {.size = playerSize,
                     .position = (vec2_t){400, screenBounds.screenSize.y - playerSize.y},
                     .color = GREEN,
                     .max_speed = 300};

  ball_t ball = {
    .position = player.position,
    .size = 10,
    .maxSpeed = 300,
    .velocity = {1,-1},
    .color = RED
  };

  GridBlocks gridBlocks = {
    .blockSize = {50, 20},
    .gridSize = {screenBounds.screenSize.x/50, 10}
  };

  BlockStats bricks[16][10] = {0};
  for (int i = 0; i < gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; i < gridBlocks.gridSize.y; i++)
    {
      bricks[i][j].health = GetRandomValue(1, 5);
      bricks[i][j].position = (vec2_t){i * gridBlocks.blockSize.x, j * gridBlocks.blockSize.y};
      bricks[i][j].size = gridBlocks.blockSize;
    }
    
  }
  

  bool isPaused = false;
  bool isGameOver = false;

  InitWindow(screenBounds.screenSize.x, screenBounds.screenSize.y, "Block Kuzushi");
  while (!WindowShouldClose()) {
    
    while (!isGameOver)
    {
      float delta_time = GetFrameTime();
    //vec2_t mouse = GetMousePosition();

    player.velocity = (vec2_t){0.0f, 0.0f};

    if (IsKeyDown(KEY_A) && player.position.x > 0) {
      player.velocity.x = -1;
    } else if (IsKeyDown(KEY_D) && (player.position.x + player.size.x) < screenBounds.screenSize.x) {
      player.velocity.x = 1;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    
    for (int i = 0; i < gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; i < gridBlocks.gridSize.y; i++)
    {
      if(bricks[i][j].health > 0){
        DrawRectangle(i*gridBlocks.gridSize.x, j*gridBlocks.gridSize.y, 
        gridBlocks.blockSize.x, gridBlocks.blockSize.y, GetColorByHp(bricks[i][j].health));
        DrawText("2", bricks[i][j].position.x, bricks[i][j].position.y, 10, WHITE);
        // Check Collision
        if (CheckCollisionCircleRec(ball.position, ball.size,
                (Rectangle){ bricks[i][j].position.x - bricks[i][j].size.x/2, bricks[i][j].position.y - bricks[i][j].size.y/2, 
                bricks[i][j].size.x, bricks[i][j].size.y}))
            {
              bricks[i][j].health--;
                if (ball.velocity.y > 0)
                {
                    ball.velocity.y *= -1;
                    ball.velocity.x = (ball.position.x - player.position.x)/(player.size.x/2);
                }
            }
      }
    }
    
  }



    // Player
    player.velocity = Vector2Scale(Vector2Normalize(player.velocity), player.max_speed * delta_time);
    player.position = Vector2Add(player.position, player.velocity);
    DrawRectangle(player.position.x, player.position.y - screenBounds.boundsSize, player.size.x, player.size.y, player.color);


    // Ball
    if(ball.position.x >= screenBounds.screenSize.x - ball.size || 
    ball.position.x <= ball.size){
      ball.velocity.x *= -1;
    }
    if(ball.position.y <= ball.size){
      ball.velocity.y *= -1;
    }
    if(ball.position.y >= screenBounds.screenSize.y - ball.size){
      // Game Over Condition
      //ball.velocity.y *= -1;
      isGameOver = true;
      break;
    }
    ball.position.x += ball.velocity.x * ball.maxSpeed * delta_time;
    ball.position.y += ball.velocity.y * ball.maxSpeed * delta_time;
    
    DrawCircle(ball.position.x, ball.position.y, ball.size, ball.color);

    // Detect Collision between player and ball

    if (CheckCollisionCircleRec(ball.position, ball.size,
                (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ball.velocity.y > 0)
                {
                    ball.velocity.y *= -1;
                    ball.velocity.x = (ball.position.x - player.position.x)/(player.size.x/2);
                }
            }
  
    EndDrawing();
    }



    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("Press Enter to Restart", screenBounds.screenSize.x/2, screenBounds.screenSize.y/3, 20, BLACK);

    DrawText("GAME OVER", screenBounds.screenSize.x/2, screenBounds.screenSize.y/2, 20, BLACK);
    EndDrawing();
    
  }
  return 0;
}