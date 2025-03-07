#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ASTEROIDS 20
#define SCREEN_HEIGHT 495
#define SCREEN_WIDTH 800
#define ASTEROID_STEP 2.5
#define FORWARD 0
#define BACKWARD -1
#define SPACE_SHIP_WIDTH 60
#define SPACE_SHIP_HEIGHT 60
#define BULLET_WIDTH 10
#define BULLET_HEIGHT 10
#define ASTEROID_WIDTH 40
#define ASTEROID_HEIGHT 40

typedef struct Bullet Bullet;
typedef struct Asteroid Asteroid;

struct Bullet
{
  Vector2 pos;
  Bullet *next;
};

struct Asteroid
{
  Vector2 pos;
  int dirX;
  int dirY;
  bool isShot;
};

int GenerateRand(int min, int max)
{
  return min + rand() % (max - min + 1);
}

void AddNewBullet(Bullet **list, float posX, float posY)
{
  // TraceLog(LOG_INFO,"Inside add bullet");

  Bullet *newBullet = malloc(sizeof(Bullet));
  newBullet->pos.x = posX;
  newBullet->pos.y = posY;
  newBullet->next = NULL;

  if (*list == NULL)
  {
    *list = newBullet;
  }
  else
  {
    Bullet *temp = *list;
    while (temp->next != NULL)
    {
      temp = temp->next;
    }

    temp->next = newBullet;
  }
}

void DrawBullets(Bullet *list)
{
  Bullet *temp = list;
  // TraceLog(LOG_INFO,"Inside the draw bullets");

  while (temp != NULL)
  {
    // TraceLog(LOG_INFO,"Inside the bullet loop");

    float x = temp->pos.x;
    float y = temp->pos.y;

    DrawRectangle(x + 25, y - 30, BULLET_WIDTH, BULLET_HEIGHT, YELLOW);

    temp->pos.y -= 15;

    temp = temp->next;
  }
}

void ClearDeadBullets(Bullet **list)
{
  // No Bullets return
  if (*list == NULL)
  {
    return;
  }

  // Remove the starting node in the list when out of screen
  // This should be enough as all bullets go out eventually
  while ((*list != NULL) && ((*list)->pos.y < 0))
  {
    Bullet *temp = *list;

    (*list) = (*list)->next;

    // TraceLog(LOG_INFO, "Freeing the bullet out of screen");
    free(temp);
  }
}

Asteroid *InitAsteroids()
{
  Asteroid *asteroids = malloc(sizeof(Asteroid) * NUM_ASTEROIDS);

  for (int i = 0; i < NUM_ASTEROIDS; i++)
  {
    asteroids[i].pos.x = GenerateRand(1, SCREEN_WIDTH);
    asteroids[i].pos.y = GenerateRand(1, SCREEN_HEIGHT / 2);
    asteroids[i].dirX = GenerateRand(-1, 0);
    asteroids[i].dirY = GenerateRand(-1, 0);
    asteroids[i].isShot = false;
  }

  return asteroids;
}

void DrawAsteroids(Asteroid *asteroids, Texture2D asteriodTexture)
{
  for (int i = 0; i < NUM_ASTEROIDS; i++)
  {
    if (asteroids[i].isShot)
    {
      continue;
    }

    DrawTexture(asteriodTexture, asteroids[i].pos.x, asteroids[i].pos.y, WHITE);

    if (asteroids[i].dirX < 0)
    {
      asteroids[i].pos.x -= ASTEROID_STEP;
    }
    else
    {
      asteroids[i].pos.x += ASTEROID_STEP;
    }

    if (asteroids[i].dirY < 0)
    {
      asteroids[i].pos.y -= ASTEROID_STEP;
    }
    else
    {
      asteroids[i].pos.y += ASTEROID_STEP;
    }

    if (asteroids[i].pos.x < 0)
    {
      asteroids[i].dirX = FORWARD;
    }

    if (asteroids[i].pos.x > SCREEN_WIDTH)
    {
      asteroids[i].dirX = BACKWARD;
    }

    if (asteroids[i].pos.y < 0)
    {
      asteroids[i].dirY = FORWARD;
    }
    if (asteroids[i].pos.y > SCREEN_HEIGHT)
    {
      asteroids[i].dirY = BACKWARD;
    }
  }
}

void CheckSpaceShipAsteroidCollision(Rectangle spaceshipRect, Asteroid *asteroids, bool *isSpaceshipActive, Sound spaceshipExplosion)
{
  if (*isSpaceshipActive == false)
  {
    return;
  }

  for (int i = 0; i < NUM_ASTEROIDS; i++)
  {
    if (asteroids[i].isShot)
      continue;

    Rectangle asteroidRect = {asteroids[i].pos.x, asteroids[i].pos.y, 40, 40};

    if (CheckCollisionRecs(spaceshipRect, asteroidRect))
    {
      // TraceLog(LOG_INFO, "Collision Detected %d", rand());
      *isSpaceshipActive = false;
      PlaySound(spaceshipExplosion);
    }
  }
}

