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

enum TitleSubmenus {
    TITLE_SUB_INIT,
    TITLE_SUB_LOGO_GROW,
    TITLE_SUB_LOGO_SHRINK,
    TITLE_SUB_LOGO_IDLE,
    TITLE_SUB_MOVE_INSIDE,
    TITLE_SUB_MAIN_MENU,
    TITLE_SUB_MOVE_OPT,
    TITLE_SUB_QUICK_PLAY,
    TITLE_SUB_CHALLENGES,
    TITLE_SUB_TUTORIALS,
    TITLE_SUB_CONFIG,
    TITLE_SUB_CREDITS,
    TITLE_SUB_LEVEL_SELECT,
};

extern int gPlayerWinner;
extern int gNumTies;

void menu_render(int updateRate, float updateRateF);
