#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "ai.h"
#include "boot.h"
#include "input.h"
#include "main.h"

char *gMemSizeTags[] = {
	"B", "KB", "MB"
};

// Key:

uint8_t map[13 * 11] = {
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     7,
	0, 24, 0, 0, 24, 0, 0, 24, 0, 0, 24, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 16, 0, 0, 16, 0, 0, 16, 0, 0, 16, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,


	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     7,
};

ArrowData gMapArrows[13 * 10];
T3DMat4FP gArrowMtx[12 * 10];
T3DMat4FP gTroopsMtx[TROOP_COUNT][2];
T3DMat4FP gBaseMtx[4];
T3DMat4FP gSpawnermtx[8];
T3DVec3 gBasePos[4];
T3DVec3 gSpawnerPos[8];
int gSpawnerCD[8];
int gPoints[4];
int gSpawnerGlobalTime;
int gSpawnerRuinTime;
int gSpawnerRuinID;
int gPointerCD[4];
int gPointerGlobalTime;
int gGameTimer;
int gPlayerIDs[4];

TroopObj gTroops[TROOP_COUNT];

int mapHeights[] = {
	0,
	16,
	32,
	64,
	-16,
	-32,
	-64,
	-20000
};

color_t gPlayerColours[] = {
	RGBA32(255, 0, 0, 255),
	RGBA32(0, 0, 255, 255),
	RGBA32(0, 255, 0, 255),
	RGBA32(255, 255, 0, 255),
};

float gPlayerCursors[4][3];
float gPlayerPointers[4][2];
int gPlayerCount;
int gCursorCount;
T3DMat4FP gCursorMtx[4][2];
T3DMat4FP gPointerMtx[4][2];

rspq_block_t *gArrowBlock;
rspq_block_t *cursor;
rspq_block_t *dplMapWalls;
rspq_block_t *dplMapFloor;
rspq_block_t *gBaseBlock;

sprite_t *gCursorSprite;
sprite_t *gArrowSprite;
sprite_t *gPointerSprite;

float memsize_float(int size, int *tag) {
    if (size < 1024) {
        *tag = 0;
        return (float) size;
    } else if (size < (1024 * 1024)) {
        *tag = 1;
        return (float) ((float) size / 1024.0f);
    } else {
        *tag = 2;
        return (float) ((float) size / (1024.0f * 1024.0f));
    }
}


