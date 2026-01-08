#pragma once
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>
#include <t3d/t3dskeleton.h>

#define TROOP_COUNT 100
#define LEVEL_COUNT 8
#define ARROW_TOTAL (12 * 10)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

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

enum LogicRates {
    LOGIC_60FPS = 1,
    LOGIC_30FPS,
    LOGIC_20FPS,
    LOGIC_15FPS
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
    uint8_t frame;
    uint8_t spriteID;
} TroopObj;

typedef struct LevelData {
	uint8_t floorTiles[12 * 10];
	uint8_t floorTextures[12 * 10];
	uint8_t wallTiles[12 * 10];
	uint8_t wallTextures[12 * 10];
	uint8_t objects[12 * 10];
} LevelData;

extern int mapHeights[];
extern uint8_t *gMapData;
extern ArrowData gMapArrows[];
extern T3DVec3 gBasePos[4];
extern T3DVec3 gSpawnerPos[8];
extern int gSpawnerCD[8];
extern int gPoints[4];
extern int gPointsVisual[4];
extern int gSpawnerGlobalTime;
extern int gSpawnerRuinTime;
extern int gSpawnerRuinID;
extern int gPointerCD[4];
extern int gPointerGlobalTime;
extern int gGameTimer;
extern sprite_t *gCursorSprite;
extern sprite_t *gArrowSprite;
extern sprite_t *gPointerSprite;
extern sprite_t *gNumberSprites;
extern sprite_t *gNumberBGSprite;
extern sprite_t *gScoreBoardSprite;
extern sprite_t *gScoreBorderSprite;
extern sprite_t *gScoreBoardPlayerSprites[4];
extern sprite_t *gLevelSprites[16];
extern sprite_t *gScoreLeaderSprite;
extern sprite_t *gPauseOptionSprites[5];
extern sprite_t *gScoreUnderlaySprite;
extern float gPlayerCursors[4][3];
extern float gPlayerPointers[4][2];
extern color_t gPlayerColours[4];
extern int gPlayerCount;
extern int gCursorCount;
extern T3DMat4FP gBaseMtx[4];
extern T3DMat4FP gSpawnermtx[8];
extern TroopObj gTroops[TROOP_COUNT];
extern int gMenuID;
extern int gLevelID;
extern int gGamePaused;
extern int gPlayerIDs[4];
extern int ticks;
extern int gPlayerWins[4];
extern int gTimerStage;
extern T3DVec3 gCameraPos;
extern T3DVec3 gCameraFocus;
extern LevelData *gCurrentLevel;
extern float gMapOffsetX;
extern int gClearblack;
extern rdpq_font_t *gFonts[4];
extern int gArrowCount;

extern LevelData gMapLevel2[];
extern uint8_t gMapLevelHeights[];

extern rspq_block_t *gArrowBlock;
extern rspq_block_t *cursor;
extern rspq_block_t *dplMapWalls[16];
extern rspq_block_t *dplMapFloor[16];
extern rspq_block_t *gBaseBlock;
extern rspq_block_t *gBaseBlock2;
extern rspq_block_t *dplMapBottom[16];
extern rspq_block_t *gTroopBlock;
extern sprite_t *gTroopSprites[4][5][6];

extern T3DModel *gArmyGatorModel;
extern rspq_block_t *gArmyGatorBlock;
extern T3DSkeleton gArmyGatorSkel;
extern T3DAnim gArmyGatorAnims[4];
extern T3DModel *gMenuLevelModel;
extern T3DModel *gBaseBuildingModel;
extern T3DModel *gBaseBuildingShadowModel;
extern int gArmyGatorAnimID;

float lerpf(float a, float b, float f);
void reset_game_time(void);