#include <libdragon.h>
#include "input.h"

#include "ai.h"
#include "main.h"

float gAICursorTarget[4][2];
int gAIUpdateTimes[4];
int gAIStickVel[4];
int gAIPlaced[4];
int gAIDifficulty[4];
int gAIStickDir[4][2];
float gHeatmap[2];
int gActiveTroopCount;

void ai_input_stick_x(int playerID, int stickX) {
    gInputData[playerID].stick[STICK_LEFT][0] = stickX;
}

void ai_input_stick_y(int playerID, int stickY) {
    gInputData[playerID].stick[STICK_LEFT][1] = -stickY;
}

void ai_input_button(int playerID, int button) {
    gInputData[playerID].button[INPUT_PRESSED][button] = 0;
}

int ai_pick_spot(int playerID) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 12; j++) {
            
        }
    }
    if (gAIPlaced[playerID] == false) {
        return false;
    }
    while (true) {
        int pX;
        int pZ;
        int radiusX;
        int radiusZ;
        //if (gActiveTroopCount < 15) {
            pX = 6;
            pZ = 5;
            radiusX = 12;
            radiusZ = 10;
        /*} else if (gActiveTroopCount < 30) {
            pX = gHeatmap[0];
            pZ = gHeatmap[1];
            radiusX = 6;
            radiusZ = 5;
        } else {
            pX = gHeatmap[0];
            pZ = gHeatmap[1];
            radiusX = 2;
            radiusZ = 2;
        }*/

        /*if (gActiveTroopCount >= 20) {
            if (gAIDifficulty[playerID] == AIDIFF_MEDIUM) {
                radiusX *= 0.8f;
                radiusZ *= 0.8f;
            } else if (gAIDifficulty[playerID] == AIDIFF_HARD) {
                radiusX /= 2;
                radiusZ /= 2;
            }
        }*/
        radiusX = CLAMP(radiusX, 2, 12);
        radiusZ = CLAMP(radiusZ, 2, 10);
        gAICursorTarget[playerID][0] = (pX - (radiusX / 2)) + (rand() % radiusX);
        gAICursorTarget[playerID][1] = (pZ - (radiusZ / 2)) + (rand() % radiusZ);
        gAICursorTarget[playerID][0] = CLAMP(gAICursorTarget[playerID][0], 0, 11);
        gAICursorTarget[playerID][1] = CLAMP(gAICursorTarget[playerID][1], 0, 9);

        int id = ((int) gAICursorTarget[playerID][1] * 12) + (int) gAICursorTarget[playerID][0];

        if (gCurrentLevel->objects[id] != 0) {
            continue;
        }

        
        int distX = (gBasePos[playerID].x - gAICursorTarget[playerID][0]);
        int distZ = (gBasePos[playerID].z - gAICursorTarget[playerID][1]);
        int dir;

        if (fabs(distX) > fabs(distZ)) {
            if (distX > 0) {
                dir = DIR_RIGHT;
            } else {
                dir = DIR_LEFT;
            }
        } else {
            if (distZ > 0) {
                dir = DIR_DOWN;
            } else {
                dir = DIR_UP;
            }
        }

        if (gMapArrows[id].dir == dir) {
            continue;
        }

        int basePos[2];
        basePos[0] = gBasePos[playerID].x;
        basePos[1] = gBasePos[playerID].z;
        gAIPlaced[playerID] = false;
        if (gAIDifficulty[playerID] == AIDIFF_EASY) {
            return false;
        } else {
            return false;
        }
    }
}

