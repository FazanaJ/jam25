#pragma once

extern int gScreenMul;
extern int gScreenDiv;

enum MenuIDs {
    MENU_NONE,
    MENU_TITLE,
    MENU_START_COUNTDOWN,
    MENU_FINISH,
};

void menu_render(int updateRate, float updateRateF);
