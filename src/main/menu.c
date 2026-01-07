#include <libdragon.h>
#include <malloc.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#include "ai.h"
#include "audio.h"
#include "boot.h"
#include "input.h"
#include "game.h"
#include "main.h"
#include "menu.h"

sprite_t *gMenuSprites[16];
float gTitleScreenScale;
float gTitleScreenVel;
int gSubMenu;
int gPauseSub;
int gSubMenuOpt;
int gMenuOption[4];
int gMenuFlickerTimer;
int gMenuFlicker;
int gMenuStickTime[2];
int gMenuStickPhase[2];
int gScreenMul;
int gScreenDiv;
int gPrevMenu;
int gNumTies;
int gPlayerWinner;

float gScoreboardOffsetY = 0;
int gScoreTimerOpacity;
int gScoreTimerCurOpacity;
int gPlayerSprites[4];

int gTitleAttractTimer;
float gTitleLogoX;
float gTitleLogoY;
int gAttractTransition;
int gTitleOptionsY;
int gAttractLevelID;
int gAttractLevelTimer;
int gTitleOptionContentY[5];
float gPlayerSelectScale[5];
float gTitleOptionScale[5];
char *gTitleStrings[] = {
    "Quick Play",
    "Challenges",
    "Tutorial",
    "Options",
    "Credits"
};
char *gOptionsMenuStrings[] = {
    "Screen",
    "Screen",
    "Language",
    "Sound",

    "Off",
    "On",
    "Off",
    "LQ",
    "HQ",
    "NTSC",
    "PAL50",
    "PAL60",
    "M-PAL",
    "Mono",
    "Stereo"
 };

static void menu_stick_options(int padID, int *optX, int *optY) {
    if (optX != NULL) {
        int dir = 0;
        if (input_stick_x(padID, STICK_LEFT) > gDeadzone || input_held(padID, INPUT_DRIGHT, 0)) {
            dir = 1;
        } else if (input_stick_x(padID, STICK_LEFT) < -gDeadzone || input_held(padID, INPUT_DLEFT, 0)) {
            dir = -1;
        }
        if (dir != 0) {
            if (gMenuStickTime[STICK_X] > 0) {
                gMenuStickTime[STICK_X]--;
            }
            if (gMenuStickTime[STICK_X] == 0) {
                if (gMenuStickPhase[STICK_X] == 0) {
                    gMenuStickPhase[STICK_X] = 1;
                    gMenuStickTime[STICK_X] = 15;
                } else {
                    gMenuStickTime[STICK_X] = 5;
                }
                *optX += dir;
            }

        } else {
            gMenuStickPhase[STICK_X] = 0;
            gMenuStickTime[STICK_X] = 0;
        }
    }

    if (optY != NULL) {
        int dir = 0;
        if (input_stick_y(padID, STICK_LEFT) > gDeadzone || input_held(padID, INPUT_DDOWN, 2)) {
            dir = 1;
        } else if (input_stick_y(padID, STICK_LEFT) < -gDeadzone || input_held(padID, INPUT_DUP, 2)) {
            dir = -1;
        }
        if (dir != 0) {
            if (gMenuStickTime[STICK_Y] > 0) {
                gMenuStickTime[STICK_Y]--;
            }
            if (gMenuStickTime[STICK_Y] == 0) {
                if (gMenuStickPhase[STICK_Y] == 0) {
                    gMenuStickPhase[STICK_Y] = 1;
                    gMenuStickTime[STICK_Y] = 15;
                } else {
                    gMenuStickTime[STICK_Y] = 5;
                }
                *optY += dir;
            }

        } else {
            gMenuStickPhase[STICK_Y] = 0;
            gMenuStickTime[STICK_Y] = 0;
        }
    }
}

void text_draw_centre(int x, int y, const char *str, color_t colour) {
    rdpq_fontstyle_t style = {0};
    rdpq_textparms_t textP = {0};
    textP.style_id = 1;
    textP.width = display_get_width();
    textP.align = ALIGN_CENTER;
    style.color = RGBA32(0, 0, 0, 255);
    rdpq_font_style(gFonts[0], 1, &style);
    rdpq_text_print(&textP, 2, 2 + x, y + 2, str);
    style.color = colour;
    rdpq_font_style(gFonts[0], 0, &style);
    textP.style_id = 0;
    rdpq_text_print(&textP, 2, x, y, str);
}

void titlescreen_load_assets(void) {
    gMenuSprites[0] = sprite_load("rom://logo.ci8.sprite");
    gMenuSprites[1] = sprite_load("rom://menuoptclosed.rgba32.sprite");
    gMenuSprites[2] = sprite_load("rom://menuoptopen.rgba32.sprite");
    gMenuSprites[5] = sprite_load("rom://menuopt02.rgba32.sprite");
    gMenuSprites[6] = sprite_load("rom://menuopt12.rgba32.sprite");
    gMenuSprites[7] = sprite_load("rom://menuopt22.rgba32.sprite");
    gMenuSprites[8] = sprite_load("rom://menuopt32.rgba32.sprite");
    gMenuSprites[9] = sprite_load("rom://menuopt42.rgba32.sprite");
    gMenuSprites[10] = sprite_load("rom://controller.rgba32.sprite");
    gGamePaused = true;
}

