#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3danim.h>
#include <t3d/t3dskeleton.h>

#include "ai.h"
#include "audio.h"
#include "boot.h"
#include "input.h"
#include "game.h"
#include "main.h"
#include "menu.h"

char *gMemSizeTags[] = {
	"B", "KB", "MB"
};

ArrowData gMapArrows[12 * 10];
T3DMat4FP gArrowMtx[12 * 10];
T3DMat4FP gTroopsMtx[TROOP_COUNT][2];
T3DMat4FP gBaseMtx[4];
T3DMat4FP gSpawnermtx[8];
T3DMat4FP gMapMtx[2];
T3DVec3 gBasePos[4];
T3DVec3 gSpawnerPos[8];
int gSpawnerCD[8];
int gPoints[4];
int gPointsVisual[4];
int gSpawnerGlobalTime;
int gSpawnerRuinTime;
int gSpawnerRuinID;
int gPointerCD[4];
int gPointerGlobalTime;
int gGameTimer;
int gPlayerIDs[4];
int gPlayerWins[4];
int gMenuID;
int gLevelID;
int gTimerStage;
int gGamePaused;
float gMapOffsetX;
T3DVec3 gCameraPos;
T3DVec3 gCameraFocus;
float gCameraPhase = 5;
int gClearblack;

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
	RGBA32(255, 64, 64, 255),
	RGBA32(64, 64, 255, 255),
	RGBA32(64, 255, 64, 255),
	RGBA32(255, 255, 64, 255),
};

T3DVec3 gMainMenuSelectCoords[] = {
	{{100, 20, -150}},
	{{-50, 20, 0}},
	{{75, 20, 25}},
	{{100, 20, 80}},
};

T3DVec3 gMainMenuCameraPath[] = {
	{{150, 100, 300}},
	{{0, 100, 300}},
	{{-65, 50, 200}},
	{{-65, 50, 100}},
	{{0, 50, 0}},
};

T3DVec3 gMainMenuCameraPathFocus[] = {
	{{0, 20, 0}},
	{{0, 20, 0}},
	{{-65, 20, 0}},
	{{-65, 20, 0}},
	{{0, 20, -100}},
};

float gPlayerCursors[4][3];
float gPlayerPointers[4][2];
int gPlayerCount;
int gCursorCount;
int ticks = 0;
T3DMat4FP gCursorMtx[4][2];
T3DMat4FP gPointerMtx[4][2];

rspq_block_t *gArrowBlock;
rspq_block_t *cursor;
rspq_block_t *dplMapBottom[16];
rspq_block_t *dplMapWalls[16];
rspq_block_t *dplMapFloor[16];
rspq_block_t *gBaseBlock;

sprite_t *gCursorSprite;
sprite_t *gArrowSprite;
sprite_t *gPointerSprite;
sprite_t *gNumberSprites;
sprite_t *gNumberBGSprite;
sprite_t *gScoreBoardSprite;
sprite_t *gScoreLeaderSprite;
sprite_t *gScoreBorderSprite;
sprite_t *gScoreUnderlaySprite;
sprite_t *gPauseOptionSprites[5];
sprite_t *gScoreBoardPlayerSprites[4];
sprite_t *gLevelSprites[16];

T3DModel *gArmyGatorModel;
rspq_block_t *gArmyGatorBlock;
T3DModel *gMenuLevelModel;
rspq_block_t *gMenuModelBlock;
T3DSkeleton gArmyGatorSkel;
T3DAnim gArmyGatorAnims;

LevelData *gCurrentLevel;

float lerpf(float a, float b, float f) {
    float diff = b - a;
    a = a + (diff * f);
    return a;
}

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

static unsigned int sDeltaTime = 0;
static unsigned int sPrevTime = 0;
static unsigned int sCurTime;
static unsigned int sDeltaTimePrev = 0;
unsigned char sResetTime = false;
static unsigned char sPrevDelta;
static float sPrevDeltaF;

