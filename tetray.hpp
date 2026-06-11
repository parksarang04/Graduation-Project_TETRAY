#pragma once

#include <cstdint>
#include <string>
#include "./raylib.h"

// 테트리스 블록(미노) 종류 정의
#define MINO_NONE   (byte)0
#define Z_MINO      (byte)1
#define L_MINO      (byte)2
#define O_MINO      (byte)3
#define S_MINO      (byte)4
#define I_MINO      (byte)5
#define J_MINO      (byte)6
#define T_MINO      (byte)7

// 이동 및 회전 방향 정의
#define LEFT    (sbyte)-1
#define RIGHT   (sbyte) 1
#define HALF    (sbyte) 2

typedef std::uint_fast8_t byte;
typedef std::int_fast8_t sbyte;
typedef unsigned long score_t;

namespace tet
{
    static constexpr sbyte BOARD_DEPTH = 25;
    static constexpr sbyte BOARD_WIDTH = 10;

    static constexpr byte HOLD_SIZE = 5;

    static constexpr byte INDICATE_WIDTH = 6;
    static constexpr byte INDICATE_HEGHIT = 25;

    void SetSeed();

    class Game
    {
    public:
        Game();
        Game(const char* instanceName);
        Game(std::string instanceName);
        ~Game();

        void Init();
        void Run();
        void Stop();
        bool IsEnd();
        void TryMove(const sbyte& to);
        void TryRotate(const sbyte& to);
        void SoftDrop();
        void HardDrop();
        void Hold();
        Color GetColor(const sbyte& row, const sbyte& column);
        Color GetHoldColor(const byte& row, const byte& column);
        Color GetIndicateColor(const byte& row, const byte& column);
        score_t GetScore();
        score_t GetHighScore();

        // 오디오 규격을 맞추기 위한 업데이트 함수 선언
        void UpdateMusic();

    private:
        struct Impl;
        Impl* impl;

        bool signal;

        score_t score;
        score_t highScore;
        std::string scoreFilePath;

        int coloredBoard[BOARD_DEPTH];
        byte currentShape;
        byte currentRotate;
        sbyte currentBitCount;
        sbyte currentDepth;

        sbyte previewDepth;

        size_t shuffled_index;
        byte list[6];

        byte holdMino;
        bool isHeld;

        byte getNext();
        void fillNextMinos();
        bool placeable(const sbyte& bitCount, const sbyte& depth, const byte& shapeIndex, const byte& shapeRotateIndex);
        void draw(const sbyte& bitCount, const sbyte& depth, const byte& shapeIndex, const byte& shapeRotateIndex);
        score_t updateBoard();
        bool trySuperRotationSystem(const byte& target, sbyte* out_x, sbyte* out_y);
        void updatePreview();
        void run();
        void loadHighScore();
        void saveHighScore();
    };
} // namespace tet