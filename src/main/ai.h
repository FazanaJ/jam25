#pragma once

enum AIDifficulty {
    AIDIFF_EASY,
    AIDIFF_MEDIUM,
    AIDIFF_HARD
};

extern float gAICursorTarget[4][2];
extern int gAIPlaced[4];
extern int gAIUpdateTimes[4];
extern int gAIDifficulty[4];
extern float gHeatmap[2];
extern int gActiveTroopCount;

void ai_run(int playerID, int updateRate);