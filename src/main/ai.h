#pragma once

enum AIDifficulty {
    AIDIFF_EASY,
    AIDIFF_MEDIUM,
    AIDIFF_HARD
};

extern float gAICursorTarget[4][2];

void ai_run(int playerID, int updateRate);