void ai_move(int player, int updateRate) {
    // Cursor target is set, so move towards it.
    if (gAICursorTarget[player][0] != gPlayerCursors[player][0] || gAICursorTarget[player][1] != gPlayerCursors[player][2]) {
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

        if (gAICursorTarget[player][0] != (int) gPlayerCursors[player][0]) {
            if (gAICursorTarget[player][0] < gPlayerCursors[player][0]) {
                gAIStickDir[player][0] = DIR_LEFT;
                ai_input_stick_x(player, -gAIStickVel[player]);
            } else {
                gAIStickDir[player][0] = DIR_RIGHT;
                ai_input_stick_x(player, gAIStickVel[player]);
            }
        }
        if (gAICursorTarget[player][1] != (int) gPlayerCursors[player][2]) {
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

int ai_check_obstruction(int player, int posX, int posZ, int *dir, int *button, int h, int v) {
    // Check left arrows are not identical
    int *p;
    int bounds;
    int acc;
    switch (*dir) {
        case DIR_LEFT:
        case DIR_RIGHT:
        default:
            p = &posX;
            bounds = 12;
            acc = h;
            break;
        case DIR_UP:
        case DIR_DOWN:
            p = &posZ;
            bounds = 10;
            acc = v;
            break;
    }
    int arrowFound = false;
    int placeArrow = false;
    *p += acc;
    while(true) {
        if (*p > 0 && *p < bounds) {
            int id = (posZ * 12) + posX;
            if (gArrowCount > 60) {
                if ((rand() % 100) < 50) {
                    placeArrow = true;
                    break;
                }
            }
            // Break early because an object was found. Means no arrows are there.
            if (gCurrentLevel->objects[id] != 0 && gCurrentLevel->objects[id] < 5) {
                if (gCurrentLevel->objects[id] == player + 1) {
                    placeArrow = true;
                } else {
                    placeArrow = false;
                }
                break;
            }
            // Break early because there's a wall in the way
            /*if (gCurrentLevel->wallTiles[id] != 0 && gCurrentLevel->wallTiles[id] > 2) {
                placeArrow = true;
                break;
            }*/
            // Break early because there's a hole in the floor.
            if (gCurrentLevel->floorTiles[id] == 0) {
                placeArrow = true;
                break;
            }
            // Identical arrow found, break and skip placing arrow.
            if (gMapArrows[id].dir == *dir) {
                arrowFound = true;
                placeArrow = false;
                break;
            }
            // Opposite arrow found, change direction of arrow placement.
            if (gMapArrows[id].dir != DIR_NONE) {
                int dirDiff = fabs(gMapArrows[id].dir - *dir);
                if (dirDiff == 2) {
                    arrowFound = true;
                    placeArrow = true;
                    if (*dir == DIR_LEFT || DIR_RIGHT) {
                        if (v == 1) {
                            *dir = DIR_UP;
                            *button = INPUT_CUP;
                        } else {
                            *dir = DIR_DOWN;
                            *button = INPUT_CDOWN;
                        }
                    } else {
                        if (h == 1) {
                            *dir = DIR_RIGHT;
                            *button = INPUT_CRIGHT;
                        } else {
                            *dir = DIR_LEFT;
                            *button = INPUT_CLEFT;
                        }
                    }
                    break;
                }
            }
        } else {
            placeArrow = true;
            break;
        }
        *p += acc;
    }
    return placeArrow;
}

int ai_place_arrow(int player) {
    if (gAIPlaced[player] == false) {
        if (gAICursorTarget[player][0] == gPlayerCursors[player][0] && 
            gAICursorTarget[player][1] == gPlayerCursors[player][2]) {
            int distX = (gBasePos[player].x - gAICursorTarget[player][0]);
            int distZ = (gBasePos[player].z - gAICursorTarget[player][1]);
            int button;
            int vert;
            int hor;
            int dir;

            if (fabs(distX) > fabs(distZ)) {
                if (distX > 0) {
                    hor = 1;
                    dir = DIR_RIGHT;
                    button = INPUT_CRIGHT;
                } else {
                    hor = -1;
                    dir = DIR_LEFT;
                    button = INPUT_CLEFT;
                }
                if (distZ > 0) {
                    vert = 1;
                } else {
                    vert = -1;
                }
            } else {
                if (distZ > 0) {
                    vert = 1;
                    dir = DIR_DOWN;
                    button = INPUT_CDOWN;
                } else {
                    vert = -1;
                    dir = DIR_UP;
                    button = INPUT_CUP;
                }
                if (distX > 0) {
                    hor = 1;
                } else {
                    hor = -1;
                }
            }
            int posX = gAICursorTarget[player][0];
            int posZ = gAICursorTarget[player][1];
            int id;
            int arrowFound = false;
            int placeArrow = false;
            int oldButton = button;
            placeArrow = ai_check_obstruction(player, posX, posZ, &dir, &button, hor, vert);
            if (placeArrow == false) {
                placeArrow = ai_check_obstruction(player, posX, posZ, &dir, &button, hor, vert);
            }
            if (placeArrow) {
                ai_input_button(player, button);
            }
            gAIPlaced[player] = true;
            return true;
        }
    }
    return false;
}

void ai_run(int player, int updateRate) {
    ai_move(player, updateRate);

    if (gAIUpdateTimes[player] > 0) {
        gAIUpdateTimes[player] -= updateRate;
        return;
    }

    int per;
    if (gAIDifficulty[player] == AIDIFF_EASY) {
        per = 105;
    } else if (gAIDifficulty[player] == AIDIFF_MEDIUM) {
        per = 75;
    } else {
        per = 45;
    }

    gAIUpdateTimes[player] = (rand() % per) + 15;

    if (ai_place_arrow(player)) {
        return;
    }
    if (ai_pick_spot(player)) {
        return;
    }
}
