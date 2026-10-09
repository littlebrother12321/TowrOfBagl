#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"
#include "box2d/box2d.h"

//#define RAYMATH_IMPLEMENTATION
//#include "raymath.h"
//#include "rlgl.h" // Needed for rl*

#include <assert.h>

bool debug_mode = false;
bool wireframe = false;

typedef struct Entity
{
  b2BodyId bodyId;
  b2Vec2 extent;
  Texture texture;
} Entity;

void DrawEntity(const Entity* entity) {
  // Boxes are created centered on the bodies, but raylib uses the top left corner.
  // b2Body_GetWorldPoint gets the top left corner of the box. accounting for rotation.
  b2Vec2 p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2) { -entity->extent.x, -entity->extent.y});
  b2Rot rotation = b2Body_GetRotation(entity->bodyId);
  float radians = b2Rot_GetAngle(rotation);

  Vector2 ps = {p.x, p.y};

  // I don't need texture, i need RED RECTANGLE
  //DrawTextureEx(entity->texture, ps, RAD2DEG * radians, 1.0f, WHITE);

  //DrawRectangle(ps.x, ps.y, entity->texture.width, entity->texture.height, RED);
  Rectangle rec = {ps.x, ps.y, 2.0f * 25.0f, 2.0f * 25.0f};

  /* DrawRectanglePro(recBack, (Vector2){0, 0}, RAD2DEG * radians, (Color){0,0,0, 255}); */
  if (!wireframe) {
      DrawRectanglePro(rec, (Vector2){0, 0}, RAD2DEG * radians,
                   (Color){255, 0, 0, 255});
  }

  p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2){0.0f, 0.0f});
  ps = (Vector2) { p.x, p.y};

  DrawPolyLinesEx(ps, 4, 37.0f, RAD2DEG * (radians + PI/4), 1, BLACK);

  // Draw circles for optional alignment
  if (debug_mode) {
      p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2){ -entity->extent.x, -entity->extent.y});
      ps = (Vector2) { p.x, p.y };
      DrawCircleV(ps, 5.0f, BLACK);
      p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2){0.0f, 0.0f});
      ps = (Vector2) { p.x, p.y };
      DrawCircleV(ps, 5.0f, BLUE);
      p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2){ entity->extent.x, entity->extent.y});
      ps = (Vector2) { p.x, p.y };
      DrawCircleV(ps, 5.0f, RED);
  }
}

#define GROUND_COUNT 28
#define BOX_COUNT 20

#define COLOR_BOLD "\e[1m"
#define COLOR_OFF  "\e[m"