static void menu_render_title(int updateRate, float updateRateF) {
    int shadowOff;

    if (gMenuSprites[0] == NULL) {
        titlescreen_load_assets();
    }
    if (gSubMenu == TITLE_SUB_INIT) {
        gTitleScreenScale = 0.01f;
        gSubMenu = TITLE_SUB_LOGO_GROW;
        gTitleScreenVel = 0.0f;
        gMenuFlickerTimer = 0;
        gMenuFlicker = 0;
        gTitleAttractTimer = 0;
        gTitleLogoX = display_get_width() / 2;
        gTitleLogoY = display_get_height() / 2;
        gTitleOptionsY = display_get_height() + (48 * gScreenMul);
        gTitleOptionScale[0] = 1.25f;
        gTitleOptionScale[1] = 0.75f;
        gTitleOptionScale[2] = 0.75f;
        gTitleOptionScale[3] = 0.75f;
        gTitleOptionScale[4] = 0.75f;
        gPlayerIDs[0] = PLAYER_NONE;
        gPlayerIDs[1] = PLAYER_NONE;
        gPlayerIDs[2] = PLAYER_NONE;
        gPlayerIDs[3] = PLAYER_NONE;
        gPlayerSprites[0] = PLAYER_NONE;
        gPlayerSprites[1] = PLAYER_NONE;
        gPlayerSprites[2] = PLAYER_NONE;
        gPlayerSprites[3] = PLAYER_NONE;
        gAttractTransition = 0;
        gGamePaused = true;
        sound_play_global(SOUND_MENU_LOGO);
    } else if (gSubMenu == TITLE_SUB_LOGO_GROW) {
        gTitleScreenVel += 0.005f * updateRateF;
        if (gTitleScreenVel > 0.1f) {
            gTitleScreenVel = 0.1f;
        }
        if (gTitleScreenScale < 1.0f) {
            gTitleScreenScale += gTitleScreenVel * updateRateF;
            if (gTitleScreenScale > 1.0f) {
                gSubMenu = TITLE_SUB_LOGO_SHRINK;
            }
        }
    } else if (gSubMenu == TITLE_SUB_LOGO_SHRINK) {
        gTitleScreenVel -= 0.005f * updateRateF;
        gTitleScreenScale += gTitleScreenVel * updateRateF;
        if (gTitleScreenScale < 1.0f) {
            gTitleScreenVel = 0.0f;
            gTitleScreenScale = 1.0f;
            sound_play_global(SOUND_MENU_LOGO_LAND);
            gSubMenu = TITLE_SUB_LOGO_IDLE;
        }
    }

    if (gSubMenu == TITLE_SUB_LOGO_IDLE) {
        gTitleAttractTimer += updateRate;

        if (gTitleAttractTimer > 60 * 10) {
            int offset = 40 * gScreenMul;
            
            gTitleLogoX = lerpf(gTitleLogoX, display_get_width() - (offset * 1.5f), 0.025f * updateRateF);
            gTitleLogoY = lerpf(gTitleLogoY, offset, 0.025f * updateRateF);
            gTitleScreenScale = lerpf(gTitleScreenScale, 0.33f, 0.025f * updateRateF);
            gAttractLevelTimer += updateRate;
            gGamePaused = false;
            if (gAttractLevelTimer > 30 * 39) {
                if (gLevelID == 0 && gAttractTransition == 0) {
                    gAttractTransition = 1;
                }
            }
            if (gAttractLevelTimer > 60 * 20) {
                gAttractLevelTimer = -(60 * 10);
                int prevlev = gAttractLevelID;
                while (gAttractLevelID != prevlev) {
                    gAttractLevelID = (rand() % LEVEL_COUNT);
                }
                gAttractLevelID++;
                gLevelID = -1;
            }
            if (gAttractLevelID != 0) {
                text_draw_centre(0, 40 * gScreenMul, "DEMO", RGBA32(255, 255, 255, 255));
            }
        } else {
            gAttractTransition = 0;
            gTitleLogoX = lerpf(gTitleLogoX, display_get_width() / 2, 0.075f * updateRateF);
            gTitleLogoY = lerpf(gTitleLogoY, display_get_height() / 2, 0.075f * updateRateF);
            gTitleScreenScale = lerpf(gTitleScreenScale, 1.0f, 0.075f * updateRateF);
            gTitleOptionsY = lerpf(gTitleOptionsY, display_get_height() + (48 * gScreenMul), 0.075f * updateRateF);
            if (gGamePaused == false) {
                game_init(gLevelID, 0);
            }
            if (gLevelID != 0 && gAttractLevelID == 0) {
                game_init(0, gPlayerCount);
            }
            gAttractLevelID = 0;
            gGamePaused = true;
            gAttractLevelTimer = 0;
        }

        if (gLevelID != 0) {
            if (gLevelID < gAttractLevelID) {
                gMapOffsetX = lerpf(gMapOffsetX, - 1024.0f, 0.125f * updateRateF);
                if (gMapOffsetX <= -510.0f) {
                    game_init(gAttractLevelID + 1, 0);
                    gMapOffsetX = 512.0f;
                }
            } else if (gLevelID > gAttractLevelID) {
                gMapOffsetX = lerpf(gMapOffsetX, 1024.0f, 0.125f * updateRateF);
                if (gMapOffsetX >= 510.0f) {
                    game_init(gAttractLevelID, 0);
                    gMapOffsetX = -512.0f;
                }
            } else {
                gMapOffsetX = lerpf(gMapOffsetX, 0.0f, 0.125f * updateRateF);
            }
        } else {
            gMapOffsetX = 0.0f;
        }

        if (gAttractTransition != 0) {
            if (gAttractTransition > 0) {
                gAttractTransition += updateRate;
                if (gAttractTransition > 30) {
                    if (gLevelID != 0) {
                        gTitleAttractTimer = 0;
                        game_init(0, 0);
                    }
                    gAttractTransition = -30;
                }
            } else {
                gAttractTransition += updateRate;
                if (gAttractTransition > 0) {
                    gAttractTransition = 0;
                }
            }
            rdpq_set_mode_standard();
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            int alpha = ((float) ((float) fabs(gAttractTransition) / 30.0f) * 255.0f);
            rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
            rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
        }

        if ((input_pressed(PLAYER_ALL, INPUT_A, 2) || input_pressed(PLAYER_ALL, INPUT_START, 2))) {
            input_clear(PLAYER_ALL, INPUT_A);
            input_clear(PLAYER_ALL, INPUT_START);
            if (gLevelID == 0) {
                sound_play_global(SOUND_MENU_ACCEPT);
                if (gTitleScreenScale >= 0.98f) {
                    gTitleScreenScale = 1.0f;
                    gTitleAttractTimer = 0;
                    gSubMenu = TITLE_SUB_MOVE_INSIDE;
                }
            } else {
                if (gAttractTransition == 0) {
                    gAttractTransition = 1;
                }
            }
        }
    } else {
        gTitleAttractTimer = 0;
    }

    if (gSubMenu == TITLE_SUB_MOVE_INSIDE) {
        int offset = 64 * gScreenMul;
        
        gTitleLogoX = lerpf(gTitleLogoX, display_get_width() / 2, 0.075f * updateRateF);
        gTitleLogoY = lerpf(gTitleLogoY, offset, 0.075f * updateRateF);
        gTitleOptionsY = lerpf(gTitleOptionsY, display_get_height() - (56 * gScreenMul), 0.1f * updateRateF);
        gTitleScreenScale = lerpf(gTitleScreenScale, 0.5f, 0.075f * updateRateF);

        if (gTitleScreenScale <= 0.52f) {
            gSubMenu = TITLE_SUB_MAIN_MENU;
        }
    }

    if (gSubMenu == TITLE_SUB_MAIN_MENU) {
        int offset = 64 * gScreenMul;
        gMenuOption[1] = 0;

        gTitleOptionsY = lerpf(gTitleOptionsY, display_get_height() - (56 * gScreenMul), 0.1f * updateRateF);
        gTitleLogoY = lerpf(gTitleLogoY, offset, 0.075f * updateRateF);
        if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
            input_clear(PLAYER_ALL, INPUT_B);
            sound_play_global(SOUND_MENU_BACK);
            gSubMenu = TITLE_SUB_LOGO_IDLE;
            gTitleAttractTimer = 0;
        }

        if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
            input_clear(PLAYER_ALL, INPUT_A);
            sound_play_global(SOUND_MENU_ACCEPT);
            gSubMenu = TITLE_SUB_MOVE_OPT;
            gSubMenuOpt = gMenuOption[0] + 1;
        }

        int prevOpt = gMenuOption[0];
        menu_stick_options(PLAYER_ALL, &gMenuOption[0], NULL);

        if (gMenuOption[0] <= 0) {
            gMenuOption[0] = 0;
        } else if (gMenuOption[0] >= 5) {
            gMenuOption[0] = 4;
        }
        if (prevOpt != gMenuOption[0]) {
            sound_play_global(SOUND_MENU_MOVE);
        }
    }

    if (gSubMenu == TITLE_SUB_MOVE_OPT) {
        gTitleOptionsY = lerpf(gTitleOptionsY, 0, 0.1f * updateRateF);
        gTitleLogoY = lerpf(gTitleLogoY, -112, 0.075f * updateRateF);

        if (gTitleOptionsY < 1) {
            gSubMenu = TITLE_SUB_QUICK_PLAY + gMenuOption[0];
        }
    }

    if (gSubMenu >= TITLE_SUB_QUICK_PLAY && gSubMenu < TITLE_SUB_LEVEL_SELECT) {
        if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
            input_clear(PLAYER_ALL, INPUT_B);
            if (gSubMenu == TITLE_SUB_QUICK_PLAY) {
                if (gPlayerIDs[PLAYER_1] == PLAYER_NONE && gPlayerIDs[PLAYER_2] == PLAYER_NONE && 
                    gPlayerIDs[PLAYER_3] == PLAYER_NONE && gPlayerIDs[PLAYER_4] == PLAYER_NONE) {
                    sound_play_global(SOUND_MENU_BACK);
                    gSubMenu = TITLE_SUB_MAIN_MENU;
                    gTitleAttractTimer = 0;
                } else {
                    for (int i = 0; i < 4; i++) {
                        if (input_pressed(i, INPUT_B, 2)) {
                            int port = input_port(i);
                            for (int j = 0; j < 4; j++) {
                                if (gPlayerIDs[j] == port) {
                                    sound_play_global(SOUND_MENU_CONTROLLER_OFF);
                                    gPlayerIDs[j] = PLAYER_NONE;
                                    gPlayerSprites[j] = PLAYER_NONE;
                                    break;
                                }
                            }
                        }
                    }
                }
            } else {
                sound_play_global(SOUND_MENU_BACK);
                gSubMenu = TITLE_SUB_MAIN_MENU;
                gTitleAttractTimer = 0;
            }
        }
    }

    float scale = (0.75f + (fm_sinf(ticks / 20.0f) * 0.025f)) * gTitleScreenScale * gScreenMul;

    rdpq_blitparms_t params = {0};
    params.scale_x = scale;
    params.scale_y = scale;
    params.cx = 160;
    params.cy = 90;
    params.theta = (fm_sinf(ticks / 40.0f) * 0.05f);
    shadowOff = 4 * scale * 2.0f;

    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_dithering(DITHER_BAYER_BAYER);
    if (gTitleLogoY > - (90 * gScreenMul)) {
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        rdpq_sprite_blit(gMenuSprites[0], gTitleLogoX, gTitleLogoY, &params);
    }

    rdpq_textparms_t textP = {0};
    textP.align = ALIGN_CENTER;
    textP.width = display_get_width();

    if (gSubMenu == TITLE_SUB_LOGO_IDLE) {
        if (gMenuFlicker == 0) {
            int tOffsetY = 32 * gScreenMul;
            text_draw_centre(0, (display_get_height() - tOffsetY) + 2, "PRESS START", RGBA32(255, 255, 255, 255));
        }
    }
    if (gTitleOptionsY < display_get_height() + (36 * gScreenMul)) {
        rdpq_blitparms_t params = {0};
        params.cx = 48;
        params.cy = 56;
        params.theta = 0.0f; 
        rdpq_set_mode_standard();
        rdpq_mode_filter(FILTER_BILINEAR);
        const int gapsize = 48 * gScreenMul;
        int x = (display_get_width() - (4 * gapsize)) / 2;
        for (int i = 0; i < 5; i++) {
            int open;
            float s;
            if (gMenuOption[0] == i) {
                s = 1.25f;
                open = true;
            } else {
                s = 0.75f;
                open = false;
            }

            if (gSubMenu >= TITLE_SUB_MOVE_OPT) {
                s *= 0.66f;
            }

            s /= gScreenDiv;

            if (gMenuOption[0] == i) {
                gTitleOptionContentY[i] = lerpf(gTitleOptionContentY[i], 32, 0.2f * updateRateF);
            } else {
                gTitleOptionContentY[i] = lerpf(gTitleOptionContentY[i], 0, 0.2f * updateRateF);
            }

            gTitleOptionScale[i] = lerpf(gTitleOptionScale[i], s, 0.25f); 
            params.scale_x = gTitleOptionScale[i];
            params.scale_y = gTitleOptionScale[i];
            int y = gTitleOptionsY + (fm_sinf((ticks + ((i & 1) * 7600)) / 30.0f) * 4);
            params.theta = fm_sinf((ticks + (i * 7600)) / 30.0f) * 0.05f;
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            if (open) {
                rdpq_sprite_blit(gMenuSprites[1 + open], x, y + ((gTitleOptionContentY[i] * 0.75f) * gScreenMul), &params);
            }

            if (gTitleOptionContentY[i] > 12) {
                rdpq_blitparms_t params2 = {0};
                params2.cx = 32;
                params2.cy = 38;
                params2.scale_x = gTitleOptionScale[i] * ((float) gTitleOptionContentY[i] / 16.0f);
                params2.scale_y = gTitleOptionScale[i] * ((float) gTitleOptionContentY[i] / 16.0f);
                params2.theta = fm_sinf((ticks + ((i + 17) * 3600)) / 30.0f) * 0.05f;
                int offY = (32 - gTitleOptionContentY[i] - ((fm_sinf((ticks + ((i & 1) * 7600)) / 30.0f) * 8))) * gScreenMul;
                rdpq_sprite_blit(gMenuSprites[5 + i], x, y + offY, &params2);
            }

            if (open == false) {
                rdpq_sprite_blit(gMenuSprites[1 + open], x, y + ((gTitleOptionContentY[i] * 0.75f) * gScreenMul), &params);
            }

            x += gapsize;
        }
        x = (display_get_width() - (4 * gapsize)) / 2;
        for (int i = 0; i < 5; i++) {
            int open;
            float s;
            if (gMenuOption[0] == i) {
                s = 1.25f;
                open = true;
            } else {
                s = 0.75f;
                open = false;
            }

            if (gSubMenu >= TITLE_SUB_MOVE_OPT) {
                s *= 0.66f;
            }

            s /= gScreenDiv;

            if (gMenuOption[0] == i) {
                gTitleOptionContentY[i] = lerpf(gTitleOptionContentY[i], 32, 0.2f * updateRateF);
            } else {
                gTitleOptionContentY[i] = lerpf(gTitleOptionContentY[i], 0, 0.2f * updateRateF);
            }

            gTitleOptionScale[i] = lerpf(gTitleOptionScale[i], s, 0.25f); 
            params.scale_x = gTitleOptionScale[i];
            params.scale_y = gTitleOptionScale[i];
            int y = gTitleOptionsY + (fm_sinf((ticks + ((i & 1) * 7600)) / 30.0f) * 4);
            params.theta = fm_sinf((ticks + (i * 7600)) / 30.0f) * 0.05f;
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);

            if (gTitleOptionContentY[i] > 12) {
                rdpq_blitparms_t params2 = {0};
                params2.cx = 32;
                params2.cy = 38;
                params2.scale_x = gTitleOptionScale[i] * ((float) gTitleOptionContentY[i] / 16.0f);
                params2.scale_y = gTitleOptionScale[i] * ((float) gTitleOptionContentY[i] / 16.0f);
                params2.theta = fm_sinf((ticks + ((i + 17) * 3600)) / 30.0f) * 0.05f;
                int offY = (32 - gTitleOptionContentY[i] - ((fm_sinf((ticks + ((i & 1) * 7600)) / 30.0f) * 8))) * gScreenMul;
                rdpq_sprite_blit(gMenuSprites[5 + i], x, y + offY, &params2);
            }

            x += gapsize;
        }

        text_draw_centre(0, gTitleOptionsY + (32 * gScreenMul), gTitleStrings[gMenuOption[0]], RGBA32(255, 255, 255, 255));
    }
    
    if (gSubMenu <= TITLE_SUB_LOGO_IDLE) {
        textP.align = ALIGN_LEFT;
        rdpq_text_print(&textP, 1, 20, display_get_height() - (20 * gScreenMul), "2026 Fazana");
    }

    if (gSubMenuOpt == 1) {
        int gapSize = 96 * gScreenMul;
        int x = ((display_get_width() -(gapSize)) / 2) * gScreenMul;
        int y = (88 + gTitleOptionsY) * gScreenMul;

        if (gSubMenu == TITLE_SUB_QUICK_PLAY) {
            gTitleOptionsY = lerpf(gTitleOptionsY, 0, 0.1f * updateRateF);
            if (input_pressed(PLAYER_ALL, INPUT_START, 2)) {
                sound_play_global(SOUND_MENU_ACCEPT);
                gSubMenu = TITLE_SUB_LEVEL_SELECT;
            }
        }

        if (gTitleOptionsY < display_get_height() - (50 * gScreenMul)) {
            text_draw_centre(0, y - (28 * gScreenMul), "Press A to join", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (128 * gScreenMul), "Press start when ready", RGBA32(255, 255, 255, 255));
        }

        for (int i = 0; i < 4; i++) {
            char *playerStrings[] = {"COM", "P1", "P2", "P3", "P4"};
            rdpq_blitparms_t params2 = {0};
            if (gPlayerIDs[i] != PLAYER_NONE) {
                params2.theta = fm_sinf((ticks + ((i + 17) * 3600)) / 15.0f) * 0.05f;
                gPlayerSelectScale[i] = lerpf(gPlayerSelectScale[i], 2.0f, 0.1f * updateRateF);
            } else {
                gPlayerSelectScale[i] = lerpf(gPlayerSelectScale[i], 1.0f, 0.1f * updateRateF);
                params2.theta = 0.0f;
            }
            
            if (gSubMenu == TITLE_SUB_QUICK_PLAY) {
                if (input_pressed(i, INPUT_A, 2)) {
                    input_clear(i, INPUT_A);
                    int found = false;
                    int port = input_port(i);
                    for (int j = 0; j < 4; j++) {
                        if (gPlayerIDs[j] == port) {
                            found = true;
                            break;
                        }
                    }
                    if (found == false) {
                        for (int j = 0; j < 4; j++) {
                            if (gPlayerIDs[j] == PLAYER_NONE) {
                                gPlayerIDs[j] = port;
                                gPlayerSprites[j] = port;
                                sound_play_global(SOUND_MENU_CONTROLLER_ON);
                                break;
                            }
                        }
                    }
                }
            }

            params2.cx = 40;
            params2.cy = 40;
            float scale = gPlayerSelectScale[i] / gScreenDiv;
            params2.scale_x = scale;
            params2.scale_y = scale;

            if (y + (40 * gScreenMul) > 0 && y < display_get_height()) {
                rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                rdpq_mode_combiner(RDPQ_COMBINER_TEX);
                rdpq_sprite_blit(gMenuSprites[10], x, y, &params2);
                text_draw_centre(x - (display_get_width() / 2), y + (32 * gScreenMul), playerStrings[gPlayerIDs[i] + 1], gPlayerColours[i]);
            }

            x += gapSize;
            if (i == 1) {
                x -= gapSize * 2;
                y += 76 * gScreenMul;
            }
        }
    } else if (gSubMenuOpt == 5) {
        int y = (64 + gTitleOptionsY) * gScreenMul;

        if (y < display_get_height() - (50 * gScreenMul)) {
            text_draw_centre(0, y + (0 * gScreenMul), "Lead Programmer - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (16 * gScreenMul), "Graphics - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (32 * gScreenMul), "Voices - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (48 * gScreenMul), "Ideas - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (64 * gScreenMul), "Investment - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (80 * gScreenMul), "Credits - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (96 * gScreenMul), "SFX - pixabay.com/sound-effects", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (112 * gScreenMul), "Special Thanks - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (128 * gScreenMul), "Junior Lead Deputy Programmer - Fazana", RGBA32(255, 255, 255, 255));
            text_draw_centre(0, y + (144 * gScreenMul), "Super Special Thanks - Fazana", RGBA32(255, 255, 255, 255));
        }
    }
    if (gSubMenu == TITLE_SUB_LEVEL_SELECT) {
        gTitleOptionsY = lerpf(gTitleOptionsY, -240, 0.1f * updateRateF);

        int prevOpt = gMenuOption[3];
        menu_stick_options(PLAYER_ALL, &gMenuOption[3], NULL);

        if (gMenuOption[3] <= 0) {
            gMenuOption[3] = 0;
        } else if (gMenuOption[3] > LEVEL_COUNT - 1) {
            gMenuOption[3] = LEVEL_COUNT - 1;
        }
        if (prevOpt != gMenuOption[3]) {
            sound_play_global(SOUND_MENU_SWOOSH);
        }

        if (gLevelID - 1 < gMenuOption[3]) {
            gMapOffsetX = lerpf(gMapOffsetX, - 1024.0f, 0.125f * updateRateF);
            if (gMapOffsetX <= -510.0f) {
                game_init(gMenuOption[3] + 1, 0);
                gMapOffsetX = 512.0f;
            }
        } else if (gLevelID - 1 > gMenuOption[3]) {
            gMapOffsetX = lerpf(gMapOffsetX, 1024.0f, 0.125f * updateRateF);
            if (gMapOffsetX >= 510.0f) {
                game_init(gMenuOption[3] + 1, 0);
                gMapOffsetX = -512.0f;
            }
        } else {
            gMapOffsetX = lerpf(gMapOffsetX, 0.0f, 0.125f * updateRateF);
        }

        if (fabsf(gMapOffsetX) < 4.0f) {
            if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
                input_clear(PLAYER_ALL, INPUT_B);
                sound_play_global(SOUND_MENU_BACK);
                gMapOffsetX = 0.0f;
                if (gLevelID != 0) {
                    game_init(0, gPlayerCount);
                }
                gSubMenu = TITLE_SUB_QUICK_PLAY;
            } else if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
                input_clear(PLAYER_ALL, INPUT_A);
                gMenuID = MENU_START_COUNTDOWN;
                gSubMenu = 0;
                gMapOffsetX = 0.0f;
                sound_play_global(SOUND_MENU_ACCEPT);
                gSubMenuOpt = 0;
                game_init(gMenuOption[3] + 1, 0);
            }
        }
    }
    if (gSubMenuOpt == 4) {
        int y = (80 + gTitleOptionsY) * gScreenMul;
        rdpq_textparms_t textP = {0};
        for (int i = 0; i < 4; i++) {
            if (y < display_get_height()) {
                rdpq_text_print(&textP, 1, 64 * gScreenMul, y, gOptionsMenuStrings[i]);
            }

            y += 12 * gScreenMul;
        }
    }
}

