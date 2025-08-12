#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define SPEED 5.0f
#define NUM_ASTEROIDS 20
#define SCREEN_HEIGHT 495
#define SCREEN_WIDTH 800
#define BULLET_WIDTH 5
#define BULLET_HEIGHT 5
#define SPACE_SHIP_WIDTH 30
#define SPACE_SHIP_HEIGHT 30
#define FRAME_COUNT 30
#define NEBULA_PARTICLES 1000

enum Size
{
  SMALL,
  LARGE,
  MEDUIM
};

typedef struct
{
  bool isActive;
  float rotation;
  Texture2D texture;
  Vector2 pos;
} Spaceship;

typedef struct Bullet
{
  bool isActive;
  float angle;
  Vector2 pos;
  struct Bullet *next;
} Bullet;

typedef struct Asteroid
{
  uint8_t speed;
  uint8_t angle;
  bool isActive;
  bool isLeftType;
  enum Size size;
  struct Asteroid *next;
  Vector2 pos;
} Asteroid;

typedef struct
{
  Vector2 position;
  float size;
  float speed;
  float alpha;
  Color color;
} NebulaParticle;

Color nebulaColors[] = {
     {0, 255, 0, 255},  // Green
    {255, 255, 0, 255},  // Yellow
};

void InitSpaceship(Spaceship *ship, Texture2D spaceshipTexture)
{
  ship->texture = spaceshipTexture;
  ship->pos.x = ((float) SCREEN_WIDTH / 2.0) - ((float) SPACE_SHIP_WIDTH / 2.0);
  ship->pos.y = ((float) SCREEN_HEIGHT / 2.0) - ((float) SPACE_SHIP_HEIGHT / 2.0);
  ship->rotation = 0.0;
  ship->isActive = true;
}

void InitNebula(NebulaParticle *nebula)
{
  for (int i = 0; i < NEBULA_PARTICLES; i++)
  {
    nebula[i].position.x = GetRandomValue(0, SCREEN_WIDTH);
    nebula[i].position.y = GetRandomValue(0, SCREEN_HEIGHT);
    nebula[i].color = nebulaColors[GetRandomValue(0, 3)];
    nebula[i].size = GetRandomValue(10, 30);
    nebula[i].speed = GetRandomValue(1, 2)/10.0;
    nebula[i].alpha = GetRandomValue(20, 60);
  }
}

void UpdateAndDrawNebula(Spaceship *ship, NebulaParticle *nebula)
{
  for (int i = 0; i < NEBULA_PARTICLES; i++)
  {
    nebula[i].position.x -= nebula[i].speed * sin(ship->rotation * DEG2RAD * SPEED);
    nebula[i].position.y += nebula[i].speed * cos(ship->rotation * DEG2RAD * SPEED);

    // Wrap around screen
    if (nebula[i].position.x < -nebula[i].size)
      nebula[i].position.x = SCREEN_WIDTH + nebula[i].size;
    if (nebula[i].position.x > SCREEN_WIDTH + nebula[i].size)
      nebula[i].position.x = -nebula[i].size;
    if (nebula[i].position.y < -nebula[i].size)
      nebula[i].position.y = SCREEN_HEIGHT + nebula[i].size;
    if (nebula[i].position.y > SCREEN_HEIGHT + nebula[i].size)
      nebula[i].position.y = -nebula[i].size;

    // Draw nebula particle as a translucent circle
    Color currentColor = nebula[i].color;
    currentColor.a = nebula[i].alpha;
    DrawCircle(nebula[i].position.x,nebula[i].position.y,1,nebula[i].color);
  }
}