void CheckBulletAsteroidCollision(Asteroid *asteroids, Bullet *bulletList, Sound rockDestroyed)
{
  if (bulletList == NULL)
    return;

  Bullet *head = bulletList;

  while (head != NULL)
  {
    Rectangle bulletRect = {head->pos.x, head->pos.y, BULLET_WIDTH, BULLET_HEIGHT};

    for (int i = 0; i < NUM_ASTEROIDS; i++)
    {
      Rectangle asteroidRect = {asteroids[i].pos.x, asteroids[i].pos.y, ASTEROID_WIDTH, ASTEROID_HEIGHT};

      if (CheckCollisionRecs(bulletRect, asteroidRect) && !asteroids[i].isShot)
      {
        // TraceLog(LOG_INFO, "Collision with bullet");
        asteroids[i].isShot = true;
        PlaySound(rockDestroyed);
      }
    }

    head = head->next;
  }
}

int main(void)
{
  srand(time(NULL));

  const float screenWidth = SCREEN_WIDTH;
  const float screenHeight = SCREEN_HEIGHT;

  float flightPosX = screenWidth / 2 - 25;
  float flightPosY = screenHeight - 100;

  // Initialize the window
  InitWindow(screenWidth, screenHeight, "Space Invaders");

  // Initialzie the audio device
  InitAudioDevice();

  bool gamePaused = true;
  bool spaceShipActive = true;

  // Initialize the active bullet list
  Bullet *bulletList = NULL;

  // Initialize the asteroids
  Asteroid *asteroids = InitAsteroids();

  // Load Audio
  Sound laser = LoadSound("assets/laser.mp3");
  Sound rockDestoyed = LoadSound("assets/rock_destroyed.mp3");
  Sound spaceshipExplosion = LoadSound("assets/spaceship_explosion.mp3");

  // Load images
  Image spaceshipImage = LoadImage("assets/spaceship_blue.png");
  ImageResize(&spaceshipImage, SPACE_SHIP_WIDTH, SPACE_SHIP_HEIGHT);

  Image asteriodImage = LoadImage("assets/asteroid_2.png");
  ImageResize(&asteriodImage, 40, 40);

  Image background = LoadImage("assets/background_copper.png");
  ImageResize(&background, screenWidth, screenHeight);

  // Load textures from images
  Texture2D asteriodTexture = LoadTextureFromImage(asteriodImage);
  Texture2D spaceshipTexture = LoadTextureFromImage(spaceshipImage);
  Texture2D backgroundTexture = LoadTextureFromImage(background);

  // Play background loop
  Sound backgroundMusic = LoadSound("assets/game_loop.mp3");
  PlaySound(backgroundMusic);

  // set target fps
  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(WHITE);
    DrawTexture(backgroundTexture, 0, 0, WHITE);

    if (IsKeyPressed(KEY_ENTER))
    {
      gamePaused = !gamePaused;
    }

    if (!gamePaused)
    {
      if (IsKeyDown(KEY_LEFT))
      {
        if (flightPosX > 10)
        {
          flightPosX -= 10;
        }
      }

      if (IsKeyDown(KEY_UP))
      {
        if (flightPosY > 10)
        {
          flightPosY -= 10;
        }
      }

      if (IsKeyDown(KEY_RIGHT))
      {
        if (flightPosX < screenWidth - 70)
        {
          flightPosX += 10;
        }
      }

      if (IsKeyDown(KEY_DOWN))
      {
        if (flightPosY < screenHeight - 100)
        {
          flightPosY += 10;
        }
      }

      if (IsKeyPressed(KEY_SPACE) && spaceShipActive)
      {
        PlaySound(laser);
        AddNewBullet(&bulletList, flightPosX, flightPosY);
      }

      DrawAsteroids(asteroids, asteriodTexture);
      DrawBullets(bulletList);

      if (spaceShipActive)
      {
        DrawTexture(spaceshipTexture, flightPosX, flightPosY, WHITE);
        if (!IsSoundPlaying(backgroundMusic))
        {
          PlaySound(backgroundMusic);
        }
      }
      else
      {
        DrawText("Game Over", screenWidth / 2 - 50, screenHeight / 2, 32, YELLOW);
        if (IsSoundPlaying(backgroundMusic))
        {
          StopSound(backgroundMusic);
        }
      }

      Rectangle spaceshipRect = {flightPosX, flightPosY, SPACE_SHIP_WIDTH, SPACE_SHIP_HEIGHT};

      CheckBulletAsteroidCollision(asteroids, bulletList, rockDestoyed);
      CheckSpaceShipAsteroidCollision(spaceshipRect, asteroids, &spaceShipActive, spaceshipExplosion);
    }
    else
    {
      DrawText("Game Paused", (screenWidth / 2) - 100, (screenHeight / 2) - 50, 32, YELLOW);
      DrawText("Press ENTER to start", (screenWidth / 2) - 100, (screenHeight / 2) - 10, 24, YELLOW);
    }

    ClearDeadBullets(&bulletList);
    EndDrawing();
  }

  // Unload allocated resources
  UnloadImage(spaceshipImage);
  UnloadTexture(spaceshipTexture);
  UnloadTexture(backgroundTexture);
  UnloadSound(laser);
  UnloadSound(rockDestoyed);
  UnloadSound(backgroundMusic);
  UnloadSound(spaceshipExplosion);

  CloseAudioDevice();
  CloseWindow();

  return 0;
}