#include <cstdint>
#include <array>
#include <cmath>
#include <time.h>
#include <nds.h>
#include "Minecraft.hpp"
#include "Renderer.hpp"
#include "perlin/PerlinNoise.hpp"
#include "atlas.h"
extern "C" uint32_t xorshift32(uint32_t *state);
extern float cameraAngleX;
extern float cameraAngleY;
Block blockregister[4];
Texture textureregister[4];
Chunk loadedChunks[LOADED_X][LOADED_Z];
Chunk swapChunk;
GhostChunk ghostChunks[16];
Player local_player;
RaycastHit raycastBlock(float maxDist) {
    RaycastHit result = { false, 0, 0, 0, 0, 0, 0 };

    float pHeight = crouch ? 1.5f : 1.8f;
    float ox = local_player.x;
    float oy = local_player.y + (pHeight * 0.85f);
    float oz = local_player.z;

    float radX = cameraAngleX * (M_PI / 180.0f);
    float radY = cameraAngleY * (M_PI / 180.0f);

    float cosY = cosf(radY);
    float dirX = -sinf(radX) * cosY;
    float dirY = sinf(radY);
    float dirZ = -cosf(radX) * cosY;

    float step = 0.1f;
    int lastBX = (int)std::floor(ox);
    int lastBY = (int)std::floor(oy);
    int lastBZ = (int)std::floor(oz);

    for (float d = 0.0f; d <= maxDist; d += step) {
        float px = ox + dirX * d;
        float py = oy + dirY * d;
        float pz = oz + dirZ * d;

        int bx = (int)std::floor(px);
        int by = (int)std::floor(py);
        int bz = (int)std::floor(pz);

        if (isSolidAt(px, py, pz)) {
            result.hit = true;
            result.bx = bx;
            result.by = by;
            result.bz = bz;
            result.prevX = lastBX;
            result.prevY = lastBY;
            result.prevZ = lastBZ;
            return result;
        }

        lastBX = bx;
        lastBY = by;
        lastBZ = bz;
    }

    return result;
}
void RegisterSetup(void) {
    blockregister[0] = { "Air", 0, 0, 16, 16 };

    blockregister[1] = { "Grass", 1, 1, 16, 16 };
    textureregister[1] = { false, 0, 3, 1, 1 };

    blockregister[2] = { "Dirt", 2, 2, 16, 16};
    textureregister[2] = { true, 3, 0, 0, 0 };

    blockregister[3] = { "Cobblestone", 3, 3, 16, 16 };
    textureregister[3] = { true, 2, 0, 0, 0 };
}

void GenerateWorld(void) {
    static uint32_t seedg = 0;
    if (seedg == 0) {
        seedg = (uint32_t)time(NULL) ^ (uint32_t)TIMER0_DATA;
        if (seedg == 0) seedg = 123456789;
    }
    const siv::PerlinNoise::seed_type seed = xorshift32(&seedg);
    static siv::PerlinNoise perlin;
    perlin.reseed(seed);

    for (int gx = 0; gx < LOADED_X; gx++) {
        for (int gz = 0; gz < LOADED_Z; gz++) {
            Chunk& c = loadedChunks[gx][gz];
            memset(c.blocks, 0, sizeof(c.blocks));
            memset(c.light, 0xF0, sizeof(c.light));

            c.chunkX = gx - 1;
            c.chunkZ = gz - 1;
            c.loaded = true;
            c.active = true;
            c.modified = false;
            c.dirty = true;
            c.displayListId = -1;
            
            for (int y = 0; y <= 16; y++) {
                uint8_t blockId = 0;
                if (y < 14) blockId = 3;
                else if (y < 16) blockId = 2;
                else blockId = 1;

                for (int z = 0; z < CHUNK_WIDTH; z++) {
                    for (int x = 0; x < CHUNK_WIDTH; x++) {
                        c.blocks[y][z][x] = blockId;
                    }
                }
            }
        }
    }
    local_player.x = 8.0f;
    local_player.y = 18.0f;
    local_player.z = 8.0f;
    local_player.vx = 0.0f;
    local_player.vy = 0.0f;
    local_player.vz = 0.0f;
    local_player.is_on_ground = false;
}
uint8_t getBlockIDAt(float wx, float wy, float wz) {
    int bx = (int)std::floor(wx);
    int by = (int)std::floor(wy);
    int bz = (int)std::floor(wz);

    if (by < 0 || by >= CHUNK_HEIGHT) return 0;

    int cx = (bx >= 0) ? (bx / CHUNK_WIDTH) : ((bx - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int cz = (bz >= 0) ? (bz / CHUNK_WIDTH) : ((bz - CHUNK_WIDTH + 1) / CHUNK_WIDTH);

    for (int gx = 0; gx < LOADED_X; gx++) {
        for (int gz = 0; gz < LOADED_Z; gz++) {
            const Chunk& c = loadedChunks[gx][gz];
            if (c.loaded && c.chunkX == cx && c.chunkZ == cz) {
                int lx = bx - (cx * CHUNK_WIDTH);
                int lz = bz - (cz * CHUNK_WIDTH);
                return c.blocks[by][lz][lx];
            }
        }
    }

    return 0;
}

bool isSolidAt(float x, float y, float z) {
    uint8_t id = getBlockIDAt(x, y, z);
    return id != 0;
}
static void markChunkDirtyAt(int cx, int cz) {
    for (int gx = 0; gx < LOADED_X; gx++) {
        for (int gz = 0; gz < LOADED_Z; gz++) {
            Chunk& c = loadedChunks[gx][gz];
            if (c.loaded && c.chunkX == cx && c.chunkZ == cz) {
                c.dirty = true;
                return;
            }
        }
    }
}

void setBlockAt(int wx, int wy, int wz, uint8_t newBlockId) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return;

    int cx = (wx >= 0) ? (wx / CHUNK_WIDTH) : ((wx - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int cz = (wz >= 0) ? (wz / CHUNK_WIDTH) : ((wz - CHUNK_WIDTH + 1) / CHUNK_WIDTH);

    for (int gx = 0; gx < LOADED_X; gx++) {
        for (int gz = 0; gz < LOADED_Z; gz++) {
            Chunk& c = loadedChunks[gx][gz];
            if (c.loaded && c.chunkX == cx && c.chunkZ == cz) {
                int lx = wx - (cx * CHUNK_WIDTH);
                int lz = wz - (cz * CHUNK_WIDTH);

                if (c.blocks[wy][lz][lx] == newBlockId) return;

                c.blocks[wy][lz][lx] = newBlockId;
                c.dirty = true;
                c.modified = true;
                if (lx == 0)               markChunkDirtyAt(cx - 1, cz);
                if (lx == CHUNK_WIDTH - 1) markChunkDirtyAt(cx + 1, cz);
                if (lz == 0)               markChunkDirtyAt(cx, cz - 1);
                if (lz == CHUNK_WIDTH - 1) markChunkDirtyAt(cx, cz + 1);
                return;
            }
        }
    }
}