void UpdateShipPosition(Spaceship *ship)
{
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
  {
    float tempPosX = ship->pos.x + (sin(ship->rotation * DEG2RAD) * SPEED);
    if (tempPosX >= SPEED && tempPosX <= (SCREEN_WIDTH - SPEED))
    {
      ship->pos.x = tempPosX;
    }

    float tempPosY = ship->pos.y - (cos(ship->rotation * DEG2RAD) * SPEED);
    if (tempPosY >= SPEED && tempPosY < (SCREEN_HEIGHT - SPEED))
    {
      ship->pos.y = tempPosY;
    }

    // TraceLog(LOG_INFO, "Angle %f", ship->rotation)
  }

  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
  {
    float tempPosX = ship->pos.x - (sin(ship->rotation * DEG2RAD) * SPEED);
    if (tempPosX >= SPEED && tempPosX <= (SCREEN_WIDTH - SPEED))
    {
      ship->pos.x = tempPosX;
    }

    float tempPosY = ship->pos.y + (cos(ship->rotation * DEG2RAD) * SPEED);
    if (tempPosY >= SPEED && tempPosY < (SCREEN_HEIGHT - SPEED))
    {
      ship->pos.y = tempPosY;
    }

    // TraceLog(LOG_INFO, "Angle %f", ship->rotation);
  }
}

void AddBullet(Bullet **bulletList, Spaceship *ship)
{

  Bullet *newBullet = (Bullet *)malloc(sizeof(Bullet));
  newBullet->angle = ship->rotation;
  newBullet->next = NULL;
  newBullet->pos = ship->pos;
  newBullet->isActive = true;

  if (*bulletList == NULL)
  {
    *bulletList = newBullet;
  }
  else
  {
    Bullet *temp = *bulletList;

    while (temp->next != NULL)
    {
      temp = temp->next;
    }
    temp->next = newBullet;
  }
}

void DrawBullets(Bullet *bulletList)
{
  Bullet *temp = bulletList;

  if (bulletList == NULL)
    return;

  while (temp != NULL)
  {
    if (!temp->isActive)
    {
      temp = temp->next;
      continue;
    }

    float angle = temp->angle;
    temp->pos.x += (sin(temp->angle * DEG2RAD) * (SPEED + 2));
    temp->pos.y -= (cos(temp->angle * DEG2RAD) * (SPEED + 2));

    int posX = (int)(temp->pos.x);
    int posY = (int)(temp->pos.y);

    DrawRectangle(posX, posY, BULLET_WIDTH, BULLET_HEIGHT, ORANGE);

    temp = temp->next;
  }
}

void CleanBullets(Bullet **bulletList)
{
  if (*bulletList == NULL)
    return;

  while (*bulletList != NULL &&
         ((*bulletList)->pos.y < 0 || (*bulletList)->pos.y > SCREEN_HEIGHT ||
          (*bulletList)->pos.x < 0 || (*bulletList)->pos.x > SCREEN_WIDTH ||
          !(*bulletList)->isActive))
  {
    Bullet *temp = *bulletList;
    *bulletList = (*bulletList)->next;
    free(temp);
  }

  Bullet *prev = *bulletList;
  if (prev == NULL)
    return;

  Bullet *cur = (*bulletList)->next;

  while (cur != NULL)
  {
    if (cur->pos.y < 0 || cur->pos.y > SCREEN_HEIGHT ||
        cur->pos.x < 0 || cur->pos.x > SCREEN_WIDTH ||
        !cur->isActive)
    {
      Bullet *temp = cur;
      cur = cur->next;
      prev->next = cur;
      free(temp);
    }
    else
    {
      prev = cur;
      cur = cur->next;
    }
  }
}

