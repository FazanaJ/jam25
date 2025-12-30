#pragma once

extern int gScreenMul;
extern int gScreenDiv;
extern int gMenuOption[4];
extern int gSubMenu;

enum MenuIDs {
    MENU_NONE,
    MENU_TITLE,
    MENU_START_COUNTDOWN,
    MENU_FINISH,
    MENU_LOGOS,
};

void menu_render(int updateRate, float updateRateF);
