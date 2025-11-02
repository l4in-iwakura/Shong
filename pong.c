#include "raylib.h"

#define MAX_BALLS 6

const int BORDER_THICKNESS = 10;
const int PADDLE_SPEED = 5;
const float STAMINA_DRAIN = 2.5f;
const float STAMINA_REGEN = 0.85f;
const float WALL_BOUNCE_SPEEDUP = 1.05f;
const float PADDLE_BOUNCE_SPEEDUP = 1.1f;

enum GameState {
    STATE_INACTIVE,
    STATE_PLAYING,
    STATE_P1W,
    STATE_P2W
};

struct Paddle {
    int x_pos;
    int y_pos;
    int width;
    int height;
    int score;
    float stamina;
    float sprint_speed;
};

struct Ball{
    float ball_speedx;
    float ball_speedy;
    float x_pos;
    float y_pos;
    int width;
    int height;
    bool active;
    bool isPlayBall;
};

void SpawnBall(struct Ball balls[MAX_BALLS], int boardWidth, int boardHeight)
{
    for (int i = 0; i < MAX_BALLS; i++)
    {
        if (!balls[i].active)
        {
            balls[i].active = true;
            balls[i].isPlayBall = true;
            balls[i].x_pos = boardWidth / 2.0f;
            balls[i].y_pos = boardHeight / 2.0f;
            balls[i].width = 10;
            balls[i].height = 10;

            balls[i].ball_speedy = GetRandomValue(-4, 4);
            if (balls[i].ball_speedy == 0) balls[i].ball_speedy = 4;

            if (GetRandomValue(0, 1) == 0)
                balls[i].ball_speedx = -4;
            else
                balls[i].ball_speedx = 4;

            break;
        }
    }
}

void ResetGame(struct Paddle *p1, struct Paddle *p2, struct Ball balls[MAX_BALLS], int boardWidth, int boardHeight)
{
    p1->score = 0;
    p1->stamina = 100.0f;
    p1->y_pos = boardHeight / 2 - p1->height / 2;

    p2->score = 0;
    p2->stamina = 100.0f;
    p2->y_pos = boardHeight / 2 - p2->height / 2;

    for (int i = 0; i < MAX_BALLS; i++)//DEACTIVATES THE BALLS
    {
        balls[i].active = false;
    }
}

void UpdatePaddle(struct Paddle *paddle, int keyUp, int keyDown, int keySprint, int boardHeight)
{
    if (IsKeyDown(keyUp))
    {
        if (paddle->y_pos > BORDER_THICKNESS)
        {
            if (IsKeyDown(keySprint) && paddle->stamina > 0)
            {
                paddle->stamina -= STAMINA_DRAIN;
                paddle->y_pos -= paddle->sprint_speed;
            }
            else paddle->y_pos -= PADDLE_SPEED;
        }
    }
    if (IsKeyDown(keyDown))
    {
        if (paddle->y_pos + paddle->height < boardHeight - BORDER_THICKNESS)
        {
            if (IsKeyDown(keySprint) && paddle->stamina > 0)
            {
                paddle->stamina -= STAMINA_DRAIN;
                paddle->y_pos += paddle->sprint_speed;
            }
            else paddle->y_pos += PADDLE_SPEED;
        }
    }

    // Stamina regen
    if (!IsKeyDown(keySprint) && paddle->stamina < 100.0f)
    {
        paddle->stamina += STAMINA_REGEN;
        if (paddle->stamina > 100.0f) paddle->stamina = 100.0f;
    }
}