void GenerateAsteroids(Asteroid **asteroidList, int iterationCount)
{

  // Generate one asteroid for every 20 frames
  if (iterationCount % FRAME_COUNT != 0)
    return;

  // 0 asteroid from left
  // 1 asteroid from right

  int type = GetRandomValue(0, 1);

  Asteroid *asteroid = (Asteroid *)malloc(sizeof(Asteroid));
  asteroid->isActive = true;
  asteroid->pos.y = GetRandomValue(20, SCREEN_HEIGHT - 20);
  asteroid->speed = GetRandomValue(1, 6);
  asteroid->next = NULL;

  int randSizeIndex = GetRandomValue(0, 2);

  switch (randSizeIndex)
  {
  case 0:
    asteroid->size = SMALL;
    break;
  case 2:
    asteroid->size = LARGE;
    break;
  default:
    asteroid->size = MEDUIM;
    break;
  }

  if (type == 0)
  {
    asteroid->pos.x = -10;
    asteroid->isLeftType = true;
    asteroid->angle = GetRandomValue(30, 120);
  }
  else
  {
    asteroid->pos.x = SCREEN_WIDTH + 10;
    asteroid->isLeftType = false;
    asteroid->angle = GetRandomValue(-10, -180);
  }

  if (*asteroidList == NULL)
  {
    *asteroidList = asteroid;
  }
  else
  {
    Asteroid *temp = *asteroidList;

    while (temp->next != NULL)
    {
      temp = temp->next;
    }

    temp->next = asteroid;
  }
}

void DrawAsteroids(Asteroid *asteroidList, Texture2D smallTexture, Texture2D mediumTexture, Texture2D largeTexture)
{
  if (asteroidList == NULL)
    return;

  Asteroid *temp = asteroidList;

  while (temp != NULL)
  {
    if (!temp->isActive)
    {
      temp = temp->next;
      continue;
    }

    if (temp->isLeftType)
    {
      temp->pos.x += (sin(temp->angle * DEG2RAD) * (temp->speed));
    }
    else
    {
      temp->pos.x -= (sin(temp->angle * DEG2RAD) * (temp->speed));
    }

    temp->pos.y += (cos(temp->angle * DEG2RAD) * (temp->speed));

    int posX = (int)temp->pos.x;
    int posY = (int)temp->pos.y;

    if (temp->size == SMALL)
    {
      DrawTexture(smallTexture, posX, posY, WHITE);
    }
    else if (temp->size == MEDUIM)
    {
      DrawTexture(mediumTexture, posX, posY, WHITE);
    }
    else if (temp->size == LARGE)
    {
      DrawTexture(largeTexture, posX, posY, WHITE);
    }

    temp = temp->next;
  }
}

void CleanAsteroids(Asteroid **asteroidList)
{
  if (*asteroidList == NULL)
    return;

  // remove invalid head asteroids
  while ((*asteroidList != NULL) && (((*asteroidList)->pos.y < -10) || ((*asteroidList)->pos.y > SCREEN_HEIGHT + 10)))
  {
    Asteroid *temp = (*asteroidList);
    (*asteroidList) = (*asteroidList)->next;

    // TraceLog(LOG_INFO, "Freeing the asteroid %p \n", temp);
    free(temp);
  }

  if (*asteroidList == NULL)
    return;

  Asteroid *prev = (*asteroidList);
  Asteroid *cur = (*asteroidList)->next;

  while (cur != NULL)
  {
    if ((cur->pos.y < -10) || (cur->pos.y > SCREEN_HEIGHT + 10))
    {
      Asteroid *temp = cur;
      cur = cur->next;
      prev->next = cur;

      // TraceLog(LOG_INFO, "Freeing the asteroid %p \n", temp);
      free(temp);
    }
    else
    {
      prev = cur;
      cur = cur->next;
    }
  }
}

