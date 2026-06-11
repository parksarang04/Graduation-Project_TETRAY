#include <fstream>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <queue>
#include <span>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <chrono>
#include <cmath>

#include "tetray.hpp"


// 디지털 신호처리(DSP) 기반 8비트 사이버펑크 오디오 합성 엔진 (안전성 강화 버전)

static constexpr int AUDIO_SAMPLE_RATE = 44100;
const float CYBER_BASS_LINE[] = { 55.00f, 55.00f, 65.41f, 73.42f, 82.41f, 82.41f, 73.42f, 65.41f };
const int BASS_LINE_COUNT = 8;

float audioPhaseTime = 0.0f;
int currentBassIndex = 0;
float noteDurationTimer = 0.0f;

// 런타임 크래시 방지를 위한 글로벌 오디오 상태 관리 변수
static bool isAudioInitialized = false;
static AudioStream proceduralSynthStream;

void RealtimeAudioSynthesisCallback(void* buffer, unsigned int frames)
{
    float* monoOutputBuffer = (float*)buffer;
    for (unsigned int i = 0; i < frames; i++)
    {
        noteDurationTimer += 1.0f / AUDIO_SAMPLE_RATE;
        if (noteDurationTimer >= 0.125f)
        {
            noteDurationTimer = 0.0f;
            currentBassIndex = (currentBassIndex + 1) % BASS_LINE_COUNT;
        }

        float targetFrequency = CYBER_BASS_LINE[currentBassIndex];
        float rawSineWave = std::sin(2.0f * PI * targetFrequency * audioPhaseTime);
        float chipTuneTone = (rawSineWave > 0.0f) ? 0.12f : -0.12f;

        monoOutputBuffer[i] = chipTuneTone;

        audioPhaseTime += 1.0f / AUDIO_SAMPLE_RATE;
        if (audioPhaseTime > 100.0f) audioPhaseTime = 0.0f;
    }
}
// =========================================================================

namespace tet
{
    namespace shape
    {
        static constexpr sbyte SHAPE_COUNT = 8;
        static constexpr sbyte SHAPE_ROTATE_COUNT = 4;
        static constexpr sbyte SHAPE_SIZE = 5;
        static constexpr sbyte SHAPE_OFFSET = SHAPE_SIZE / 2;
        static constexpr sbyte SHAPE_LINE_MASK = 0b11111;

        static const Color colors[] = { BLACK, RED, ORANGE, YELLOW, GREEN, SKYBLUE, BLUE, PURPLE };
        static constexpr int shapes[SHAPE_COUNT][SHAPE_ROTATE_COUNT] =
        {
            { 0, 0, 0, 0, }, // NONE
            { // Z
                0b00000'00010'00110'00100'00000,
                0b00000'00000'01100'00110'00000,
                0b00000'00100'01100'01000'00000,
                0b00000'01100'00110'00000'00000,
            },
            { // L
                0b00000'00100'00100'00110'00000,
                0b00000'00000'01110'01000'00000,
                0b00000'01100'00100'00100'00000,
                0b00000'00010'01110'00000'00000,
            },
            { // O
                0b00000'00000'00110'00110'00000,
                0b00000'00000'01100'01100'00000,
                0b00000'01100'01100'00000'00000,
                0b00000'00110'00110'00000'00000,
            },
            { // S
                0b00000'00100'00110'00010'00000,
                0b00000'00000'00110'01100'00000,
                0b00000'01000'01100'00100'00000,
                0b00000'00110'01100'00000'00000,
            },
            { // I
                0b00000'00100'00100'00100'00100,
                0b00000'00000'11110'00000'00000,
                0b00100'00100'00100'00100'00000,
                0b00000'00000'01111'00000'00000,
            },
            { // J
                0b00000'00110'00100'00100'00000,
                0b00000'00000'01110'00010'00000,
                0b00000'00100'00100'01100'00000,
                0b00000'01000'01110'00000'00000,
            },
            { // T
                0b00000'00100'00110'00100'00000,
                0b00000'00000'01110'00100'00000,
                0b00000'00100'01100'00100'00000,
                0b00000'00100'01110'00000'00000,
            },
        };

