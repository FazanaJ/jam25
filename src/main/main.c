#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

uint8_t map[12 * 10] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 9, 9, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 9, 9, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int mapHeights[] = {
	0,
	16,
	32,
	64
};

int main(void) {
    debug_init_isviewer();
    debug_init_usblog();
    asset_init_compression(2);

    dfs_init(DFS_DEFAULT_LOCATION);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

    rdpq_init();
    joypad_init();

    t3d_init((T3DInitParams){});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
    T3DViewport viewport = t3d_viewport_create_buffered(2);

    T3DVertPacked *mapvtx = malloc_uncached((sizeof(T3DVertPacked) * 2) * 12 * 10);
    uint16_t norm = t3d_vert_pack_normal(&(T3DVec3){{ 0, 0, 1}}); // normals are packed in a 5.6.5 format

    /*mapvtx[0] = (T3DVertPacked){
      .posA = {-256, 0, -256}, .rgbaA = 0xFF0000FF, .normA = norm,
      .posB = { 256, 0, -256}, .rgbaB = 0x00FF00FF, .normB = norm,
    };
    mapvtx[1] = (T3DVertPacked){
      .posA = { 256,  0, 200}, .rgbaA = 0x0000FFFF, .normA = norm,
      .posB = {-256,  0, 200}, .rgbaB = 0xFF00FFFF, .normB = norm,
    };*/

	int const W = 12;
	int const H = 10;
	int const CELL = 512 / W;
	int u0 = 0,    v0 = 0;
	int u1 = 512, v1 = 512;

	int m = 0;
	int mapid = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {

			int x0 = -256 + j * CELL;
			int z0 = -256 + i * CELL;
			int x1 = x0 + CELL;
			int z1 = z0 + CELL;
			int y0;
			int y1;


			if (map[mapid] != 9) {
				int colour = ((i + j) & 1) ? 0xFFA0A0FF : 0xA0A0FFFF;
				y0 = mapHeights[map[mapid]];
				y1 = mapHeights[map[mapid]];
				mapvtx[m++] = (T3DVertPacked){
					.posA = {x0, y0, z0}, .rgbaA = colour, .normA = norm, .stA = {u0, v0},
					.posB = {x1, y1, z0}, .rgbaB = colour, .normB = norm, .stB = {u1, v0},
				};

				mapvtx[m++] = (T3DVertPacked){
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


    T3DVec3 camPos = {{0, 45.0f, 80.0f}};
    T3DVec3 camTarget = {{0, 0,-10}};

    T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

	sprite_t *tex = sprite_load("rom://sand12.ci4.sprite");
	rdpq_texparms_t parms = {0};

	parms.s.repeats = REPEAT_INFINITE;
	parms.t.repeats = REPEAT_INFINITE;

    rspq_block_begin();
		rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
		rdpq_set_mode_standard();
		rdpq_mode_zbuf(false, false);
		rdpq_mode_antialias(AA_STANDARD);
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_SHADE);
		rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
		rdpq_sprite_upload(TILE0, tex, &parms);
		rdpq_mode_persp(true);
		rdpq_mode_filter(FILTER_BILINEAR);
    	t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED);

		for (int row = 0; row < H; row++) {
			int startPacked = row * W * 2;    // 2 packed verts per cell
			int cellCount = W;                // 16
			int vertCount = cellCount * 4;    // 64

			t3d_vert_load(&mapvtx[startPacked], 0, vertCount);

			for (int v = 0; v < vertCount; v += 4) {
				// triangle 1
				t3d_tri_draw(0 + v, 1 + v, 2 + v);
				// triangle 2
				t3d_tri_draw(2 + v, 3 + v, 0 + v);
			}
		}

		//t3d_tri_draw(0, 1, 2);
		//t3d_tri_draw(2, 3, 0);
		t3d_tri_sync();
    rspq_block_t *dplMap = rspq_block_end();

    while(true) {

		camTarget.v[0] = 0;
		camTarget.v[1] = 0;
		camTarget.v[2] = 0;
		camPos.v[0] = 0;
		camPos.v[1] = 250;
		camPos.v[2] = 70;

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

		rspq_block_run(dplMap);

		rdpq_detach_show();
    }

    return 0;
}

