#include <stdio.h>
#include <time.h>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <nds.h>
#include <maxmod9.h>
#include <filesystem.h>
#include <fat.h>
#include "soundbank.h"
#include "Minecraft.hpp"
#include "Renderer.hpp"
#include "atlas.h"
#include "earthbg.h"
#include "mainhud.h"
#include "textdf.h"

#define OutputStateTF(b) ((b) ? "true" : "false")

ConsoleFont customFont = {
    .gfx = textdfTiles,
    .pal = NULL,
    .numColors = 0,
    .bpp = 4,
    .asciiOffset = 0,
    .numChars = 256    
};
bool crouch = false;

int lastTouchX = 0;
int lastTouchY = 0;
bool wasTouched = false;
float cameraAngleY = 0.0f;
float cameraAngleX = 0.0f;

bool ingame = true;
int frame_count = 0;
int current_fps = 0;
volatile bool spas = false;

void timer_handler(void) {
    current_fps = frame_count;
    frame_count = 0;
    spas = true;
}

void input(float& moveX, float& moveZ) {
    scanKeys();
    uint16_t keys = keysHeld();
    uint16_t keys_down = keysDown();

    if (keys & KEY_TOUCH) {
        touchPosition touch;
        touchRead(&touch);
        if (!wasTouched) {
            lastTouchX = touch.px;
            lastTouchY = touch.py;
            wasTouched = true;
        } else {
            int deltaX = touch.px - lastTouchX;
            int deltaY = touch.py - lastTouchY;
            cameraAngleY -= deltaY * 0.5f;
            cameraAngleX -= deltaX * 0.5f;
            lastTouchX = touch.px;
            lastTouchY = touch.py;
        }
    } else {
        wasTouched = false;
    }
    if (cameraAngleY > 89.0f)  cameraAngleY = 89.0f;
    if (cameraAngleY < -89.0f) cameraAngleY = -89.0f;

    float rad = cameraAngleX * (M_PI / 180.0f);
    float vel = 0.08f;

    if (keys & KEY_UP) {
        moveX -= sinf(rad) * vel;
        moveZ -= cosf(rad) * vel;
    }
    if (keys & KEY_DOWN) {
        moveX += sinf(rad) * vel;
        moveZ += cosf(rad) * vel;
    }
    if (keys & KEY_LEFT) {
        moveX -= cosf(rad) * vel;
        moveZ += sinf(rad) * vel;
    }
    if (keys & KEY_RIGHT) {
        moveX += cosf(rad) * vel;
        moveZ -= sinf(rad) * vel;
    }

    if (keys_down & KEY_START)  { ingame = false; }
    if (keys_down & KEY_Y)      { crouch = !crouch; }
    if ((keys_down & KEY_A) && local_player.is_on_ground) {
        local_player.vy = 0.16f;
        local_player.is_on_ground = false;
    }
    if (keys_down & KEY_L) {
        RaycastHit target = raycastBlock(4.5f);
        if (target.hit) {
            setBlockAt(target.bx, target.by, target.bz, 0);
        }
    }

    if (keys_down & KEY_R) {
        RaycastHit target = raycastBlock(4.5f);
        if (target.hit) {
            float pHeight = crouch ? 1.5f : 1.8f;
            float halfW = local_player.width * 0.5f;

            bool insidePlayer =
                (local_player.x + halfW > target.prevX) && (local_player.x - halfW < target.prevX + 1.0f) &&
                (local_player.y + pHeight > target.prevY) && (local_player.y < target.prevY + 1.0f) &&
                (local_player.z + halfW > target.prevZ) && (local_player.z - halfW < target.prevZ + 1.0f);

            if (!insidePlayer) {
                setBlockAt(target.prevX, target.prevY, target.prevZ, 3);
            }
        }
    }
}
bool checkPlayerCollision(float x, float y, float z, float w, float h) {
    float halfW = w * 0.5f;

    int minX = (int)std::floor(x - halfW);
    int maxX = (int)std::floor(x + halfW);
    int minY = (int)std::floor(y);
    int maxY = (int)std::floor(y + h - 0.01f);
    int minZ = (int)std::floor(z - halfW);
    int maxZ = (int)std::floor(z + halfW);

    for (int bx = minX; bx <= maxX; bx++) {
        for (int by = minY; by <= maxY; by++) {
            for (int bz = minZ; bz <= maxZ; bz++) {
                if (isSolidAt(bx + 0.5f, by + 0.5f, bz + 0.5f)) {
                    return true;
                }
            }
        }
    }
    return false;
}
int main(int argc, char* argv[]) {
    Renderer_Init();
    Renderer_LoadTexture(atlasBitmap);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
    videoSetModeSub(MODE_0_2D);
    videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG2_ACTIVE);
    
    int subBg = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
    dmaCopy(earthbgTiles, bgGetGfxPtr(subBg), earthbgTilesLen);
    dmaCopy(earthbgMap,   bgGetMapPtr(subBg), earthbgMapLen);
    dmaCopy(earthbgPal,   BG_PALETTE_SUB,     earthbgPalLen);

    int hudBg = bgInit(1, BgType_Text4bpp, BgSize_T_256x256, 1, 0);
    dmaCopy(mainhudTiles, bgGetGfxPtr(hudBg), mainhudTilesLen);
    dmaCopy(mainhudMap,   bgGetMapPtr(hudBg), mainhudMapLen);
    dmaCopy(mainhudPal,   BG_PALETTE,         mainhudPalLen);

    PrintConsole hudConsole;
    consoleInitEx(
        &hudConsole,
        2,
        BgType_Text4bpp,
        BgSize_T_256x256,
        4,
        2,
        15,
        0,
        true,
        false
    );
    consoleSetFont(&hudConsole, &customFont);
    consoleSelect(&hudConsole);
    dmaCopy(textdfPal, BG_PALETTE + 240, textdfPalLen);
    bgSetPriority(2, 0);
    bgSetPriority(1, 1);
    bgSetPriority(0, 2);

    printf("Minecraft: Nintendo DS Edition\n");
    printf("c0.10.0\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("     Not Licensed, by Frostier71\n");
    printf("     Selected Block: Cobblestone");


    nitroFSInit(NULL);

    soundEnable();

    mmInitDefault("nitro:/soundbank.bin");
    mmLoad(MOD_MINECRAFT);
    mmStart(MOD_MINECRAFT, MM_PLAY_LOOP);

    RegisterSetup();
    GenerateWorld();

    timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(1), timer_handler);

    while (ingame) {
        float moveX = 0.0f;
        float moveZ = 0.0f;
        input(moveX, moveZ);

        float pHeight = crouch ? 1.5f : 1.8f;
        float pWidth  = local_player.width;

        local_player.x += moveX;
        if (checkPlayerCollision(local_player.x, local_player.y, local_player.z, pWidth, pHeight)) {
            local_player.x -= moveX;
        }
        local_player.z += moveZ;
        if (checkPlayerCollision(local_player.x, local_player.y, local_player.z, pWidth, pHeight)) {
            local_player.z -= moveZ;
        }

        local_player.vy -= 0.008f;
        local_player.y += local_player.vy;

        if (checkPlayerCollision(local_player.x, local_player.y, local_player.z, pWidth, pHeight)) {
            if (local_player.vy < 0.0f) {
                local_player.y = std::ceil(local_player.y);
                local_player.is_on_ground = true;
            } else if (local_player.vy > 0.0f) {
                local_player.y -= local_player.vy;
            }
            local_player.vy = 0.0f;
        } else {
            local_player.is_on_ground = false;
        }

        float eyeHeight = pHeight * 0.85f;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-cameraAngleY, 1.0f, 0.0f, 0.0f);
        glRotatef(-cameraAngleX, 0.0f, 1.0f, 0.0f);
        glTranslatef32(
            floattof32(-local_player.x),
            floattof32(-(local_player.y + eyeHeight)),
            floattof32(-local_player.z)
        );

        Renderer_RenderWorld();

        frame_count++;
        swiWaitForVBlank();
    }

    return 0;
}