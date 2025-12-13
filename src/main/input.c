#include <libdragon.h>
#include "input.h"

#include "main.h"

//#define MULTITH

joypad_buttons_t gRawInputBufs[4][4][2];
Input gInputData[5];

int16_t gRumbleTimer;

int8_t gCurrentController[4];
uint8_t gDeadzone;
uint8_t gNumPads;
uint8_t gRawInputIter;
uint8_t gStickRange;
int8_t sPakDetectionTimer;

/**
 * Activates a controller and assigns the inputs to that ID.
 */
void input_set_id(int padID) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (gCurrentController[j] == i) {
                goto skip;
            }
        }
        joypad_inputs_t pad = joypad_get_inputs(i);
        if (joypad_is_connected(i) && pad.btn.raw) {
            gCurrentController[padID] = i;
            return;
        }
        skip:
    }
}

/**
 * Clears all inputs from the controller.
 */
void input_reset(int padID) {
    bzero(&gInputData[padID], sizeof(Input));
    for (int i = 0; i < INPUT_TOTAL; i++) {
        gInputData[padID].button[INPUT_PRESSED][i] = 250;
        gInputData[padID].button[INPUT_HELD][i] = 0;
    }
}

/**
 * Takes input from the dedicated thread.
 * Since the thread runs much more often than the main thread, 
 * it considerably improves responsiveness to input.
 * The sticks are still polled here though, the extra precision isn't necessary.
 */
void input_update(void) {
    int iter;
    int range;
    int deadzone;

    range = gStickRange;
    deadzone = gDeadzone;

    #ifndef MULTITH
    joypad_poll();
    #endif
    input_reset(PLAYER_ALL);
    for (int k = 0; k < 4; k++) {
        int p = gCurrentController[k];
        if (p == CONTROLLER_OFF) {
            input_set_id(PLAYER_1);
            input_reset(k);
            continue;
        }

        sPakDetectionTimer--;
        if (sPakDetectionTimer <= 0) {
            gInputData[k].pak = joypad_get_accessory_type(p);
            sPakDetectionTimer = 30;
        }

        if (gRumbleTimer > 0) {
            gRumbleTimer--;
            if (gRumbleTimer == 0) {
                joypad_set_rumble_active(p, false);
            }
        }

        Input *controller = &gInputData[k];
        kthread_lock();
        // Updates input data
        #ifdef MULTITH
        iter = gRawInputIter - 1;
        if (iter < 0) {
            iter += 4;
        }
        unsigned int btn[2] = {0};
        for (int j = 0; j < iter; j++) {
            btn[INPUT_PRESSED] |= gRawInputBufs[j][p][INPUT_PRESSED].raw;
            btn[INPUT_HELD] |= gRawInputBufs[j][p][INPUT_HELD].raw;
        }
        #else
        
        unsigned int btn[2] = {0};
        btn[0] = joypad_get_buttons_pressed(p).raw;
        btn[1] = joypad_get_buttons_held(p).raw;
        #endif
        gRawInputIter = 0;
        kthread_unlock();
        uint8_t pressed[INPUT_TOTAL] = {0};
        uint8_t held[INPUT_TOTAL] = {0};
        for (int j = 0; j < INPUT_TOTAL; j++) {
            if (btn[INPUT_PRESSED] & (1 << j)) {
                controller->button[INPUT_PRESSED][j] = 0;
                gInputData[PLAYER_ALL].button[INPUT_PRESSED][j] = 0;
                pressed[j] = true;
            } else {
                if (controller->button[INPUT_PRESSED][j] < 255) {
                    controller->button[INPUT_PRESSED][j]++;
                    if (pressed[j] == false) {
                        gInputData[PLAYER_ALL].button[INPUT_PRESSED][j]++;
                        pressed[j] = true;
                    }
                }
            }
        }
        for (int j = 0; j < INPUT_TOTAL; j++) {
            if (btn[INPUT_HELD] & (1 << j)) {
                if (controller->button[INPUT_HELD][j] < 255) {
                    controller->button[INPUT_HELD][j]++;
                    if (held[j] == false) {
                        gInputData[PLAYER_ALL].button[INPUT_HELD][j]++;
                        held[j] = true;
                    }
                }
            } else {
                controller->button[INPUT_HELD][j] = 0;
                gInputData[PLAYER_ALL].button[INPUT_HELD][j] = 0;
                held[j] = true;
            }
        }
        joypad_inputs_t stick = joypad_get_inputs(p);
        // This is a little bit wordy but it's worth it.
        // Take the raw stick inputs.
        // The deadzone is 10 units, so zero out anything less.
        // Take the deadzone away from the stick reading. This eliminates the sudden increase in the curve.
        // Cap the reading at range, which is around three quarters of the way, then normalise the stick mag from 0-1.
        // The end result is a smooth increase from neutral to furthest, with plenty of leeway for degraded sticks.
        int stickRaw[2][2] = {
            {stick.stick_x, -stick.stick_y}, 
            {stick.cstick_x, -stick.cstick_y}
        };
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                controller->stick[j][k] = stickRaw[j][k];
                if (fabs(controller->stick[j][k]) < deadzone) {
                    controller->stick[j][k] = 0;
                } else {
                    if (controller->stick[j][k] > 0) {
                        controller->stick[j][k] -= deadzone;
                        if (controller->stick[j][k] > range) {
                            controller->stick[j][k] = range;
                        }
                    } else {
                        controller->stick[j][k] += deadzone;
                        if (controller->stick[j][k] < -range) {
                            controller->stick[j][k] = -range;
                        }
                    }
                }
            }
            controller->stickMag[j] = fabsf(sqrtf((controller->stick[j][STICK_X] * controller->stick[j][STICK_X]) + (controller->stick[j][STICK_Y] * controller->stick[j][STICK_Y]))) / (float) STICK_RANGE;
            if (controller->stickMag[j] != 0.0f) {
                //controller->stickAngle[j] = atan2s(controller->stick[j][STICK_Y], controller->stick[j][STICK_X]);
            }
        }
        controller->type = CONTROLLER_N64;
    }
}