static void menu_start_countdown(int updateRate, float updateRateF) {
    if (gSubMenu == 0) {
        gSubMenu = 1;
        gMenuSprites[0] = sprite_load("rom://round3.rgba32.sprite");
        gMenuSprites[1] = sprite_load("rom://round2.rgba32.sprite");
        gMenuSprites[2] = sprite_load("rom://round1.rgba32.sprite");
        gMenuSprites[3] = sprite_load("rom://roundgo.rgba32.sprite");
        gTitleOptionScale[0] = 0.0f;
        gTitleOptionScale[1] = 0.0f;
        gTitleOptionScale[2] = 0.0f;
        gTitleOptionScale[3] = 0.0f;
        gTitleOptionContentY[0] = 0;
        gTitleOptionContentY[1] = 0;
        gTitleOptionContentY[2] = 0;
        gTitleOptionContentY[3] = 0;
		gPoints[0] = 0;
		gPoints[1] = 0;
		gPoints[2] = 0;
		gPoints[3] = 0;
		gPointsVisual[0] = 0;
		gPointsVisual[1] = 0;
		gPointsVisual[2] = 0;
		gPointsVisual[3] = 0;
        gMenuOption[0] = 40;
        gSubMenuOpt = 40;
        gPauseSub = 0;
        sound_play_global(SOUND_VOICE_COUNTDOWN_3);
        int counted[4] = {0};
        for (int i = 0; i < 4; i++) {
            if (gPlayerSprites[i] != PLAYER_NONE) {
                counted[gPlayerSprites[i]] = true;
                continue;
            }

            for (int j = 0; j < 4; j++) {
                if (counted[j] == false) {
                    gPlayerSprites[i] = j;
                    counted[j] = true;
                    break;
                }
            }
        }
    } else {
        int sub = gSubMenu - 1;
        gMenuOption[sub] -= updateRate;
        rdpq_blitparms_t params2 = {0};

        if (gMenuOption[sub] < 40 - 20) {
            if (gSubMenuOpt == 0) {
                gSubMenuOpt = 1;
            }
        }
        if (gMenuOption[sub] <= 0) {
            if (gSubMenu < 5) {
                if (gSubMenu < 4)
                sound_play_global(SOUND_VOICE_COUNTDOWN_3 + gSubMenu);
                gSubMenu++;
                gMenuOption[gSubMenu - 1] = 40;
                gSubMenuOpt = 0;
            } else {
                gMenuID = MENU_NONE;
                gSubMenu = 0;
                gSubMenuOpt = 0;
                gGameTimer = 60 * 60 * 3;
                //gGameTimer = 60 * 10; // TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
                gTimerStage = 0;
                gGamePaused = false;
                gScoreboardOffsetY = 64;
                return;
            }
        }

        rdpq_set_mode_standard();
        rdpq_mode_filter(FILTER_BILINEAR);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        for (int i = 0; i < 4; i++) {
            gTitleOptionContentY[i] = gMenuOption[i] - 20;
            int negative = gTitleOptionContentY[i];
            if (gTitleOptionContentY[i] < 0) {
                gTitleOptionContentY[i] = 0;
            }

            float scale = 1.5f * ((1.0f - ((float) gTitleOptionContentY[i] / 20.0f)) / gScreenDiv);
            gTitleOptionScale[i] = scale;
            if (gTitleOptionScale[i] == 0.0f) {
                continue;
            }
            int alpha;
            params2.scale_x = gTitleOptionScale[i];
            params2.scale_y = gTitleOptionScale[i];
            params2.theta = (((1.5f / gScreenDiv) - ((1.5f / gScreenDiv) * scale)) / 4.0f);
            if (i == 3) {
                params2.cx = 160;
            } else {
                params2.cx = 80;
            }
            params2.cy = 80;
            if (negative < 0) {
                negative = -negative;
                negative += 2;
                alpha = 255 - (((float) negative / 20.0f) * 255.0f);
            } else {
                alpha = 255;
            }
            if (alpha <= 10) {
                continue;
            }
            rdpq_set_prim_color(RGBA32(255, 255, 255, alpha));
            rdpq_sprite_blit(gMenuSprites[i], display_get_width() / 2, display_get_height() / 2, &params2);
        }
    }
}

