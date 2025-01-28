#include "stdlib.h"
#include "stdio.h"
#include "raylib.h"
#include "raymath.h"


typedef Vector2 vec2_t;
typedef Color color_t;

typedef struct
{
  vec2_t position;
  vec2_t velocity;
  vec2_t facing;
  vec2_t size;
  color_t color;
  float max_speed;
} player_t;

typedef struct
{
  vec2_t position;
  vec2_t velocity;
  int size;
  int collisionSize;
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
enum GameState
{
  Menu,
  InGame,
  GameOver,
  GamePassed
};
enum GamePlayState
{
  ReadyToSpawn,
  BallSpawned
};
enum MenuScreen{
  Main, Credits
};

typedef struct
{
  vec2_t position;
  char* name;
}MenuOption;
typedef struct
{
  int currentOption;
  int optionsCount;
  vec2_t selectionSize;
  enum MenuScreen currentScreen;
  vec2_t defaultOptionPos;
  MenuOption* menuOptions;
}MenuData;


typedef struct
{
  bool isBallSpawned;
  bool isGameOver;
  int livesCounter;
  int currentScore;
  int scoreMultiplier;
  char *maxLives;
  float delta_time;
} GamePlayStats;

typedef struct
{
  MenuData menuData;
  GamePlayStats gamePlayStats;
  enum GameState currentGameState;
  enum GamePlayState currentGamePlayState;
  ScreenBounds screenBounds;
  int currentLevel;
  player_t player;
  ball_t ball;
  GridBlocks gridBlocks;
  BlockStats **bricks;

} GameData;

void SetLevel(GameData *, int);
void Init(GameData *);
void HandleMenu(GameData *);
void HandleGamePlay(GameData *);
void HandleGameOver(GameData *);
void HandleBricks(GameData *);
void HandleBall(GameData *);
void HandlePlayer(GameData *);
void RenderGamePlayScreen(GameData *);
void HandleGamePass(GameData *);
bool CheckPassCondition(GameData *);
int main()
{

  //GameData *gameData = malloc(sizeof(GameData*));
  GameData *gameData;
  gameData = malloc(sizeof *gameData);
  SetLevel(gameData, 1);
  Init(gameData);
  
  InitWindow(gameData->screenBounds.screenSize.x, gameData->screenBounds.screenSize.y, "Block Kuzushi");
  //InitWindow(800, 600, "Block Kuzushi");
  while (!WindowShouldClose())
  {
    switch (gameData->currentGameState)
    {
    case 0:
      HandleMenu(gameData);
      break;
    case 1:
      HandleGamePlay(gameData);
      break;
    case 2:
      HandleGameOver(gameData);
      break;
    case 3:
      HandleGamePass(gameData);

    default:
      break;
    }
  }
  return 0;
}

void GenerateLevel(GameData *gameData)
{
  for (int i = 0; i < gameData->gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; j < gameData->gridBlocks.gridSize.y; j++)
    {
      gameData->bricks[i][j] = (BlockStats){
          .health = GetRandomValue(0, 5),
          .position = (vec2_t){i * gameData->gridBlocks.blockSize.x + gameData->gridBlocks.blockSize.x + (gameData->gridBlocks.spacing.x * i),
                               j * gameData->gridBlocks.blockSize.y + gameData->gridBlocks.blockSize.y + (gameData->gridBlocks.spacing.y * j)},
          .size = (vec2_t){gameData->gridBlocks.blockSize.x, gameData->gridBlocks.blockSize.y}};
    }
  }
}
void SetLevel(GameData *gameData, int level){
  gameData->currentLevel = level;
}
void Init(GameData *gameData)
{

  gameData->gamePlayStats = (GamePlayStats){
    .isBallSpawned = false,
    .livesCounter = 5,
    .currentScore = 0,
    .scoreMultiplier = 1,
    .maxLives = "/5",
    .delta_time = 120
  };
  
  gameData->currentGameState = 0;
  
  vec2_t playerSize = {100, 20};
  gameData->screenBounds = (ScreenBounds){
      .screenSize = (vec2_t){800, 600},
      .boundsSize = 10,
      .color = WHITE};
  
  gameData->menuData = (MenuData)
  {
    .currentOption = 0,
    .currentScreen = 0,
    .optionsCount = 3,
    .selectionSize = {100, 25},
    .defaultOptionPos = {gameData->screenBounds.screenSize.x/2, gameData->screenBounds.screenSize.y/2 - 100}
  };
  
  gameData->menuData.menuOptions = malloc(sizeof(MenuOption) * gameData->menuData.optionsCount);

  gameData->menuData.menuOptions[0] = (MenuOption){
      .name = "Play",
      .position = gameData->menuData.defaultOptionPos};
  gameData->menuData.menuOptions[1] = (MenuOption){
      .name = "Credits",
      .position = (vec2_t){
          gameData->menuData.defaultOptionPos.x, gameData->menuData.defaultOptionPos.y + 50}};
  gameData->menuData.menuOptions[2] = (MenuOption){
      .name = "Exit",
      .position = (vec2_t){
          gameData->menuData.defaultOptionPos.x, gameData->menuData.defaultOptionPos.y + 100}};

  gameData->player = (player_t){.size = {100,20},
                                .position = {400, gameData->screenBounds.screenSize.y - playerSize.y},
                                .color = GREEN,
                                .max_speed = 1000
                                };

  gameData->ball = (ball_t){
      .position = gameData->player.position,
      .size = 10,
      .collisionSize = 5,
      .maxSpeed = 800,
      .velocity = {1, -1},
      .color = RED};

  gameData->gridBlocks = (GridBlocks){
      .blockSize = {75, 40},
      //.gridSize = {8, 4},
      .gridSize = {2, 1+gameData->currentLevel <= 6 ? 1+gameData->currentLevel : 6},
      .spacing = {2, 2}};


  gameData->bricks = (BlockStats**)malloc(gameData->gridBlocks.gridSize.x * sizeof(BlockStats*));
  for (int i = 0; i < gameData->gridBlocks.gridSize.x; i++)
  {
    gameData->bricks[i] = (BlockStats*)malloc(gameData->gridBlocks.gridSize.y * sizeof(BlockStats));
  }


  for (int i = 0; i < gameData->gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; j < gameData->gridBlocks.gridSize.y; j++)
    {
      gameData->bricks[i][j] = (BlockStats){
          .health = GetRandomValue(0, 1+gameData->currentLevel < 20 ? 1+gameData->currentLevel : 20),
          .position = (vec2_t){i * gameData->gridBlocks.blockSize.x + gameData->gridBlocks.blockSize.x + (gameData->gridBlocks.spacing.x * i),
                               j * gameData->gridBlocks.blockSize.y + gameData->gridBlocks.blockSize.y + (gameData->gridBlocks.spacing.y * j)},
          .size = (vec2_t){gameData->gridBlocks.blockSize.x, gameData->gridBlocks.blockSize.y}};
    }
  }
}
void HandleMenu(GameData *gameData)
{
  BeginDrawing();
  ClearBackground(WHITE);

  switch (gameData->menuData.currentScreen)
  {
  case 0:
    if (IsKeyReleased(KEY_UP))
    {
      gameData->menuData.currentOption = (gameData->menuData.currentOption - 1) % gameData->menuData.optionsCount;
    }
    else if (IsKeyReleased(KEY_DOWN))
    {
      gameData->menuData.currentOption = (gameData->menuData.currentOption + 1) % gameData->menuData.optionsCount;
    }
    else if (IsKeyReleased(KEY_ENTER))
    {
      switch (gameData->menuData.currentOption)
      {
      case 0:

        gameData->currentGameState = 1;
        break;
      case 1:
        gameData->menuData.currentScreen = 1;
        break;
      case 2:

        break;
      default:
        break;
      }
    }

    MenuOption currentOption = gameData->menuData.menuOptions[gameData->menuData.currentOption];
    DrawRectangle(currentOption.position.x, currentOption.position.y, 
    gameData->menuData.selectionSize.x, gameData->menuData.selectionSize.y, GREEN);

    for (int i = 0; i < gameData->menuData.optionsCount; i++)
    {
      MenuOption option = gameData->menuData.menuOptions[i];
      DrawText(option.name, option.position.x, option.position.y, 25, BLACK);
    }
    

    break;
  case 1:
    if (IsKeyReleased(KEY_ENTER))
      gameData->menuData.currentScreen = 0;
    DrawText("Hunzlah Bin Saghir", gameData->screenBounds.screenSize.x / 2 - 100, gameData->screenBounds.screenSize.y / 2 - 100, 40, BLACK);
    DrawText("Press Enter to go back", gameData->screenBounds.screenSize.x / 2 - 100, gameData->screenBounds.screenSize.y / 2 - 50, 20, BLACK);
  default:
    break;
  }

  EndDrawing();
}
void HandleGamePlay(GameData *gameData)
{
  gameData->gamePlayStats.delta_time = GetFrameTime();

  BeginDrawing();
  ClearBackground(BLACK);

  HandleBricks(gameData);

  HandleBall(gameData);
  HandlePlayer(gameData);
  RenderGamePlayScreen(gameData);
  if (gameData->gamePlayStats.livesCounter <= 0)
  {
    gameData->currentGameState = 2;
  }
  EndDrawing();
}
void HandleGameOver(GameData *gameData)
{
  BeginDrawing();
  ClearBackground(GRAY);

  if (IsKeyDown(KEY_ENTER))
  {
    // gameData.gamePlayStats.isGameOver = false;
    gameData->gamePlayStats.scoreMultiplier = 1;
    gameData->gamePlayStats.currentScore = 0;
    gameData->gamePlayStats.livesCounter = 5;
    gameData->gamePlayStats.isBallSpawned = false;
    gameData->currentGameState = 1;
  }

  DrawText("GAME OVER", gameData->screenBounds.screenSize.x / 2 - 100, gameData->screenBounds.screenSize.y / 2 - 100, 40, BLACK);
  DrawText("Press Enter to Restart", gameData->screenBounds.screenSize.x / 2 - 100, gameData->screenBounds.screenSize.y / 2 - 50, 20, BLACK);

  EndDrawing();
}
void HandleBricks(GameData *gameData)
{
  for (int y = 0; y < gameData->gridBlocks.gridSize.x; y++)
  {
    for (int x = 0; x < gameData->gridBlocks.gridSize.y; x++)
    {
      if (gameData->bricks[y][x].health > 0)
      {
        DrawRectangle(gameData->bricks[y][x].position.x, gameData->bricks[y][x].position.y,
                      gameData->bricks[y][x].size.x, gameData->bricks[y][x].size.y, GetColorByHp(gameData->bricks[y][x].health));

        char str[10];
        sprintf(str, "%i", gameData->bricks[y][x].health);
        DrawText(str, gameData->bricks[y][x].position.x + gameData->bricks[y][x].size.x / 2,
                 gameData->bricks[y][x].position.y + gameData->bricks[y][x].size.y / 2, 15, BLACK);

        if (CheckCollisionCircleRec(gameData->ball.position, gameData->ball.collisionSize,
                                    (Rectangle){.x = gameData->bricks[y][x].position.x, .y = gameData->bricks[y][x].position.y, .width = gameData->bricks[y][x].size.x, .height = gameData->bricks[y][x].size.y}))
        {
          gameData->bricks[y][x].health--;
          gameData->ball.velocity.y *= -1;
          //gameData->ball.velocity.x += GetRandomValue(-5, 5);
          gameData->gamePlayStats.currentScore += 5 * gameData->gamePlayStats.scoreMultiplier;
          gameData->gamePlayStats.scoreMultiplier = gameData->gamePlayStats.scoreMultiplier + 1 <= 5 ? gameData->gamePlayStats.scoreMultiplier + 1 : 5;
        
          if(CheckPassCondition(gameData)){
            gameData->currentGameState = 3;
            break;
          }
        
        }
      }
    }
    if(gameData->currentGameState != 1) break;
  }
}
void HandleBall(GameData *gameData)
{
  // Ball
  if (gameData->gamePlayStats.isBallSpawned)
  {
    if (gameData->ball.position.x >= gameData->screenBounds.screenSize.x - gameData->ball.collisionSize ||
        gameData->ball.position.x <= gameData->ball.collisionSize)
    {
      gameData->ball.velocity.x *= -1;
    }
    if (gameData->ball.position.y <= gameData->ball.collisionSize)
    {
      gameData->ball.velocity.y *= -1;
    }
    if (gameData->ball.position.y >= (gameData->screenBounds.screenSize.y - gameData->ball.collisionSize) && gameData->gamePlayStats.livesCounter >= 0)
    {
      // Game Over Condition
      gameData->gamePlayStats.scoreMultiplier = 1;

      gameData->ball.position = (vec2_t){0, 0};

      gameData->gamePlayStats.isBallSpawned = false;
    }

    gameData->ball.velocity = Vector2Scale(Vector2Normalize(gameData->ball.velocity), gameData->ball.maxSpeed * gameData->gamePlayStats.delta_time);
    gameData->ball.position = Vector2Add(gameData->ball.position, gameData->ball.velocity);

    // Detect Collision between player and ball

    if (CheckCollisionCircleRec(gameData->ball.position, gameData->ball.collisionSize,
                                (Rectangle){gameData->player.position.x, gameData->player.position.y, gameData->player.size.x, gameData->player.size.y}))
    {
      if (gameData->ball.velocity.y > 0)
      {
        gameData->ball.velocity.y *= -1;
        gameData->ball.velocity.x = gameData->ball.velocity.x + gameData->player.velocity.x;
        //gameData->ball.velocity.x = (gameData->ball.position.x - 
        //(gameData->player.position.x + (gameData->player.size.x/2)));
        // /(gameData->player.size.x);
      }
    }
  }
  else if (IsKeyDown(KEY_SPACE))
  {
    gameData->gamePlayStats.isBallSpawned = true;
    gameData->ball.position = (vec2_t){gameData->player.position.x + gameData->player.size.x / 2, gameData->player.position.y};
    gameData->ball.velocity = (vec2_t){gameData->player.velocity.x, -1};
    gameData->gamePlayStats.livesCounter--;
  }
}
void HandlePlayer(GameData *gameData)
{
  gameData->player.velocity = (vec2_t){0.0f, 0.0f};

  if (IsKeyDown(KEY_A) && gameData->player.position.x > 0)
  {
    gameData->player.velocity.x = -1;
  }
  else if (IsKeyDown(KEY_D) && (gameData->player.position.x + gameData->player.size.x) < gameData->screenBounds.screenSize.x)
  {
    gameData->player.velocity.x = 1;
  }

  gameData->player.velocity = Vector2Scale(Vector2Normalize(gameData->player.velocity), gameData->player.max_speed * gameData->gamePlayStats.delta_time);
  gameData->player.position = Vector2Add(gameData->player.position, gameData->player.velocity);
}
void RenderGamePlayScreen(GameData *gameData)
{

  // Draw Player
  DrawRectangle(gameData->player.position.x, gameData->player.position.y - gameData->screenBounds.boundsSize, gameData->player.size.x, gameData->player.size.y, gameData->player.color);
  // Draw Ball
  if(gameData->gamePlayStats.isBallSpawned) DrawCircle(gameData->ball.position.x, gameData->ball.position.y, gameData->ball.size, gameData->ball.color);
  // Draw UI
  char livesCounterStr[10];
  sprintf(livesCounterStr, "%i", gameData->gamePlayStats.livesCounter);
  char livesString[256];
  snprintf(livesString, sizeof(livesString), "%s%s", livesCounterStr, gameData->gamePlayStats.maxLives);

  DrawText(livesString, 10, 10, 10, GREEN);

  DrawText("Score: ", gameData->screenBounds.screenSize.x / 2 - 40, 10, 15, GREEN);

  char scoreString[256];
  sprintf(scoreString, "%i", gameData->gamePlayStats.currentScore);
  DrawText(scoreString, gameData->screenBounds.screenSize.x / 2 + 40, 10, 15, GREEN);

  char scoreMultiplierString[256];
  sprintf(scoreMultiplierString, "%i", gameData->gamePlayStats.scoreMultiplier);
  char scoreMultiplierStringX[256];
  snprintf(scoreMultiplierStringX, sizeof(scoreMultiplierStringX), "%s%s", "x", scoreMultiplierString);
  DrawText(scoreMultiplierStringX, gameData->screenBounds.screenSize.x / 2 + 80, 10, 15, GOLD);
}
void HandleGamePass(GameData *gameData)
{
  BeginDrawing();
  ClearBackground(BLACK);
  
  // Draw Player
  DrawRectangle(gameData->player.position.x, gameData->player.position.y - gameData->screenBounds.boundsSize, 
  gameData->player.size.x, gameData->player.size.y, gameData->player.color);
  // Draw Ball
  if(gameData->gamePlayStats.isBallSpawned) 
    DrawCircle(gameData->ball.position.x, gameData->ball.position.y, gameData->ball.size, gameData->ball.color);
  
  if (IsKeyDown(KEY_ENTER))
  {

    int level = gameData->currentLevel+1;
    free(gameData->bricks);
    free(gameData);

    gameData = malloc(sizeof *gameData);
    SetLevel(gameData, level);
    Init(gameData);
  }

  DrawText("Level Passed", gameData->screenBounds.screenSize.x / 2 - 100, 
  gameData->screenBounds.screenSize.y / 2 - 100, 40, WHITE);
  DrawText("[ENTER] Next Level", gameData->screenBounds.screenSize.x / 2 - 100, 
  gameData->screenBounds.screenSize.y / 2 - 50, 20, WHITE);

  EndDrawing();
}
bool CheckPassCondition(GameData *gameData)
{
  bool isLevelPassed = true;
  for (int i = 0; i < gameData->gridBlocks.gridSize.x; i++)
  {
    for (int j = 0; j < gameData->gridBlocks.gridSize.y; j++)
    {
      if(gameData->bricks[i][j].health > 0)
      {
        isLevelPassed = false;
        break;
      }
    }
    if(!isLevelPassed) break;
  }
  return isLevelPassed;
}