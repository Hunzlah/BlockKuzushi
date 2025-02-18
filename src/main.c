#include "stdlib.h"
#include "stdio.h"
#include "raylib.h"
#include "raymath.h"


typedef Vector2 vec2_t;
typedef Color color_t;
typedef struct
{
  int highScore;
  int currentLevel;
}FileData;

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
  vec2_t position;
  vec2_t velocity;
  int size;
  int collisionSize;
  float maxSpeed;
  color_t color;
  bool isSpawned;
  bool isActive;
  float currentTime;
  float maxTime;
} powerup_t;

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
  int highScore;
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
  powerup_t powerUpAddLife;
  powerup_t powerUpIncreaseSize;
  GridBlocks gridBlocks;
  FileData *fileData;
  BlockStats **bricks;

} GameData;

Vector2 NormalizeVector2(Vector2);
Vector2 ScaleVector2(Vector2, float);
Vector2 AddVector2(Vector2 v1, Vector2 v2);

void SetLevel(GameData *, int);
void Init(GameData *);
void HandleMenu(GameData *);
void HandleGamePlay(GameData *);
void HandleGameOver(GameData *);
void HandleBricks(GameData *);
void HandlePowerUps(GameData *);
void HandleBall(GameData *);
void HandlePlayer(GameData *);
void RenderGamePlayScreen(GameData *);
void HandleGamePass(GameData *);
bool CheckPassCondition(GameData *);
void ReadFileData(FileData *);
void WriteFileData(FileData *, int, int);