void number_render(int x, int y, int number, int zeros, float scale, int yOffset) {
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
        rdpq_texture_rectangle_scaled(TILE0, tX, tY, tX + offset, tY + offset, 0, 0, 24 * gScreenMul, 24 * gScreenMul);
        rdpq_sync_pipe();
        tX -= width;
        tY -= yOffset;
        divCount *= 10;
    }
}

void time_render(int x, int y, int time) {
    int minutes;
    int seconds;
    int offset;
    int size;
    surface_t tex;
    rdpq_texparms_t p = {0};
    if (gScreenMul == 1) {
        p.s.scale_log = -1;
        p.t.scale_log = -1;
    }
    p.s.mirror = 0;
    p.t.mirror = 0;
    p.s.repeats = 0;
    p.t.repeats = 0;

    int timeVal = gGameTimer + (60);

    minutes = timeVal / (60 * 60);
    seconds = (timeVal / 60) % 60;
    offset = (48 * 2) / gScreenDiv;
    size = (48) / gScreenDiv;
    tex = sprite_get_pixels(gNumberSprites);

    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_set_prim_color(RGBA32(16, 48, 32, gScoreTimerOpacity * 0.75f));
    rdpq_blitparms_t b = {0};

    b.cx = 82;
    b.cy = 32;
    float scale;
    scale = 1.0f / gScreenDiv;
    b.scale_x = scale;
    b.scale_y = scale;
    rdpq_sprite_blit(gNumberBGSprite, (display_get_width() / 2) + 2, 28, &b);
    rdpq_set_prim_color(RGBA32(255, 255, 255, gScoreTimerOpacity));
    number_render(x, y, minutes, 1, 1.0f, 0);
    surface_t t = surface_make_sub(&tex, 0, 48 * 10, 48, 48);
    rdpq_tex_upload(TILE0, &t, &p);
    rdpq_texture_rectangle(TILE0, x + size, y, x + offset + size, y + size, 0, 0);
    rdpq_sync_pipe();
    number_render(x + size + (size / 3), y, seconds, 2, 1.0f, 0);
}