        static constexpr std::pair<sbyte, sbyte> JLSTZOffset[][5] =
        {
            { {  0,  0 }, { -1,  0 }, { -1, -1 }, {  0,  2 }, { -1,  2 } },
            { {  0,  0 }, {  0,  0 }, {  0,  0 }, {  0,  0 }, {  0,  0 } },
            { {  0,  0 }, {  1,  0 }, {  1, -1 }, {  0,  2 }, {  1,  2 } },
            { {  0,  0 }, {  0,  0 }, {  0,  0 }, {  0,  0 }, {  0,  0 } }
        };

        static constexpr std::pair<sbyte, sbyte> OOffset[][1] =
        {
            { { -1,  0 } }, { {  0,  0 } }, { {  0, -1 } }, { { -1, -1 } }
        };

        static constexpr std::pair<sbyte, sbyte> IOffset[][5] =
        {
            { {  0,  1 }, {  0,  1 }, {  0,  1 }, {  0, -1 }, {  0,  2 } },
            { {  0,  0 }, { -1,  0 }, {  2,  0 }, { -1,  0 }, {  2,  0 } },
            { { -1,  0 }, {  0,  0 }, {  0,  0 }, {  0,  1 }, {  0, -2 } },
            { { -1,  1 }, {  1,  1 }, { -2,  1 }, {  1,  0 }, { -2,  0 } }
        };

        static inline bool getBit(const byte& shape, const byte& rotate, const byte& r, const byte& c)
        {
            return (shapes[shape][rotate] >> (SHAPE_SIZE * r + c)) & 1;
        }

        static inline std::span<const std::pair<sbyte, sbyte>> getSRSOffset(const byte& minoType, const byte& rotate)
        {
            if (rotate >= 4) return {};
            switch (minoType)
            {
            case J_MINO: case L_MINO: case S_MINO: case T_MINO: case Z_MINO:
                return JLSTZOffset[rotate];
            case O_MINO: return OOffset[rotate];
            case I_MINO: return IOffset[rotate];
            default: return {};
            }
        }
    } // namespace shapes

    using namespace shape;

    static constexpr sbyte CREATE_DEPTH = 5;
    static constexpr sbyte MASK_COUNT = 3;
    static constexpr int COLOR_MASK = 0b111;
    static constexpr int INITIALIZE = 0;
    static constexpr byte DEFAULT_ROTATE = 1;
    static constexpr sbyte DEFAULT_BIT_COUNT = BOARD_WIDTH / 2 - 1;

    static size_t seed;
    static size_t multi;
    static size_t add;

    void SetSeed()
    {
        seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        multi = 420 * (seed % 11 + 1) + 1;
        std::vector<size_t> s{ 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 1013, 1357, 2011, 3137, 4001, 4999 };
        add = s[seed % s.size()];
    }

    static inline long long getBitSlicing(long long value, const byte& begin, const byte& end)
    {
        if (begin >= end) return 0LL;
        value >>= begin;
        if (end >= sizeof(long long) * 8UL) return value;
        return value & ((1LL << (end - begin)) - 1LL);
    }

    struct Game::Impl
    {
        std::jthread runner;
        std::mutex dataMutex;
        std::condition_variable cv;
        std::atomic<bool> isRun;
        std::atomic<bool> isPlay;
        std::atomic<time_t> elapsed;
        std::atomic<time_t> rate;
        std::queue<byte> readied;
    };

    Game::Game() : impl(new Impl())
    {
        scoreFilePath = "./score_unknown";
        Init();
    }

    Game::Game(const char* instanceName) : impl(new Impl())
    {
        if (instanceName == nullptr || instanceName[0] == '\0')
        {
            scoreFilePath = "./score_unknown";
        }
        else
        {
            scoreFilePath = "./score_";
            std::string in(instanceName);
            size_t n = in.length();
            if (n == 0)
            {
                scoreFilePath += "unknown";
            }
            else
            {
                for (size_t i = 0; i < n; i++)
                {
                    char c = instanceName[i];
                    if (!(('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')))
                    {
                        in[i] = '_';
                    }
                }
                scoreFilePath += in;
            }
        }
        Init();
    }

