#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#include "input.h"

char *gMemSizeTags[] = {
	"B", "KB", "MB"
};

// Key:

uint8_t map[13 * 11] = {
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     7,
	0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0,     7,
	0, 0, 16, 0, 0, 0, 0, 0, 0, 16, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,
	0, 0, 16, 0, 0, 0, 0, 0, 0, 16, 0, 0,     7,
	0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0,     7,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     7,


	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     7,
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

ArrowData gMapArrows[13 * 10];
T3DMat4FP gArrowMtx[12 * 10];
T3DMat4FP gTroopsMtx[300][2];
T3DMat4FP gBaseMtx[4];
T3DMat4FP gSpawnermtx[8];
T3DVec3 gBasePos[4];
T3DVec3 gSpawnerPos[8];
int gSpawnerCD[8];
int gPoints[4];
int gSpawnerGlobalTime;

TroopObj gTroops[300];

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

float gPlayerCursors[4][2];
float gPlayerCursorsTarget[4][2];
int gPlayerCount;
int gCursorCount;
T3DMat4FP gCursorMtx[4][2];



sprite_t *gCursorSprite;
sprite_t *gArrowSprite;

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

#define RATIO_304x224 (304.0f / 224.0f)
const resolution_t RESOLUTION_304x224 = {.width = 304, .height = 224, .interlaced = INTERLACE_OFF, .overscan_margin = 0.025f, .aspect_ratio = RATIO_304x224};
const resolution_t RESOLUTION_304x2242 = {.width = 640, .height = 480, .interlaced = INTERLACE_HALF};
int main(void) {
    __boot_tvtype = TV_NTSC;
    debug_init_isviewer();
    debug_init_usblog();
    asset_init_compression(2);
	int gfxFlip = 0;
	int ticks = 0;

    dfs_init(DFS_DEFAULT_LOCATION);

    display_init(RESOLUTION_304x2242, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

    rdpq_init();
    joypad_init();
	display_set_fps_limit(30.0f);

	gCursorSprite = sprite_load("rom://cursor.i4.sprite");
	gArrowSprite = sprite_load("rom://arrow.i4.sprite");

	gPlayerCursorsTarget[0][0] = 2; gPlayerCursorsTarget[0][1] = 2;
	gPlayerCursors[0][0] = 2; gPlayerCursors[0][1] = 2;

	gPlayerCursorsTarget[1][0] = 9; gPlayerCursorsTarget[1][1] = 2;
	gPlayerCursors[1][0] = 9; gPlayerCursors[1][1] = 2;

	gPlayerCursorsTarget[2][0] = 2; gPlayerCursorsTarget[2][1] = 7;
	gPlayerCursors[2][0] = 2; gPlayerCursors[2][1] = 7;

	gPlayerCursorsTarget[3][0] = 9; gPlayerCursorsTarget[3][1] = 7;
	gPlayerCursors[3][0] = 9; gPlayerCursors[3][1] = 7;

	gTroops[0].active = true;
	gTroops[0].posTarget.x = ((int) 512 / (int) 12) * 3;
	gTroops[0].posTarget.y = 0.0f;
	gTroops[0].posTarget.z = ((int) 512 / (int) 12) * 3;
	gTroops[0].pos.x = gTroops[0].posTarget.x;
	gTroops[0].pos.y = gTroops[0].posTarget.y;
	gTroops[0].pos.z = gTroops[0].posTarget.z;
	gTroops[0].dir = 1;

	gMapArrows[39].playerID = 1;
	gMapArrows[39].dir = 1;
	gMapArrows[43].playerID = 2;
	gMapArrows[43].dir = 2;
	gMapArrows[80].playerID = 3;
	gMapArrows[80].dir = 3;
	gMapArrows[75].playerID = 4;
	gMapArrows[75].dir = 4;
	gMapArrows[2].playerID = 1;
	gMapArrows[2].dir = 1;

	gBasePos[0].x = -1;
	gBasePos[1].x = -1;
	gBasePos[2].x = -1;
	gBasePos[3].x = -1;
	gBasePos[0].x = -1;

	gSpawnerGlobalTime = 25;
	for (int i = 0; i < 8; i++) {
		gSpawnerPos[i].x = -1;
		gSpawnerCD[i] = rand() % gSpawnerGlobalTime;
	}


	gPlayerCount = 1;
	gCursorCount = 4;

    t3d_init((T3DInitParams){});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
    T3DViewport viewport = t3d_viewport_create_buffered(2);

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
    rspq_block_t *gBaseBlock = rspq_block_end();

    T3DVec3 camPos = {{0, 45.0f, 80.0f}};
    T3DVec3 camTarget = {{0, 0,-10}};

    T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};


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
    rspq_block_t *dplMapFloor = rspq_block_end();

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
	rspq_block_t *dplMapWalls = rspq_block_end();

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
    rspq_block_t *cursor = rspq_block_end();
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
    rspq_block_t *gArrowBlock = rspq_block_end();

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

		for (int i = 0; i < 4; i++) {
			
		}

		for (int i = 0; i < 8; i++) {
			if (gSpawnerPos[i].x == -1) {
				continue;
			}

			gSpawnerCD[i]--;
			if (gSpawnerCD[i] <= 0) {
				gSpawnerCD[i] = gSpawnerGlobalTime;

				for (int j = 0; j < 300; j++) {
					if (gTroops[j].active) {
						continue;
					}

					gTroops[j].posTarget.x = gSpawnerPos[i].x;
					gTroops[j].posTarget.z = gSpawnerPos[i].z;
					gTroops[j].pos.x = gSpawnerPos[i].x;
					gTroops[j].pos.z = gSpawnerPos[i].z;

					gTroops[j].active = true;
					gTroops[j].type = 0;
					gTroops[j].dir = (rand() % 4) + 1;
					break;
				}
			}
		}

		for (int i = 0; i < 300; i++) {
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
								gPoints[j] += 1;
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

		bzero(&parms, sizeof(rdpq_texparms_t));
		parms.s.repeats = REPEAT_INFINITE;
		parms.t.repeats = REPEAT_INFINITE;
		parms.s.mirror = true;
		parms.t.mirror = true;

		rdpq_sprite_upload(TILE0, gArrowSprite, &parms);
		for (int i = 0; i < 12; i++) {
			for (int j = 0; j < 10; j++) {
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
		mapid = 0;
		for (int i = 0; i < H; i++) {
			for (int j = 0; j < W; j++) {
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
		for (int i = 0; i < H; i++) {
			for (int j = 0; j < W; j++) {
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
		rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
		for (int i = 0; i < 300; i++) {
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
			float z = ((int) 512 / (int) 12) * gPlayerCursors[i][1];
			t3d_mat4_identity(&mtx);
			t3d_mat4_translate(&mtx, x - 258.0f + 22.0f, y, z - 258.0f + 22.0f);
			t3d_mat4_scale(&mtx, 1.0f + scale, 1.0f, 1.0f + scale);
			t3d_mat4_to_fixed(&gCursorMtx[i][gfxFlip], &mtx);
			data_cache_hit_writeback(&gCursorMtx[i][gfxFlip], sizeof(T3DMat4FP));
			t3d_matrix_push(&gCursorMtx[i][gfxFlip]);
			rspq_block_run(cursor);
			t3d_matrix_pop(1);
		}

    	rdpq_text_printf(NULL, 1, 16, 24, "FPS: %d (%2.1fms)", (int) ceilf(display_get_fps()), (double) (1000.0f / display_get_fps()));
    	rdpq_text_printf(NULL, 1, 16, 34, "RAM: %2.3f%s", (double) memsize_float(ram, &tag), gMemSizeTags[tag]);
    	rdpq_text_printf(NULL, 1, 16, 44, "POINTS:\n1: %d\n2: %d\n3: %d\n4: %d\n", gPoints[0], gPoints[1], gPoints[2], gPoints[3]);

		rdpq_detach_show();
		ticks++;

		gfxFlip ^= 1;
    }

    return 0;
}