void CheckBulletAsteroidCollision(Asteroid *asteroidList, Bullet *bulletList, int *points, Sound rockSmash)
{
  if (bulletList == NULL || asteroidList == NULL)
    return;

  Bullet *tempBullet = bulletList;

  while (tempBullet != NULL)
  {
    // TraceLog(LOG_INFO,"Outer temp bullet");
    if (!tempBullet->isActive)
    {
      tempBullet = tempBullet->next;
      continue;
    }
    // TraceLog(LOG_INFO,"Temp bullet %p",tempBullet);

    Rectangle bulletRect = {tempBullet->pos.x, tempBullet->pos.y, BULLET_WIDTH, BULLET_HEIGHT};

    Asteroid *tempAsteroid = asteroidList;
    while (tempAsteroid != NULL)
    {
      // TraceLog(LOG_INFO,"Inner");
      if (!tempAsteroid->isActive)
      {
        tempAsteroid = tempAsteroid->next;
        continue;
      }

      // TraceLog(LOG_INFO,"Temp asteroid %p",tempAsteroid);

      Rectangle asteroidRect;

      switch (tempAsteroid->size)
      {
      case SMALL:
        asteroidRect = (Rectangle){tempAsteroid->pos.x, tempAsteroid->pos.y, 20, 20};
        break;
      case MEDUIM:
        asteroidRect = (Rectangle){tempAsteroid->pos.x, tempAsteroid->pos.y, 30, 30};
        break;
      case LARGE:
        asteroidRect = (Rectangle){tempAsteroid->pos.x, tempAsteroid->pos.y, 50, 50};
        break;
      }

      if (CheckCollisionRecs(bulletRect, asteroidRect))
      {
        tempBullet->isActive = false;
        if (tempAsteroid->size == SMALL)
        {
          tempAsteroid->isActive = false;
          *points += 1;
        }
        else if (tempAsteroid->size == MEDUIM)
        {
          tempAsteroid->size = SMALL;
          *points += 1;
        }
        else
        {
          tempAsteroid->size = MEDUIM;
          *points += 1;
        }
        PlaySound(rockSmash);

        break;
      }

      tempAsteroid = tempAsteroid->next;
    }

    tempBullet = tempBullet->next;
  }
}

void CheckAsteroidShipCollision(Asteroid *asteroidList, Spaceship *ship, uint8_t *lives, Sound explosion)
{
  if (asteroidList == NULL || !ship->isActive)
    return;

  Rectangle shipRect = {ship->pos.x, ship->pos.y, SPACE_SHIP_WIDTH, SPACE_SHIP_HEIGHT};

  Asteroid *temp = asteroidList;
  while (temp != NULL)
  {
    if (!temp->isActive)
    {
      temp = temp->next;
      continue;
    }

    Rectangle asteroidRect;
    switch (temp->size)
    {
    case SMALL:
      asteroidRect = (Rectangle){temp->pos.x, temp->pos.y, 20, 20};
      break;
    case MEDUIM:
      asteroidRect = (Rectangle){temp->pos.x, temp->pos.y, 30, 30};
      break;
    case LARGE:
      asteroidRect = (Rectangle){temp->pos.x, temp->pos.y, 50, 50};
      break;
    }

    if (CheckCollisionRecs(shipRect, asteroidRect))
    {
      temp->isActive = false;
      *lives -= 1;

      if (*lives <= 0)
      {
        ship->isActive = false;
        PlaySound(explosion);
      }
      break;
    }
    temp = temp->next;
  }
}