    Game::Game(std::string instanceName) : Game(instanceName.c_str()) {}

    Game::~Game()
    {
        Stop();
        if (impl->runner.joinable())
        {
            impl->runner.join();
        }

        //오디오 자원 수동 해제 아키텍처 적용
        if (isAudioInitialized)
        {
            UnloadAudioStream(proceduralSynthStream); // 1. 스트림 자원을 먼저 파괴
            CloseAudioDevice();                     // 2. 오디오 장치 셧다운
            isAudioInitialized = false;
        }

        delete impl;
    }

    void Game::fillNextMinos()
    {
        for (size_t i = 0; i < 5; i++) list[i] = list[i + 1];

        if (impl->readied.empty())
        {
            std::vector<byte> base = { 1, 2, 3, 4, 5, 6, 7 };
            size_t idx = shuffled_index;
            size_t f = 720;
            for (int i = 6; i > 0; i--)
            {
                auto mok = idx / f;
                idx %= f;
                impl->readied.push(base[mok]);
                base.erase(base.begin() + mok);
                f /= i;
            }
            impl->readied.push(base[0]);

            shuffled_index *= multi;
            shuffled_index += add;
            shuffled_index %= 5040;
        }

        list[5] = impl->readied.front();
        impl->readied.pop();
    }

    byte Game::getNext()
    {
        auto v = list[0];
        fillNextMinos();
        return v;
    }

    bool Game::placeable(const sbyte& bitCount, const sbyte& depth, const byte& shapeIndex, const byte& shapeRotateIndex)
    {
        if (bitCount < 0 || bitCount >= BOARD_WIDTH || depth < 0 || depth >= BOARD_DEPTH) return false;

        int shape = shapes[shapeIndex][shapeRotateIndex];
        for (sbyte i = -SHAPE_OFFSET; i <= SHAPE_OFFSET; i++)
        {
            auto shapeLine = getBitSlicing(shape, (SHAPE_OFFSET + i) * SHAPE_SIZE, (SHAPE_OFFSET + i + 1) * SHAPE_SIZE);
            if (shapeLine == 0) continue;
            if (depth + i < 0 || depth + i >= BOARD_DEPTH) return false;

            int line = coloredBoard[depth + i];
            for (sbyte j = -SHAPE_OFFSET; j <= SHAPE_OFFSET; j++)
            {
                auto shapeBit = (shapeLine >> (SHAPE_OFFSET + j)) & 1;
                if (shapeBit == 0) continue;
                if (bitCount + j < 0 || bitCount + j >= BOARD_WIDTH) return false;

                auto block = getBitSlicing(line, (bitCount + j) * MASK_COUNT, (bitCount + j + 1) * MASK_COUNT);
                if (block != 0 && shapeBit != 0) return false;
            }
        }
        return true;
    }

    void Game::draw(const sbyte& bitCount, const sbyte& depth, const byte& shapeIndex, const byte& shapeRotateIndex)
    {
        for (sbyte i = -SHAPE_OFFSET; i <= SHAPE_OFFSET; i++)
        {
            if (depth + i < 0 || depth + i >= BOARD_DEPTH) continue;

            auto shapeLine = getBitSlicing(shapes[shapeIndex][shapeRotateIndex], (SHAPE_OFFSET + i) * SHAPE_SIZE, (SHAPE_OFFSET + i + 1) * SHAPE_SIZE);
            if (shapeLine == 0) continue;

            for (sbyte j = -SHAPE_OFFSET; j <= SHAPE_OFFSET; j++)
            {
                if (bitCount + j < 0 || bitCount + j >= BOARD_WIDTH) continue;
                if (((shapeLine >> (SHAPE_OFFSET + j)) & 1) == 0) continue;

                coloredBoard[depth + i] |= (int)shapeIndex << ((bitCount + j) * MASK_COUNT);
            }
        }
    }

