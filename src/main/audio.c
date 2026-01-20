#include <libdragon.h>
#include <malloc.h>

#include "audio.h"
#include "main.h"
#include "menu.h"

char gSoundChannelNum = 0;
char gSoundPrioTable[32];
static short sNextSequenceID = -1;
static short sCurrentSequenceID = -1;
static short sSequenceFadeTimerSet;
static short sSequenceFadeTimer;
static int sChannelMask;
float gChannelVol[CHANNEL_MAX_NUM];
float gMusicVolume;
float gSoundVolume;
static xm64player_t sXMPlayer;

SoundData sSoundTable[SOUND_COUNT] = {
    {"rom://menuselect.1.wav64", 10},
    {"rom://logostay.1.wav64", 10},
    {"rom://menuback.1.wav64", 10},

    {"rom://count3.1.wav64", 10},
    {"rom://count2.1.wav64", 10},
    {"rom://count1.1.wav64", 10},
    {"rom://countgo.1.wav64", 10},

    {"rom://keyboard.1.wav64", 10},

    {"rom://timer60.1.wav64", 10},
    {"rom://timer30.1.wav64", 10},
    {"rom://timer10.1.wav64", 10},
    {"rom://timer5.1.wav64", 10},
    {"rom://timer4.1.wav64", 10},
    {"rom://timer3.1.wav64", 10},
    {"rom://timer2.1.wav64", 10},
    {"rom://timer1.1.wav64", 10},
    {"rom://timerfinish.1.wav64", 10},

    {"rom://win1.1.wav64", 10},
    {"rom://win2.1.wav64", 10},
    {"rom://win3.1.wav64", 10},
    {"rom://win4.1.wav64", 10},

    {"rom://tiebreaker.1.wav64", 10},
    {"rom://tiebreaker2.1.wav64", 10},

    {"rom://power1.1.wav64", 10},
    {"rom://power2.1.wav64", 10},
    {"rom://power3.1.wav64", 10},
    {"rom://power4.1.wav64", 10},
    {"rom://power5.1.wav64", 10},
    {"rom://power6.1.wav64", 10},
    {"rom://power7.1.wav64", 10},

    {"rom://perfect1.1.wav64", 10},
    {"rom://perfect2.1.wav64", 10},

    {"rom://leader1.1.wav64", 10},
    {"rom://leader2.1.wav64", 10},
    {"rom://leader3.1.wav64", 10},
    {"rom://leader4.1.wav64", 10},

    {"rom://arrow1.1.wav64", 10},
    {"rom://arrow2.1.wav64", 10},
    {"rom://arrow3.1.wav64", 10},
    {"rom://arrow4.1.wav64", 10},
    
    {"rom://onepoint.1.wav64", 10},
    {"rom://explosion.1.wav64", 10},
    {"rom://manypoints.1.wav64", 10},
    {"rom://roulette.1.wav64", 10},
    {"rom://keyboard.1.wav64", 10},

    {"rom://whoosh.1.wav64", 10},
    {"rom://chalkselect.1.wav64", 10},
    {"rom://chalkback.1.wav64", 10},
    {"rom://logoappear.1.wav64", 10},
    {"rom://logostay.1.wav64", 10},

    {"rom://connect.1.wav64", 10},
    {"rom://disconnect.1.wav64", 10},
};

SequenceData sSequenceTable[] = {
    {"rom://kamel.xm64", 12},
    {"rom://racer.xm64", 3},
};

void audio_boot(void) {
    audio_init(AUDIO_FREQUENCY, MIXER_BUFFER_SIZE);
    wav64_init_compression(3);
    mixer_init(32);
    gSoundChannelNum = 32;
    gMusicVolume = 0.4f;
    bzero(&gSoundPrioTable, sizeof(gSoundPrioTable));

    for (int i = 0; i < SOUND_COUNT; i++) {
        wav64_open(&sSoundTable[i].sound, sSoundTable[i].path);
    }
    music_play(0, 0);
    for (int i = 0; i < CHANNEL_MAX_NUM; i++) {
        gChannelVol[i] = 1.0f;
        mixer_ch_set_limits(i, 16, AUDIO_FREQUENCY, 0);
    }
}

void music_play(int seqID, int fadeTime) {
    sNextSequenceID = seqID;
    sSequenceFadeTimer = fadeTime;
}

void sound_play_global(int soundID) {
    wav64_play(&sSoundTable[soundID].sound, CHANNEL_GLOBAL);
}

static int find_sound_channel(int priority) {
    static char pris[CHANNEL_MAX_NUM] = { 0 };

    // Find a free channel
    int lowest = 0;
    int ch = -1;
    for (int i = 0; i < CHANNEL_MAX_NUM; i++) {
        if (!mixer_ch_playing(i + CHANNEL_DYNAMIC)) {
            ch = i;
            break;
        }
        if (pris[i] < pris[lowest]) {
            lowest = i;
        }
    }

    // If no free channel, use the lowest priority channel
    if (ch == -1) {
        ch = lowest;
    }

    // Remember the current priority
    pris[ch] = priority;
    
    // Return the channel ID
    return ch + CHANNEL_DYNAMIC;
}

void sound_play(int soundID) {
    wav64_play(&sSoundTable[soundID].sound, find_sound_channel(10));
}

void sound_play_channel(int soundID, int channel) {
    if (gMenuID == MENU_TITLE) {
        return;
    }
    wav64_play(&sSoundTable[soundID].sound, channel);
}

