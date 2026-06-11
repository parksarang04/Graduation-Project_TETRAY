#include <iostream>
#include <string>

#include "raylib.h"
#include "tetray.hpp"
#include "argparse/argparse.hpp"

#ifndef GAME_VERSION
#define GAME_VERSION "UNKNOWN"
#endif

using namespace tet;

const int WIDTH = 800;
const int HEIGHT = 600;
const char *TITLE = "Tetray";

const int FPS = 30;

const int BLOCK_SIZE = 20;
const int OFFSET_X = 50;
const int OFFSET_Y = 0;

std::string name;

void run();

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("game", GAME_VERSION);

    program.add_argument("-N", "--name")
        .help("Sets the username. Defaults to 'unknown' if not specified.\n")
        .metavar("<string>")
        .default_value("unknown");

    try
    {
        program.parse_args(argc, argv);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    name = program.get<std::string>("--name");
    
    run();
    return 0;
}

void run()
{
    InitWindow(WIDTH, HEIGHT, TITLE);
    SetTargetFPS(FPS);

    Game game(name);
reGame:
    SetSeed();
    game.Init();
    game.Run();

    while (!WindowShouldClose())
    {
        if (game.IsEnd())
        {
            if (IsKeyPressed(KEY_R))
            {
                goto reGame;
            }
        }
        else
        {
            int key = GetKeyPressed();
            switch (key)
            {
            case KEY_LEFT:
                game.TryMove(LEFT);
                break;

            case KEY_RIGHT:
                game.TryMove(RIGHT);
                break;

            case KEY_A:
                game.TryRotate(LEFT);
                break;

            case KEY_D:
                game.TryRotate(RIGHT);
                break;

            case KEY_S:
                game.TryRotate(HALF);
                break;

            case KEY_UP:
                game.HardDrop();
                break;

            case KEY_SPACE:
                game.Hold();
                break;

            default:
                break;
            }

            if (IsKeyPressedRepeat(KEY_LEFT))
            {
                game.TryMove(LEFT);
            }
            else if (IsKeyPressedRepeat(KEY_RIGHT))
            {
                game.TryMove(RIGHT);
            }

            if (IsKeyDown(KEY_DOWN))
            {
                game.SoftDrop();
            }
        }

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            // ui
            DrawText("[Left] : move left",  475, 295, 20, GRAY);
            DrawText("[Right] : move Righ", 475, 320, 20, GRAY);
            DrawText("[Up] : hard drop",    475, 345, 20, GRAY);
            DrawText("[Down] : soft drop",  475, 370, 20, GRAY);

            DrawText("[R] : replay (when game over)",  475, 260, 20, GRAY);

            DrawText("[A] : spin left",  475, 405, 20, GRAY);
            DrawText("[S] : spin half",  475, 430, 20, GRAY);
            DrawText("[D] : spin right", 475, 455, 20, GRAY);
            DrawText("[Space] : hold",   475, 480, 20, GRAY);

            DrawText("Hold", 275, 130, 20, GRAY);
            DrawText("Next", 275, 230, 20, GRAY);

            auto highScore = game.GetHighScore(), score = game.GetScore();

            char highScoreText[32], scoreText[32];
            sprintf(highScoreText, "High Score\n%lu", highScore);
            sprintf(scoreText, "Score\n%lu", score);

            DrawText(highScoreText, 475, 25, 20, GRAY);
            DrawText(scoreText, 475, 75, 20, GRAY);

            // board
            for (byte r = 0; r < BOARD_DEPTH; r++)
            {
                for (byte c = 0; c < BOARD_WIDTH; c++)
                {
                    DrawRectangle(
                        c * BLOCK_SIZE + OFFSET_X,
                        r * BLOCK_SIZE + OFFSET_Y,
                        BLOCK_SIZE,
                        BLOCK_SIZE,
                        game.GetColor(r, c)
                    );
                }
            }

            // hold
            for (byte r = 0; r < HOLD_SIZE; r++)
            {
                for (byte c = 0; c < HOLD_SIZE; c++)
                {
                    DrawRectangle(c * 15 + 275, r * 15 + 150, 15, 15, game.GetHoldColor(r, c));
                }
            }

            // indicate
            for (byte r = 0; r < INDICATE_HEGHIT; r++)
            {
                for (byte c = 0; c < INDICATE_WIDTH; c++)
                {
                    DrawRectangle(c * 10 + 275, r * 10 + 250, 10, 10, game.GetIndicateColor(r, c));
                }
            }

            // DrawLine(0, tet::BOARD_DEPTH * BLOCK_SIZE, tet::BOARD_WIDTH * BLOCK_SIZE, tet::BOARD_DEPTH * BLOCK_SIZE, WHITE);
            // DrawLine(tet::BOARD_WIDTH * BLOCK_SIZE, 0, tet::BOARD_WIDTH * BLOCK_SIZE, tet::BOARD_DEPTH * BLOCK_SIZE, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    game.Stop();
}