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
	gLevelSprites[0] = sprite_load("rom://sand12.ci4.sprite");
	gLevelSprites[1] = sprite_load("rom://stone.ci4.sprite");
	gLevelSprites[2] = sprite_load("rom://grass0.rgba16.sprite");

	gMenuID = MENU_LOGOS;

	rdpq_texparms_t parms = {0};
	parms.s.mirror = true;
	parms.t.mirror = true;
    T3DVertPacked *cursorVtx = malloc_uncached((sizeof(T3DVertPacked) * 2));
	cursorVtx[0] = (T3DVertPacked){
		.posA = {-16, 0, -16}, .stA = {0, 0},
		.posB = {16, 0, -16}, .stB = {1024, 0},
	};

	cursorVtx[1] = (T3DVertPacked){
		.posA = {16, 0, 16}, .stA = {1024, 1024},
		.posB = {-16, 0, 16}, .stB = {0, 1024},
	};
    rspq_block_begin();
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED);

		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
		rdpq_sync_pipe();
		rdpq_mode_blender(false);
    cursor = rspq_block_end();
    rspq_block_begin();
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED);

		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    gArrowBlock = rspq_block_end();

	T3DVertPacked *baseVtx = malloc_uncached(sizeof(T3DVertPacked) * 8);
	baseVtx[0] = (T3DVertPacked){
		.posA = {0, 0, 0}, .stA = {0, 0},
		.posB = {32, 0, 0}, .stB = {1024, 0},
	};
	baseVtx[1] = (T3DVertPacked){
		.posA = {32, 0, 32}, .stA = {1024, 1024},
		.posB = {0, 0, 32}, .stB = {0, 1024},
	};

	baseVtx[2] = (T3DVertPacked){
		.posA = {0, 12, 0}, .stA = {0, 0},
		.posB = {32, 12, 0}, .stB = {1024, 0},
	};
	baseVtx[3] = (T3DVertPacked){
		.posA = {32, 12, 32}, .stA = {1024, 1024},
		.posB = {0, 12, 32}, .stB = {0, 1024},
	};

    rspq_block_begin();
		rdpq_sprite_upload(TILE0, gLevelSprites[1], &parms);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH);
		t3d_vert_load(baseVtx, 0, 8);

		// Bottom
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);

		// Back
		t3d_tri_draw(0, 5, 1);
		t3d_tri_draw(0, 4, 5);

		// Left
		t3d_tri_draw(0, 3, 7);
		t3d_tri_draw(7, 4, 0);

		// Right
		t3d_tri_draw(1, 5, 6);
		t3d_tri_draw(6, 2, 1);

		// Front
		t3d_tri_draw(3, 2, 6);
		t3d_tri_draw(6, 7, 3);

		// Top
		t3d_tri_draw(4, 6, 5);
		t3d_tri_draw(6, 4, 7);
		t3d_tri_sync();
    gBaseBlock = rspq_block_end();

	//game_init(0, 0);
	gGamePaused = true;
	
	gPlayerIDs[0] = PLAYER_NONE;
	gPlayerIDs[1] = PLAYER_NONE;
	gPlayerIDs[2] = PLAYER_NONE;
	gPlayerIDs[3] = PLAYER_NONE;
}