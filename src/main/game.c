#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>

#include "ai.h"
#include "audio.h"
#include "boot.h"
#include "input.h"
#include "game.h"
#include "main.h"
#include "menu.h"

T3DVertPacked *gLevelFloorVtx[16];
T3DVertPacked *gLevelWallVtx[16];

static float approachF(float current, float target, float inc) {
    if (current < target) {
        current += inc;
        if (current > target) {
            current = target;
        }
    } else {
        current -= inc;
        if (current < target) {
            current = target;
        }
    }
    return current;
}

static int obj_approach(TroopObj *obj, float updateRateF) {
	int oneTrue = false;
    float var = 3.0f * updateRateF;
	obj->pos.x = approachF(obj->pos.x, obj->posTarget.x, var);
	obj->pos.z = approachF(obj->pos.z, obj->posTarget.z, var);

	if (fabsf(obj->pos.x - obj->posTarget.x) <= var) {
		obj->pos.x = obj->posTarget.x;
		oneTrue = true;
	}
	if (fabsf(obj->pos.z - obj->posTarget.z) <= var) {
		obj->pos.z = obj->posTarget.z;
		if (oneTrue) {
			return true;
		}
	}
	return false;
}

int map_floor_tile(int x, int z) {
	if (x < 0 || z < 0 || x >= 12 || z >= 10) {
		return -1;
	}

	int idx = gCurrentLevel->floorTiles[(z * 12) + x];
	if (idx == 0) {
		return -1;
	} else {
		return gMapLevelHeights[(idx - 1) % 8];
	}
}

int map_wall_tile(int x, int z) {
	if (x < 0 || z < 0 || x >= 12 || z >= 10) {
		return 0;
	}

	int idx = gCurrentLevel->wallTiles[(z * 12) + x];
	if (idx == 0) {
		return 0;
	} else {
		return idx;
	}
}

int troop_wall_collision(int x, int z, int xT, int zT, int dir) {
    int wallCur = map_wall_tile(x, z);
    int wallNext = map_wall_tile(xT, zT);
    
    if (wallCur != 0) {
        int wallType = wallCur;
        
        if (wallType == 1 && (dir == DIR_LEFT || dir == DIR_UP)) {
            if ((dir == DIR_LEFT && (map_wall_tile(x, z - 1) || map_wall_tile(x, z + 1))) ||
                (dir == DIR_UP && (map_wall_tile(x - 1, z) || map_wall_tile(x + 1, z)))) {
                return true;
            }
        }
        if (wallType == 2 && (dir == DIR_LEFT || dir == DIR_DOWN)) {
            if ((dir == DIR_LEFT && (map_wall_tile(x, z - 1) || map_wall_tile(x, z + 1))) ||
                (dir == DIR_DOWN && (map_wall_tile(x - 1, z) || map_wall_tile(x + 1, z)))) {
                return true;
            }
        }
        if (wallType == 3 && (dir == DIR_RIGHT || dir == DIR_UP)) {
            if ((dir == DIR_RIGHT && (map_wall_tile(x, z - 1) || map_wall_tile(x, z + 1))) ||
                (dir == DIR_UP && (map_wall_tile(x - 1, z) || map_wall_tile(x + 1, z)))) {
                return true;
            }
        }
        if (wallType == 4 && (dir == DIR_RIGHT || dir == DIR_DOWN)) {
            if ((dir == DIR_RIGHT && (map_wall_tile(x, z - 1) || map_wall_tile(x, z + 1))) ||
                (dir == DIR_DOWN && (map_wall_tile(x - 1, z) || map_wall_tile(x + 1, z)))) {
                return true;
            }
        }
    }
    
    if (wallNext != 0) {
        int wallType = wallNext;
        
        if (wallType == 1 && (dir == DIR_RIGHT || dir == DIR_DOWN)) {
            if ((dir == DIR_RIGHT && (map_wall_tile(xT, zT - 1) || map_wall_tile(xT, zT + 1))) ||
                (dir == DIR_DOWN && (map_wall_tile(xT - 1, zT) || map_wall_tile(xT + 1, zT)))) {
                return true;
            }
        }
        if (wallType == 2 && (dir == DIR_RIGHT || dir == DIR_UP)) {
            if ((dir == DIR_RIGHT && (map_wall_tile(xT, zT - 1) || map_wall_tile(xT, zT + 1))) ||
                (dir == DIR_UP && (map_wall_tile(xT - 1, zT) || map_wall_tile(xT + 1, zT)))) {
                return true;
            }
        }
        if (wallType == 3 && (dir == DIR_LEFT || dir == DIR_DOWN)) {
            if ((dir == DIR_LEFT && (map_wall_tile(xT, zT - 1) || map_wall_tile(xT, zT + 1))) ||
                (dir == DIR_DOWN && (map_wall_tile(xT - 1, zT) || map_wall_tile(xT + 1, zT)))) {
                return true;
            }
        }
        if (wallType == 4 && (dir == DIR_LEFT || dir == DIR_UP)) {
            if ((dir == DIR_LEFT && (map_wall_tile(xT, zT - 1) || map_wall_tile(xT, zT + 1))) ||
                (dir == DIR_UP && (map_wall_tile(xT - 1, zT) || map_wall_tile(xT + 1, zT)))) {
                return true;
            }
        }
    }
    
    return false;
}