void hud_render(int updateRate, float updateRateF) {
    if (gGameTimer) {
        if (gScoreTimerOpacity < 255) {
            gScoreTimerOpacity += updateRate * 32;
            if (gScoreTimerOpacity > 255) {
                gScoreTimerOpacity = 255;
            }
        }
        gScoreTimerCurOpacity = gScoreTimerOpacity;
    } else {
        if (gScoreTimerOpacity > 0) {
            gScoreTimerOpacity -= updateRate * 32;
            if (gScoreTimerOpacity < 0) {
                gScoreTimerOpacity = 0;
            }
        }
        gScoreTimerCurOpacity = gScoreTimerOpacity;
    }
    if (gMenuFlicker && gGameTimer < 60 * 10) {
        gScoreTimerCurOpacity = 0;
    }

    if (gGamePaused == false) {
        gScoreboardOffsetY = lerpf(gScoreboardOffsetY, 0, 0.1f * updateRateF);
    } else {
        gScoreboardOffsetY = lerpf(gScoreboardOffsetY, -144, 0.1f * updateRateF);
    }

    rdpq_set_mode_standard();
    time_render((display_get_width() / 2) - (36 * gScreenMul), 16 * gScreenMul, gGameTimer);

    int y0 = display_get_height() - ((48 - gScoreboardOffsetY) * gScreenMul);
    int y1 = y0 + (160 * gScreenMul);
    int x0 = 32 * gScreenMul;
    int x1 = display_get_width() - (32 * gScreenMul);
    int inlay = 8 * gScreenMul;
    int textX;
    int textY;

    rdpq_mode_blender(false);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_texparms_t t = {0};
    t.s.repeats = REPEAT_INFINITE;
    t.t.repeats = REPEAT_INFINITE;
    rdpq_sprite_upload(TILE0, gScoreBorderSprite, &t);
    // Top
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0, y0, 0.0f, 0.0f, 1.0f},
        (float[]){x0 + inlay, y0 + inlay, 0.0f, 16.0f, 1.0f},
        (float[]){x1 - inlay, y0 + inlay, 128.0f, 16.0f, 1.0f}
    );
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x1 - inlay, y0 + inlay, 128.0f, 16.0f, 1.0f},
        (float[]){x1, y0, 128.0f, 0.0f, 1.0f},
        (float[]){x0, y0, 0.0f, 0.0f, 1.0f}
    );
    // Left
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0, y0, 128.0f, 0.0f, 1.0f},
        (float[]){x0, y1, 0.0f,   0.0f, 1.0f},
        (float[]){x0 + inlay, y1 - inlay, 0.0f,  16.0f, 1.0f}
    );
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0, y0, 128.0f, 0.0f, 1.0f},
        (float[]){x0 + inlay, y1 - inlay, 0.0f,  16.0f, 1.0f},
        (float[]){x0 + inlay, y0 + inlay, 128.0f,16.0f, 1.0f}
    );
    // Right
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x1 - inlay, y0 + inlay, 128.0f, 128.0f, 1.0f},
        (float[]){x1, y0, 128.0f, 112.0f, 1.0f},
        (float[]){x1, y1, 0.0f, 112.0f, 1.0f}
    );
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x1 - inlay, y0 + inlay, 128.0f, 128.0f, 1.0f},
        (float[]){x1, y1, 0.0f, 112.0f, 1.0f},
        (float[]){x1 - inlay, y1 - inlay,   0.0f, 128.0f, 1.0f}
    );
    // Bottom
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0 + inlay, y1 - inlay, 128.0f, 16.0f, 1.0f},
        (float[]){x1 - inlay, y1 - inlay, 0.0f,   16.0f, 1.0f},
        (float[]){x1, y1, 0.0f,   0.0f,  1.0f}
    );
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0 + inlay, y1 - inlay, 128.0f, 16.0f, 1.0f},
        (float[]){x1, y1, 0.0f,   0.0f,  1.0f},
        (float[]){x0, y1, 128.0f, 0.0f,  1.0f}
    );
    t.s.mirror = true;
    t.t.mirror = true;
    rdpq_sprite_upload(TILE0, gScoreBoardSprite, &t);
    rdpq_mode_combiner(RDPQ_COMBINER1((TEX0, PRIM, PRIM, PRIM), (TEX0, 0, ENV, 0)));
    rdpq_set_prim_color(RGBA32(255, 240, 254, 255));
    // Middle
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x0 + inlay, y0 + inlay, 0.0f, 0.0f, 1.0f},
        (float[]){x0 + inlay, y1 - inlay, 0.0f, 128.0f, 1.0f},
        (float[]){x1 - inlay, y1 - inlay, 128.0f, 128.0f, 1.0f}
    );
    rdpq_triangle(&TRIFMT_TEX,
        (float[]){x1 - inlay, y1 - inlay, 128.0f, 128.0f, 1.0f},
        (float[]){x1 - inlay, y0 + inlay, 128.0f, 0.0f, 1.0f},
        (float[]){x0 + inlay, y0 + inlay, 0.0f, 0.0f, 1.0f}
    );
    rdpq_blitparms_t b = {0};
    textX = x0 + (10 * gScreenMul);
    textY = y0 + (24 * gScreenMul);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    color_t cols[] = {RGBA32(255, 127, 127, 255), RGBA32(127, 127, 255, 255), RGBA32(127, 255, 127, 255), RGBA32(255, 255, 127, 255)};
    color_t colsA[] = {RGBA32(255, 127, 127, 127), RGBA32(127, 127, 255, 127), RGBA32(127, 255, 127, 127), RGBA32(255, 255, 127, 127)};
    int leader[4] = {false};
    int tempLeader = -1;
    int pointRecord = 0;
    int numLeaders = 0;

    for (int i = 0; i < 4; i++) {
        if (gPoints[i] == 0) {
            continue;
        }
        if (gPoints[i] == pointRecord) {
            leader[numLeaders++] = i;
            pointRecord = gPoints[i];
        } else if (gPoints[i] > pointRecord) {
            tempLeader = i;
            pointRecord = gPoints[i];
            if (numLeaders > 0) {
                numLeaders = 0;
            }
        }
    }
    if (numLeaders == 0) {
        leader[0] = tempLeader;
        numLeaders = 1;
    }
    for (int i = 0; i < 4; i++) {
        if (gPointsVisual[i] < gPoints[i]) {
            gPointsVisual[i] += updateRate;
            if (gPointsVisual[i] > gPoints[i]) {
                gPointsVisual[i] = gPoints[i];
            }
        } else if (gPointsVisual[i] > gPoints[i]) {
            gPointsVisual[i] -= updateRate;
            if (gPointsVisual[i] < gPoints[i]) {
                gPointsVisual[i] = gPoints[i];
            }
        }
        if (gPointsVisual[i] > 999) {
            gPointsVisual[i] = 999;
        }
        b.scale_x = 0.58f / gScreenDiv;
        b.scale_y = 0.58f / gScreenDiv;
        rdpq_set_prim_color(colsA[i]);
        rdpq_sprite_blit(gScoreUnderlaySprite, textX + (4 * gScreenMul), textY - (12 * gScreenMul), &b);
        b.scale_x = 0.75f / gScreenDiv;
        b.scale_y = 0.75f / gScreenDiv;
        rdpq_set_prim_color(cols[i]);
        rdpq_sprite_blit(gScoreBoardPlayerSprites[gPlayerSprites[i]], textX, textY, &b);
        number_render(textX + (12 * gScreenMul), textY - (12 * gScreenMul), gPointsVisual[i], 1, 0.75f, 2);

        for (int j = 0; j < numLeaders; j++) {
            if (leader[j] == i && gMenuFlicker) {
                b.scale_x = 0.5f / gScreenDiv;
                b.scale_y = 0.5f / gScreenDiv;
                rdpq_set_prim_color(RGBA32(255, 255, 144, 255));
                rdpq_sprite_blit(gScoreLeaderSprite, textX, textY - (14 * gScreenMul), &b);
            }
        }

        textX += gScreenMul * 58;
    }

    if (gScoreboardOffsetY < -4) {
        if (gPauseSub == 0) {
            int prevOption = gMenuOption[0];
            if (gGamePaused) {
                menu_stick_options(PLAYER_ALL, NULL, &gMenuOption[0]);

                if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    input_clear(PLAYER_ALL, INPUT_A);
                    if (gMenuOption[0] == 0) {
                        gGamePaused = false;
                    } else {
                        gPauseSub = 1;
                        gMenuOption[1] = 1;
                    }
                } else if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    input_clear(PLAYER_ALL, INPUT_B);
                    gGamePaused = false;
                    gMenuOption[0] = 0;
                }
            }
            if (gMenuOption[0] < 0) {
                gMenuOption[0] = 0;
            } else if (gMenuOption[0] > 2) {
                gMenuOption[0] = 2;
            }
            if (prevOption != gMenuOption[0]) {
                sound_play_global(SOUND_MENU_CHALKSELECT);
            }
            int y = y0 + (76 * gScreenMul);
            for (int i = 0; i < 3; i++) {
                rdpq_blitparms_t b = {0};
                if (gMenuOption[0] == i) {
                    b.cx = 90;
                    b.cy = 60;
                    b.scale_x = 0.6f * gScreenMul;
                    b.scale_y = 0.25f * gScreenMul;
                    rdpq_set_prim_color(RGBA32(255, 255, 255, 127));
                    rdpq_sprite_blit(gScoreUnderlaySprite, display_get_width() / 2, y, &b);
                }

                b.cx = 160;
                b.cy = 80;
                b.scale_x = 0.4f * gScreenMul;
                b.scale_y = 0.4f * gScreenMul;

                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                rdpq_sprite_blit(gPauseOptionSprites[i], display_get_width() / 2, y, &b);

                y += (24 * gScreenMul);
            }
        } else if (gPauseSub) {
            int prevOption = gMenuOption[1];
            if (gGamePaused) {
                menu_stick_options(PLAYER_ALL, NULL, &gMenuOption[1]);
                if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
                    input_clear(PLAYER_ALL, INPUT_A);
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    if (gMenuOption[1] == 0) {
                        if (gMenuOption[0] == 1) {
                            gMenuOption[0] = 0;
                            gMenuOption[1] = 0;
                            game_init(gLevelID, gPlayerCount);
                            gMenuID = MENU_START_COUNTDOWN;
                            gSubMenu = 0;
                            gSubMenuOpt = 0;
                        } else {
                            gMenuID = MENU_TITLE;
                            gSubMenu = 0;
                            game_init(0, gPlayerCount);
                            gSubMenuOpt = 0;
                            gMenuOption[0] = 0;
                            gMenuOption[1] = 0;
                            for (int i = 0; i < 4; i++) {
                                gPlayerWins[i] = 0;
                            }
                            gNumTies = false;
                        }
                    } else {
                        gPauseSub = 0;
                    }
                }
                if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
                    input_clear(PLAYER_ALL, INPUT_B);
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    gPauseSub = 0;
                }
            }
            if (gMenuOption[1] < 0) {
                gMenuOption[1] = 0;
            } else if (gMenuOption[1] > 1) {
                gMenuOption[1] = 1;
            }
            if (prevOption != gMenuOption[1]) {
                sound_play_global(SOUND_MENU_CHALKSELECT);
            }
            int y = y0 + (88 * gScreenMul);
            for (int i = 0; i < 2; i++) {
                rdpq_blitparms_t b = {0};
                if (gMenuOption[1] == i) {
                    b.cx = 90;
                    b.cy = 60;
                    b.scale_x = 0.6f * gScreenMul;
                    b.scale_y = 0.25f * gScreenMul;
                    rdpq_set_prim_color(RGBA32(255, 255, 255, 127));
                    rdpq_sprite_blit(gScoreUnderlaySprite, display_get_width() / 2, y, &b);
                }

                b.cx = 160;
                b.cy = 80;
                b.scale_x = 0.4f * gScreenMul;
                b.scale_y = 0.4f * gScreenMul;

                rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
                rdpq_sprite_blit(gPauseOptionSprites[3 + i], display_get_width() / 2, y, &b);

                y += (24 * gScreenMul);
            }
        }
    }
}