#ifdef MULTITH
int thread_input(void *param) {
    gRawInputIter = 0;

    while (1) {
        uint32_t firstTime = timer_ticks();
        int iter = gRawInputIter;

        if (iter < 4) {
            joypad_poll();

            for (int i = 0; i < 4; i++) {
                gRawInputBufs[iter][i][INPUT_PRESSED] = joypad_get_buttons_pressed(i);
                gRawInputBufs[iter][i][INPUT_HELD] = joypad_get_buttons_held(i);
            }

            gRawInputIter++;
            if (gRawInputIter >= 4) {
                gRawInputIter = 0;
            }
        }
        thread_tickrate(gInputThreadTicks, firstTime);
    }
}
#endif

/**
 * Returns true if this button has been pressed in the last number of frames.
 */
int input_pressed(int padID, int input, int numFrames) {
    return gInputData[padID].button[INPUT_PRESSED][input] <= numFrames;
}

/**
 * Returns true if the button has been held for at least this number of frames.
 * Zero means just if it's held.
 */
int input_held(int padID, int input, int numFrames) {
    return gInputData[padID].button[INPUT_HELD][input] > numFrames;
}

/**
 * Returns true if the button hasn't been pressed in the last number of frames.
 */
int input_released(int padID, int input, int numFrames) {
    return gInputData[padID].button[INPUT_PRESSED][input] >= numFrames;
}

/**
 * Returns stick X range. 0-64
 */
int input_stick_x(int padID, int type) {
    return gInputData[padID].stick[type][STICK_X];
}

/**
 * Returns stick Y range. o-64
 */
int input_stick_y(int padID, int type) {
    return gInputData[padID].stick[type][STICK_Y];
}

/**
 * Returns the stick direction.
 */
short input_stick_angle(int padID, int type) {
    return gInputData[padID].stickAngle[type];
}

/**
 * Returns a normalised (0-1) stick range.
 */
float input_stick_mag(int padID, int type) {
    return gInputData[padID].stickMag[type];
}

/**
 * Returns whetherthe controller belongs to the N64 or the Gamecube
 */
int input_type(int padID) {
    return gInputData[padID].type;
}

/**
 * Enables rumble on that controller. Timer is in frames, so 20 is 1 second.
 */
void input_rumble(int padID, int timer) {
    if (gInputData[padID].pak != JOYPAD_ACCESSORY_TYPE_RUMBLE_PAK) {
        return;
    }

    joypad_set_rumble_active(gCurrentController[padID], true);
    gRumbleTimer = timer;
}

/**
 * Reset the button to be unpressed.
 * Do this after responding to an input to prevent it from unintentionally triggering
 * other things.
 * Do not use for held buttons, unless you want a steady delay to a held input like you're
 * designing a schmup.
*/
void input_clear(int padID, int input) {
    gInputData[padID].button[INPUT_PRESSED][input] = 250;
    gInputData[padID].button[INPUT_HELD][input] = 0;
}