float approachF(float current, float target, float inc) {
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

static int obj_approach(TroopObj *obj) {
	int oneTrue = false;
	obj->pos.x = approachF(obj->pos.x, obj->posTarget.x, 8.0f);
	obj->pos.z = approachF(obj->pos.z, obj->posTarget.z, 8.0f);

	if (fabsf(obj->pos.x - obj->posTarget.x) < 8.0f) {
		obj->pos.x = obj->posTarget.x;
		oneTrue = true;
	}
	if (fabsf(obj->pos.z - obj->posTarget.z) < 8.0f) {
		obj->pos.z = obj->posTarget.z;
		if (oneTrue) {
			return true;
		}
	}
	return false;
}

int main(void) {
	int gfxFlip = 0;
	int ticks = 0;
    T3DVec3 camPos = {{0, 45.0f, 80.0f}};
    T3DVec3 camTarget = {{0, 0,-10}};
    boot();

    T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};
    T3DViewport viewport = t3d_viewport_create_buffered(2);

	gPlayerIDs[0] = PLAYER_1;
	gPlayerIDs[1] = -1;
	gPlayerIDs[2] = -1;
	gPlayerIDs[3] = -1;

    while(true) {
    	struct mallinfo mem_info = mallinfo();
		int ram =  (mem_info.uordblks + (size_t) (((unsigned int) HEAP_START_ADDR - 0x80000000) + 0x10000));
		int tag;

		camTarget.v[0] = -4;
		camTarget.v[1] = 0;
		camTarget.v[2] = 0;
		camPos.v[0] = -4;
		camPos.v[1] = 250;
		camPos.v[2] = 70;

		input_update();
		for (int i = 0; i < 4; i++) {
			if (gPlayerIDs[i] == -1) {
				input_reset(i);
				ai_run(i);
			}
			gPlayerPointers[i][0] += (float) input_stick_x(i, STICK_LEFT) / 250.0f;
			if (gPlayerPointers[i][0] < 0.0f) {

				gPlayerPointers[i][0] = 0.0f;
			}
			if (gPlayerPointers[i][0] >= 12.0f) {
				gPlayerPointers[i][0] = 11.999f;
			}
			gPlayerPointers[i][1] += (float) input_stick_y(i, STICK_LEFT) / 250.0f;
			if (gPlayerPointers[i][1] < 0.0f) {
				gPlayerPointers[i][1] = 0.0f;
			}
			if (gPlayerPointers[i][1] >= 10.0f) {
				gPlayerPointers[i][1] = 9.999f;
			}
			gPlayerCursors[i][0] = fm_floorf(gPlayerPointers[i][0]);
			gPlayerCursors[i][2] = fm_floorf(gPlayerPointers[i][1]);

			if (gPointerCD[i] == 0) {
				int dir = 0;
				if (input_pressed(i, INPUT_CLEFT, 2)) {
					input_clear(i, INPUT_CLEFT);
					dir = 2;
				} else 
				if (input_pressed(i, INPUT_CRIGHT, 2)) {
					input_clear(i, INPUT_CRIGHT);
					dir = 4;
				} else 
				if (input_pressed(i, INPUT_CUP, 2)) {
					input_clear(i, INPUT_CUP);
					dir = 3;
				} else 
				if (input_pressed(i, INPUT_CDOWN, 2)) {
					input_clear(i, INPUT_CDOWN);
					dir = 1;
				}

				if (dir) {
					int mapObj = map[(int)((gPlayerCursors[i][2] * 13) + gPlayerCursors[i][0])];
					if (mapObj < 16 && (mapObj % 8) != 7) {
						gMapArrows[(int)((gPlayerCursors[i][2] * 12) + gPlayerCursors[i][0])].dir = dir;
						gMapArrows[(int)((gPlayerCursors[i][2] * 12) + gPlayerCursors[i][0])].playerID = i + 1;
						gPointerCD[i] = gPointerGlobalTime;
					}
				}

			} else {
				gPointerCD[i]--;
				if (gPointerCD[i] < 0) {
					gPointerCD[i] = 0;
				}
			}
		}

		if (gSpawnerRuinTime > 0) {
			gSpawnerRuinTime--;
		}
		if (gSpawnerRuinTime <= 0) {
			gSpawnerRuinID = rand() % 8;
		}
		for (int i = 0; i < 8; i++) {
			if (gSpawnerPos[i].x == -1) {
				continue;
			}

			gSpawnerCD[i]--;
			if (gSpawnerCD[i] <= 0) {
				gSpawnerCD[i] = gSpawnerGlobalTime;

				for (int j = 0; j < TROOP_COUNT; j++) {
					if (gTroops[j].active) {
						continue;
					}

					gTroops[j].posTarget.x = gSpawnerPos[i].x;
					gTroops[j].posTarget.z = gSpawnerPos[i].z;
					gTroops[j].pos.x = gSpawnerPos[i].x;
					gTroops[j].pos.z = gSpawnerPos[i].z;

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
			if (obj_approach(&gTroops[i])) {
				int x = gTroops[i].pos.x / ((int) 512 / (int) 12);
				int z = gTroops[i].pos.z / ((int) 512 / (int) 12);
				if (gMapArrows[(z * 12) + x].playerID) {
					gTroops[i].dir = gMapArrows[(z * 12) + x].dir;
				}
				while (true) {
					switch (gTroops[i].dir) {
						case 1:
							gTroops[i].posTarget.x -= ((int) 512 / (int) 12) * 0;
							gTroops[i].posTarget.z += ((int) 512 / (int) 12) * 1;
							break;
						case 2:
							gTroops[i].posTarget.x -= ((int) 512 / (int) 12) * 1;
							gTroops[i].posTarget.z += ((int) 512 / (int) 12) * 0;
							break;
						case 3:
							gTroops[i].posTarget.x -= ((int) 512 / (int) 12) * 0;
							gTroops[i].posTarget.z -= ((int) 512 / (int) 12) * 1;
							break;
						default:
							gTroops[i].posTarget.x += ((int) 512 / (int) 12) * 1;
							gTroops[i].posTarget.z += ((int) 512 / (int) 12) * 0;
							break;
					}
					int xT = gTroops[i].posTarget.x / ((int) 512 / (int) 12);
					int zT = gTroops[i].posTarget.z / ((int) 512 / (int) 12);
					if (xT < 0 || zT < 0 || xT >= 12 || zT >= 10) {
						gTroops[i].dir--;
						if (gTroops[i].dir == 0) {
							gTroops[i].dir = 4;
						}
						gTroops[i].posTarget.x = gTroops[i].pos.x;
						gTroops[i].posTarget.z = gTroops[i].pos.z;
					} else {
						for (int j = 0; j < 4; j++) {
							if ((int) gTroops[i].pos.x == (int) gBasePos[j].x && (int) gTroops[i].pos.z == (int) gBasePos[j].z) {
								
								switch (gTroops[i].type) {
									case TROOP_NORMAL:
										gPoints[j] += 1;
										break;
									case TROOP_HOOLIGAN:
										gPoints[j] /= 2;
										break;
									case TROOP_25:
										gPoints[j] += 25;
										break;
									case TROOP_50:
										gPoints[j] += 50;
										break;
									case TROOP_ROULETTE:
										gPoints[j] += 1;
										break;
								}
								gTroops[i].active = false;
							}
						}
						break;
					}
				}
			}
			
			int x = gTroops[i].pos.x / ((int) 512 / (int) 12);
			int z = gTroops[i].pos.z / ((int) 512 / (int) 12);
			if (x < 0 || z < 0 || x >= 12 || z >= 10) {
				gTroops[i].pos.y -= 4.0f;
				if (gTroops[i].pos.y < -20.0f) {
					gTroops[i].active = false;
				}
			}
		}

		t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 25.0f, 300.0f);
		t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

		// ======== Draw (3D) ======== //
		rdpq_attach(display_get(), display_get_zbuf());
		t3d_frame_start();
		t3d_viewport_attach(&viewport);

		t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
		t3d_screen_clear_depth();

		t3d_light_set_ambient(colorAmbient);
		t3d_light_set_directional(0, colorDir, &lightDirVec);
		t3d_light_set_count(1);

		rspq_block_run(dplMapFloor);
		//rspq_block_run(dplMapWalls);
		rdpq_texparms_t parms = {0};
		bzero(&parms, sizeof(rdpq_texparms_t));
		parms.s.repeats = REPEAT_INFINITE;
		parms.t.repeats = REPEAT_INFINITE;
		parms.s.mirror = true;
		parms.t.mirror = true;

		rdpq_sprite_upload(TILE0, gArrowSprite, &parms);
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 12; j++) {
				int idx = (i * 12) + j;
				int val = gMapArrows[idx].playerID;
				if (val == 0) {
					continue;
				}

				color_t colour = gPlayerColours[val - 1];
				float dir = gMapArrows[idx].dir * M_PI_2;
				rdpq_set_prim_color(colour);
				T3DMat4 mtx;
				//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
				float x = ((int) 512 / (int) 12) * j;
				float y = 0.0f;
				float z = ((int) 512 / (int) 12) * i;
				t3d_mat4_identity(&mtx);
				T3DVec3 angle = {{0, 1, 0}};
				t3d_mat4_rotate(&mtx, &angle, dir);
				t3d_mat4_translate(&mtx, x - 258.0f + 22.0f, y, z - 258.0f + 22.0f);
				//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
				t3d_mat4_to_fixed(&gArrowMtx[idx], &mtx);
				data_cache_hit_writeback(&gArrowMtx[idx], sizeof(T3DMat4FP));
				t3d_matrix_push(&gArrowMtx[idx]);
				rspq_block_run(gArrowBlock);
				t3d_matrix_pop(1);
			}
		}
		
		int basesPlaced = 0;
		int mapid = 0;
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 13; j++) {
				if (map[mapid] >= 16 && map[mapid] < 24) {
					color_t colour = gPlayerColours[basesPlaced];
					rdpq_set_prim_color(colour);
					T3DMat4 mtx;
					//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
					float x = ((int) 512 / (int) 12) * j;
					float y = mapHeights[map[mapid] % 8];
					float z = ((int) 512 / (int) 12) * i;
					gBasePos[basesPlaced].x = x;
					gBasePos[basesPlaced].z = z;
					t3d_mat4_identity(&mtx);
					T3DVec3 angle = {{0, 1, 0}};
					t3d_mat4_translate(&mtx, x - 258.0f, y, z - 258.0f);
					//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
					t3d_mat4_to_fixed(&gBaseMtx[basesPlaced], &mtx);
					data_cache_hit_writeback(&gBaseMtx[basesPlaced], sizeof(T3DMat4FP));
					t3d_matrix_push(&gBaseMtx[basesPlaced]);
					rspq_block_run(gBaseBlock);
					t3d_matrix_pop(1);
					basesPlaced++;
					if (basesPlaced == 4) {
						goto baseSkip;
					}
				}
				mapid++;
			}
		}
		baseSkip:

		mapid = 0;
		basesPlaced = 0;
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 13; j++) {
				if (map[mapid] >= 24 && map[mapid] < 32) {
					rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
					T3DMat4 mtx;
					float x = ((int) 512 / (int) 12) * j;
					float y = mapHeights[map[mapid] % 8];
					float z = ((int) 512 / (int) 12) * i;
					gSpawnerPos[basesPlaced].x = x;
					gSpawnerPos[basesPlaced].z = z;
					//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
					t3d_mat4_identity(&mtx);
					T3DVec3 angle = {{0, 1, 0}};
					t3d_mat4_translate(&mtx, x - 258.0f, y, z - 258.0f);
					//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
					t3d_mat4_to_fixed(&gSpawnermtx[basesPlaced], &mtx);
					data_cache_hit_writeback(&gSpawnermtx[basesPlaced], sizeof(T3DMat4FP));
					t3d_matrix_push(&gSpawnermtx[basesPlaced]);
					rspq_block_run(gBaseBlock);
					t3d_matrix_pop(1);
					basesPlaced++;
				}
				mapid++;
			}
		}
		
		rdpq_sprite_upload(TILE0, gArrowSprite, &parms);
		for (int i = 0; i < TROOP_COUNT; i++) {
			if (gTroops[i].active == false) {
				continue;
			}

			float dir = gTroops[i].dir * M_PI_2;
			T3DMat4 mtx;
			//float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
			float x = gTroops[i].pos.x;
			float y = gTroops[i].pos.y;
			float z = gTroops[i].pos.z;
			t3d_mat4_identity(&mtx);
			T3DVec3 angle = {{0, 1, 0}};
			t3d_mat4_rotate(&mtx, &angle, dir);
			t3d_mat4_translate(&mtx, x - 258.0f + 22.0f, y, z - 258.0f + 22.0f);
			//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
			t3d_mat4_to_fixed(&gTroopsMtx[i][gfxFlip], &mtx);
			data_cache_hit_writeback(&gTroopsMtx[i][gfxFlip], sizeof(T3DMat4FP));
			t3d_matrix_push(&gTroopsMtx[i][gfxFlip]);
			switch (gTroops[i].type) {
				case TROOP_NORMAL:
					rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
					break;
				case TROOP_HOOLIGAN:
					rdpq_set_prim_color(RGBA32(255, 127, 0, 255));
					break;
				case TROOP_25:
					rdpq_set_prim_color(RGBA32(127, 127, 255, 255));
					break;
				case TROOP_50:
					rdpq_set_prim_color(RGBA32(127, 255, 127, 255));
					break;
				case TROOP_ROULETTE:
					rdpq_set_prim_color(RGBA32(255, 0, 255, 255));
					break;
			}
			rspq_block_run(gArrowBlock);
			t3d_matrix_pop(1);
		}

		rdpq_sprite_upload(TILE0, gCursorSprite, &parms);
		for (int i = 0; i < gCursorCount; i++) {
			rdpq_set_prim_color(gPlayerColours[i]);
			T3DMat4 mtx;
			float scale = fm_sinf((float) ticks / 2.5f) * 0.1f;
			float x = ((int) 512 / (int) 12) * gPlayerCursors[i][0];
			float y = 0.0f;
			float z = ((int) 512 / (int) 12) * gPlayerCursors[i][2];
			t3d_mat4_identity(&mtx);
			t3d_mat4_translate(&mtx, x - 258.0f + 22.0f, y, z - 258.0f + 22.0f);
			t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
			t3d_mat4_to_fixed(&gCursorMtx[i][gfxFlip], &mtx);
			data_cache_hit_writeback(&gCursorMtx[i][gfxFlip], sizeof(T3DMat4FP));
			t3d_matrix_push(&gCursorMtx[i][gfxFlip]);
			rspq_block_run(cursor);
			t3d_matrix_pop(1);
		}
		rdpq_sprite_upload(TILE0, gPointerSprite, &parms);
		for (int i = 0; i < gCursorCount; i++) {
			rdpq_set_prim_color(gPlayerColours[i]);
			T3DMat4 mtx;
			float x = ((int) 512 / (int) 12) * gPlayerPointers[i][0];
			float z = ((int) 512 / (int) 12) * gPlayerPointers[i][1];
			t3d_mat4_identity(&mtx);
			T3DVec3 angle = {{1, 0, 0}};
			t3d_mat4_rotate(&mtx, &angle, 0);
			t3d_mat4_translate(&mtx, x - 258.0f + 16.0f, 0.0f, z - 258.0f + 16.0f);
			t3d_mat4_scale(&mtx, 0.75f, 1.0f, 0.75f);
			t3d_mat4_to_fixed(&gPointerMtx[i][gfxFlip], &mtx);
			data_cache_hit_writeback(&gPointerMtx[i][gfxFlip], sizeof(T3DMat4FP));
			t3d_matrix_push(&gPointerMtx[i][gfxFlip]);
			rspq_block_run(cursor);
			t3d_matrix_pop(1);
		}

    	rdpq_text_printf(NULL, 1, 16, 24, "FPS: %d (%2.1fms)", (int) ceilf(display_get_fps()), (double) (1000.0f / display_get_fps()));
    	rdpq_text_printf(NULL, 1, 16, 34, "RAM: %2.3f%s", (double) memsize_float(ram, &tag), gMemSizeTags[tag]);
    	rdpq_text_printf(NULL, 1, 16, 44, "Time: %02d:%02d", gGameTimer / (60 * 30), (gGameTimer / 30) % 60);
    	rdpq_text_printf(NULL, 1, 16, 54, "POINTS:\n1: %d\n2: %d\n3: %d\n4: %d\n", gPoints[0], gPoints[1], gPoints[2], gPoints[3]);

		rdpq_detach_show();
		ticks++;
		gGameTimer--;

		gfxFlip ^= 1;
    }

    return 0;
}