int main(void){

    float hue = 0.0;
    int game_state = STATE_INACTIVE;
    const int boardWidth = 800;
    const int boardHeight = 450;

    // initialize with the board size first
    InitWindow(boardWidth, boardHeight, "shong");

    // THEN get monitor size and go fullscreen very important to not miss this fucking thing
    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);
    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();

    RenderTexture2D target = LoadRenderTexture(boardWidth, boardHeight);

    struct Ball balls[MAX_BALLS];
    for (int i = 0; i < MAX_BALLS; i++)
    {
        balls[i].active = false;
    }

    struct Paddle paddle1;
    paddle1.width = 10;
    paddle1.height = 75;
    paddle1.x_pos = 40;
    paddle1.sprint_speed = PADDLE_SPEED * 2;

    struct Paddle paddle2;
    paddle2.width = 10;
    paddle2.height = 75;
    paddle2.x_pos = boardWidth - 40 - paddle2.width;
    paddle2.sprint_speed = PADDLE_SPEED * 2;

    ResetGame(&paddle1, &paddle2, balls, boardWidth, boardHeight);

    SetTargetFPS(60);

    int x_offset = (screenWidth - boardWidth) / 2;
    int y_offset = (screenHeight - boardHeight) / 2;

    while (!WindowShouldClose())
    {
        switch (game_state) {
            case STATE_P1W:
            {
                if (IsKeyDown(KEY_LEFT_SHIFT))
                {
                    ResetGame(&paddle1, &paddle2, balls, boardWidth, boardHeight);
                    SpawnBall(balls, boardWidth, boardHeight);
                    game_state = STATE_PLAYING;
                }
            } break;

            case STATE_P2W:
            {
                if (IsKeyDown(KEY_RIGHT_SHIFT))
                {
                    ResetGame(&paddle1, &paddle2, balls, boardWidth, boardHeight);
                    SpawnBall(balls, boardWidth, boardHeight);
                    game_state = STATE_PLAYING;
                }
            } break;

            case STATE_INACTIVE:
            {
                if (GetKeyPressed() != 0)
                {
                    game_state = STATE_PLAYING;
                    SpawnBall(balls, boardWidth, boardHeight);
                }
            } break;

            case STATE_PLAYING:
            {
                hue++;
                if (hue >= 360.0f) hue = 0.0f;

                // input
                UpdatePaddle(&paddle1, KEY_W, KEY_S, KEY_LEFT_SHIFT, boardHeight);
                UpdatePaddle(&paddle2, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT, boardHeight);

            } break;
        }

        Rectangle paddle1Rect = { (float)paddle1.x_pos, (float)paddle1.y_pos, (float)paddle1.width, (float)paddle1.height };
        Rectangle paddle2Rect = { (float)paddle2.x_pos, (float)paddle2.y_pos, (float)paddle2.width, (float)paddle2.height };

        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (!balls[i].active) continue;

            if (balls[i].isPlayBall && game_state != STATE_PLAYING)
            {
                continue;
            }

            // move ball
            balls[i].x_pos = balls[i].x_pos + balls[i].ball_speedx;
            balls[i].y_pos = balls[i].y_pos + balls[i].ball_speedy;

            if (balls[i].isPlayBall)
            {
                // logic for the MAIN ball (inside the board)
                if(balls[i].y_pos <= BORDER_THICKNESS){
                    balls[i].ball_speedy = balls[i].ball_speedy * -1;
                    balls[i].ball_speedx *= WALL_BOUNCE_SPEEDUP;
                    balls[i].ball_speedy *= WALL_BOUNCE_SPEEDUP;
                }
                if((balls[i].y_pos + balls[i].height) >= (boardHeight - BORDER_THICKNESS)){
                    balls[i].ball_speedy = balls[i].ball_speedy * -1;
                    balls[i].ball_speedx *= WALL_BOUNCE_SPEEDUP;
                    balls[i].ball_speedy *= WALL_BOUNCE_SPEEDUP;
                }

                // paddles
                Rectangle ballRect = { (float)balls[i].x_pos, (float)balls[i].y_pos, (float)balls[i].width, (float)balls[i].height };
                if (CheckCollisionRecs(ballRect, paddle1Rect))
                {
                    balls[i].ball_speedx = balls[i].ball_speedx * -1;
                    balls[i].ball_speedx *= PADDLE_BOUNCE_SPEEDUP;
                }
                if (CheckCollisionRecs(ballRect, paddle2Rect))
                {
                    balls[i].ball_speedx = balls[i].ball_speedx * -1;
                    balls[i].ball_speedx *= PADDLE_BOUNCE_SPEEDUP;
                }

                if(balls[i].x_pos < 0){
                    paddle2.score++;
                    if(paddle2.score > (paddle1.score)+1){
                        game_state = STATE_P2W;
                    }
                    balls[i].isPlayBall = false;
                    balls[i].x_pos += x_offset;
                    balls[i].y_pos += y_offset;
                    if (game_state == STATE_PLAYING) SpawnBall(balls, boardWidth, boardHeight);
                }

                if(balls[i].x_pos > boardWidth){
                    paddle1.score++;
                    if(paddle1.score > (paddle2.score)+1){
                        game_state = STATE_P1W;
                    }
                    balls[i].isPlayBall = false;
                    balls[i].x_pos += x_offset;
                    balls[i].y_pos += y_offset;
                    if (game_state == STATE_PLAYING) SpawnBall(balls, boardWidth, boardHeight);
                }
            }
            else
            {
                // logic for TRAIL balls (outside the board)
                if (balls[i].x_pos < -balls[i].width || balls[i].x_pos > screenWidth)
                {
                    balls[i].active = false;
                }
            }
        }

        BeginTextureMode(target);
        ClearBackground(BLACK);
        DrawRectangle(0, 0, boardWidth, BORDER_THICKNESS, ColorFromHSV(hue, 1.0, 1.0)); // ROOF
        DrawRectangle(0, boardHeight - BORDER_THICKNESS, boardWidth, BORDER_THICKNESS, ColorFromHSV(hue, 1.0, 1.0)); //FLOOR
        DrawRectangle(paddle1.x_pos, paddle1.y_pos, paddle1.width, paddle1.height, ColorFromHSV((paddle1.stamina / 100.0f) * 120, 1.0f, 1.0f));
        DrawRectangle(paddle2.x_pos, paddle2.y_pos, paddle2.width, paddle2.height, ColorFromHSV((paddle2.stamina / 100.0f) * 120, 1.0f, 1.0f));
        DrawText(TextFormat("%d", paddle1.score), (paddle1.x_pos)+20, (paddle1.y_pos)+25, 20, ColorFromHSV(hue, 1.0, 1.0)); //paddle1 score
        DrawText(TextFormat("%d", paddle2.score), (paddle2.x_pos)-20, (paddle2.y_pos)+25, 20, ColorFromHSV(hue, 1.0, 1.0)); //paddle2 score

        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (balls[i].active && balls[i].isPlayBall)
            {
                DrawRectangle(balls[i].x_pos, balls[i].y_pos, balls[i].width, balls[i].height, ColorFromHSV(hue, 1.0, 1.0));
            }
        }
        EndTextureMode();

        // real screen
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTextureRec(target.texture,
                       (Rectangle){ 0, 0, (float)boardWidth, (float)-boardHeight },
                       (Vector2){ (float)x_offset, (float)y_offset },
                       WHITE);

        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (balls[i].active && !balls[i].isPlayBall)
            {
                DrawRectangle(balls[i].x_pos, balls[i].y_pos, balls[i].width, balls[i].height, ColorFromHSV(hue, 1.0, 1.0));
            }
        }

        if (game_state == STATE_INACTIVE) { }
        if (game_state == STATE_P1W)
        {
            DrawText("Left Shift to Rematch", x_offset + boardWidth / 2 - 160, y_offset + boardHeight / 2 + 20, 20, LIGHTGRAY);
        }
        if (game_state == STATE_P2W)
        {
            DrawText("Right Shift to Rematch", x_offset + boardWidth / 2 - 170, y_offset + boardHeight / 2 + 20, 20, LIGHTGRAY);
        }
        EndDrawing();
    }

    UnloadRenderTexture(target); // unload virtual canvas
    CloseWindow();

    return 0;
}
