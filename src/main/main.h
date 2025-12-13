#pragma once
#include <t3d/t3d.h>

#define TROOP_COUNT 100

enum PlayerIDs {
    PLAYER_NONE = -1,
    PLAYER_1,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4,
    PLAYER_ALL // Used for a combination of everybody's input.
};

enum TroopIDs {
    TROOP_NORMAL,
    TROOP_HOOLIGAN,
    TROOP_25,
    TROOP_50,
    TROOP_ROULETTE
};

enum Directions {
    DIR_NONE,
    DIR_DOWN,
    DIR_LEFT,
    DIR_UP,
    DIR_RIGHT
};

typedef struct ArrowData {
	uint8_t playerID;
	uint8_t dir;
} ArrowData;

typedef struct TroopObj {
	T3DVec3 pos;
	T3DVec3 posTarget;
	uint8_t dir;
	uint8_t type;
	uint8_t active;
} TroopObj;

extern int mapHeights[];
extern uint8_t map[13 * 11];
extern ArrowData gMapArrows[13 * 10];
extern T3DVec3 gBasePos[4];
extern T3DVec3 gSpawnerPos[8];
extern int gSpawnerCD[8];
extern int gPoints[4];
extern int gSpawnerGlobalTime;
extern int gSpawnerRuinTime;
extern int gSpawnerRuinID;
extern int gPointerCD[4];
extern int gPointerGlobalTime;
extern int gGameTimer;
extern sprite_t *gCursorSprite;
extern sprite_t *gArrowSprite;
extern sprite_t *gPointerSprite;
extern float gPlayerCursors[4][3];
extern float gPlayerPointers[4][2];
extern int gPlayerCount;
extern int gCursorCount;
extern TroopObj gTroops[TROOP_COUNT];
extern rspq_block_t *gArrowBlock;
extern rspq_block_t *cursor;
extern rspq_block_t *dplMapWalls;
extern rspq_block_t *dplMapFloor;
extern rspq_block_t *gBaseBlock;

float approachF(float current, float target, float inc);