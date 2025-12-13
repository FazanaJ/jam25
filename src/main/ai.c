#include <libdragon.h>
#include "input.h"

#include "ai.h"
#include "main.h"

int gAICursorTarget[4][2];
int gAIUpdateTimes[4];

void ai_input_stick(int playerID, int stickX, int stickY) {
    gInputData[playerID].stick[STICK_LEFT][0] = stickX;
    gInputData[playerID].stick[STICK_LEFT][1] = stickY;
}

void ai_input_button(int playerID, int button) {
    gInputData[playerID].button[0][button] = 0;
}

void ai_run(int player) {
    if (gAIUpdateTimes[player] > 0) {
        gAIUpdateTimes[player]--;
        return;
    }
}
