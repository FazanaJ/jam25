#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#include "audio.h"
#include "boot.h"
#include "input.h"
#include "game.h"
#include "main.h"
#include "menu.h"

void boot(void) {
    __boot_tvtype = TV_NTSC;
    debug_init_isviewer();
    debug_init_usblog();
    asset_init_compression(2);

    dfs_init(DFS_DEFAULT_LOCATION);
	gCameraFocus.v[0] = -4;
	gCameraFocus.v[1] = 0;
	gCameraFocus.v[2] = 30;
	gCameraPos.v[0] = -4;
	gCameraPos.v[1] = 250;
	gCameraPos.v[2] = 100;

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
	if (display_get_height() == 480) {
		gScreenMul = 2;
		gScreenDiv = 1;
	} else {
		gScreenMul = 1;
		gScreenDiv = 2;
	}

    rdpq_init();
    input_init();
	audio_boot();
    t3d_init((T3DInitParams){});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
	gFonts[0] = rdpq_font_load("rom://corn.font64");
    rdpq_text_register_font(2, gFonts[0]);

	gBaseBuildingModel = t3d_model_load("rom://basebuilding.t3dm");
	gBaseBuildingShadowModel = t3d_model_load("rom://basebuildingshadow.t3dm");
	gCursorSprite = sprite_load("rom://cursor.i4.sprite");
	gArrowSprite = sprite_load("rom://arrow.i4.sprite");
	gPointerSprite = sprite_load("rom://pointer.ia4.sprite");
	gNumberSprites = sprite_load("rom://numbers.i4.sprite");
	gNumberBGSprite = sprite_load("rom://timershadow.i8.sprite");
	gScoreBoardSprite = sprite_load("rom://blackboard.i8.sprite");
	gScoreBorderSprite = sprite_load("rom://blackboardframe.ci8.sprite");
	gScoreUnderlaySprite = sprite_load("rom://scoreunderlay.i4.sprite");
	gScoreLeaderSprite = sprite_load("rom://scoreleader.i4.sprite");
	gScoreBoardPlayerSprites[0] = sprite_load("rom://chalkp1.i4.sprite");
	gScoreBoardPlayerSprites[1] = sprite_load("rom://chalkp2.i4.sprite");
	gScoreBoardPlayerSprites[2] = sprite_load("rom://chalkp3.i4.sprite");
	gScoreBoardPlayerSprites[3] = sprite_load("rom://chalkp4.i4.sprite");
	gPauseOptionSprites[0] = sprite_load("rom://pauseopt1.i8.sprite");
	gPauseOptionSprites[1] = sprite_load("rom://pauseopt2.i8.sprite");
	gPauseOptionSprites[2] = sprite_load("rom://pauseopt3.i8.sprite");
	gPauseOptionSprites[3] = sprite_load("rom://menuconfirm1.i8.sprite");
	gPauseOptionSprites[4] = sprite_load("rom://menuconfirm2.i8.sprite");
	gLevelSprites[0] = sprite_load("rom://sand12.ci4.sprite");
	gLevelSprites[1] = sprite_load("rom://roadpath0.rgba16.sprite");
	gLevelSprites[2] = sprite_load("rom://grass0.rgba16.sprite");
	gLevelSprites[3] = sprite_load("rom://fence2.rgba16.sprite");
	gLevelSprites[4] = sprite_load("rom://wall1.i8.sprite");
	gLevelSprites[5] = sprite_load("rom://bricks4.rgba16.sprite");

	gTroopSprites[0][DIR_DOWN][0] = sprite_load("rom://down_0_0.rgba32.sprite");
	gTroopSprites[0][DIR_DOWN][1] = sprite_load("rom://down_0_1.rgba32.sprite");
	gTroopSprites[0][DIR_DOWN][2] = gTroopSprites[0][DIR_DOWN][0];
	gTroopSprites[0][DIR_DOWN][3] = sprite_load("rom://down_0_3.rgba32.sprite");
	gTroopSprites[0][DIR_DOWN][4] = sprite_load("rom://down_0_4.rgba32.sprite");
	gTroopSprites[0][DIR_DOWN][5] = gTroopSprites[0][DIR_DOWN][3];
	gTroopSprites[0][DIR_LEFT][0] = sprite_load("rom://left_0_0.rgba32.sprite");
	gTroopSprites[0][DIR_LEFT][1] = sprite_load("rom://left_0_1.rgba32.sprite");
	gTroopSprites[0][DIR_LEFT][2] = sprite_load("rom://left_0_2.rgba32.sprite");
	gTroopSprites[0][DIR_LEFT][3] = sprite_load("rom://left_0_3.rgba32.sprite");
	gTroopSprites[0][DIR_LEFT][4] = sprite_load("rom://left_0_4.rgba32.sprite");
	gTroopSprites[0][DIR_LEFT][5] = sprite_load("rom://left_0_5.rgba32.sprite");
	gTroopSprites[0][DIR_UP][0] = sprite_load("rom://up_0_0.rgba32.sprite");
	gTroopSprites[0][DIR_UP][1] = sprite_load("rom://up_0_1.rgba32.sprite");
	gTroopSprites[0][DIR_UP][2] = sprite_load("rom://up_0_2.rgba32.sprite");
	gTroopSprites[0][DIR_UP][3] = sprite_load("rom://up_0_3.rgba32.sprite");
	gTroopSprites[0][DIR_UP][4] = sprite_load("rom://up_0_4.rgba32.sprite");
	gTroopSprites[0][DIR_UP][5] = sprite_load("rom://up_0_5.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][0] = sprite_load("rom://right_0_0.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][1] = sprite_load("rom://right_0_1.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][2] = sprite_load("rom://right_0_2.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][3] = sprite_load("rom://right_0_3.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][4] = sprite_load("rom://right_0_4.rgba32.sprite");
	gTroopSprites[0][DIR_RIGHT][5] = sprite_load("rom://right_0_5.rgba32.sprite");
	
	gTroopSprites[1][DIR_DOWN][0] = sprite_load("rom://down_0_0.rgba32.sprite");
	gTroopSprites[1][DIR_DOWN][1] = sprite_load("rom://down_0_1.rgba32.sprite");
	gTroopSprites[1][DIR_DOWN][2] = gTroopSprites[1][DIR_DOWN][0];
	gTroopSprites[1][DIR_DOWN][3] = sprite_load("rom://down_0_3.rgba32.sprite");
	gTroopSprites[1][DIR_DOWN][4] = sprite_load("rom://down_0_4.rgba32.sprite");
	gTroopSprites[1][DIR_DOWN][5] = gTroopSprites[1][DIR_DOWN][3];
	gTroopSprites[1][DIR_LEFT][0] = sprite_load("rom://left_0_0.rgba32.sprite");
	gTroopSprites[1][DIR_LEFT][1] = sprite_load("rom://left_0_1.rgba32.sprite");
	gTroopSprites[1][DIR_LEFT][2] = sprite_load("rom://left_0_2.rgba32.sprite");
	gTroopSprites[1][DIR_LEFT][3] = sprite_load("rom://left_0_3.rgba32.sprite");
	gTroopSprites[1][DIR_LEFT][4] = sprite_load("rom://left_0_4.rgba32.sprite");
	gTroopSprites[1][DIR_LEFT][5] = sprite_load("rom://left_0_5.rgba32.sprite");
	gTroopSprites[1][DIR_UP][0] = sprite_load("rom://up_0_0.rgba32.sprite");
	gTroopSprites[1][DIR_UP][1] = sprite_load("rom://up_0_1.rgba32.sprite");
	gTroopSprites[1][DIR_UP][2] = sprite_load("rom://up_0_2.rgba32.sprite");
	gTroopSprites[1][DIR_UP][3] = sprite_load("rom://up_0_3.rgba32.sprite");
	gTroopSprites[1][DIR_UP][4] = sprite_load("rom://up_0_4.rgba32.sprite");
	gTroopSprites[1][DIR_UP][5] = sprite_load("rom://up_0_5.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][0] = sprite_load("rom://right_0_0.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][1] = sprite_load("rom://right_0_1.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][2] = sprite_load("rom://right_0_2.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][3] = sprite_load("rom://right_0_3.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][4] = sprite_load("rom://right_0_4.rgba32.sprite");
	gTroopSprites[1][DIR_RIGHT][5] = sprite_load("rom://right_0_5.rgba32.sprite");
	
	gTroopSprites[2][DIR_DOWN][0] = sprite_load("rom://down_0_0.rgba32.sprite");
	gTroopSprites[2][DIR_DOWN][1] = sprite_load("rom://down_0_1.rgba32.sprite");
	gTroopSprites[2][DIR_DOWN][2] = gTroopSprites[2][DIR_DOWN][0];
	gTroopSprites[2][DIR_DOWN][3] = sprite_load("rom://down_0_3.rgba32.sprite");
	gTroopSprites[2][DIR_DOWN][4] = sprite_load("rom://down_0_4.rgba32.sprite");
	gTroopSprites[2][DIR_DOWN][5] = gTroopSprites[2][DIR_DOWN][3];
	gTroopSprites[2][DIR_LEFT][0] = sprite_load("rom://left_0_0.rgba32.sprite");
	gTroopSprites[2][DIR_LEFT][1] = sprite_load("rom://left_0_1.rgba32.sprite");
	gTroopSprites[2][DIR_LEFT][2] = sprite_load("rom://left_0_2.rgba32.sprite");
	gTroopSprites[2][DIR_LEFT][3] = sprite_load("rom://left_0_3.rgba32.sprite");
	gTroopSprites[2][DIR_LEFT][4] = sprite_load("rom://left_0_4.rgba32.sprite");
	gTroopSprites[2][DIR_LEFT][5] = sprite_load("rom://left_0_5.rgba32.sprite");
	gTroopSprites[2][DIR_UP][0] = sprite_load("rom://up_0_0.rgba32.sprite");
	gTroopSprites[2][DIR_UP][1] = sprite_load("rom://up_0_1.rgba32.sprite");
	gTroopSprites[2][DIR_UP][2] = sprite_load("rom://up_0_2.rgba32.sprite");
	gTroopSprites[2][DIR_UP][3] = sprite_load("rom://up_0_3.rgba32.sprite");
	gTroopSprites[2][DIR_UP][4] = sprite_load("rom://up_0_4.rgba32.sprite");
	gTroopSprites[2][DIR_UP][5] = sprite_load("rom://up_0_5.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][0] = sprite_load("rom://right_0_0.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][1] = sprite_load("rom://right_0_1.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][2] = sprite_load("rom://right_0_2.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][3] = sprite_load("rom://right_0_3.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][4] = sprite_load("rom://right_0_4.rgba32.sprite");
	gTroopSprites[2][DIR_RIGHT][5] = sprite_load("rom://right_0_5.rgba32.sprite");
	
	gTroopSprites[3][DIR_DOWN][0] = sprite_load("rom://down_0_0.rgba32.sprite");
	gTroopSprites[3][DIR_DOWN][1] = sprite_load("rom://down_0_1.rgba32.sprite");
	gTroopSprites[3][DIR_DOWN][2] = gTroopSprites[3][DIR_DOWN][0];
	gTroopSprites[3][DIR_DOWN][3] = sprite_load("rom://down_0_3.rgba32.sprite");
	gTroopSprites[3][DIR_DOWN][4] = sprite_load("rom://down_0_4.rgba32.sprite");
	gTroopSprites[3][DIR_DOWN][5] = gTroopSprites[3][DIR_DOWN][3];
	gTroopSprites[3][DIR_LEFT][0] = sprite_load("rom://left_0_0.rgba32.sprite");
	gTroopSprites[3][DIR_LEFT][1] = sprite_load("rom://left_0_1.rgba32.sprite");
	gTroopSprites[3][DIR_LEFT][2] = sprite_load("rom://left_0_2.rgba32.sprite");
	gTroopSprites[3][DIR_LEFT][3] = sprite_load("rom://left_0_3.rgba32.sprite");
	gTroopSprites[3][DIR_LEFT][4] = sprite_load("rom://left_0_4.rgba32.sprite");
	gTroopSprites[3][DIR_LEFT][5] = sprite_load("rom://left_0_5.rgba32.sprite");
	gTroopSprites[3][DIR_UP][0] = sprite_load("rom://up_0_0.rgba32.sprite");
	gTroopSprites[3][DIR_UP][1] = sprite_load("rom://up_0_1.rgba32.sprite");
	gTroopSprites[3][DIR_UP][2] = sprite_load("rom://up_0_2.rgba32.sprite");
	gTroopSprites[3][DIR_UP][3] = sprite_load("rom://up_0_3.rgba32.sprite");
	gTroopSprites[3][DIR_UP][4] = sprite_load("rom://up_0_4.rgba32.sprite");
	gTroopSprites[3][DIR_UP][5] = sprite_load("rom://up_0_5.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][0] = sprite_load("rom://right_0_0.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][1] = sprite_load("rom://right_0_1.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][2] = sprite_load("rom://right_0_2.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][3] = sprite_load("rom://right_0_3.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][4] = sprite_load("rom://right_0_4.rgba32.sprite");
	gTroopSprites[3][DIR_RIGHT][5] = sprite_load("rom://right_0_5.rgba32.sprite");

	gMenuID = MENU_LOGOS;

	rdpq_texparms_t parms = {0};
	parms.s.mirror = true;
	parms.t.mirror = true;
    T3DVertPacked *cursorVtx = malloc_uncached_aligned(0x10, (sizeof(T3DVertPacked) * 2));
	cursorVtx[0] = (T3DVertPacked){
		.posA = {-16, 0, -16}, .stA = {0, 0},
		.posB = {16, 0, -16}, .stB = {1024, 0},
	};

	cursorVtx[1] = (T3DVertPacked){
		.posA = {16, 0, 16}, .stA = {1024, 1024},
		.posB = {-16, 0, 16}, .stB = {0, 1024},
	};
    T3DVertPacked *troopVtx = malloc_uncached_aligned(0x10, (sizeof(T3DVertPacked) * 2));
	troopVtx[0] = (T3DVertPacked){
		.posA = {-16, 0, 16}, .stA = {0, 1024},
		.posB = {16, 0, 16}, .stB = {1024, 1024},
	};

	troopVtx[1] = (T3DVertPacked){
		.posA = {16, 24, -16}, .stA = {1024, 0},
		.posB = {-16, 24, -16}, .stB = {0, 0},
	};
	data_cache_writeback_invalidate_all();
    rspq_block_begin();
		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
		rdpq_sync_pipe();
    cursor = rspq_block_end();
    rspq_block_begin();
		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    gArrowBlock = rspq_block_end();
	
    rspq_block_begin();
		t3d_vert_load(troopVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    gTroopBlock = rspq_block_end();

    rspq_block_begin();
		t3d_model_draw(gBaseBuildingShadowModel);
    gBaseBlock = rspq_block_end();

    rspq_block_begin();
		t3d_model_draw(gBaseBuildingModel);
    gBaseBlock2 = rspq_block_end();

	//game_init(0, 0);
	gGamePaused = true;
	
	gPlayerIDs[0] = PLAYER_NONE;
	gPlayerIDs[1] = PLAYER_NONE;
	gPlayerIDs[2] = PLAYER_NONE;
	gPlayerIDs[3] = PLAYER_NONE;
}