#define MEMALIGN 0x8

#ifndef _ALIGN16
#define _ALIGN16(a) (void *) (((uint32_t) (a) & ~0xF) + 0x10)
#endif
#ifndef _ALIGN8
#define ALIGNCHECK (MEMALIGN - 1)
#define _ALIGN8(a) (void *) (((uint32_t) (a) & ~ALIGNCHECK) + MEMALIGN)
#endif

void game_run(int updateRate, float updateRateF) {
    if (gLevelID == 0) {
        return;
    }
	gCameraFocus.v[0] = 192 + (fm_sinf((float) ticks / 300.0f) * 2.5f);
	gCameraFocus.v[1] = 0;
	gCameraFocus.v[2] = 192 + 30 + (fm_cosf(((float) ticks + 3000.0f) / 300.0f) * 2.5f);
	if (gGamePaused) {
		return;
	}
    for (int i = 0; i < 4; i++) {
		if (gPlayerIDs[i] == -1) {
			input_reset(i);
			ai_run(i, updateRate);
		}
		gPlayerPointers[i][0] += (float) (input_stick_x(i, STICK_LEFT) / 750.0f) * updateRateF;
		if (gPlayerPointers[i][0] < 0.0f) {

			gPlayerPointers[i][0] = 0.0f;
		}
		if (gPlayerPointers[i][0] >= 12.0f) {
			gPlayerPointers[i][0] = 11.999f;
		}
		gPlayerPointers[i][1] += (float) (input_stick_y(i, STICK_LEFT) / 750.0f) * updateRateF;
		if (gPlayerPointers[i][1] < 0.0f) {
			gPlayerPointers[i][1] = 0.0f;
		}
		if (gPlayerPointers[i][1] >= 10.0f) {
			gPlayerPointers[i][1] = 9.999f;
		}
		gPlayerCursors[i][0] = fm_floorf(gPlayerPointers[i][0]);
		gPlayerCursors[i][2] = fm_floorf(gPlayerPointers[i][1]);

		if (gPointerCD[i] == 0) {
			int dir = DIR_NONE;
			if (input_pressed(i, INPUT_CLEFT, 2)) {
				input_clear(i, INPUT_CLEFT);
				dir = DIR_LEFT;
			} else 
			if (input_pressed(i, INPUT_CRIGHT, 2)) {
				input_clear(i, INPUT_CRIGHT);
				dir = DIR_RIGHT;
			} else 
			if (input_pressed(i, INPUT_CUP, 2)) {
				input_clear(i, INPUT_CUP);
				dir = DIR_UP;
			} else 
			if (input_pressed(i, INPUT_CDOWN, 2)) {
				input_clear(i, INPUT_CDOWN);
				dir = DIR_DOWN;
			}

			if (dir != DIR_NONE) {
				int mapObj = gCurrentLevel->objects[(int)((gPlayerCursors[i][2] * 12) + gPlayerCursors[i][0])];
				if (mapObj < 16 && (mapObj % 8) != 7) {
					gMapArrows[(int)((gPlayerCursors[i][2] * 12) + gPlayerCursors[i][0])].dir = dir;
					gMapArrows[(int)((gPlayerCursors[i][2] * 12) + gPlayerCursors[i][0])].playerID = i + 1;
					gPointerCD[i] = gPointerGlobalTime;
					sound_play_channel(SOUND_ARROW_1 + i, CHANNEL_ENV1);
				}
			}

		} else {
			gPointerCD[i] -= updateRate;
			if (gPointerCD[i] < 0) {
				gPointerCD[i] = 0;
			}
		}
	}

	if (gSpawnerRuinTime > 0) {
		gSpawnerRuinTime -= updateRate;
	}
	if (gSpawnerRuinTime <= 0) {
		gSpawnerRuinID = rand() % 8;
	}
	for (int i = 0; i < 8; i++) {
		if (gSpawnerPos[i].x == -1) {
			if (gSpawnerRuinID == i) {
				gSpawnerRuinID = rand() % 8;
			}
			continue;
		}

		gSpawnerCD[i]--;
		if (gSpawnerCD[i] <= 0) {
			gSpawnerCD[i] = gSpawnerGlobalTime;

			for (int j = 0; j < TROOP_COUNT; j++) {
				if (gTroops[j].active) {
					continue;
				}

				gTroops[j].posTarget.x = gSpawnerPos[i].x * 32;
				gTroops[j].posTarget.y = gSpawnerPos[i].y * 32;
				gTroops[j].posTarget.z = gSpawnerPos[i].z * 32;
				gTroops[j].pos.x = gSpawnerPos[i].x * 32;
				gTroops[j].pos.z = gSpawnerPos[i].z * 32;
				gTroops[j].pos.y = gSpawnerPos[i].y * 32;

				gTroops[j].active = true;
				if (gSpawnerRuinID == i && gSpawnerRuinTime == 0) {
					gSpawnerRuinTime = 250;
					gTroops[j].type = (rand() % 4) + 1;
				} else {
					
					gTroops[j].type = TROOP_NORMAL;
				}
				gTroops[j].dir = (rand() % 4) + 1;
				break;
			}
		}
	}

	for (int i = 0; i < TROOP_COUNT; i++) {
		if (gTroops[i].active == false) {
			continue;
		}
		if (obj_approach(&gTroops[i], updateRateF)) {
			int x = gTroops[i].pos.x / 32;
			int z = gTroops[i].pos.z / 32;
			for (int j = 0; j < 4; j++) {

				if (x == (int) gBasePos[j].x && z == (int) gBasePos[j].z) {
					switch (gTroops[i].type) {
						case TROOP_NORMAL:
							gPoints[j] += 1;
							sound_play_channel(SOUND_TROOP_SECURE, CHANNEL_ENV1);
							break;
						case TROOP_HOOLIGAN:
							gPoints[j] /= 2;
							sound_play_channel(SOUND_TROOP_SECURE_HOOLIGAN, CHANNEL_ENV2);
							break;
						case TROOP_25:
							gPoints[j] += 25;
							sound_play_channel(SOUND_TROOP_SECURE_SPECIAL, CHANNEL_ENV2);
							break;
						case TROOP_50:
							gPoints[j] += 50;
							sound_play_channel(SOUND_TROOP_SECURE_SPECIAL, CHANNEL_ENV2);
							break;
						case TROOP_ROULETTE:
							gPoints[j] += 1;
							sound_play_channel(SOUND_TROOP_SECURE_ROULETTE, CHANNEL_ENV2);
							break;
					}
					gTroops[i].active = false;
					goto nextTroop;
				}
			}
			if (map_floor_tile(x, z) != -1) {
				if (gMapArrows[(z * 12) + x].playerID) {
					gTroops[i].dir = gMapArrows[(z * 12) + x].dir;
				}
				checkDirAgain:
				int dirX = 0;
				int dirZ = 0;
				switch (gTroops[i].dir) {
					case DIR_DOWN:
						dirX = 0;
						dirZ = 1;
						break;
					case DIR_LEFT:
						dirX = -1;
						dirZ = 0;
						break;
					case DIR_UP:
						dirX = 0;
						dirZ = -1;
						break;
					default:
						dirX = 1;
						dirZ = 0;
						break;
				}
				if (troop_wall_collision(x, z, x + dirX, z + dirZ, gTroops[i].dir)) {
					gTroops[i].dir--;
					if (gTroops[i].dir == DIR_NONE) {
						gTroops[i].dir = DIR_RIGHT;
					}
					goto checkDirAgain;
				}
				gTroops[i].posTarget.x += dirX * 32;
				gTroops[i].posTarget.z += dirZ * 32;
			} else {
				gTroops[i].pos.y -= 4.0f * updateRateF;
				if (gTroops[i].pos.y < -32) {
					gTroops[i].active = false;
					sound_play_channel(SOUND_TROOP_DIE, CHANNEL_ENV1);
				}
			}
		}
		nextTroop:
	}
}