/**
 * Generate the delta time values.
 * Use a while loop with a frame debt system to generate accurate integer delta, with just a regular comparison for floats.
 * Multiply the float value by 1.2 for PAL users. For integer, use timer_int(int timer) to set a region corrected timer instead.
*/
static inline void update_game_time(int *updateRate, float *updateRateF) {
    
    sCurTime = timer_ticks();
    *updateRateF = ((float) TIMER_MICROS(sCurTime - sPrevTime) / 16666.666f);
    if (*updateRateF <= 0.0001f) {
        *updateRateF = 0.0001f;
    }
    sDeltaTime += TIMER_MICROS(sCurTime - sPrevTime);
    sPrevTime = sCurTime;
    sDeltaTime -= 16666;
    *updateRate = LOGIC_60FPS;
    while (sDeltaTime > 16666) {
        sDeltaTime -= 16666;
        *updateRate = *updateRate + 1;
        if (*updateRate == LOGIC_15FPS) {
            sDeltaTime = 0;
        }
    }

    if (sResetTime) {
        sResetTime = false;
        sDeltaTime = sDeltaTimePrev;
        sPrevTime = sCurTime + sDeltaTimePrev;
        *updateRateF = sPrevDeltaF;
        *updateRate = sPrevDelta;
    }
}

void reset_game_time(void) {
    sResetTime = true;
}

void deltatime_snapshot(int updateRate, float updateRateF) {
    sPrevDelta = updateRate;
    sPrevDeltaF = updateRateF;
    sDeltaTimePrev = sDeltaTime;
}

rspq_block_t *gBackgroundBlock;

void bg_render(void) {
	if (gLevelID == 0 || fabsf(gMapOffsetX) > 2.0f) {
		t3d_screen_clear_depth();
		if (gClearblack) {
			rdpq_set_scissor(0, 0, display_get_width() , display_get_height());
			gClearblack--;
			rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));
			rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
			rdpq_set_mode_standard();
		} else {
			rdpq_set_mode_fill(RGBA32(96, 180, 224, 255));
			if (gLevelID == 0) {
				rdpq_fill_rectangle(12, 12, display_get_width() - 12, display_get_height() - 12);
			} else {
				rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
			}
			rdpq_set_mode_standard();
			//t3d_screen_clear_color(RGBA32(96, 180, 224, 255));
		}
		return;
	}
	if (gBackgroundBlock == NULL) {
		rspq_block_begin();
		rdpq_set_mode_fill(RGBA32(96, 180, 225, 255));

		int width = display_get_width();
		int height = (display_get_height() * 0.8f) / 16;
		int x = 72 * gScreenMul;
		int x2 = display_get_width() - (72 * gScreenMul);
		float step = 4 * gScreenMul;
		int y = 8 * gScreenMul;

		rdpq_fill_rectangle(0, 0, width, y);
		rdpq_fill_rectangle(x, y, x2, y + height);
		for (int i = 0; i < 16; i++) {
			rdpq_fill_rectangle(0, y, x, y + height);
			rdpq_fill_rectangle(x2, y, width, y + height);
			
			x -= step;
			x2 += step;
			y += height;
		}
		rdpq_fill_rectangle(x + (step * 2), y - (height * 2), x2 + (step * 2), y - height);
		rdpq_fill_rectangle(x + step, y - height, x2 + step, y);
		rdpq_fill_rectangle(0, y, width, display_get_height());
		gBackgroundBlock = rspq_block_end();
	}
	rspq_block_run(gBackgroundBlock);
}

static inline void t3d_model_bvh(const T3DModel* model, T3DModelDrawConf conf)
{
  T3DModelState state = t3d_model_state_create();
  state.drawConf = &conf;
  int balls = 0;

  T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
  while(t3d_model_iter_next(&it))
  {
    if((conf.filterCb && !conf.filterCb(conf.userData, it.object)) || it.object->isVisible == false) {
      continue;
    }
	balls++;

    if(it.object->material) {
      t3d_model_draw_material(it.object->material, &state);
    }
    t3d_model_draw_object(it.object, conf.matrices);
	
	it.object->isVisible = false;
  }

  debugf("Main menu rendered %d parts\n", balls);

  if(state.lastVertFXFunc != T3D_VERTEX_FX_NONE)t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
}

