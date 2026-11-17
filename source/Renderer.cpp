#include <cmath>
#include <nds.h>
#include "Renderer.hpp"

extern float cameraAngleX;

uint32_t chunkDisplayLists[LOADED_X * LOADED_Z][MAX_DL_WORDS];
uint32_t swapDisplayList[MAX_DL_WORDS];

static int terrainTextureID = 0;

#define SHADE_TOP    RGB15(31, 31, 31)
#define SHADE_SIDE_Z RGB15(25, 25, 25)
#define SHADE_SIDE_X RGB15(19, 19, 19)
#define SHADE_BOTTOM RGB15(15, 15, 15)

void Renderer_Init(void) {
    videoSetMode(MODE_0_3D);
    
    glInit();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glClearColor(16, 22, 31, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);

    glViewport(0, 0, 255, 191);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, 256.0 / 192.0, 0.5, 80.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer_LoadTexture(const void* textureData) {
    vramSetBankA(VRAM_A_TEXTURE);

    glGenTextures(1, &terrainTextureID);
    glBindTexture(0, terrainTextureID);

    // Carrega o atlas 32x32 16-bit gerado pelo grit
    glTexImage2D(
        0, 0, GL_RGBA,
        TEXTURE_SIZE_32, TEXTURE_SIZE_32,
        0,
        TEXGEN_TEXCOORD,
        textureData
    );
}

static inline bool isBlockSolidLocal(const Chunk& chunk, int x, int y, int z) {
    if (y < 0) return true;
    if (y >= CHUNK_HEIGHT) return false;

    if (x >= 0 && x < CHUNK_WIDTH && z >= 0 && z < CHUNK_WIDTH) {
        return chunk.blocks[y][z][x] != 0;
    }

    int wx = (chunk.chunkX * CHUNK_WIDTH) + x;
    int wz = (chunk.chunkZ * CHUNK_WIDTH) + z;

    if (wx < -CHUNK_WIDTH || wx >= CHUNK_WIDTH * 2 || wz < -CHUNK_WIDTH || wz >= CHUNK_WIDTH * 2) {
        return y <= 16;
    }

    return isSolidAt((float)wx + 0.5f, (float)y + 0.5f, (float)wz + 0.5f);
}
void Renderer_DrawBlockHighlight(int bx, int by, int bz) {
    glBindTexture(0, 0);

    glPolyFmt(POLY_ALPHA(7) | POLY_CULL_BACK | POLY_ID(1));

    glPushMatrix();
    glTranslatef32(inttof32(bx), inttof32(by), inttof32(bz));

    v16 minV = floattov16(-0.005f);
    v16 maxV = floattov16(1.005f);

    glBegin(GL_QUADS);
        glColor3b(255, 255, 255);

        glVertex3v16(minV, maxV, minV);
        glVertex3v16(minV, maxV, maxV);
        glVertex3v16(maxV, maxV, maxV);
        glVertex3v16(maxV, maxV, minV);

        glVertex3v16(minV, minV, minV);
        glVertex3v16(maxV, minV, minV);
        glVertex3v16(maxV, minV, maxV);
        glVertex3v16(minV, minV, maxV);

        glVertex3v16(minV, maxV, maxV);
        glVertex3v16(minV, minV, maxV);
        glVertex3v16(maxV, minV, maxV);
        glVertex3v16(maxV, maxV, maxV);

        glVertex3v16(maxV, maxV, minV);
        glVertex3v16(maxV, minV, minV);
        glVertex3v16(minV, minV, minV);
        glVertex3v16(minV, maxV, minV);

        glVertex3v16(maxV, maxV, maxV);
        glVertex3v16(maxV, minV, maxV);
        glVertex3v16(maxV, minV, minV);
        glVertex3v16(maxV, maxV, minV);

        glVertex3v16(minV, maxV, minV);
        glVertex3v16(minV, minV, minV);
        glVertex3v16(minV, minV, maxV);
        glVertex3v16(minV, maxV, maxV);
    glEnd();

    glPopMatrix(1);
}
static inline uint8_t getEffectiveLightLocal(const Chunk& chunk, int x, int y, int z) {
    if (y >= CHUNK_HEIGHT) return 15;
    if (y < 0) return 0;

    if (x >= 0 && x < CHUNK_WIDTH && z >= 0 && z < CHUNK_WIDTH) {
        uint8_t raw = chunk.light[y][z][x];
        uint8_t sky   = (raw >> 4) & 0x0F;
        uint8_t block = raw & 0x0F;
        return (sky > block) ? sky : block;
    }

    return 15;
}

static inline uint16_t applyLightToShade(int baseShade, uint8_t lightLevel) {
    int val = (baseShade * (lightLevel + 2)) / 17;
    if (val > 31) val = 31;
    return RGB15(val, val, val);
}
static inline void emitQuad(
    uint32_t* dl, int& pos, uint16_t color, uint8_t tileIdx,
    v16 x0, v16 y0, v16 z0,
    v16 x1, v16 y1, v16 z1,
    v16 x2, v16 y2, v16 z2,
    v16 x3, v16 y3, v16 z3
) {
    if (pos + 16 >= MAX_DL_WORDS) return;

    int u0 = (tileIdx % TILES_PER_ROW) * TILE_SIZE_PX;
    int v0 = (tileIdx / TILES_PER_ROW) * TILE_SIZE_PX;
    int u1 = u0 + TILE_SIZE_PX;
    int v1 = v0 + TILE_SIZE_PX;

    t16 tu0 = inttot16(u0) + 2;
    t16 tv0 = inttot16(v0) + 2;
    t16 tu1 = inttot16(u1) - 2;
    t16 tv1 = inttot16(v1) - 2;

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_COLOR, FIFO_NOP, FIFO_NOP);
    dl[pos++] = GL_QUADS;
    dl[pos++] = color;

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP, FIFO_NOP);
    dl[pos++] = TEXTURE_PACK(tu0, tv0);
    dl[pos++] = VERTEX_PACK(x0, y0);
    dl[pos++] = VERTEX_PACK(z0, 0);

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP, FIFO_NOP);
    dl[pos++] = TEXTURE_PACK(tu0, tv1);
    dl[pos++] = VERTEX_PACK(x1, y1);
    dl[pos++] = VERTEX_PACK(z1, 0);

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP, FIFO_NOP);
    dl[pos++] = TEXTURE_PACK(tu1, tv1);
    dl[pos++] = VERTEX_PACK(x2, y2);
    dl[pos++] = VERTEX_PACK(z2, 0);

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP, FIFO_NOP);
    dl[pos++] = TEXTURE_PACK(tu1, tv0);
    dl[pos++] = VERTEX_PACK(x3, y3);
    dl[pos++] = VERTEX_PACK(z3, 0);
}