void set_sound_channel_count(int channelCount) {
    mixer_close();
    mixer_init(channelCount);
    gSoundChannelNum = channelCount;
}

void set_music_volume(float volume) {
    xm64player_set_vol(&sXMPlayer, volume);
}

void sound_channel_off(int channel) {
    sChannelMask |= (1 << channel);
}

void sound_channel_on(int channel) {
    sChannelMask &= ~(1 << channel);
}

static void update_sequence(int updateRate) {
    if (sNextSequenceID != sCurrentSequenceID) {
        sSequenceFadeTimer -= updateRate;
        if (sSequenceFadeTimer < 0) {
            if (sXMPlayer.fd) {
                xm64player_close(&sXMPlayer);
            }
            if (sNextSequenceID != -1) {
                SequenceData *s = &sSequenceTable[sNextSequenceID];
                xm64player_open(&sXMPlayer, s->seqPath);
                xm64player_set_vol(&sXMPlayer, gMusicVolume);
                xm64player_play(&sXMPlayer, 12);
                for (int i = gSoundChannelNum - s->channelCount; i < CHANNEL_MAX_NUM; i++) {
                    gChannelVol[i] = 1.0f;
                }
                set_music_volume(gMusicVolume);
            }
            sSequenceFadeTimer = 0;
            sCurrentSequenceID = sNextSequenceID;
        } else if (sCurrentSequenceID != -1) {
            float fade;
            fade = ((float) sSequenceFadeTimer / (float) sSequenceFadeTimerSet) * gMusicVolume;
            if (fade < 0.0001f) {
                fade = 0.0001f;
            }
            xm64player_set_vol(&sXMPlayer, fade);
        }
    }
}

static void update_sound(float updateRateF) {
    for (int i = 0; i < gSoundChannelNum; i++) {
        if (sChannelMask & (1 << i)) {
            if (gChannelVol[i] > 0.0f) {
                gChannelVol[i] -= 0.05f * updateRateF;
                if (gChannelVol[i] < 0.0f) {
                    gChannelVol[i] = 0.0f;
                }
                mixer_ch_set_vol(i, gChannelVol[i], gChannelVol[i]);
            }
        } else {
            if (gChannelVol[i] < 1.0f) {
                gChannelVol[i] += 0.05f * updateRateF;
                if (gChannelVol[i] > 1.0f) {
                    gChannelVol[i] = 1.0f;
                }
                mixer_ch_set_vol(i, gChannelVol[i], gChannelVol[i]);
            }
        }
    }
}

void audio_loop(int updateRate, float updateRateF) {
    update_sound(updateRateF);
    update_sequence(updateRate);
    if (audio_can_write()) {    	
		short *buf = audio_write_begin();
		mixer_poll(buf, audio_get_buffer_length());
		audio_write_end();
	}
}

void play_sound_global_pitch(int soundID, float pitch) {
    wav64_play(&sSoundTable[soundID].sound, CHANNEL_GLOBAL);
    mixer_ch_set_freq(CHANNEL_GLOBAL, (32000 / 2) * pitch);
}

static int get_sound_pan(int channel, float pos[3]) {
    return 1;
    /*float volume = 1.0f;
    float pan;
    float dist = SQR(800.0f);
    volume = 1.0f - (DIST3(pos, gCamera[PLAYER_1]->pos) / dist);
    if (gConfig.soundMode == SOUND_MONO) {
        pan = 0.5f;
    } else {
        float absX = fabsf(pos[0]);
        float absZ = fabsf(pos[2]);
        if (absX > dist) {
            absX = dist;
        }
        if (absZ > dist) {
            absZ = dist;
        }

        if (pos[0] == 0.0f && pos[2] == 0.0f) {
            pan = 0.5f;
        } else if (pos[0] >= 0.0f && absX >= absZ) {
            pan = 1.0f - (2 * dist - absX) / (3.0f * (2 * dist - absZ));
        } else if (pos[0] < 0 && absX > absZ) {
            pan = (2 * dist - absX) / (3.0f * (2 * dist - absZ));
        } else {
            pan = 0.5f + pos[0] / (6.0f * absZ);
        }
    }
    if (volume > 0.0f) {
        mixer_ch_set_vol_pan(channel, volume * gSoundVolume, pan);
        return 1;
    } else {
        return 0;
    }*/
}

void play_sound_spatial(int soundID, float pos[3]) {
    int channel = find_sound_channel(sSoundTable[soundID].priority);
    if (get_sound_pan(channel, pos) == 0) {
        return;
    }
    mixer_ch_set_freq(channel, AUDIO_FREQUENCY * 2);
    wav64_play(&sSoundTable[soundID].sound, channel);
}

void play_sound_spatial_pitch(int soundID, float pos[3], float pitch) {
    int channel = find_sound_channel(sSoundTable[soundID].priority);
    if (get_sound_pan(channel, pos) == 0) {
        return;
    }
    //mixer_ch_set_freq(channel, (AUDIO_FREQUENCY * 2) * pitch);
    wav64_play(&sSoundTable[soundID].sound, channel);
}

void set_background_music(int seqID, int fadeTime) {
    return;
    sNextSequenceID = seqID;
    sSequenceFadeTimerSet = fadeTime;
    sSequenceFadeTimer = fadeTime;
}
