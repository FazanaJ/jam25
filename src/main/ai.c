#include <libdragon.h>
#include "input.h"

#include "ai.h"
#include "main.h"

float gAICursorTarget[4][2];
int gAIUpdateTimes[4];
int gAIStickVel[4];
int gAIDifficulty[4];
int gAIStickDir[4][2];

void ai_input_stick_x(int playerID, int stickX) {
    gInputData[playerID].stick[STICK_LEFT][0] = stickX;
}

void ai_input_stick_y(int playerID, int stickY) {
    gInputData[playerID].stick[STICK_LEFT][1] = -stickY;
}

void ai_input_button(int playerID, int button) {
    gInputData[playerID].button[INPUT_PRESSED][button] = 0;
}

void ai_pick_spot(int playerID) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 12; j++) {
            
        }
    }
}

void ai_move(int player, int updateRate) {
    // Cursor target is set, so move towards it.
    if (gAICursorTarget[player][0] != gPlayerCursors[player][0] || gAICursorTarget[player][1] != gPlayerCursors[player][1]) {
        if (gAIStickVel[player] < 64) {
            if (gAIDifficulty[player] == AIDIFF_EASY) {
                gAIStickVel[player] += 4 * updateRate;
            } else if (gAIDifficulty[player] == AIDIFF_MEDIUM) {
                gAIStickVel[player] += 8 * updateRate;
            } else {
                gAIStickVel[player] = 64;
            }
            if (gAIStickVel[player] > 64) {
                gAIStickVel[player] = 64;
            }
        }

        if (gAICursorTarget[player][0] != gPlayerCursors[player][0]) {
            if (gAICursorTarget[player][0] < gPlayerCursors[player][0]) {
                gAIStickDir[player][0] = DIR_LEFT;
                ai_input_stick_x(player, -gAIStickVel[player]);
            } else {
                gAIStickDir[player][0] = DIR_RIGHT;
                ai_input_stick_x(player, gAIStickVel[player]);
            }
        }
        if (gAICursorTarget[player][1] != gPlayerCursors[player][2]) {
            if (gAICursorTarget[player][1] < gPlayerCursors[player][2]) {
                gAIStickDir[player][1] = DIR_DOWN;
                ai_input_stick_y(player, gAIStickVel[player]);
            } else {
                gAIStickDir[player][1] = DIR_UP;
                ai_input_stick_y(player, -gAIStickVel[player]);
            }
        }
    } else {
        if (gAIStickVel[player] > 0) {
            if (gAIDifficulty[player] == AIDIFF_EASY) {
                gAIStickVel[player] -= 4 * updateRate;
            } else if (gAIDifficulty[player] == AIDIFF_MEDIUM) {
                gAIStickVel[player] -= 8 * updateRate;
            } else {
                gAIStickVel[player] = 0;
            }
            if (gAIStickVel[player] < 0) {
                gAIStickVel[player] = 0;
            }
        }
    }
}

void ai_run(int player, int updateRate) {
    ai_move(player, updateRate);

    if (gAIUpdateTimes[player] > 0) {
        gAIUpdateTimes[player] -= updateRate;
        return;
    }

    gAIUpdateTimes[player] = 120;
}