static inline v16 toSafeV16(int val) {
    return (v16)(val << 10);
}

void rebuildChunkList(Chunk& chunk, int listIndex) {
    uint32_t* dl = chunkDisplayLists[listIndex];
    int pos = 1;

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_WIDTH; z++) {
            for (int x = 0; x < CHUNK_WIDTH; x++) {
                uint8_t id = chunk.blocks[y][z][x];
                if (id == 0) continue;

                const Texture& tex = textureregister[id];
                uint8_t topTile    = tex.topimg;
                uint8_t bottomTile = tex.uniqueface ? tex.topimg : tex.bottomimg;
                uint8_t sideTile   = tex.uniqueface ? tex.topimg : tex.side0img;

                v16 x0 = toSafeV16(x);
                v16 y0 = toSafeV16(y);
                v16 z0 = toSafeV16(z);
                v16 x1 = toSafeV16(x + 1);
                v16 y1 = toSafeV16(y + 1);
                v16 z1 = toSafeV16(z + 1);

                if (!isBlockSolidLocal(chunk, x, y + 1, z)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x, y + 1, z);
                    uint16_t color = applyLightToShade(31, light);
                    emitQuad(dl, pos, color, topTile,
                             x0, y1, z0,  x0, y1, z1,  x1, y1, z1,  x1, y1, z0);
                }

                if (!isBlockSolidLocal(chunk, x, y - 1, z)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x, y - 1, z);
                    uint16_t color = applyLightToShade(15, light);
                    emitQuad(dl, pos, color, bottomTile,
                             x0, y0, z0,  x1, y0, z0,  x1, y0, z1,  x0, y0, z1);
                }

                if (!isBlockSolidLocal(chunk, x, y, z + 1)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x, y, z + 1);
                    uint16_t color = applyLightToShade(25, light);
                    emitQuad(dl, pos, color, sideTile,
                             x0, y1, z1,  x0, y0, z1,  x1, y0, z1,  x1, y1, z1);
                }

                if (!isBlockSolidLocal(chunk, x, y, z - 1)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x, y, z - 1);
                    uint16_t color = applyLightToShade(25, light);
                    emitQuad(dl, pos, color, sideTile,
                             x1, y1, z0,  x1, y0, z0,  x0, y0, z0,  x0, y1, z0);
                }

                if (!isBlockSolidLocal(chunk, x + 1, y, z)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x + 1, y, z);
                    uint16_t color = applyLightToShade(19, light);
                    emitQuad(dl, pos, color, sideTile,
                             x1, y1, z1,  x1, y0, z1,  x1, y0, z0,  x1, y1, z0);
                }

                if (!isBlockSolidLocal(chunk, x - 1, y, z)) {
                    uint8_t light = getEffectiveLightLocal(chunk, x - 1, y, z);
                    uint16_t color = applyLightToShade(19, light);
                    emitQuad(dl, pos, color, sideTile,
                             x0, y1, z0,  x0, y0, z0,  x0, y0, z1,  x0, y1, z1);
                }
            }
        }
    }

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP);
    dl[0] = pos - 1;

    chunk.displayListId = listIndex;
    chunk.dirty = false;
}


void Renderer_RenderWorld(void) {
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
    glBindTexture(0, terrainTextureID);

    float rad = cameraAngleX * (M_PI / 180.0f);
    float lookX = -sinf(rad);
    float lookZ = -cosf(rad);

    int listIdx = 0;
    for (int gx = 0; gx < LOADED_X; gx++) {
        for (int gz = 0; gz < LOADED_Z; gz++) {
            Chunk& c = loadedChunks[gx][gz];
            if (!c.active || !c.loaded) {
                listIdx++;
                continue;
            }

            if (c.dirty || c.displayListId < 0) {
                rebuildChunkList(c, listIdx);
            }

            float centerX = (c.chunkX * CHUNK_WIDTH) + 8.0f;
            float centerZ = (c.chunkZ * CHUNK_WIDTH) + 8.0f;

            float dx = centerX - local_player.x;
            float dz = centerZ - local_player.z;

            float dot = (dx * lookX) + (dz * lookZ);

            if (dot > -26.0f) {
                glPushMatrix();
                glTranslatef32(inttof32(c.chunkX * CHUNK_WIDTH), 0, inttof32(c.chunkZ * CHUNK_WIDTH));
                glScalef32(inttof32(4), inttof32(4), inttof32(4));
                glCallList(chunkDisplayLists[listIdx]);
                glPopMatrix(1);
            }

            listIdx++;
        }
    }

    RaycastHit target = raycastBlock(4.5f);
    if (target.hit) {
        Renderer_DrawBlockHighlight(target.bx, target.by, target.bz);
    }

    glFlush(0);
}