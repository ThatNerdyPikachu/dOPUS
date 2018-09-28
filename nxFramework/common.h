#pragma once

#include <string.h>
#include <vector>
#include <stdio.h>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>
#include <switch.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL_ttf.h>

#include "SDL/SDLRender.h"
#include "SDL/SDLHelper.h"

#define wait(msec) svcSleepThread(10000000 * (s64)msec)

#define ASSERT_OK(rc_out, desc) if (R_FAILED(rc_out)) { char msg[256] = {0}; snprintf(msg, 256-1, "%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, rc_out); throw std::runtime_error(msg); }

#ifdef DEBUG
#define LOG(f_, ...)            do {    printf("%s:%u: \t", __func__, __LINE__);                \
                                        printf((f_), ##__VA_ARGS__);                            \
                                    } while(0)

#define LOG_FILENAME(f_, ...)   do {    printf("%s:%s:%u: \t", __FILE__, __func__, __LINE__);   \
                                        printf((f_), ##__VA_ARGS__);                            \
                                    } while(0)
#else
#define LOG(f_, ...) ;
#define LOG_FILENAME(f_, ...) ;
#endif

#define ROOT_PATH "/"

static void printBytes(u8 *bytes, size_t size, bool includeHeader)
{
    int count = 0;

    if (includeHeader)
    {
        printf("\n\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        printf("-----------------------------------------------\n");
    }

    for (int i = 0; i < (int)size; i++)
    {
        printf("%02x ", bytes[i]);
        count++;
        if ((count % 16) == 0)
            printf("\n");
    }
    printf("\n");
}