    score_t Game::updateBoard()
    {
        unsigned long long result = 0;
        sbyte depth = BOARD_DEPTH - 1;
        while (depth >= 0)
        {
            bool isFilled = [this, depth]()
                {
                    for (int i = 0; i < 10; i++)
                    {
                        if (((coloredBoard[depth] >> (i * MASK_COUNT)) & COLOR_MASK) == 0) return false;
                    }
                    return true;
                }();

            if (!isFilled)
            {
                depth--;
                continue;
            }

            for (sbyte i = depth; i > 0; i--) coloredBoard[i] = coloredBoard[i - 1];
            result += result * 2 + 50;
        }
        return result;
    }

    bool Game::trySuperRotationSystem(const byte& target, sbyte* out_x, sbyte* out_y)
    {
        auto from = getSRSOffset(currentShape, currentRotate);
        auto to = getSRSOffset(currentShape, target);
        auto size = from.size();
        for (size_t i = 0; i < size; i++)
        {
            sbyte x = from[i].first - to[i].first;
            sbyte y = to[i].second - from[i].second;
            if (placeable(currentBitCount + x, currentDepth + y, currentShape, target))
            {
                *out_x = x; *out_y = y;
                return true;
            }
        }
        return false;
    }

    void Game::updatePreview()
    {
        auto depth = currentDepth;
        while (placeable(currentBitCount, depth + 1, currentShape, currentRotate)) depth++;
        previewDepth = depth;
    }

    void Game::run()
    {
        while (impl->isRun && impl->isPlay)
        {
            std::unique_lock<std::mutex> lock(impl->dataMutex);
            impl->cv.wait_for(lock, std::chrono::milliseconds(1000 / impl->rate), [this] { return signal; });
            signal = false;

            if (placeable(currentBitCount, currentDepth + 1, currentShape, currentRotate))
            {
                currentDepth++;
            }
            else
            {
                draw(currentBitCount, currentDepth, currentShape, currentRotate);

                currentShape = getNext();
                currentRotate = DEFAULT_ROTATE;
                currentBitCount = DEFAULT_BIT_COUNT;
                currentDepth = CREATE_DEPTH;
                isHeld = false;

                if (!placeable(currentBitCount, currentDepth, currentShape, currentRotate))
                {
                    impl->isRun = false;
                    impl->isPlay = false;
                    break;
                }
                score += updateBoard();
            }
            updatePreview();
            impl->elapsed++;
        }

        if (score > highScore)
        {
            highScore = score;
            saveHighScore();
        }
    }

    void Game::Init()
    {
        impl->elapsed = 0ULL;
        impl->rate = 1;
        for (int i = 0; i < BOARD_DEPTH; i++) coloredBoard[i] = INITIALIZE;
        impl->isRun = false;
        impl->isPlay = false;

        shuffled_index = seed % 5040;
        for (int i = 0; i < 6; i++) fillNextMinos();

        holdMino = 0;

        std::lock_guard<std::mutex> lock(impl->dataMutex);

        currentShape = getNext();
        currentRotate = DEFAULT_ROTATE;
        currentDepth = CREATE_DEPTH;
        currentBitCount = DEFAULT_BIT_COUNT;

        updatePreview();
        score = 0;
        highScore = 0;
        loadHighScore();

        // 중복 초기화 방지 및 안전한 장치 기동 처리
        if (!isAudioInitialized)
        {
            InitAudioDevice();
            if (IsAudioDeviceReady()) // 하드웨어가 정상적으로 준비되었는지 검증
            {
                proceduralSynthStream = LoadAudioStream(AUDIO_SAMPLE_RATE, 32, 1);
                SetAudioStreamCallback(proceduralSynthStream, RealtimeAudioSynthesisCallback);
                PlayAudioStream(proceduralSynthStream);
                isAudioInitialized = true;
            }
        }
    }

    void Game::loadHighScore()
    {
        std::ifstream iFile(scoreFilePath);
        if (iFile.is_open()) iFile >> highScore;
        else
        {
            std::ofstream oFile(scoreFilePath);
            if (oFile.is_open()) oFile << 0;
        }
    }

    void Game::saveHighScore()
    {
        std::ofstream scoreFile(scoreFilePath);
        if (scoreFile.is_open()) scoreFile << highScore;
    }

