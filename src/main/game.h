#pragma once

extern T3DVertPacked *gLevelFloorVtx[16];
extern T3DVertPacked *gLevelWallVtx[16];

void game_run(int updateRate, float updateRateF);
void game_init(int levelID, int playerCount);