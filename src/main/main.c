#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

/**
 * Example project showcasing the usage of the animation system.
 * This includes instancing animations, blending animations, and controlling playback.
 */

#define FB_COUNT 3

float get_time_s() {
  return (float)((double)get_ticks_us() / (double) 1000000.0);
}

int main()
{
  debug_init_isviewer();
  debug_init_usblog();
  asset_init_compression(2);

  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, FB_COUNT, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

  rdpq_init();
  joypad_init();

  t3d_init((T3DInitParams){});
  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
  T3DViewport viewport = t3d_viewport_create_buffered(FB_COUNT);

  T3DMat4FP* modelMatFP = malloc_uncached(sizeof(T3DMat4FP) * FB_COUNT);

  T3DMat4FP* mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){1.0f, 1.0f, 1.0f}, (float[3]){0, 0, 0}, (float[3]){0, 0, 0});

  T3DVec3 camPos = {{0, 45.0f, 80.0f}};
  T3DVec3 camTarget = {{0, 0,-10}};

  T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
  t3d_vec3_norm(&lightDirVec);

  uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
  uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

  T3DModel *modelMap = t3d_model_load("rom:/map.t3dm");

  rspq_block_begin();
    t3d_matrix_push(mapMatFP);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(modelMap);
    t3d_matrix_pop(1);
  rspq_block_t *dplMap = rspq_block_end();

  for(;;) {

    // position the camera behind the player
    camTarget.v[0] = 0;
    camTarget.v[1] = 0;
    camTarget.v[2] = 60;
    camPos.v[0] = 0;
    camPos.v[1] = 250;
    camPos.v[2] = 180;

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