static inline void t3d_model_bvh_draw(const T3DModel* model) {
  t3d_model_bvh(model, (T3DModelDrawConf){
    .userData = NULL,
    .tileCb = NULL,
    .filterCb = NULL
  });
}

int main(void) {
	int gfxFlip = 0;
    boot();

    T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};
    T3DViewport viewport = t3d_viewport_create_buffered(2);

    while(true) {
    	struct mallinfo mem_info = mallinfo();
		int ram =  (mem_info.uordblks + (size_t) (((unsigned int) HEAP_START_ADDR - 0x80000000) + 0x10000 + (display_get_width() * display_get_height() * 2)));
		int tag;
		int updateRate;
		float updateRateF;

		update_game_time(&updateRate, &updateRateF);

		input_update(updateRate);
		game_run(updateRate, updateRateF);
		audio_loop(updateRate, updateRateF);

		t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 12.0f, 250.0f);
		t3d_viewport_look_at(&viewport, &gCameraPos, &gCameraFocus, &(T3DVec3){{0,1,0}});

		// ======== Draw (3D) ======== //
		rdpq_attach(display_get(), display_get_zbuf());
		t3d_frame_start();
		t3d_viewport_attach(&viewport);
		bg_render();
    	if (gLevelID == 0) {
			rdpq_set_scissor(12, 12, display_get_width() - 12, display_get_height() - 12);
		} else {
			rdpq_set_scissor(0, 0, display_get_width() , display_get_height());
		}


		t3d_light_set_ambient(colorAmbient);
		t3d_light_set_directional(0, colorDir, &lightDirVec);
		t3d_light_set_count(1);

		if (fabsf(gMapOffsetX) > 192.0f) {
			goto skipDraw;
		}
		T3DMat4 mtx;
		t3d_mat4_identity(&mtx);
		t3d_mat4_translate(&mtx, gMapOffsetX, 0.0f, 0.0f);
		t3d_mat4_to_fixed(&gMapMtx[gfxFlip], &mtx);
		data_cache_hit_writeback(&gMapMtx[gfxFlip], sizeof(T3DMat4FP));
		t3d_matrix_push(&gMapMtx[gfxFlip]);
		rdpq_set_mode_standard();
		if (gLevelID == 0) {
			rdpq_mode_antialias(AA_NONE);
		} else {
			rdpq_mode_antialias(AA_STANDARD);
		}
		rdpq_mode_filter(FILTER_BILINEAR);
		rdpq_mode_persp(true);
		for (int i = 0; i < 16; i++) {
			if (dplMapBottom[i]) {
				rspq_block_run(dplMapBottom[i]);
			}
		}
		for (int i = 0; i < 16; i++) {
			if (dplMapFloor[i]) {
				rspq_block_run(dplMapFloor[i]);
			}
		}
		for (int i = 0; i < 16; i++) {
			if (dplMapWalls[i]) {
				rspq_block_run(dplMapWalls[i]);
			}
		}

		if (gLevelID == 0 && gArmyGatorBlock) {
			T3DMat4 mtx;
			int camID = gCameraPhase;
			if (gCameraPhase < 5.0f) {
				gCameraPos.x = gMainMenuCameraPath[camID].x;
				gCameraPos.y = gMainMenuCameraPath[camID].y;
				gCameraPos.z = gMainMenuCameraPath[camID].z;
				gCameraFocus.x = gMainMenuCameraPathFocus[camID].x;
				gCameraFocus.y = gMainMenuCameraPathFocus[camID].y;
				gCameraFocus.z = gMainMenuCameraPathFocus[camID].z;
			} else {
				gCameraPos.x = 25;
				gCameraPos.y = 40;
				gCameraPos.z = 75;
				gCameraFocus.x = lerpf(gCameraFocus.x, gMainMenuSelectCoords[gMenuOption[0]].x, 0.1f * updateRateF);
				gCameraFocus.y = lerpf(gCameraFocus.y, gMainMenuSelectCoords[gMenuOption[0]].y, 0.1f * updateRateF);
				gCameraFocus.z = lerpf(gCameraFocus.z, gMainMenuSelectCoords[gMenuOption[0]].z, 0.1f * updateRateF);
			}
			rdpq_mode_zbuf(true, true);
			t3d_anim_update(&gArmyGatorAnims, updateRateF * 0.02f);
			t3d_skeleton_update(&gArmyGatorSkel);
			t3d_mat4_identity(&mtx);
			T3DVec3 angle = {{0, 1, 0}};
			t3d_mat4_rotate(&mtx, &angle, 1 * M_PI_2);
			t3d_mat4_translate(&mtx, 25, 0.0f, -50.0f);
			t3d_mat4_scale(&mtx, 0.5f, 0.5f, 0.5f);
			t3d_mat4_to_fixed(&gBaseMtx[gfxFlip], &mtx);
			data_cache_hit_writeback(&gBaseMtx[gfxFlip], sizeof(T3DMat4FP));
    		t3d_skeleton_use(&gArmyGatorSkel);
			t3d_matrix_push(&gBaseMtx[gfxFlip]);
				rspq_block_run(gArmyGatorBlock);
			t3d_matrix_pop(1);
			rdpq_sync_pipe();
			const T3DBvh *bvh = t3d_model_bvh_get(gMenuLevelModel);
			t3d_model_bvh_query_frustum(bvh, &viewport.viewFrustum);
			t3d_model_bvh_draw(gMenuLevelModel);
			rdpq_sync_pipe();
			rdpq_mode_zbuf(false, false);
		}

		rdpq_texparms_t parms = {0};
		bzero(&parms, sizeof(rdpq_texparms_t));
		parms.s.repeats = REPEAT_INFINITE;
		parms.t.repeats = REPEAT_INFINITE;
		parms.s.mirror = true;
		parms.t.mirror = true;

		if (gLevelID != 0) {
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
					float x = j * 32;
					float y = 0.0f;
					float z = i * 32;
					t3d_mat4_identity(&mtx);
					T3DVec3 angle = {{0, 1, 0}};
					t3d_mat4_rotate(&mtx, &angle, dir);
					t3d_mat4_translate(&mtx, x + 16.0f, y, z + 16.0f);
					//t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
					t3d_mat4_to_fixed(&gArrowMtx[idx], &mtx);
					data_cache_hit_writeback(&gArrowMtx[idx], sizeof(T3DMat4FP));
					t3d_matrix_push(&gArrowMtx[idx]);
					rspq_block_run(gArrowBlock);
					t3d_matrix_pop(1);
				}
			}
			
			for (int i = 0; i < 4; i++) {
				if (gBasePos[i].x == -1) {
					continue;
				}
				color_t colour = gPlayerColours[i];
				rdpq_set_prim_color(colour);
				t3d_matrix_push(&gBaseMtx[i]);
				rspq_block_run(gBaseBlock);
				t3d_matrix_pop(1);

			}
			rdpq_sync_pipe();

			for (int i = 0; i < 8; i++) {
				if (gSpawnerPos[i].x == -1) {
					continue;
				}
				rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
				t3d_matrix_push(&gSpawnermtx[i]);
				rspq_block_run(gBaseBlock);
				t3d_matrix_pop(1);

			}
			rdpq_sync_pipe();
			
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
				t3d_mat4_translate(&mtx, x + 16.0f, y, z + 16.0f);
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
			rdpq_sync_pipe();

			if (gGamePaused == false) {
				rdpq_sprite_upload(TILE0, gCursorSprite, &parms);
				for (int i = 0; i < gCursorCount; i++) {
					rdpq_set_prim_color(gPlayerColours[i]);
					T3DMat4 mtx;
					float scale = fm_sinf((float) ticks / 5.0f) * 0.1f;
					float x = fm_floorf(gPlayerCursors[i][0]) * 32.0f;
					float y = 0.0f;
					float z = fm_floorf(gPlayerCursors[i][2]) * 32.0f;
					t3d_mat4_identity(&mtx);
					t3d_mat4_translate(&mtx, x + 16.0f, y, z + 16.0f);
					t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
					t3d_mat4_to_fixed(&gCursorMtx[i][gfxFlip], &mtx);
					data_cache_hit_writeback(&gCursorMtx[i][gfxFlip], sizeof(T3DMat4FP));
					t3d_matrix_push(&gCursorMtx[i][gfxFlip]);
					rspq_block_run(cursor);
					t3d_matrix_pop(1);
				}
				rdpq_sync_pipe();
				rdpq_sprite_upload(TILE0, gPointerSprite, &parms);
				for (int i = 0; i < gCursorCount; i++) {
					rdpq_set_prim_color(gPlayerColours[i]);
					T3DMat4 mtx;
					float x = gPlayerPointers[i][0] * 32.0f;
					float z = gPlayerPointers[i][1] * 32.0f;
					t3d_mat4_identity(&mtx);
					T3DVec3 angle = {{1, 0, 0}};
					t3d_mat4_rotate(&mtx, &angle, 0);
					t3d_mat4_translate(&mtx, x + 12.0f, 0.0f, z + 12.0f);
					t3d_mat4_scale(&mtx, 0.75f, 1.0f, 0.75f);
					t3d_mat4_to_fixed(&gPointerMtx[i][gfxFlip], &mtx);
					data_cache_hit_writeback(&gPointerMtx[i][gfxFlip], sizeof(T3DMat4FP));
					t3d_matrix_push(&gPointerMtx[i][gfxFlip]);
					rspq_block_run(cursor);
					t3d_matrix_pop(1);
				}
				rdpq_sync_pipe();
			}
		}
		t3d_matrix_pop(1);
		skipDraw:

		menu_render(updateRate, updateRateF);

    	rdpq_text_printf(NULL, 1, 16, 24, "FPS: %d (%2.1fms)", (int) ceilf(display_get_fps()), (double) (1000.0f / display_get_fps()));
    	rdpq_text_printf(NULL, 1, 16, 34, "RAM: %2.3f%s", (double) memsize_float(ram, &tag), gMemSizeTags[tag]);

		rdpq_detach_show();
		ticks += updateRate;
		if (gGamePaused == false && gMenuID == MENU_NONE) {
			gGameTimer -= updateRate;
			switch (gTimerStage) {
				case 0:
					if (gGameTimer < 60 * 60) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_60);
					}
					break;
				case 1:
					if (gGameTimer < 60 * 30) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_30);
					}
					break;
				case 2:
					if (gGameTimer < 60 * 10) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_10);
					}
					break;
				case 3:
					if (gGameTimer < 60 * 5) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_5);
					}
					break;
				case 4:
					if (gGameTimer < 60 * 4) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_4);
					}
					break;
				case 5:
					if (gGameTimer < 60 * 3) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_3);
					}
					break;
				case 6:
					if (gGameTimer < 60 * 2) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_2);
					}
					break;
				case 7:
					if (gGameTimer < 60 * 1) {
						gTimerStage++;
						sound_play_global(SOUND_VOICE_TIMER_1);
					}
					break;
			}
			if (gGameTimer <= 0) {
				gGamePaused = true;
				sound_play_global(SOUND_VOICE_TIMER_FINISH);
				gGameTimer = 0;
				gMenuID = MENU_FINISH;
			}
		}

		gfxFlip ^= 1;
    }

    return 0;
}

