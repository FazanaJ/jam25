#pragma once

#define DEADZONE 10.0f
#define STICK_RANGE 64
#define CONTROLLER_OFF -1

enum InputButtons {
    INPUT_CRIGHT,
    INPUT_CLEFT,
    INPUT_CDOWN,
    INPUT_CUP,
    INPUT_R,
    INPUT_L,
    INPUT_X,
    INPUT_Y,
    INPUT_DRIGHT,
    INPUT_DLEFT,
    INPUT_DDOWN,
    INPUT_DUP,
    INPUT_START,
    INPUT_Z,
    INPUT_B,
    INPUT_A,

    INPUT_TOTAL,

};

enum InputType {
    INPUT_PRESSED,
    INPUT_HELD
};

enum ControllerType {
    CONTROLLER_N64,
    CONTROLLER_GAMECUBE
};

enum StickType {
    STICK_LEFT,
    STICK_RIGHT
};

enum StickDir {
    STICK_X,
    STICK_Y
};

typedef struct Input {
    float stickMag[2];
    unsigned char button[2][INPUT_TOTAL];
    char stick[2][2];
    char pak;
    char type;
    short stickAngle[2];
} Input;

extern int8_t gCurrentController[4];
extern uint8_t gNumPads;
extern Input gInputData[5];
extern uint8_t gDeadzone;
extern uint8_t gStickRange;

void input_update(void);
int input_pressed(int padID, int input, int numFrames);
int input_held(int padID, int input, int numFrames);
int input_released(int padID, int input, int numFrames);
int input_stick_x(int padID, int type);
int input_stick_y(int padID, int type);
short input_stick_angle(int padID, int type);
float input_stick_mag(int padID, int type);
int input_type(int padID);
void input_rumble(int padID, int timer);
void input_clear(int padID, int input);
int thread_input(void *param);
void input_init(void);