int main(void)
{
  srand(time(NULL));

  const float screenWidth = SCREEN_WIDTH;
  const float screenHeight = SCREEN_HEIGHT;

  // Initialize the window
  InitWindow(screenWidth, screenHeight, "Asteroids");
  InitAudioDevice();

  // Load images
  Image background = LoadImage("assets/background.jpg");
  Image spaceshipImage = LoadImage("assets/spaceship_blue.png");
  Image smallAsteroid = LoadImage("assets/asteroid_2.png");
  Image mediumAsteroid = LoadImage("assets/asteroid_2.png");
  Image largeAsteroid = LoadImage("assets/asteroid_2.png");

  ImageResize(&spaceshipImage, SPACE_SHIP_WIDTH, SPACE_SHIP_HEIGHT);
  ImageResize(&smallAsteroid, 20, 20);
  ImageResize(&mediumAsteroid, 30, 30);
  ImageResize(&largeAsteroid, 50, 50);

  // Load sounds
  Sound laser = LoadSound("assets/laser.mp3");
  Sound explosion = LoadSound("assets/spaceship_explosion.mp3");
  Sound rockSmash = LoadSound("assets/rock_destroyed.mp3");

  // Load textures from images
  Texture2D spaceshipTexture = LoadTextureFromImage(spaceshipImage);
  Texture2D backgroundTexture = LoadTextureFromImage(background);
  Texture2D smallAsteroidTexture = LoadTextureFromImage(smallAsteroid);
  Texture2D mediumAsteroidTexture = LoadTextureFromImage(mediumAsteroid);
  Texture2D largeAsteroidTexture = LoadTextureFromImage(largeAsteroid);

  // Initialize a spaceship
  Spaceship ship;
  InitSpaceship(&ship, spaceshipTexture);

  NebulaParticle nebula[NEBULA_PARTICLES];
  InitNebula(nebula);

  // Initialize the bullet list
  Bullet *bulletList = NULL;

  // Initialize the asteroid list
  Asteroid *asteroidList = NULL;

  // IterationCount
  int iterationCount = 0;
  int points = 0;
  uint8_t lives = 3;

  // set target fps
  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    BeginDrawing();
    iterationCount++;

    ClearBackground(BLACK);
    UpdateAndDrawNebula(&ship,nebula);
    GenerateAsteroids(&asteroidList, iterationCount);

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    {
      ship.rotation += 5;

      if (ship.rotation > 360)
      {
        ship.rotation = fmodf(ship.rotation, 360.0);
      }

      UpdateShipPosition(&ship);
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    {
      ship.rotation -= 5;
      if (ship.rotation < 360)
      {
        ship.rotation = fmodf(ship.rotation, 360.0);
      }

      UpdateShipPosition(&ship);
    }

    UpdateShipPosition(&ship);

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_SPACE)) && ship.isActive)
    {
      AddBullet(&bulletList, &ship);
      PlaySound(laser);
    }

    DrawBullets(bulletList);
    DrawAsteroids(asteroidList, smallAsteroidTexture, mediumAsteroidTexture, largeAsteroidTexture);

    CheckBulletAsteroidCollision(asteroidList, bulletList, &points, rockSmash);

    DrawText(TextFormat("Score : %d", points), 20, screenHeight - 50, 30, YELLOW);

    for (int i = 0; i < lives; i++)
    {
      DrawTexture(ship.texture, (20 + (i * 10)), screenHeight - 80, WHITE);
    }

    if (ship.isActive)
    {
      CheckAsteroidShipCollision(asteroidList, &ship, &lives, explosion);
      DrawTexturePro(ship.texture, (Rectangle){0, 0, ship.texture.width, ship.texture.height}, (Rectangle){ship.pos.x, ship.pos.y, 30, 30}, (Vector2){ship.texture.width / 2.0, ship.texture.height / 2.0}, ship.rotation, WHITE);
    }
    else
    {
      DrawText(TextFormat("Game Over"), screenWidth / 2 - 100, screenHeight / 2 - 50, 40, YELLOW);
      DrawText(TextFormat("Press R to restart"), screenWidth / 2 - 100, screenHeight / 2, 20, YELLOW);

      if (IsKeyPressed(KEY_R))
      {
        // Reset the game
        iterationCount = 0;
        lives = 3;
        points = 0;
        ship.isActive = true;
      }
    }

    CleanBullets(&bulletList);
    CleanAsteroids(&asteroidList);
    EndDrawing();
  }

  // Unload allocated resources
  UnloadImage(background);
  UnloadImage(spaceshipImage);
  UnloadImage(smallAsteroid);
  UnloadImage(mediumAsteroid);
  UnloadImage(largeAsteroid);
  UnloadSound(laser);
  UnloadSound(rockSmash);
  UnloadTexture(spaceshipTexture);
  UnloadTexture(smallAsteroidTexture);
  UnloadTexture(largeAsteroidTexture);
  UnloadTexture(mediumAsteroidTexture);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}