void level_free(void) {
	for (int i = 0; i < 16; i++) {
		if (dplMapBottom[i]) {
			rspq_block_free(dplMapBottom[i]);
			dplMapBottom[i] = NULL;
		}
		if (dplMapFloor[i]) {
			rspq_block_free(dplMapFloor[i]);
			dplMapFloor[i] = NULL;
		}
		if (dplMapWalls[i]) {
			rspq_block_free(dplMapWalls[i]);
			dplMapWalls[i] = NULL;
		}
		if (gLevelFloorVtx[i]) {
			free(gLevelFloorVtx[i]);
			gLevelFloorVtx[i] = NULL;
		}
		if (gLevelWallVtx[i]) {
			free(gLevelWallVtx[i]);
			gLevelWallVtx[i] = NULL;
		}
	}
}

void level_0_free(void) {
	t3d_anim_destroy(&gArmyGatorAnims);
	t3d_skeleton_destroy(&gArmyGatorSkel);
	rspq_block_free(gArmyGatorBlock);
	t3d_model_free(gArmyGatorModel);
	rspq_block_free(gMenuModelBlock);
	t3d_model_free(gMenuLevelModel);
}

void game_init(int levelID, int playerCount) {

	rspq_call_deferred((void *) level_free, NULL);
	rspq_wait();
	bzero(&gTroops, sizeof(TroopObj) * TROOP_COUNT);
	bzero(&gMapArrows, sizeof(ArrowData) * 12 * 10);
	gPoints[0] = 0;
	gPoints[1] = 0;
	gPoints[2] = 0;
	gPoints[3] = 0;
	gPointsVisual[0] = 0;
	gPointsVisual[1] = 0;
	gPointsVisual[2] = 0;
	gPointsVisual[3] = 0;
	gLevelID = levelID;
    if (levelID == 0) {
		gMenuLevelModel = t3d_model_load("rom:/mainmenu.t3dm");
		gArmyGatorModel = t3d_model_load("rom:/armygator_lp.t3dm");
		gArmyGatorSkel = t3d_skeleton_create_buffered(gArmyGatorModel, 2);
  		gArmyGatorAnims = t3d_anim_create(gArmyGatorModel, "Yappin");
		t3d_anim_attach(&gArmyGatorAnims, &gArmyGatorSkel);
		rspq_block_begin();
			t3d_model_draw_skinned(gArmyGatorModel, &gArmyGatorSkel);
		gArmyGatorBlock = rspq_block_end();
		rspq_block_begin();
			t3d_model_draw(gMenuLevelModel);
		gMenuModelBlock = rspq_block_end();
        return;
    }

	if (gArmyGatorBlock) {
		rspq_call_deferred((void *) level_0_free, NULL);
		rspq_wait();
		gArmyGatorBlock = NULL;
	}

	gCameraFocus.v[0] = 0 + 192;
	gCameraFocus.v[1] = 0;
	gCameraFocus.v[2] = 30 + 192;
	gCameraPos.v[0] = 0 + 192;
	gCameraPos.v[1] = 200;
	gCameraPos.v[2] = 100 + 192;

	gSpawnerRuinTime = 200;

	gBasePos[0].x = -1;
	gBasePos[1].x = -1;
	gBasePos[2].x = -1;
	gBasePos[3].x = -1;

	gPointerGlobalTime = 30;

	gGameTimer = 60 * 60 * 3;

	gSpawnerGlobalTime = 50;
	for (int i = 0; i < 8; i++) {
		gSpawnerPos[i].x = -1;
		gSpawnerCD[i] = rand() % gSpawnerGlobalTime;
	}

	gPlayerCount = playerCount;
	gCursorCount = 4;

	gCurrentLevel = &gMapLevel2[levelID - 1];

	for (int k = 0; k < 16; k++) {
		int verts = 0;
		int tris = 0;
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 12; j++) {
				int idx = (i * 12) + j;
				if (gCurrentLevel->floorTextures[idx] == k) {
					verts += 4;
					tris += 2;
				}
			}
		}

		if (verts == 0) {
			continue;
		}

		gLevelFloorVtx[k] = malloc((sizeof(T3DVertPacked) * verts) + 0x10);
		T3DVertPacked *valign = _ALIGN16(gLevelFloorVtx[k]);

		int x = 0;
		int z = 0;
		int u = 0;
		int v = 0;
		int p = 0;
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 12; j++) {
				int idx = (i * 12) + j;
				if (gCurrentLevel->floorTextures[idx] == k) {
					int colourA;
					int colourB;
					int alpha = 255;
					int y;
					if (gCurrentLevel->floorTiles[idx] != 0) {
						y = gMapLevelHeights[(gCurrentLevel->floorTiles[idx] - 1) % 8];
						colourA = 0xFFFFFF00;
						colourB = 0xFFFFFF00;
						if (map_floor_tile(j, i - 1) == -1) {
							colourA = 0x7F7F7F00;
							colourB = 0x7F7F7F00;
						} else {
							if (map_floor_tile(j - 1, i - 1) == -1) {
								colourA = 0x7F7F7F00;
							}
							if (map_floor_tile(j + 1, i - 1) == -1) {
								colourB = 0x7F7F7F00;
							}
						}
						if (map_floor_tile(j - 1, i) == -1) {
							colourA = 0x7F7F7F00;
						}
						if (map_floor_tile(j + 1, i) == -1) {
							colourB = 0x7F7F7F00;
						}
						colourA += alpha;
						colourB += alpha;
					} else {
						colourA = 0x000000FF;
						colourB = 0x000000FF;
						y = -32;
					}
					valign[p++] = (T3DVertPacked){
						.posA = {x, y, z}, .rgbaA = colourA, .stA = {u, v},
						.posB = {x + 32, y, z}, .rgbaB = colourB, .stB = {u + 512, v},
					};
					if (gCurrentLevel->floorTiles[idx] != 0) {
						colourA = 0xFFFFFF00;
						colourB = 0xFFFFFF00;
						if (map_floor_tile(j, i + 1) == -1) {
							colourA = 0x7F7F7F00;
							colourB = 0x7F7F7F00;
						} else {
							if (map_floor_tile(j - 1, i + 1) == -1) {
								colourA = 0x7F7F7F00;
							}
							if (map_floor_tile(j + 1, i + 1) == -1) {
								colourB = 0x7F7F7F00;
							}
						}
						if (map_floor_tile(j - 1, i) == -1) {
							colourA = 0x7F7F7F00;
						}
						if (map_floor_tile(j + 1, i) == -1) {
							colourB = 0x7F7F7F00;
						}
						colourA += alpha;
						colourB += alpha;
					} else {
						colourA = 0x000000FF;
						colourB = 0x000000FF;
					}
					valign[p++] = (T3DVertPacked){
						.posA = {x + 32, y, z + 32}, .rgbaA = colourB, .stA = {u + 512, v + 512},
						.posB = {x, y, z + 32}, .rgbaB = colourA, .stB = {u, v + 512},
					};
				}
				x += 32;
				u += 512;
			}
			x = 0;
			u = 0;
			z += 32;
			v += 512;
		}

		rdpq_texparms_t parms = {0};
		parms.s.repeats = REPEAT_INFINITE;
		parms.t.repeats = REPEAT_INFINITE;

		data_cache_hit_writeback(valign, sizeof(T3DVertPacked) * verts);

		rspq_block_begin();
			rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
			rdpq_sprite_upload(TILE0, gLevelSprites[k], &parms);
			t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED | T3D_FLAG_NO_LIGHT);

			int vertOff = 0;
			while (verts > 0) {
				const int size = (T3D_VERTEX_CACHE_SIZE / 4) * 4;
				int loaded = MIN(size, verts);
				t3d_vert_load(&valign[vertOff], 0, loaded);
				for (int i = 0; i < loaded; i += 4) {
					t3d_tri_draw(0 + i, 1 + i, 2 + i);
					t3d_tri_draw(2 + i, 3 + i, 0 + i);
				}
				vertOff += loaded / 2;
				verts -= loaded;
			}
			t3d_tri_sync();
		dplMapFloor[k] = rspq_block_end();
	}

	for (int k = 0; k < 16; k++) {
		int verts = 0;
		int tris = 0;
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 12; j++) {
				int idx = (i * 12) + j;
				if (gCurrentLevel->wallTiles[idx] != 0 && gCurrentLevel->wallTextures[idx] == k) {
					int type = gCurrentLevel->wallTiles[idx];
					if (type == 3 || type == 4) {
						if (map_wall_tile(j - 1, i)) {
							verts += 6;
							tris += 4;
						}
					} else {
						if (map_wall_tile(j + 1, i)) {
							verts += 6;
							tris += 4;
						}
					}
					if (map_wall_tile(j, i + 1)) {
						verts += 6;
						tris += 4;
					}
					if (type == 2 || type == 4) {
						if (map_wall_tile(j, i - 1)) {
							verts += 6;
							tris += 4;
						}
					} else {
						if (map_wall_tile(j, i + 1)) {
							verts += 6;
							tris += 4;
						}
					}
				}
			}
		}

		if (verts > 0) {
			gLevelWallVtx[k] = malloc(sizeof(T3DVertPacked) * verts);
			T3DVertPacked *valign = _ALIGN16(gLevelWallVtx[k]);
			int x = 0;
			int z = 0;
			int u = 0;
			int v = 0;
			int p = 0;
			for (int i = 0; i < 10; i++) {
				for (int j = 0; j < 12; j++) {
					int idx = (i * 12) + j;
					if (gCurrentLevel->wallTiles[idx] != 0 && gCurrentLevel->wallTextures[idx] == k) {
						int type = gCurrentLevel->wallTiles[idx];
						int colourA;
						int colourB;
						int colourC;
						int alpha = 255;
						int y;
						int alwaysX;
						int alwaysZ;
						int offsetX;
						int offsetZ;

						if (gCurrentLevel->floorTiles[idx] == 0) {
							y = 0;
						} else {
							y = gMapLevelHeights[(gCurrentLevel->floorTiles[idx] - 1) % 8];
						}

						offsetX = x;
						offsetZ = z;
						if (type == 3 || type == 4) {
							offsetX += 32;
						}
						if (type == 2 || type == 4) {
							offsetZ += 32;
						}
						if (map_wall_tile(j + 1, i)) {
							alwaysX = true;
						} else {
							if ((type == 3 || type == 4) && map_wall_tile(j - 1, i)) {
								alwaysX = true;
							} else {
								alwaysX = false;
							}
						}
						if (map_wall_tile(j, i + 1)) {
							alwaysZ = true;
						} else {
							if ((type == 2 || type == 4) && map_wall_tile(j, i - 1)) {
								alwaysZ = true;
							} else {
								alwaysZ = false;
							}
						}
						if (alwaysX) {
							colourA = 0xFFFFFF00;
							colourB = 0xFFFFFF00;
							colourC = 0xFFFFFF00;
							if (map_floor_tile(j + 1, i) == -1) {
								colourB = 0x7F7F7F00;
							}
							if (map_floor_tile(j - 1, i) == -1) {
								colourA = 0x7F7F7F00;
							}
							if ((type == 1 || type == 3) && map_floor_tile(j, i - 1) == -1) {
								colourA = 0x7F7F7F00;
								colourB = 0x7F7F7F00;
							}
							if ((type == 2 || type == 4) && map_floor_tile(j, i + 1) == -1) {
								colourA = 0x7F7F7F00;
								colourB = 0x7F7F7F00;
							}
							if ((type == 1 || type == 3)) {
								if (map_floor_tile(j - 1, i - 1) == -1) {
									colourA = 0x7F7F7F00;
								}
								if (map_floor_tile(j + 1, i - 1) == -1) {
									colourB = 0x7F7F7F00;
								}
							} else if ((type == 2 || type == 4)) {
								if (map_floor_tile(j - 1, i + 1) == -1) {
									colourA = 0x7F7F7F00;
								}
								if (map_floor_tile(j + 1, i + 1) == -1) {
									colourB = 0x7F7F7F00;
								}
							}
							colourA += alpha;
							colourB += alpha;
							colourC += alpha;
							valign[p++] = (T3DVertPacked){
								.posA = {x, y, offsetZ}, .rgbaA = colourA, .stA = {u, v + 1024},
								.posB = {x + 32, y, offsetZ}, .rgbaB = colourB, .stB = {u + 1024, v + 1024},
							};
							valign[p++] = (T3DVertPacked){
								.posA = {x + 32, y + 10, offsetZ},.rgbaA = colourC, .stA = {u + 1024, v + 512},
								.posB = {x, y + 10, offsetZ}, .rgbaB = colourC, .stB = {u, v + 512},
							};
							valign[p++] = (T3DVertPacked){
								.posA = {x, y + 20, offsetZ}, .rgbaA = colourC, .stA = {u, v},
								.posB = {x + 32, y + 20, offsetZ},.rgbaB = colourC, .stB = {u + 1024, v},
							};
						}
						if (alwaysZ) {
							colourA = 0xFFFFFF00;
							colourB = 0xFFFFFF00;
							colourC = 0xFFFFFF00;
							if (map_floor_tile(j, i + 1) == -1) {
								colourB = 0x7F7F7F00;
							}
							if (map_floor_tile(j, i - 1) == -1) {
								colourA = 0x7F7F7F00;
							}
							if ((type == 1 || type == 2) && map_floor_tile(j - 1, i) == -1) {
								colourA = 0x7F7F7F00;
								colourB = 0x7F7F7F00;
							}
							if ((type == 3 || type == 4) && map_floor_tile(j + 1, i) == -1) {
								colourA = 0x7F7F7F00;
								colourB = 0x7F7F7F00;
							}
							if ((type == 1 || type == 2)) {
								if (map_floor_tile(j - 1, i - 1) == -1) {
									colourA = 0x7F7F7F00;
								}
								if (map_floor_tile(j - 1, i + 1) == -1) {
									colourB = 0x7F7F7F00;
								}
							} else if ((type == 3 || type == 4)) {
								if (map_floor_tile(j + 1, i - 1) == -1) {
									colourA = 0x7F7F7F00;
								}
								if (map_floor_tile(j + 1, i + 1) == -1) {
									colourB = 0x7F7F7F00;
								}
							}
							colourA += alpha;
							colourB += alpha;
							colourC += alpha;
							valign[p++] = (T3DVertPacked){
								.posA = {offsetX, y, z}, .rgbaA = colourA, .stA = {u + 1024, v + 1024},
								.posB = {offsetX, y, z + 32}, .rgbaB = colourB, .stB = {u, v + 1024},
							};
							valign[p++] = (T3DVertPacked){
								.posA = {offsetX, y + 10, z + 32}, .rgbaA = colourC, .stA = {u, v + 512},
								.posB = {offsetX, y + 10, z}, .rgbaB = colourC, .stB = {u + 1024, v + 512},
							};
							valign[p++] = (T3DVertPacked){
								.posA = {offsetX, y + 20, z}, .rgbaA = colourC, .stA = {u + 1024, v},
								.posB = {offsetX, y + 20, z + 32}, .rgbaB = colourC, .stB = {u, v},
							};
						}
					}
					x += 32;
					u += 1024;
				}
				x = 0;
				u = 0;
				z += 32;
				v += 1024;
			}

			data_cache_hit_writeback(valign, sizeof(T3DVertPacked) * verts);
			rdpq_texparms_t parms = {0};
			parms.s.repeats = REPEAT_INFINITE;
			parms.t.repeats = REPEAT_INFINITE;
			rspq_block_begin();
				rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
				rdpq_sprite_upload(TILE0, gLevelSprites[k], &parms);
				t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED | T3D_FLAG_NO_LIGHT);

				int vertOff = 0;
				while (verts > 0) {
					const int size = (T3D_VERTEX_CACHE_SIZE / 6) * 6;
					int loaded = MIN(size, verts);
					t3d_vert_load(&valign[vertOff], 0, loaded);
					for (int v = 0; v < loaded; v += 6) {
						t3d_tri_draw(0 + v, 1 + v, 2 + v);
						t3d_tri_draw(2 + v, 3 + v, 0 + v);
						t3d_tri_draw(3 + v, 2 + v, 5 + v);
						t3d_tri_draw(5 + v, 4 + v, 3 + v);
					}
					vertOff += loaded / 2;
					verts -= loaded;
				}
				t3d_tri_sync();
			dplMapWalls[k] = rspq_block_end();
		}

		

	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 12; j++) {
			int idx = gCurrentLevel->objects[(i * 12) + j];
			if (idx > 0 && idx <= 4) {
				int baseID = idx - 1;
				gBasePos[baseID].x = j;
				gBasePos[baseID].z = i;
				
				gPlayerCursors[baseID][0] = j; 
				gPlayerCursors[baseID][2] = i;
				gPlayerPointers[baseID][0] = j + 0.75f; 
				gPlayerPointers[baseID][1] = i + 0.75f; 
				gAICursorTarget[baseID][0] = j;
				gAICursorTarget[baseID][1] = i;
				T3DMat4 mtx;
				//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
				float x = (j * 32);
				float y;
				float z = (i * 32);
				y = map_floor_tile(j, i);
				if (y == -1) {
					y = 0.0f;
				}
				t3d_mat4_identity(&mtx);
				t3d_mat4_translate(&mtx, x, y, z);
				//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
				t3d_mat4_to_fixed(&gBaseMtx[baseID], &mtx);
				data_cache_hit_writeback(&gBaseMtx[baseID], sizeof(T3DMat4FP));
			}
		}
	}

	int spawnerCount = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 12; j++) {
			int idx = gCurrentLevel->objects[(i * 12) + j];
			if (idx == 5) {
				gSpawnerPos[spawnerCount].x = j;
				gSpawnerPos[spawnerCount].z = i;
				T3DMat4 mtx;
				//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
				float x = (j * 32);
				float y;
				float z = (i * 32);
				y = map_floor_tile(j, i);
				if (y == -1) {
					y = 0.0f;
				}
				gSpawnerPos[spawnerCount].y = fm_floorf(y / 32);
				t3d_mat4_identity(&mtx);
				t3d_mat4_translate(&mtx, x, y, z);
				//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
				t3d_mat4_to_fixed(&gSpawnermtx[spawnerCount], &mtx);
				data_cache_hit_writeback(&gSpawnermtx[spawnerCount], sizeof(T3DMat4FP));
				spawnerCount++;
			}
		}
	}
}