void menu_game_finish(int updateRate, float updateRateF) {
    if (gSubMenu == 0) {
        gSubMenu = 1;
        gMenuSprites[0] = sprite_load("rom://roundfinish.rgba32.sprite");
        gTitleOptionScale[0] = 0.0f;
        gSubMenuOpt = 0;
        gPlayerWinner = -1;
        gMenuOption[1] = 0;
    } else if (gSubMenu == 1) {
        gTitleOptionScale[0] = lerpf(gTitleOptionScale[0], 1.0f, 0.2f * updateRateF);

        rdpq_blitparms_t b = {0};

        b.scale_x = gTitleOptionScale[0] / gScreenDiv;
        b.scale_y = gTitleOptionScale[0] / gScreenDiv;
        b.cx = 160;
        b.cy = 80;

        if (gTitleOptionScale[0] > 0.0f) {
            rdpq_set_mode_standard();
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_sprite_blit(gMenuSprites[0], display_get_width() / 2, display_get_height() / 2, &b);

            if (gTitleOptionScale[0] > 0.95f) {
                rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
                gSubMenuOpt += updateRate;
                if (gSubMenuOpt > 120) {
                    gSubMenuOpt = 120;
                }
                int alpha = ((float) ((float) gSubMenuOpt / 120.0f) * 255.0f);

                rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
                if (alpha != 0) {
                    rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
                }

                if (alpha == 255) {
                    gSubMenu = 2;
                    game_init(0, gPlayerCount);
                    gSubMenuOpt = 120;
                }
            }
        }
    } else if (gSubMenu == 2) {
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        gSubMenuOpt -= updateRate;
        if (gSubMenuOpt < 0) {
            gSubMenuOpt = 0;
        }
        int alpha = ((float) ((float) gSubMenuOpt / 120.0f) * 255.0f);
        rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
        if (alpha != 0) {
            rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
        }

        if (alpha == 0 && gTitleAttractTimer == 0) {
            gTitleAttractTimer = 1;
                int tempLeader = -1;
                int pointRecord = 0;
                int numLeaders = 0;

            for (int i = 0; i < 4; i++) {
                if (gPoints[i] == 0) {
                    continue;
                }
                if (gPoints[i] == pointRecord) {
                    pointRecord = gPoints[i];
                } else if (gPoints[i] > pointRecord) {
                    tempLeader = i;
                    pointRecord = gPoints[i];
                    if (numLeaders > 0) {
                        numLeaders = 0;
                    }
                }
            }
            if (numLeaders == 0) {
                gPlayerWins[tempLeader]++;
                gPlayerWinner = tempLeader;
            } else {
                gPlayerWinner = -1;
            }
        }

        if (gTitleAttractTimer > 0) {
            gTitleAttractTimer += updateRate;

            if (gTitleAttractTimer > 30) {
                gTitleAttractTimer = 30;

                if (gPlayerWinner != -1) {
                    gNumTies = false;
                    sound_play_global(SOUND_VOICE_WIN_1 + gPlayerWinner);
                } else {
                    sound_play_global(SOUND_VOICE_TIE + gNumTies);
                    gNumTies = true;
                }
            }

            int alpha = ((float) gTitleAttractTimer / 30.0f) * 255;

            if (gTitleAttractTimer == 30) {
                if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
                    input_clear(PLAYER_ALL, INPUT_A);
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    gSubMenu = 3;
                }
            }
        }
    } else if (gSubMenu == 3) {
        int prevOpt = gMenuOption[1];
        menu_stick_options(PLAYER_ALL, NULL, &gMenuOption[1]);
        if (gMenuOption[1] < 0) {
            gMenuOption[1] = 0;
        } else if (gMenuOption[1] > 1) {
            gMenuOption[1] = 1;
        }
        if (prevOpt != gMenuOption[1]) {
            sound_play_global(SOUND_MENU_CHALKSELECT);
        }
        if (input_pressed(PLAYER_ALL, INPUT_A, 2)) {
            input_clear(PLAYER_ALL, INPUT_A);
            if (gMenuOption[1] == 0) {
                game_init(gLevelID, 0);
                gMenuID = MENU_TITLE;
                gSubMenu = TITLE_SUB_LEVEL_SELECT;
                gSubMenuOpt = 1;
                gMenuOption[0] = 0;
                gMenuOption[1] = 0;
            } else {
                game_init(0, gPlayerCount);
                gSubMenuOpt = 0;
                gMenuOption[0] = 0;
                gMenuOption[1] = 0;
                gMenuID = MENU_TITLE;
                for (int i = 0; i < 4; i++) {
                    gPlayerWins[i] = 0;
                }
                gNumTies = false;
                gSubMenu = TITLE_SUB_INIT;
            }
            return;
        }
        if (input_pressed(PLAYER_ALL, INPUT_B, 2)) {
            input_clear(PLAYER_ALL, INPUT_B);
            sound_play_global(SOUND_MENU_CHALKSELECT);
            gSubMenu = 2;
        }
    }
}

