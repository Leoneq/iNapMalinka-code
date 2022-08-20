#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "low_battery.h"
#include "gpu.h"
#include "spi.h"
#include "text.h"
#include <time.h>

#define LOW_BATTERY_ICON_TOP_LEFT_X 5
#define LOW_BATTERY_ICON_TOP_LEFT_Y 446
#define SOUND_ICON_TOP_LEFT_X 5
#define SOUND_ICON_TOP_LEFT_Y 400
#define LOW_BATTERY_ICON_WIDTH 11
#define LOW_BATTERY_ICON_HEIGHT 29
#define SOUND_ICON_HEIGHT 10
#define SOUND_ICON_WIDTH 13
#define LOW_BATTERY_FORE_COLOR 65535
#define LOW_BATTERY_BACK_COLOR 0

static bool lowBattery = false;
static uint64_t lowBatteryLastPolled = 0;
time_t t = time(NULL);
struct tm tm;
char time_text[6];
char sound_text[5];
char battery_text[5];
bool draw_overlay = true;

// custom overlay by @leoneq112
static uint16_t lowBatteryIcon [LOW_BATTERY_ICON_HEIGHT][LOW_BATTERY_ICON_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0}};

static uint16_t soundIcon [SOUND_ICON_HEIGHT][SOUND_ICON_WIDTH] = {
    {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};


void InitLowBatterySystem()
{
    for(int y = 0; y < LOW_BATTERY_ICON_HEIGHT; ++y)
    {
        for(int x = 0; x < LOW_BATTERY_ICON_WIDTH; ++x)
        {
            lowBatteryIcon[y][x] = lowBatteryIcon[y][x] ? LOW_BATTERY_FORE_COLOR : LOW_BATTERY_BACK_COLOR;
        }
    }

    for(int y = 0; y < SOUND_ICON_HEIGHT; ++y)
    {
        for(int x = 0; x < SOUND_ICON_WIDTH; ++x)
        {
            soundIcon[y][x] = soundIcon[y][x] ? LOW_BATTERY_FORE_COLOR : LOW_BATTERY_BACK_COLOR;
        }
    }

    PollLowBattery();
}

void PollLowBattery()
{
    uint64_t now = tick();
    if (now - lowBatteryLastPolled > LOW_BATTERY_POLLING_INTERVAL)
    {
        //get time
        tm = *localtime(&t);
        sprintf(time_text, "%02d:%02d", tm.tm_hour, tm.tm_min);

        //get sound volume
        system("sudo -u pi amixer get Headphone -M | awk '$0~/%/{print $4}' | tr -d '[]' > sound_volume");
        FILE* FILE_handle = fopen("sound_volume", "r");
        fgets(sound_text, sizeof(sound_text), FILE_handle);
        fclose(FILE_handle);

        //get battery
        FILE_handle = fopen("battery", "r");
        fgets(battery_text, sizeof(battery_text), FILE_handle);
        if(strcmp(battery_text, "non") == 0)
        {
            draw_overlay = 0;
        }
        else
        {
            draw_overlay = 1;
        }
        fclose(FILE_handle);

        lowBatteryLastPolled = now;
    }
}

void DrawLowBatteryIcon(uint16_t *framebuffer)
{
    //don't draw overlay if it's hidden
    if(draw_overlay == 0)
    {
        return;
    }

    //battery icon
    for(int y = 0; y < LOW_BATTERY_ICON_HEIGHT; ++y)
    {
        int framebuffer_start_offset = (LOW_BATTERY_ICON_TOP_LEFT_Y+y)*(gpuFramebufferScanlineStrideBytes>>1)+LOW_BATTERY_ICON_TOP_LEFT_X;
        memcpy(framebuffer+framebuffer_start_offset, lowBatteryIcon[y], LOW_BATTERY_ICON_WIDTH*2);
    }
    if(strcmp(battery_text, "100%") == 0)
    {
        DrawText(framebuffer, gpuFrameWidth, gpuFramebufferScanlineStrideBytes, gpuFrameHeight, "100%", 448, 8, 0xFFFF, 0);
    }
    else
    {
        battery_text[3] = 0;
        DrawText(framebuffer, gpuFrameWidth, gpuFramebufferScanlineStrideBytes, gpuFrameHeight, battery_text, 451, 8, 0xFFFF, 0);
    }

    //time
    DrawText(framebuffer, gpuFrameWidth, gpuFramebufferScanlineStrideBytes, gpuFrameHeight, time_text, 9, 9, 0xFFFF, 0);

    //sound volume
    for(int y = 0; y < SOUND_ICON_HEIGHT; ++y)
    {
        int framebuffer_start_offset = (SOUND_ICON_TOP_LEFT_Y+y)*(gpuFramebufferScanlineStrideBytes>>1)+SOUND_ICON_TOP_LEFT_X;
        memcpy(framebuffer+framebuffer_start_offset, soundIcon[y], SOUND_ICON_WIDTH*2);
    }
    DrawText(framebuffer, gpuFrameWidth, gpuFramebufferScanlineStrideBytes, gpuFrameHeight, sound_text, 414, 9, 0xFFFF, 0);

}
