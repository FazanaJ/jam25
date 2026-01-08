#pragma once

enum FloorGaps {
    GAP_NONE,
    GAP_TOP = (1 << 0),
    GAP_BOTTOM = (1 << 1),
    GAP_LEFT = (1 << 2),
    GAP_RIGHT = (1 << 3),
    GAP_UPLEFT = (1 << 4),
    GAP_UPRIGHT = (1 << 5),
    GAP_DOWNLEFT = (1 << 6),
    GAP_DOWNRIGHT = (1 << 7),
};  

extern T3DVertPacked *gLevelFloorVtx[16];
extern T3DVertPacked *gLevelWallVtx[16];

void game_run(int updateRate, float updateRateF);
void game_init(int levelID, int playerCount);