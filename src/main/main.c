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
int gSmallScreen;
int gLevelID;
int gArrowCount;
int gTimerStage;
int gGamePaused;
float gMapOffsetX;
T3DVec3 gCameraPos;
T3DVec3 gCameraFocus;
T3DVec3 gArmyGatorPos;
float gCameraPhase = 0;
int gClearblack;
rdpq_font_t *gFonts[4];

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

T3DVec3 gMainMenuCameraPath[] = {
	{{300, 200, 600}},
	{{50, 80, 150}},

	{{220, 50, -180}},
	{{-160, 70, -170}},
	{{120, 75, 130}},
	{{160, 70, 156}},
	{{140, 70, 0}},
};

T3DVec3 gMainMenuCameraPathFocus[] = {
	{{0, 40, 0}},
	{{200, 40, -300}},
};

T3DVec3 gMainMenuSelectCoords[] = {
	{{220, 40, -300}},
	{{-190, 60, -250}},
	{{140, 40, 50}},
	{{200, 40, 160}},
	{{200, 80, 10}},
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
T3DSkeleton gArmyGatorSkel;
T3DAnim gArmyGatorAnims[4];
int gArmyGatorAnimID = -1;

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

sprite_t *gFinishSprites[17];
T3DVertPacked *gFinishVerts;
T3DMat4FP gFinishMtx[32];
int gNumbersRendered;

void finish_render_verts(float x, float y, float z, int mtxID, float scaleX, float scaleY) {
	T3DMat4 mtx;
	t3d_mat4_identity(&mtx);
	t3d_mat4_translate(&mtx, x, y, z);
	t3d_mat4_scale(&mtx, scaleX, scaleY, 1.0f);
	t3d_mat4_to_fixed(&gFinishMtx[mtxID], &mtx);
	data_cache_hit_writeback(&gFinishMtx[mtxID], sizeof(T3DMat4FP));
	t3d_matrix_push(&gFinishMtx[mtxID]);
	t3d_vert_load(&gFinishVerts[0], 0, 4);
	t3d_tri_draw(0, 1, 2);
	t3d_tri_draw(2, 3, 0);
	t3d_matrix_pop(1);
	rdpq_sync_pipe();
}

void finish_score_render(float x, float y, int number, int zeros, float scale, int yOffset) {
    int num;
    int digit;
    int total;
    int divCount;
    int offset;
    int tX;
    int width;
    int tY;
    surface_t tex;

    total = 1;
    num = number;
    while (true) {
        if (number >= 10) {
            total++;
            number /= 10;
        } else {
            break;
        }
    }

    if (total < zeros) {
        total = zeros;
    }


    yOffset *= gScreenMul;
    offset = (48 / gScreenDiv) * scale;
    width = (40 / gScreenDiv) * scale;
    divCount = 1;
    tX = x + (total * width) - width;
    tY = y + (total * yOffset) - yOffset;
    rdpq_texparms_t p = {0};
    if (gScreenMul == 1) {
        p.s.scale_log = -1;
        p.t.scale_log = -1;
    }
    p.s.mirror = MIRROR_NONE;
    p.t.mirror = MIRROR_NONE;
    p.s.repeats = 0;
    p.t.repeats = 0;
    for (int i = 0; i < total; i++) {
        digit = (int) (((float) num / (float) divCount)) % 10;
        tex = sprite_get_pixels(gNumberSprites);
        surface_t t = surface_make_sub(&tex, 0, 48 * digit, 48, 48);
        
        rdpq_tex_upload(TILE0, &t, &p);
		finish_render_verts(tX, tY, -250.0f, 12 + gNumbersRendered, scale, scale);
		gNumbersRendered++;
        rdpq_sync_pipe();
        tX -= width;
        tY -= yOffset;
        divCount *= 10;
    }
}

void render_finish(int updateRate, float updateRateF) {
	if (gFinishVerts == NULL) {
		gFinishVerts = malloc_uncached_aligned(0x10, sizeof(T3DVertPacked) * 4);
		gFinishVerts[0] = (T3DVertPacked){
			.posA = {-16, -16, 0}, .stA = {0, 1024},
			.posB = {16, -16, 0}, .stB = {1024, 1024},
		};
		gFinishVerts[1] = (T3DVertPacked){
			.posA = {16, 16, 0}, .stA = {1024, 0},
			.posB = {-16, 16, 0}, .stB = {0, 0},
		};
		gFinishSprites[0] = sprite_load("rom://resultstitle.i4.sprite");
		gFinishSprites[1] = sprite_load("rom://chalkp1.i4.sprite");
		gFinishSprites[2] = sprite_load("rom://chalkp2.i4.sprite");
		gFinishSprites[3] = sprite_load("rom://chalkp3.i4.sprite");
		gFinishSprites[4] = sprite_load("rom://chalkp4.i4.sprite");
		gFinishSprites[5] = sprite_load("rom://numbers.i4.sprite");
		gFinishSprites[6] = sprite_load("rom://resultssubtitle.i4.sprite");
		gFinishSprites[7] = sprite_load("rom://finishwinnerunderlay.i4.sprite");
		gFinishSprites[8] = sprite_load("rom://finishp1.i4.sprite");
		gFinishSprites[9] = sprite_load("rom://finishp2.i4.sprite");
		gFinishSprites[10] = sprite_load("rom://finishp3.i4.sprite");
		gFinishSprites[11] = sprite_load("rom://finishp4.i4.sprite");
		gFinishSprites[12] = sprite_load("rom://finishtie1.i4.sprite");
		gFinishSprites[13] = sprite_load("rom://finishtie2.i4.sprite");
		gFinishSprites[14] = sprite_load("rom://finishopt1.i4.sprite");
		gFinishSprites[15] = sprite_load("rom://finishopt2.i4.sprite");
		gFinishSprites[16] = sprite_load("rom://finishselect.i4.sprite");
	}
	rdpq_sync_pipe();
	rdpq_set_mode_standard();
	rdpq_mode_antialias(AA_STANDARD);
	rdpq_mode_filter(FILTER_BILINEAR);
	rdpq_mode_dithering(DITHER_BAYER_BAYER);
	rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	t3d_state_set_drawflags(T3D_FLAG_NO_LIGHT | T3D_FLAG_TEXTURED);
	rdpq_mode_persp(true);
	rdpq_texparms_t p = {0};
	float x;

	gNumbersRendered = 0;
	p.s.scale_log = -2;
	if (gSubMenu < 3) {
		rdpq_sprite_upload(TILE0, gFinishSprites[0], &p);
		finish_render_verts(-185.0f, 98.0f, -250.0f, 0, 1.5f, 0.66f);
	} else {
		rdpq_sprite_upload(TILE0, gFinishSprites[6], &p);
		finish_render_verts(-185.0f, 98.0f, -250.0f, 0, 1.25f, 0.5f);
	}
	p.s.scale_log = -1;
	p.t.scale_log = -1;
	if (gSubMenu == 2) {
		if (gPlayerWinner != -1) {
			float scale = 1.0f + (fm_cosf((float) ticks / 25.0f) * 0.1f);
			gPlayerColours[gPlayerWinner].r /= 2;
			gPlayerColours[gPlayerWinner].g /= 2;
			gPlayerColours[gPlayerWinner].b /= 2;
			gPlayerColours[gPlayerWinner].a = 144;
			rdpq_set_prim_color(gPlayerColours[gPlayerWinner]);
			rdpq_sprite_upload(TILE0, gFinishSprites[7], &p);
			gPlayerColours[gPlayerWinner].r *= 2;
			gPlayerColours[gPlayerWinner].g *= 2;
			gPlayerColours[gPlayerWinner].b *= 2;
			gPlayerColours[gPlayerWinner].a = 255;
			finish_render_verts(-225.0f + (25.0f * gPlayerWinner), 80.0f, -250.0f, 10, scale, scale);
		}
		rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
		p.s.scale_log = -2;
		if (gPlayerWinner == -1) {
			rdpq_sprite_upload(TILE0, gFinishSprites[12 + gNumTies], &p);
			finish_render_verts(-190.0f, 47.0f, -250.0f, 7, 1.25f, 0.75f);
		} else {
			rdpq_sprite_upload(TILE0, gFinishSprites[8 + gPlayerWinner], &p);
			finish_render_verts(-190.0f, 47.0f, -250.0f, 7, 1.25f, 0.75f);
		}
	}
	if (gSubMenu < 3) {
		p.s.scale_log = -1;
		x = -225.0f;
		for (int i = 0; i < 4; i++) {
			rdpq_set_prim_color(gPlayerColours[i]);
			rdpq_sprite_upload(TILE0, gFinishSprites[1 + i], &p);
			finish_render_verts(x, 80.0f, -250.0f, 1 + i, 0.60f, 0.60f);
			rdpq_set_prim_color(RGBA32(255, 255, 255, 192));
			finish_score_render(x - 7.0f, 65.0f, gPoints[i], 1, 0.35f, 0);

			x += 25.0f;
		}
	} else {
		if (gPlayerWinner != -1) {
			float scale = 1.0f + (fm_cosf((float) ticks / 25.0f) * 0.1f);
			gPlayerColours[gPlayerWinner].r /= 2;
			gPlayerColours[gPlayerWinner].g /= 2;
			gPlayerColours[gPlayerWinner].b /= 2;
			gPlayerColours[gPlayerWinner].a = 144;
			rdpq_set_prim_color(gPlayerColours[gPlayerWinner]);
			rdpq_sprite_upload(TILE0, gFinishSprites[7], &p);
			gPlayerColours[gPlayerWinner].r *= 2;
			gPlayerColours[gPlayerWinner].g *= 2;
			gPlayerColours[gPlayerWinner].b *= 2;
			gPlayerColours[gPlayerWinner].a = 255;
			finish_render_verts(-225.0f + (25.0f * gPlayerWinner), 80.0f, -250.0f, 10, scale, scale);
		}
		x = -225.0f;
		for (int i = 0; i < 4; i++) {
			rdpq_set_prim_color(gPlayerColours[i]);
			rdpq_sprite_upload(TILE0, gFinishSprites[1 + i], &p);
			finish_render_verts(x, 80.0f, -250.0f, 1 + i, 0.60f, 0.60f);
			rdpq_set_prim_color(RGBA32(255, 255, 255, 144));
			finish_score_render(x - 7.0f, 65.0f, gPlayerWins[i], 1, 0.35f, 0);
			x += 25.0f;
		}
		rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
		float y = 52.0f;
		p.s.scale_log = -2;
		for (int i = 0; i < 2; i++) {
			if (gMenuOption[1] == i) {
				p.s.scale_log = -1;
				rdpq_sprite_upload(TILE0, gFinishSprites[16], &p);
				rdpq_set_prim_color(RGBA32(255, 255, 255, 192));
				finish_render_verts(-185.0f, y, -250.0f, 11, 1.75f, 0.66f);
				rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
				p.s.scale_log = -2;
			}

			surface_t tex = sprite_get_pixels(gFinishSprites[14 + i]);
			surface_t t = surface_make_sub(&tex, 0, 0, 128, 64);
			rdpq_tex_upload(TILE0, &t, &p);
			finish_render_verts(-200, y, -250.0f, 6 + i, 0.75f, 0.60f);
			t = surface_make_sub(&tex, 128, 0, 128, 64);
			
			rdpq_tex_upload(TILE0, &t, &p);
			finish_render_verts(-200 + (32 * 0.75f), y, -250.0f, 8 + i, 0.75f, 0.60f);
			y -= 15.0f;
		}
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
		if (gLevelID == 0 && sys_bbplayer() == false) {
			gSmallScreen = true;
		} else {
			gSmallScreen = false;
		}
		if (gClearblack) {
			rdpq_set_scissor(0, 0, display_get_width() , display_get_height());
			gClearblack--;
			rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));
			rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
			rdpq_set_mode_standard();
		} else {
			rdpq_set_mode_fill(RGBA32(96, 180, 224, 255));
			if (gSmallScreen) {
				rdpq_fill_rectangle(16, 16, display_get_width() - 16, display_get_height() - 16);
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
		uint32_t first = get_user_ticks();
    	struct mallinfo mem_info = mallinfo();
		int ram =  (mem_info.uordblks + (size_t) (((unsigned int) HEAP_START_ADDR - 0x80000000) + 0x10000 + (display_get_width() * display_get_height() * 2)));
		int tag;
		int fps = fm_ceilf((display_get_fps()));
		int updateRate;
		float updateRateF;
		int prevAnim = gArmyGatorAnimID;

		update_game_time(&updateRate, &updateRateF);

		input_update(updateRate);
		game_run(updateRate, updateRateF);
		audio_loop(updateRate, updateRateF);

		t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 25.0f, 500.0f);
		t3d_viewport_look_at(&viewport, &gCameraPos, &gCameraFocus, &(T3DVec3){{0,1,0}});

		// ======== Draw (3D) ======== //
		rdpq_attach(display_get(), display_get_zbuf());
		t3d_frame_start();
		t3d_viewport_attach(&viewport);
		bg_render();
    	if (gSmallScreen) {
			rdpq_set_scissor(16, 16, display_get_width() - 16, display_get_height() - 16);
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
		rdpq_mode_antialias(AA_STANDARD);
		rdpq_mode_filter(FILTER_BILINEAR);
		rdpq_mode_dithering(DITHER_BAYER_BAYER);
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

		if (gMenuID != MENU_TITLE) {
			gCameraPhase = 0.0f;
		}
		if (gLevelID == 0 && gArmyGatorBlock) {
			T3DMat4 mtx;
			if (gMenuID != MENU_FINISH) {
				gArmyGatorAnimID = 0;
				gArmyGatorPos.x = 50.0f;
				gArmyGatorPos.y = 0.0f;
				gArmyGatorPos.z = -100.0f;
				int camID = fm_floorf(gCameraPhase);
				if (gSubMenu < 4) {
					int camID1 = fm_floorf(gCameraPhase) + 1;
					if (gSubMenu < 4) {
						gCameraPhase = lerpf(gCameraPhase, 0.0f, 0.1f * updateRateF);
					} else {
						gCameraPhase = lerpf(gCameraPhase, 1.0f, 0.1f * updateRateF);
					}
					float lp = gCameraPhase;
					gCameraPos.x = lerpf(gMainMenuCameraPath[camID].x, gMainMenuCameraPath[camID1].x, lp);
					gCameraPos.y = lerpf(gMainMenuCameraPath[camID].y, gMainMenuCameraPath[camID1].y, lp);
					gCameraPos.z = lerpf(gMainMenuCameraPath[camID].z, gMainMenuCameraPath[camID1].z, lp);
					gCameraFocus.x = lerpf(gMainMenuCameraPathFocus[camID].x, gMainMenuCameraPathFocus[camID1].x, lp);
					gCameraFocus.y = lerpf(gMainMenuCameraPathFocus[camID].y, gMainMenuCameraPathFocus[camID1].y, lp);
					gCameraFocus.z = lerpf(gMainMenuCameraPathFocus[camID].z, gMainMenuCameraPathFocus[camID1].z, lp);
				} else {
					if (gSubMenu < 6) {
						gCameraPhase = lerpf(gCameraPhase, 1.0f, 0.1f * updateRateF);
						gCameraPos.x = lerpf(gCameraPos.x, 50, 0.1f * updateRateF);
						gCameraPos.y = lerpf(gCameraPos.y, 80, 0.1f * updateRateF);
						gCameraPos.z = lerpf(gCameraPos.z, 150, 0.1f * updateRateF);
						gCameraFocus.x = lerpf(gCameraFocus.x, gMainMenuSelectCoords[gMenuOption[0]].x, 0.1f * updateRateF);
						gCameraFocus.y = lerpf(gCameraFocus.y, gMainMenuSelectCoords[gMenuOption[0]].y, 0.1f * updateRateF);
						gCameraFocus.z = lerpf(gCameraFocus.z, gMainMenuSelectCoords[gMenuOption[0]].z, 0.1f * updateRateF);
					} else {
						if (gCameraPhase <= 1.0f) {
							gCameraPhase = 1.01f;
						}
						gCameraPhase = lerpf(gCameraPhase, 2.0f, 0.1f * updateRateF);
						float lp = gCameraPhase - 1.0f;
						int camID = 1;
						int camID1 = gMenuOption[0] + 2;
						gCameraPos.x = lerpf(gMainMenuCameraPath[camID].x, gMainMenuCameraPath[camID1].x, lp);
						gCameraPos.y = lerpf(gMainMenuCameraPath[camID].y, gMainMenuCameraPath[camID1].y, lp);
						gCameraPos.z = lerpf(gMainMenuCameraPath[camID].z, gMainMenuCameraPath[camID1].z, lp);
						gCameraFocus.x = lerpf(gCameraFocus.x, gMainMenuSelectCoords[gMenuOption[0]].x, 0.1f * updateRateF);
						gCameraFocus.y = lerpf(gCameraFocus.y, gMainMenuSelectCoords[gMenuOption[0]].y, 0.1f * updateRateF);
						gCameraFocus.z = lerpf(gCameraFocus.z, gMainMenuSelectCoords[gMenuOption[0]].z, 0.1f * updateRateF);
					}
				}
			} else {
				gArmyGatorAnimID = 1;
				gArmyGatorPos.x = -120.0f;
				gArmyGatorPos.y = 0.0f;
				gArmyGatorPos.z = -215.0f;
				gCameraPos.x = -200;
				gCameraPos.y = 70;
				gCameraPos.z = -170;
				gCameraFocus.x = -170;
				gCameraFocus.y = 60;
				gCameraFocus.z = -250;
			}
			rdpq_mode_zbuf(true, true);
			t3d_fog_set_enabled(false);
			t3d_anim_attach(&gArmyGatorAnims[gArmyGatorAnimID], &gArmyGatorSkel);
			t3d_anim_update(&gArmyGatorAnims[gArmyGatorAnimID], updateRateF * 0.02f);
			t3d_skeleton_update(&gArmyGatorSkel);
			t3d_mat4_identity(&mtx);
			T3DVec3 angle = {{0, 1, 0}};
			t3d_mat4_rotate(&mtx, &angle, 1 * M_PI_2);
			t3d_mat4_translate(&mtx, gArmyGatorPos.x, gArmyGatorPos.y, gArmyGatorPos.z);
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
			gCameraPhase = 0.0f;
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

		if (gLevelID == 0 && gMenuID == MENU_FINISH) {
			render_finish(updateRate, updateRateF);
		} else {
			for (int i = 0; i < 17; i++) {
				if (gFinishSprites[i]) {
					sprite_free(gFinishSprites[i]);
					gFinishSprites[i] = NULL;
				}
				if (gFinishVerts) {
					free_uncached(gFinishVerts);
					gFinishVerts = NULL;
				}
			}
		}

		menu_render(updateRate, updateRateF);

		unsigned int cpu = TICKS_TO_US(get_user_ticks() - first);
    	//rdpq_text_printf(NULL, 1, 16, 24, "FPS: %d (%2.1fms)", (int) ceilf(display_get_fps()), (double) (1000.0f / display_get_fps()));
    	//rdpq_text_printf(NULL, 1, 16, 34, "CPU: %d (%2.1f%%)", (int) cpu, (double) (cpu / 333));
    	//rdpq_text_printf(NULL, 1, 16, 44, "RAM: %2.3f%s", (double) memsize_float(ram, &tag), gMemSizeTags[tag]);
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