int main()
{
  GameData *gameData;
  gameData = malloc(sizeof *gameData);
  gameData->fileData = malloc(sizeof(*gameData->fileData));
  SetLevel(gameData, 1);
  //WriteFileData(gameData->fileData, 0, 1);
  Init(gameData);
  
  InitWindow(gameData->screenBounds.screenSize.x, gameData->screenBounds.screenSize.y, "Block Kuzushi");
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

Vector2 NormalizeVector2(Vector2 v)
{
    Vector2 result = { 0 };
    float length = sqrtf((v.x*v.x) + (v.y*v.y));

    if (length > 0)
    {
        float ilength = 1.0f/length;
        result.x = v.x*ilength;
        result.y = v.y*ilength;
    }

    return result;
}
Vector2 ScaleVector2(Vector2 v, float scale)
{
    Vector2 result = { v.x*scale, v.y*scale };

    return result;
}
Vector2 AddVector2(Vector2 v1, Vector2 v2)
{
    Vector2 result = { v1.x + v2.x, v1.y + v2.y };

    return result;
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
  
  gameData->powerUpAddLife = (powerup_t){
    .position = gameData->player.position,
      .size = 10,
      .collisionSize = 5,
      .maxSpeed = 300,
      .velocity = {0, 1},
      .color = GOLD,
      .isSpawned = false
  };

  gameData->powerUpIncreaseSize = (powerup_t){
    .position = gameData->player.position,
      .size = 10,
      .collisionSize = 5,
      .maxSpeed = 300,
      .velocity = {0, 1},
      .color = GOLD,
      .isSpawned = false,
      .isActive = false,
      .maxTime = 5,
      .currentTime = 0
  };

  ReadFileData(gameData->fileData);

  gameData->gamePlayStats.highScore = gameData->fileData->highScore;
  gameData->currentLevel = gameData->fileData->currentLevel;

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
    .optionsCount = 4,
    .selectionSize = {100, 25},
    .defaultOptionPos = {gameData->screenBounds.screenSize.x/2, gameData->screenBounds.screenSize.y/2 - 100}
  };
  
  gameData->menuData.menuOptions = malloc(sizeof(MenuOption) * gameData->menuData.optionsCount);

  gameData->menuData.menuOptions[0] = (MenuOption){
      .name = "Play",
      .position = gameData->menuData.defaultOptionPos};
  gameData->menuData.menuOptions[1] = (MenuOption){
      .name = "Reset",
      .position = (vec2_t){
          gameData->menuData.defaultOptionPos.x, gameData->menuData.defaultOptionPos.y + 50}};
  gameData->menuData.menuOptions[2] = (MenuOption){
      .name = "Credits",
      .position = (vec2_t){
          gameData->menuData.defaultOptionPos.x, gameData->menuData.defaultOptionPos.y + 100}};
  gameData->menuData.menuOptions[3] = (MenuOption){
      .name = "Exit",
      .position = (vec2_t){
          gameData->menuData.defaultOptionPos.x, gameData->menuData.defaultOptionPos.y + 150}};

  gameData->player = (player_t){.size = {100,20},
                                .position = {400, gameData->screenBounds.screenSize.y - playerSize.y},
                                .color = GREEN,
                                .max_speed = 1000
                                };

  gameData->ball = (ball_t){
      .position = gameData->player.position,
      .size = 10,
      .collisionSize = 1,
      .maxSpeed = 1000,
      .velocity = {1, -1},
      .color = RED};

  gameData->gridBlocks = (GridBlocks){
      .blockSize = {75, 40},
      //.gridSize = {8, 4},
      .gridSize = {1 + gameData->currentLevel <= 8 ? 2 + gameData->currentLevel : 8, 
      1 + gameData->currentLevel <= 6 ? 1+gameData->currentLevel : 6},
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
    if (IsKeyReleased(KEY_UP) && gameData->menuData.currentOption > 0)
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
        WriteFileData(gameData->fileData, 0, 1);
        ReadFileData(gameData->fileData);
        break;
      case 2:
        gameData->menuData.currentScreen = 1;
        break;
      case 3:
        exit(0);
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
  HandlePowerUps(gameData);
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

          if (GetRandomValue(0,10) > 7 && !gameData->powerUpAddLife.isSpawned)
            {
              gameData->powerUpAddLife.position.x = gameData->bricks[y][x].position.x;
              gameData->powerUpAddLife.position.y = gameData->bricks[y][x].position.y;
              gameData->powerUpAddLife.isSpawned = true;
            }
            if (GetRandomValue(0,10) > 7 && !gameData->powerUpIncreaseSize.isSpawned && !gameData->powerUpIncreaseSize.isActive)
            {
              gameData->powerUpIncreaseSize.position.x = gameData->bricks[y][x].position.x;
              gameData->powerUpIncreaseSize.position.y = gameData->bricks[y][x].position.y;
              gameData->powerUpIncreaseSize.isSpawned = true;
            }
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
void HandlePowerUps(GameData *gameData){
  if(gameData->powerUpAddLife.isSpawned){
    if (gameData->powerUpAddLife.position.y >= (gameData->screenBounds.screenSize.y - gameData->ball.collisionSize))
    {
      gameData->powerUpAddLife.isSpawned = false;
    }

    gameData->powerUpAddLife.velocity = ScaleVector2(NormalizeVector2(gameData->powerUpAddLife.velocity), gameData->powerUpAddLife.maxSpeed * gameData->gamePlayStats.delta_time);
    gameData->powerUpAddLife.position = AddVector2(gameData->powerUpAddLife.position, gameData->powerUpAddLife.velocity);


    if (CheckCollisionCircleRec(gameData->powerUpAddLife.position, gameData->powerUpAddLife.collisionSize,
                                (Rectangle){gameData->player.position.x, gameData->player.position.y, gameData->player.size.x, gameData->player.size.y}))
    {
      gameData->gamePlayStats.livesCounter = gameData->gamePlayStats.livesCounter + 1 > 5 ? 5 : gameData->gamePlayStats.livesCounter + 1;
      gameData->powerUpAddLife.isSpawned = false;
    }
  }

  if(gameData->powerUpIncreaseSize.isSpawned && !gameData->powerUpIncreaseSize.isActive){
    if (gameData->powerUpIncreaseSize.position.y >= (gameData->screenBounds.screenSize.y - gameData->ball.collisionSize))
    {
      gameData->powerUpIncreaseSize.isSpawned = false;
    }

    gameData->powerUpIncreaseSize.velocity = ScaleVector2(NormalizeVector2(gameData->powerUpIncreaseSize.velocity), gameData->powerUpIncreaseSize.maxSpeed * gameData->gamePlayStats.delta_time);
    gameData->powerUpIncreaseSize.position = AddVector2(gameData->powerUpIncreaseSize.position, gameData->powerUpIncreaseSize.velocity);


    if (CheckCollisionCircleRec(gameData->powerUpIncreaseSize.position, gameData->powerUpIncreaseSize.collisionSize,
                                (Rectangle){gameData->player.position.x, gameData->player.position.y, gameData->player.size.x, gameData->player.size.y}))
    {
      gameData->gamePlayStats.livesCounter = gameData->gamePlayStats.livesCounter + 1 > 5 ? 5 : gameData->gamePlayStats.livesCounter + 1;
      gameData->powerUpIncreaseSize.isSpawned = false;
      gameData->powerUpIncreaseSize.isActive = true;
      gameData->player.size.x *= 2; 
    }
  }
  else if(gameData->powerUpIncreaseSize.isActive){
    gameData->powerUpIncreaseSize.currentTime += gameData->gamePlayStats.delta_time;
    if(gameData->powerUpIncreaseSize.currentTime >= gameData->powerUpIncreaseSize.maxTime){
      gameData->player.size.x /= 2;
      gameData->powerUpIncreaseSize.isActive = false;
    }
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

    gameData->ball.velocity = ScaleVector2(NormalizeVector2(gameData->ball.velocity), gameData->ball.maxSpeed * gameData->gamePlayStats.delta_time);
    gameData->ball.position = AddVector2(gameData->ball.position, gameData->ball.velocity);

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

  gameData->player.velocity = ScaleVector2(NormalizeVector2(gameData->player.velocity), gameData->player.max_speed * gameData->gamePlayStats.delta_time);
  gameData->player.position = AddVector2(gameData->player.position, gameData->player.velocity);
}
void RenderGamePlayScreen(GameData *gameData)
{

  // Draw Player
  DrawRectangle(gameData->player.position.x, gameData->player.position.y - gameData->screenBounds.boundsSize, gameData->player.size.x, gameData->player.size.y, gameData->player.color);
  // Draw Ball
  if(gameData->gamePlayStats.isBallSpawned) DrawCircle(gameData->ball.position.x, gameData->ball.position.y, gameData->ball.size, gameData->ball.color);

  if(gameData->powerUpAddLife.isSpawned) DrawCircle(gameData->powerUpAddLife.position.x, gameData->powerUpAddLife.position.y, 
  gameData->powerUpAddLife.size, gameData->powerUpAddLife.color);

  if(gameData->powerUpIncreaseSize.isSpawned) DrawCircle(gameData->powerUpIncreaseSize.position.x, gameData->powerUpIncreaseSize.position.y, 
  gameData->powerUpIncreaseSize.size, gameData->powerUpIncreaseSize.color);

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

  char highScore[256];
  sprintf(highScore, "%i", gameData->gamePlayStats.highScore > gameData->gamePlayStats.currentScore ? 
  gameData->gamePlayStats.highScore : gameData->gamePlayStats.currentScore);

  char highScoreString[256];
  snprintf(highScoreString, sizeof(highScoreString), "%s%s", "HighScore: ", highScore);
  DrawText(highScoreString, gameData->screenBounds.screenSize.x / 2 + 120, 10, 20, GOLD);
  
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

    //int level = gameData->currentLevel+1;

    WriteFileData(gameData->fileData, gameData->gamePlayStats.currentScore > gameData->fileData->highScore ? 
    gameData->gamePlayStats.currentScore : gameData->fileData->highScore, gameData->currentLevel + 1);

    free(gameData->bricks);
    free(gameData->fileData);
    free(gameData);
    

    gameData = malloc(sizeof *gameData);
    gameData->fileData = malloc(sizeof(*gameData->fileData));
    //SetLevel(gameData, level);
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

void ReadFileData(FileData *fileData){
    FILE* fptr;
    if ((fptr = fopen("./Data.bin", "rb")) == NULL) {
        printf("Error! opening file");
        // If file pointer will return NULL
        // Program will exit.
        //exit(1);
        fileData->currentLevel = 1;
        fileData->highScore = 0;
    }
    // else it will return a pointer to the file.
    for (int n = 1; n < 5; ++n) {
        fread(fileData, sizeof(FileData), 1, fptr);
        printf("n1: %d\tn2: %d", fileData->highScore, fileData->currentLevel);
    }
    fclose(fptr);
}
void WriteFileData(FileData *fileData, int highScore, int level){
    FILE* fptr;
    if ((fptr = fopen("./Data.bin", "wb")) == NULL) {
        printf("Error! opening file");
        // If file pointer will return NULL
        // Program will exit.
        exit(1);
    }
    int flag = 0;
    // else it will return a pointer to the file.
    for (int n = 1; n < 5; ++n) {
        fileData->highScore = highScore;
        fileData->currentLevel = level;
        flag = fwrite(fileData, sizeof(FileData), 1,
                      fptr);
    }

    // checking if the data is written
    if (!flag) {
        printf("Write Operation Failure");
    }
    else {
        printf("Write Operation Successful");
    }

    fclose(fptr);
}
