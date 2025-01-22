#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

//#define MAX_LIVES "3"

typedef Vector2 vec2_t;
typedef Color color_t;

typedef struct
{
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
} ball_t;

typedef struct
{
  int health;
  vec2_t position;
  vec2_t size;
} BlockStats;

typedef struct
{
  vec2_t gridSize;
  vec2_t blockSize;
  vec2_t spacing;
} GridBlocks;

typedef struct
{
  vec2_t screenSize;
  float boundsSize;
  color_t color;
} ScreenBounds;

color_t GetColorByHp(int hp)
{
  color_t result;
  switch (hp)
  {
  case 1:
    result = GREEN;
    break;
  case 2:
    result = ORANGE;
    break;
  case 3:
    result = YELLOW;
    break;
  case 4:
    result = BROWN;
    break;
  case 5:
    result = VIOLET;
    break;

  default:
    result = BLUE;
    break;
  }
  return result;
}

int main()
{

  bool isBallSpawned = false;
  int livesCounter = 5;
  int currentScore = 0;
  int scoreMultiplier = 1;
  char *maxLives = "/5";

  vec2_t playerSize = {100, 20};
  ScreenBounds screenBounds = {
      .screenSize = {800, 600},
      .boundsSize = 10,
      .color = WHITE};
  player_t player = {.size = playerSize,
                     .position = (vec2_t){400, screenBounds.screenSize.y - playerSize.y},
                     .color = GREEN,
                     .max_speed = 500};

  ball_t ball = {
      .position = player.position,
      .size = 10,
      .maxSpeed = 1000,
      .velocity = {1, -1},
      .color = RED};

  GridBlocks gridBlocks = {
      .blockSize = {75, 40},
      .gridSize = {8, 4},
      .spacing = {10, 10}};

  BlockStats bricks[(int)gridBlocks.gridSize.x][(int)gridBlocks.gridSize.y];
  for (int i = 0; i < gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; j < gridBlocks.gridSize.y; j++)
    {
      bricks[i][j] = (BlockStats){
          .health = GetRandomValue(1, 5),
          .position = (vec2_t){i * gridBlocks.blockSize.x + gridBlocks.blockSize.x + (gridBlocks.spacing.x * i),
                               j * gridBlocks.blockSize.y + gridBlocks.blockSize.y + (gridBlocks.spacing.y * j)},
          .size = (vec2_t){gridBlocks.blockSize.x, gridBlocks.blockSize.y}};
    }
  }

  bool isPaused = false;
  bool isGameOver = false;

  InitWindow(screenBounds.screenSize.x, screenBounds.screenSize.y, "Block Kuzushi");
  while (!WindowShouldClose())
  {

    while (!isGameOver)
    {
      float delta_time = GetFrameTime();
      // vec2_t mouse = GetMousePosition();

      player.velocity = (vec2_t){0.0f, 0.0f};

      if (IsKeyDown(KEY_A) && player.position.x > 0)
      {
        player.velocity.x = -1;
      }
      else if (IsKeyDown(KEY_D) && (player.position.x + player.size.x) < screenBounds.screenSize.x)
      {
        player.velocity.x = 1;
      }

      BeginDrawing();
      ClearBackground(BLACK);

      for (int y = 0; y < gridBlocks.gridSize.x; y++)
      {
        for (int x = 0; x < gridBlocks.gridSize.y; x++)
        {
          if (bricks[y][x].health > 0)
          {
            DrawRectangle(bricks[y][x].position.x, bricks[y][x].position.y,
                          bricks[y][x].size.x, bricks[y][x].size.y, GetColorByHp(bricks[y][x].health));

            char str[10];
            sprintf(str, "%i", bricks[y][x].health);
            DrawText(str, bricks[y][x].position.x + bricks[y][x].size.x / 2,
                     bricks[y][x].position.y + bricks[y][x].size.y / 2, 15, BLACK);

            if (CheckCollisionCircleRec(ball.position, ball.size,
                                        (Rectangle){.x = bricks[y][x].position.x, .y = bricks[y][x].position.y, .width = bricks[y][x].size.x, .height = bricks[y][x].size.y}))
            {
              bricks[y][x].health--;
              ball.velocity.y *= -1;
              ball.velocity.x *= -1;
              currentScore += 5 * scoreMultiplier;
              scoreMultiplier = scoreMultiplier + 1 <= 5 ? scoreMultiplier + 1 : 5;
            }
          }
        }
      }

      // Player
      player.velocity = Vector2Scale(Vector2Normalize(player.velocity), player.max_speed * delta_time);
      player.position = Vector2Add(player.position, player.velocity);
      DrawRectangle(player.position.x, player.position.y - screenBounds.boundsSize, player.size.x, player.size.y, player.color);

      // Ball
      if (isBallSpawned)
      {
        if (ball.position.x >= screenBounds.screenSize.x - ball.size ||
            ball.position.x <= ball.size)
        {
          ball.velocity.x *= -1;
        }
        if (ball.position.y <= ball.size)
        {
          ball.velocity.y *= -1;
        }
        if (ball.position.y >= (screenBounds.screenSize.y - ball.size) && livesCounter > 0)
        {
          // Game Over Condition
          //isGameOver = true;
          //break;
          livesCounter--;
          isBallSpawned = false;
          scoreMultiplier = 1;
        }
        else if(livesCounter <= 0)
        {
          isGameOver = true;
          break;
        }

        ball.velocity = Vector2Scale(Vector2Normalize(ball.velocity), ball.maxSpeed * delta_time);
        ball.position = Vector2Add(ball.position, ball.velocity);
        //ball.position.x += ball.velocity.x * ball.maxSpeed * delta_time;
        //ball.position.y += ball.velocity.y * ball.maxSpeed * delta_time;

        DrawCircle(ball.position.x, ball.position.y, ball.size, ball.color);

        // Detect Collision between player and ball

        if (CheckCollisionCircleRec(ball.position, ball.size,
                                    (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y}))
        {
          if (ball.velocity.y > 0)
          {
            ball.velocity.y *= -1;
            ball.velocity.x = (ball.position.x - player.position.x) / (player.size.x / 2);
          }
        }
      }
      else if(IsKeyDown(KEY_SPACE))
      {
        isBallSpawned = true;
        ball.position = (vec2_t){player.position.x + player.size.x/2, player.position.y};
        ball.velocity = (vec2_t){player.velocity.x,-1};
        livesCounter--;
      }

      char livesCounterStr[10];
      sprintf(livesCounterStr, "%i", livesCounter);
      char livesString[256];
      snprintf(livesString, sizeof(livesString), "%s%s", livesCounterStr, maxLives);

      DrawText(livesString, 10, 10, 10, GREEN);

      DrawText("Score: ", screenBounds.screenSize.x/2 - 40, 10, 15, GREEN);

      char scoreString[256];
      sprintf(scoreString, "%i", currentScore);
      DrawText(scoreString, screenBounds.screenSize.x/2 + 40, 10, 15, GREEN);

      char scoreMultiplierString[256];
      sprintf(scoreMultiplierString, "%i", scoreMultiplier);
      char scoreMultiplierStringX[256];
      snprintf(scoreMultiplierStringX, sizeof(scoreMultiplierStringX), "%s%s", "x", scoreMultiplierString);
      DrawText(scoreMultiplierStringX, screenBounds.screenSize.x/2 + 80, 10, 15, GOLD);


      EndDrawing();
    }

    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("Press Enter to Restart", screenBounds.screenSize.x / 2, screenBounds.screenSize.y / 3, 20, BLACK);

    DrawText("GAME OVER", screenBounds.screenSize.x / 2, screenBounds.screenSize.y / 2, 20, BLACK);
    EndDrawing();
  }
  return 0;
}