#define LOGO_TIMER 60

void menu_logos(int updateRate, float updateRateF) {
    if (gSubMenu == 0) {
        gTitleAttractTimer = 0;
        gSubMenu = 1;
    }

    if (gSubMenu == 1) {
        gTitleAttractTimer += updateRate;
        if (gTitleAttractTimer > LOGO_TIMER) {
            game_init(0, 0);
            gSubMenu = 0;
            gTitleAttractTimer = 0;
            gMenuID = MENU_TITLE;
        }
        rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));
        rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
        rdpq_text_print(NULL, 1, 32, 64, "Fazana :))");
    }
}

void menu_render(int updateRate, float updateRateF) {
    gMenuFlickerTimer += updateRate;
    if (gMenuFlickerTimer > 30) {
        gMenuFlickerTimer = 0;
        gMenuFlicker ^= 1;
    }
	rdpq_sync_pipe();

    if (gPrevMenu != gMenuID) {
        rspq_wait();
        gPrevMenu = gMenuID;
        for (int i = 0; i < 16; i++) {
            if (gMenuSprites[i]) {
                sprite_free(gMenuSprites[i]);
                gMenuSprites[i] = NULL;
            }
        }
    }

    switch(gMenuID) {
        default:
            if (gGameTimer > 0) {
                if (input_pressed(PLAYER_ALL, INPUT_START, 2)) {
                    input_clear(PLAYER_ALL, INPUT_START);
                    sound_play_global(SOUND_MENU_CHALKBACK);
                    gGamePaused ^= 1;
                }
            }
            hud_render(updateRate, updateRateF);
            break;
        case MENU_TITLE:
            menu_render_title(updateRate, updateRateF);
            break;
        case MENU_START_COUNTDOWN:
            menu_start_countdown(updateRate, updateRateF);
            break;
        case MENU_FINISH:
            menu_game_finish(updateRate, updateRateF);
            break;
        case MENU_LOGOS:
            menu_logos(updateRate, updateRateF);
            break;
    }

    //hud_render(updateRate, updateRateF);
}