int main(int argc, char* argv[]) {
    char *help = "\n"
        //"      Tower Of Bagel\n"
        //"\n"
        "Usage: TowerOfBagel [OPTIONS]..."
        "\n"
        COLOR_BOLD "   --wireframe" COLOR_OFF "    Remove filling of boxes\n"
        COLOR_BOLD "   --debug"     COLOR_OFF "        Add circles for box alignment\n"
        "\n"
        "Lovingly made with Raylib and Box2D\n";

    for (int i=0; i<argc; i++) {
    //printf("Arguments: %d, %s\n", argc, argv[i]);
    if (strcmp(argv[i], "--debug") == 0) {
      debug_mode = true;
    }
    if (strcmp(argv[i], "--wireframe") == 0) {
        wireframe = true;
    }
    if (strcmp(argv[i], "--help") == 0) {
        printf("%s\n", help);
        exit(0);
    }
  }

  printf("Hello world!\n");
  printf("Now making a window!\n");
  printf("Debug mode is: %d!\n", debug_mode);

  // Initialize constants and vars
  // (raylib) Set screen size, but enable dynamic window resizing
  const int screenWidth = 1920;
  const int screenHeight = 1080;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  // (box2d) Set the time step (60Hz), not to be tied to frame rate
  const float timeStep = 1.0f / 60.0f;
  // (box2d) Enable sub stepping (adjusting means trade-off between performance and accuracy)
  const int subStepCount = 4;

  // (both) Set the scale factor between them. e.g.
  // Raylib uses pixels, Box2d uses 'meters'. This is the Pixels-per-meter ratio.
  // The boxes will be 128 pixels wide.
  float lengthUnitsPerMeter = 128.0f;
  b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

  // (raylib) Initialize window with GLFW (auto)
  InitWindow(screenWidth, screenHeight, "Tower of bagel (box2d)");

  // (raylib) Set FPS cap in frames per second
  SetTargetFPS(60);


  // (raylib) Assign a texture ot the box and ground
  Texture boxTexture = LoadTexture("assets/box.png");
  Texture groundTexture = LoadTexture("assets/ground.png");

  // (box2d) Test square body creation
  //-----------------------------------------------------
  // Create a world
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity.y = 9.8f * lengthUnitsPerMeter;
  b2WorldId worldId = b2CreateWorld(&worldDef);

  // Using standard colors now!
  //b2Vec2 groundExtent = { 0.5f * groundTexture.width, 0.5f * groundTexture.height};
  //b2Vec2 boxExtent = {0.5f * boxTexture.width, 0.5f * boxTexture.height};
  b2Vec2 groundExtent = {25.0f, 25.0f};
  b2Vec2 boxExtent = {25.0f, 25.0f};

  b2Polygon groundPolygon = b2MakeBox(groundExtent.x, groundExtent.y);
  b2Polygon boxPolygon = b2MakeBox(boxExtent.x, boxExtent.y);

  // (box2d) Create an arbitrary dynamic box (physics object) to interact with the ground
  Entity groundEntities[GROUND_COUNT] = { 0 };
  for (int i = 0; i < GROUND_COUNT; ++i)
  {
    Entity* entity = groundEntities + i;
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = (b2Vec2){ (1.0f * i * 2.0f) * groundExtent.x, screenHeight / 2 - groundExtent.y + 50.0f};

    entity->bodyId = b2CreateBody(worldId, &bodyDef);
    entity->extent = groundExtent;
    entity->texture = groundTexture;
    //entity->texture = { 0 };
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(entity->bodyId, &shapeDef, &groundPolygon);
  }

  Entity boxEntities[BOX_COUNT] = { 0 };
  int boxIndex = 0;
  for (int i = 0; i < 6; ++i)
    {
      float y = screenHeight / 2 - groundExtent.y - 50.0f - (2.5f * i + 2.0f) * boxExtent.y - 20.0f;

      for (int j = i; j < 6; ++j)
        {
          float x = 0.25f * screenWidth + (3.0f * j - i - 3.0f) * boxExtent.x;
          assert(boxIndex < BOX_COUNT);

          Entity* entity = boxEntities + boxIndex;
          b2BodyDef bodyDef = b2DefaultBodyDef();
          bodyDef.type = b2_dynamicBody;
          bodyDef.position = (b2Vec2){ x, y };
          entity->bodyId = b2CreateBody(worldId, &bodyDef);
          entity->texture = boxTexture;
          entity->extent = boxExtent;
          b2ShapeDef shapeDef = b2DefaultShapeDef();
          b2CreatePolygonShape(entity->bodyId, &shapeDef, &boxPolygon);

          boxIndex += 1;
        }
    }

  bool pause = false;

  // Main game loop while window is open
  // -----------------------------------------
  while (!WindowShouldClose()) {
    // Pause logic
    if (IsKeyPressed(KEY_P))
      {
        pause = !pause;
      }

    if (pause == false)
      {
        float deltaTime = GetFrameTime();
        b2World_Step(worldId, deltaTime, 4);
      }

    // Update variables first
    // TODO


    BeginDrawing();
    ClearBackground(DARKGRAY);

    const char* message = "Hello, Box2D!";
    int fontSize = 36;
    int textWidth = MeasureText("Hello, Box2D!", fontSize);
    DrawText(message, 50, 50, fontSize, LIGHTGRAY);

    for ( int i = 0; i < GROUND_COUNT; ++i )
      {
        DrawEntity(groundEntities + i);
      }

    for ( int i = 0; i < BOX_COUNT; ++i )
      {
        DrawEntity(boxEntities + i);
      }


    EndDrawing();
  }
  // Clean up the program
  // ------------------------------------------
  // (raylib) CLEANUP: Unload textures
  UnloadTexture(groundTexture);
  UnloadTexture(boxTexture);
  // (raylib) CLEANUP: Close window
  CloseWindow();

  // Goodbye
  printf("Thank you for playing with this program, no AI was used to generate code. Do No Evil.\n");

 return 0;
}