    void Game::Run()
    {
        if (impl->isRun) return;
        impl->isRun = true;
        impl->isPlay = true;
        impl->runner = std::jthread(&Game::run, this);
    }

    bool Game::IsEnd() { return !impl->isPlay.load(); }
    void Game::Stop() { impl->isRun = false; impl->cv.notify_one(); }

    void Game::TryMove(const sbyte& to)
    {
        std::lock_guard<std::mutex> lock(impl->dataMutex);
        if (placeable(currentBitCount + to, currentDepth, currentShape, currentRotate))
        {
            currentBitCount += to;
            updatePreview();
        }
    }

    void Game::TryRotate(const sbyte& rotate)
    {
        std::lock_guard<std::mutex> lock(impl->dataMutex);
        byte rotateTo = (currentRotate + 4 + rotate) & 0b11;
        sbyte x, y;
        if (trySuperRotationSystem(rotateTo, &x, &y))
        {
            currentBitCount += x; currentDepth += y; currentRotate = rotateTo;
            updatePreview();
        }
    }

    void Game::SoftDrop() { std::lock_guard<std::mutex> lock(impl->dataMutex); signal = true; impl->cv.notify_one(); }
    void Game::HardDrop() { std::lock_guard<std::mutex> lock(impl->dataMutex); currentDepth = previewDepth; signal = true; impl->cv.notify_one(); }

    void Game::Hold()
    {
        std::lock_guard<std::mutex> lock(impl->dataMutex);
        if (isHeld) return;
        if (holdMino == 0) { holdMino = currentShape; currentShape = getNext(); }
        else { auto temp = currentShape; currentShape = holdMino; holdMino = temp; }

        currentRotate = DEFAULT_ROTATE;
        currentBitCount = DEFAULT_BIT_COUNT;
        currentDepth = CREATE_DEPTH - 1;
        isHeld = true;
        signal = true;
        impl->cv.notify_one();
    }

    Color Game::GetColor(const sbyte& row, const sbyte& column)
    {
        if (row < 0 || column < 0 || row >= BOARD_DEPTH || column >= BOARD_WIDTH) return BLACK;

        std::lock_guard<std::mutex> b_lock(impl->dataMutex);
        auto value = (coloredBoard[row] >> (column * MASK_COUNT)) & COLOR_MASK;
        auto res = colors[value];
        if (value == 0)
        {
            sbyte rowDiff = 0, colDiff = 0;
            if (std::abs(rowDiff = row - previewDepth) <= 2 && std::abs(colDiff = column - currentBitCount) <= 2)
            {
                value = getBit(currentShape, currentRotate, SHAPE_OFFSET + rowDiff, SHAPE_OFFSET + colDiff) * currentShape;
                res = ColorLerp(colors[value], BLACK, 0.5f);
            }
            if (std::abs(rowDiff = row - currentDepth) <= 2 && std::abs(colDiff = column - currentBitCount) <= 2)
            {
                value = getBit(currentShape, currentRotate, SHAPE_OFFSET + rowDiff, SHAPE_OFFSET + colDiff) * currentShape;
                if (value) res = colors[value];
            }
        }
        return res;
    }

    Color Game::GetHoldColor(const byte& row, const byte& column)
    {
        if (row >= SHAPE_SIZE || column >= SHAPE_SIZE) return BLANK;
        return colors[getBit(holdMino, 1, row, column) * holdMino];
    }

    Color Game::GetIndicateColor(const byte& row, const byte& column)
    {
        if (row >= INDICATE_HEGHIT || column >= INDICATE_WIDTH) return BLANK;
        if (row == INDICATE_HEGHIT - 1 || column == INDICATE_WIDTH - 1) return BLACK;

        constexpr byte boundarySize = 4;
        byte listIdx = row / boundarySize;
        byte shapeLineIdx = row % boundarySize;
        byte shapeIdx = list[listIdx];
        return getBit(shapeIdx, 1, shapeLineIdx, column) ? colors[shapeIdx] : BLACK;
    }

    score_t Game::GetScore() { return score; }
    score_t Game::GetHighScore() { return std::max(score, highScore); }
    void Game::UpdateMusic() {}
} // namespace tet