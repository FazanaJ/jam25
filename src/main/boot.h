#pragma once

typedef struct ConfigBits {
    int8_t res : 1;
    int8_t language : 4;
    int8_t region : 2;
    int8_t sound : 2;

    
    int8_t magic : 7;
} ConfigBits;

typedef struct Config {
    int8_t res;
    int8_t language;
    int8_t region;
    int8_t sound;
} Config;

void boot(void);