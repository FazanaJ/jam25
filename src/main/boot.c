#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "boot.h"
#include "input.h"
#include "main.h"


void boot(void) {
    __boot_tvtype = TV_NTSC;
    debug_init_isviewer();
    debug_init_usblog();
    asset_init_compression(2);

    dfs_init(DFS_DEFAULT_LOCATION);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

    rdpq_init();
    input_init();
	display_set_fps_limit(30.0f);

	gCursorSprite = sprite_load("rom://cursor.i4.sprite");
	gArrowSprite = sprite_load("rom://arrow.i4.sprite");
	gPointerSprite = sprite_load("rom://pointer.ia4.sprite");

	gPlayerCursors[0][0] = 2; 
	gPlayerCursors[0][2] = 2;
	gPlayerCursors[1][0] = 9; 
	gPlayerCursors[1][2] = 2;
	gPlayerCursors[2][0] = 2; 
	gPlayerCursors[2][2] = 7;
	gPlayerCursors[3][0] = 9; 
	gPlayerCursors[3][2] = 7;

	gPlayerPointers[0][0] = 2.75f; 
	gPlayerPointers[0][1] = 2.75f;
	gPlayerPointers[1][0] = 9.75f; 
	gPlayerPointers[1][1] = 2.75f;
	gPlayerPointers[2][0] = 2.75f; 
	gPlayerPointers[2][1] = 7.75f;
	gPlayerPointers[3][0] = 9.75f; 
	gPlayerPointers[3][1] = 7.75f;

	gSpawnerRuinTime = 100;

	gBasePos[0].x = -1;
	gBasePos[1].x = -1;
	gBasePos[2].x = -1;
	gBasePos[3].x = -1;

	gPointerGlobalTime = 15;

	gGameTimer = 30 * 60 * 3;

	gSpawnerGlobalTime = 25;
	for (int i = 0; i < 8; i++) {
		gSpawnerPos[i].x = -1;
		gSpawnerCD[i] = rand() % gSpawnerGlobalTime;
	}


	gPlayerCount = 1;
	gCursorCount = 4;

    t3d_init((T3DInitParams){});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

    T3DVertPacked *mapfloorvtx = malloc_uncached((sizeof(T3DVertPacked) * 2) * 13 * 11);
    uint16_t norm = t3d_vert_pack_normal(&(T3DVec3){{ 0, 0, 1}}); // normals are packed in a 5.6.5 format

	int const W = 13;
	int const H = 11;
	int const CELL = 512 / 12;
	int u0 = 0,    v0 = 0;
	int u1 = 512, v1 = 512;

	int m = 0;
	int mapid = 0;
	// Floors
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			int mall = map[mapid] % 8;
			if (mall != 7) {
				int x0 = -256 + j * CELL;
				int z0 = -256 + i * CELL;
				int x1 = x0 + CELL;
				int z1 = z0 + CELL;
				int y0;
				int y1;
				int colour = ((i + j) & 1) ? 0xFFFFFFFF : 0xDFDFFFFF;
				y0 = mapHeights[mall];
				y1 = y0;
				mapfloorvtx[m++] = (T3DVertPacked){
					.posA = {x0, y0, z0}, .rgbaA = colour, .normA = norm, .stA = {u0, v0},
					.posB = {x1, y1, z0}, .rgbaB = colour, .normB = norm, .stB = {u1, v0},
				};

				mapfloorvtx[m++] = (T3DVertPacked){
					.posA = {x1, y0, z1}, .rgbaA = colour, .normA = norm, .stA = {u1, v1},
					.posB = {x0, y1, z1}, .rgbaB = colour, .normB = norm, .stB = {u0, v1},
				};
			}

			u0 += 512;
			u1 += 512;
			mapid++;
		}
		u0 = 0;
		u1 = 512;
		v0 += 512;
		v1 += 512;
	}
	// --- Compute wallCount first ---
	int wallCount = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if (map[i*W + j] >= 8 && map[i*W + j] < 16) {
				// Each wall can generate up to 2 packed verts per side
				if (j < W - 1 && map[i*W + j + 1] >= 8) wallCount += 2;
				if (i < H - 1 && map[(i+1)*W + j] >= 8) wallCount += 2;
			}
		}
	}

	// --- Allocate exactly what you need ---
	T3DVertPacked *mapwallvtx = malloc_uncached(sizeof(T3DVertPacked) * wallCount);
	if (!mapwallvtx) while(1); // allocation failed

	// --- Fill wall vertices ---
	m = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			int cell = map[i*W + j];
			if (cell >= 8) {
				int x0 = -256 + j * CELL;
				int z0 = -256 + i * CELL;
				int x1 = x0 + CELL;
				int z1 = z0 + CELL;
				int y0 = mapHeights[cell % 8];
				int y1 = y0 + 20;
				int colour = 0x6F6F6FFF;

				// Right wall
				if (j < W - 1 && map[i*W + j + 1] >= 8 && map[i*W + j + 1] < 16) {
					mapwallvtx[m++] = (T3DVertPacked){
						.posA = {x0, y0, z0}, .posB = {x1, y0, z0}, .rgbaA = colour, .rgbaB = colour, .normA = norm, .normB = norm, .stA = {0,0}, .stB = {1024,0}
					};
					mapwallvtx[m++] = (T3DVertPacked){
						.posA = {x1, y1, z0}, .posB = {x0, y1, z0}, .rgbaA = colour, .rgbaB = colour, .normA = norm, .normB = norm, .stA = {1024,1024}, .stB = {0,1024}
					};
				}

				// Front wall
				if (i < H - 1 && map[(i+1)*W + j] >= 8 && map[(i+1)*W + j] < 16) {
					mapwallvtx[m++] = (T3DVertPacked){
						.posA = {x0, y0, z0}, .posB = {x0, y0, z1}, .rgbaA = colour, .rgbaB = colour, .normA = norm, .normB = norm, .stA = {0,0}, .stB = {1024,0}
					};
					mapwallvtx[m++] = (T3DVertPacked){
						.posA = {x0, y1, z1}, .posB = {x0, y1, z0}, .rgbaA = colour, .rgbaB = colour, .normA = norm, .normB = norm, .stA = {1024,1024}, .stB = {0,1024}
					};
				}
			}
		}
	}

	debugf("%d\n", wallCount);

	T3DVertPacked *baseVtx = malloc_uncached(sizeof(T3DVertPacked) * 8);
	m = 0;
	baseVtx[m++] = (T3DVertPacked){
		.posA = {0, 0, 0}, .stA = {u0, v0},
		.posB = {42, 0, 0}, .stB = {u1, v0},
	};
	baseVtx[m++] = (T3DVertPacked){
		.posA = {42, 0, 42}, .stA = {u1, v1},
		.posB = {0, 0, 42}, .stB = {u0, v1},
	};

	baseVtx[m++] = (T3DVertPacked){
		.posA = {0, 16, 0}, .stA = {u0, v0},
		.posB = {42, 16, 0}, .stB = {u1, v0},
	};
	baseVtx[m++] = (T3DVertPacked){
		.posA = {42, 16, 42}, .stA = {u1, v1},
		.posB = {0, 16, 42}, .stB = {u0, v1},
	};
	sprite_t *tex = sprite_load("rom://sand12.ci4.sprite");
	rdpq_texparms_t parms = {0};

    rspq_block_begin();
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(true, true);
		rdpq_mode_antialias(AA_STANDARD);
		rdpq_sprite_upload(TILE0, tex, &parms);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED | T3D_FLAG_DEPTH);
		t3d_vert_load(baseVtx, 0, 8);

		// Bottom
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);

		// Top
		t3d_tri_draw(4, 6, 5);
		t3d_tri_draw(6, 4, 7);

		// Front
		t3d_tri_draw(3, 2, 6);
		t3d_tri_draw(6, 7, 3);

		// Back
		t3d_tri_draw(0, 5, 1);
		t3d_tri_draw(0, 4, 5);

		// Left
		t3d_tri_draw(0, 3, 7);
		t3d_tri_draw(7, 4, 0);

		// Right
		t3d_tri_draw(1, 5, 6);
		t3d_tri_draw(6, 2, 1);
		t3d_tri_sync();
    gBaseBlock = rspq_block_end();


	parms.s.repeats = REPEAT_INFINITE;
	parms.t.repeats = REPEAT_INFINITE;

    rspq_block_begin();
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(false, false);
		rdpq_mode_antialias(AA_REDUCED);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
		rdpq_sprite_upload(TILE0, tex, &parms);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
    	t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED);

		for (int row = 0; row < H; row++) {
			int startPacked = row * W * 2;    // 2 packed verts per cell
			int cellCount = W;                // 16
			int vertCount = cellCount * 4;    // 64

			t3d_vert_load(&mapfloorvtx[startPacked], 0, vertCount);

			for (int v = 0; v < vertCount; v += 4) {
				// triangle 1
				t3d_tri_draw(0 + v, 1 + v, 2 + v);
				// triangle 2
				t3d_tri_draw(2 + v, 3 + v, 0 + v);
			}
		}
		t3d_tri_sync();
    dplMapFloor = rspq_block_end();

    rspq_block_begin();
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(false, false);
		rdpq_mode_antialias(AA_NONE);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
		rdpq_sprite_upload(TILE0, tex, &parms);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
		t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED);

		const int MAX_PACKED_PER_LOAD = 70;
		int packedOffset = 0;
		while (packedOffset < wallCount) {
			int remaining = wallCount - packedOffset;
			int chunk = remaining > MAX_PACKED_PER_LOAD ? MAX_PACKED_PER_LOAD : remaining;

			// Ensure even number of packed verts
			chunk &= ~1;
			if (chunk == 0) break;

			int unpackedCount = chunk * 2; // 2 verts per packed vert
			t3d_vert_load(&mapwallvtx[packedOffset], 0, unpackedCount);

			for (int v = 0; v + 3 < unpackedCount; v += 4) {
				t3d_tri_draw(v+0, v+1, v+2);
				t3d_tri_draw(v+0, v+2, v+3);
			}

			packedOffset += chunk;
		}
		t3d_tri_sync();
	dplMapWalls = rspq_block_end();

	parms.s.mirror = true;
	parms.t.mirror = true;
    T3DVertPacked *cursorVtx = malloc_uncached((sizeof(T3DVertPacked) * 2));
	cursorVtx[0] = (T3DVertPacked){
		.posA = {-22, 0, -22}, .stA = {0, 0},
		.posB = {22, 0, -22}, .stB = {1024, 0},
	};

	cursorVtx[1] = (T3DVertPacked){
		.posA = {22, 0, 22}, .stA = {1024, 1024},
		.posB = {-22, 0, 22}, .stB = {0, 1024},
	};
    rspq_block_begin();
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(false, false);
		rdpq_mode_antialias(AA_NONE);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED);

		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    cursor = rspq_block_end();
    rspq_block_begin();
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(false, false);
		rdpq_mode_antialias(AA_NONE);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
    	t3d_state_set_drawflags(T3D_FLAG_TEXTURED);

		t3d_vert_load(cursorVtx, 0, 4);
		t3d_tri_draw(0, 1, 2);
		t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    gArrowBlock = rspq_block_end();
}