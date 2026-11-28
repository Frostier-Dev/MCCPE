#include <cmath>
#include <nds.h>
#include "Renderer.hpp"
#include "java/Frustum.hpp"

int gfxPolys = 0;
int gfxVerts = 0;

uint32_t chunkDisplayLists[LOADED_X * LOADED_Z][TILE_COUNT][MAX_DL_WORDS];
uint32_t swapDisplayList[MAX_DL_WORDS];

static int terrainTextureIDs[TILE_COUNT];

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

    const u16* atlas = (const u16*)textureData;
    static u16 tile[TILE_SIZE_PX * TILE_SIZE_PX];

    glGenTextures(TILE_COUNT, terrainTextureIDs);
    for (int t = 0; t < TILE_COUNT; t++) {
        const int tx = (t % TILES_PER_ROW) * TILE_SIZE_PX;
        const int ty = (t / TILES_PER_ROW) * TILE_SIZE_PX;

        for (int y = 0; y < TILE_SIZE_PX; y++)
            for (int x = 0; x < TILE_SIZE_PX; x++)
                tile[y * TILE_SIZE_PX + x] = atlas[(ty + y) * ATLAS_SIZE_PX + tx + x];

        glBindTexture(0, terrainTextureIDs[t]);
        glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_16, TEXTURE_SIZE_16, 0,
                     TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, tile);
    }
}

static inline bool isBlockSolidLocal(const Chunk& chunk, int x, int y, int z) {
    if (y < 0) return false;
    if (y >= CHUNK_HEIGHT) return false;

    if (x >= 0 && x < CHUNK_WIDTH && z >= 0 && z < CHUNK_WIDTH) {
        return chunk.blocks[y][z][x] != 0;
    }

    int wx = (chunk.chunkX * CHUNK_WIDTH) + x;
    int wz = (chunk.chunkZ * CHUNK_WIDTH) + z;

    if (wx < -CHUNK_WIDTH || wx >= CHUNK_WIDTH * 2 || wz < -CHUNK_WIDTH || wz >= CHUNK_WIDTH * 2) {
        return false;
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
    if (y < 0) return 15;

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

#define EDGE_EPS 3

#define QUAD_WORDS 19

static inline void emitQuad(uint32_t* dl, int& pos, uint16_t color,
                            int uTexels, int vTexels,
                            const v16* vx, const v16* vy, const v16* vz) {
    if (pos + QUAD_WORDS + 1 >= MAX_DL_WORDS) return;

    const t16 tu = inttot16(uTexels);
    const t16 tv = inttot16(vTexels);
    const t16 U[4] = { 0, 0, tu, tu };
    const t16 V[4] = { 0, tv, tv, 0 };

    dl[pos++] = FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_COLOR, FIFO_NOP, FIFO_NOP);
    dl[pos++] = GL_QUADS;
    dl[pos++] = color;

    for (int i = 0; i < 4; i++) {
        dl[pos++] = FIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP, FIFO_NOP);
        dl[pos++] = TEXTURE_PACK(U[i], V[i]);
        dl[pos++] = VERTEX_PACK(vx[i], vy[i]);
        dl[pos++] = VERTEX_PACK(vz[i], 0);
    }
}

struct FaceDef {
    int8_t n;
    int8_t positive;
    int8_t baseShade;
    int8_t uAlongA;
    int8_t corner[4][2];
};

static const FaceDef FACES[6] = {
    {1, 1, 31, 1, {{0,0},{0,1},{1,1},{1,0}}},
    {1, 0, 15, 0, {{0,0},{1,0},{1,1},{0,1}}},
    {2, 1, 25, 1, {{0,1},{0,0},{1,0},{1,1}}},
    {2, 0, 25, 1, {{1,1},{1,0},{0,0},{0,1}}},
    {0, 1, 19, 1, {{1,1},{1,0},{0,0},{0,1}}},
    {0, 0, 19, 1, {{0,1},{0,0},{1,0},{1,1}}},
};

void rebuildChunkList(Chunk& chunk, int listIndex) {
    static const int DIM[3] = { CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH };
    int pos[TILE_COUNT];
    for (int t = 0; t < TILE_COUNT; t++) pos[t] = 1;

    uint8_t mask[CHUNK_HEIGHT][CHUNK_WIDTH];

    for (int f = 0; f < 6; f++) {
        const FaceDef& fd = FACES[f];
        const int n     = fd.n;
        const int aAxis = (n == 0) ? 2 : 0;
        const int bAxis = (n == 1) ? 2 : 1;
        const int sizeA = DIM[aAxis];
        const int sizeB = DIM[bAxis];
        const int sizeS = DIM[n];
        const int step  = fd.positive ? 1 : -1;

        for (int s = 0; s < sizeS; s++) {
            for (int b = 0; b < sizeB; b++) {
                for (int a = 0; a < sizeA; a++) {
                    int p[3];
                    p[n] = s; p[aAxis] = a; p[bAxis] = b;
                    mask[b][a] = 0;

                    const uint8_t id = chunk.blocks[p[1]][p[2]][p[0]];
                    if (id == 0) continue;

                    int q[3] = { p[0], p[1], p[2] };
                    q[n] += step;
                    if (isBlockSolidLocal(chunk, q[0], q[1], q[2])) continue;

                    const Texture& tex = textureregister[id];
                    uint8_t tile = (f == 0) ? tex.topimg
                                 : (f == 1) ? (tex.uniqueface ? tex.topimg : tex.bottomimg)
                                            : (tex.uniqueface ? tex.topimg : tex.side0img);
                    const uint8_t light = getEffectiveLightLocal(chunk, q[0], q[1], q[2]);
                    mask[b][a] = 1 + (((tile & 3) << 4) | light);
                }
            }

            for (int b0 = 0; b0 < sizeB; b0++) {
                for (int a0 = 0; a0 < sizeA; a0++) {
                    const uint8_t key = mask[b0][a0];
                    if (!key) continue;

                    int w = 1;
                    while (a0 + w < sizeA && mask[b0][a0 + w] == key) w++;

                    int h = 1;
                    for (bool ok = true; ok && b0 + h < sizeB; ) {
                        for (int i = 0; i < w; i++) {
                            if (mask[b0 + h][a0 + i] != key) { ok = false; break; }
                        }
                        if (ok) h++;
                    }

                    for (int j = 0; j < h; j++)
                        for (int i = 0; i < w; i++)
                            mask[b0 + j][a0 + i] = 0;

                    const int tile  = (key - 1) >> 4;
                    const int light = (key - 1) & 15;
                    const int plane = s + fd.positive;

                    v16 vx[4], vy[4], vz[4];
                    for (int c = 0; c < 4; c++) {
                        const int ca = fd.corner[c][0];
                        const int cb = fd.corner[c][1];

                        int raw[3];
                        raw[n]     = plane << 9;
                        raw[aAxis] = ((a0 + ca * w) << 9) + (ca ? EDGE_EPS : -EDGE_EPS);
                        raw[bAxis] = ((b0 + cb * h) << 9) + (cb ? EDGE_EPS : -EDGE_EPS);

                        vx[c] = (v16)raw[0];
                        vy[c] = (v16)raw[1];
                        vz[c] = (v16)raw[2];
                    }

                    const int uExt = fd.uAlongA ? w : h;
                    const int vExt = fd.uAlongA ? h : w;
                    emitQuad(chunkDisplayLists[listIndex][tile], pos[tile],
                             applyLightToShade(fd.baseShade, (uint8_t)light),
                             uExt * TILE_SIZE_PX, vExt * TILE_SIZE_PX, vx, vy, vz);
                }
            }
        }
    }

    for (int t = 0; t < TILE_COUNT; t++) {
        uint32_t* dl = chunkDisplayLists[listIndex][t];
        if (pos[t] == 1) { dl[0] = 0; continue; }
        dl[pos[t]++] = FIFO_COMMAND_PACK(FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP);
        dl[0] = pos[t] - 1;
    }

    chunk.displayListId = listIndex;
    chunk.dirty = false;
}

void Renderer_RenderWorld(void) {
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

    const CamFrustum cam = buildCamFrustum();

    int   order[LOADED_X * LOADED_Z];
    float dist2[LOADED_X * LOADED_Z];
    int   n = 0;

    for (int idx = 0; idx < LOADED_X * LOADED_Z; idx++) {
        Chunk& c = loadedChunks[idx / LOADED_Z][idx % LOADED_Z];
        if (!c.active || !c.loaded) continue;

        const float cx = c.chunkX * CHUNK_WIDTH + 8.0f;
        const float cz = c.chunkZ * CHUNK_WIDTH + 8.0f;
        if (!sphereInView(cam, cx, CHUNK_HEIGHT * 0.5f, cz, 20.0f)) continue;

        const float dx = cx - local_player.x;
        const float dz = cz - local_player.z;
        const float d2 = dx * dx + dz * dz;

        int j = n++;
        while (j > 0 && dist2[j - 1] > d2) {
            dist2[j] = dist2[j - 1];
            order[j] = order[j - 1];
            j--;
        }
        dist2[j] = d2;
        order[j] = idx;
    }

    for (int k = 0; k < n; k++) {
        const int idx = order[k];
        Chunk& c = loadedChunks[idx / LOADED_Z][idx % LOADED_Z];

        if (c.dirty || c.displayListId < 0) rebuildChunkList(c, idx);

        glPushMatrix();
        glTranslatef32(inttof32(c.chunkX * CHUNK_WIDTH), 0, inttof32(c.chunkZ * CHUNK_WIDTH));
        glScalef32(inttof32(8), inttof32(8), inttof32(8));
        for (int t = 0; t < TILE_COUNT; t++) {
            const uint32_t* dl = chunkDisplayLists[idx][t];
            if (dl[0] > 0) {
                glBindTexture(0, terrainTextureIDs[t]);
                glCallList(dl);
            }
        }
        glPopMatrix(1);
    }

    RaycastHit target = raycastBlock(4.5f);
    if (target.hit) Renderer_DrawBlockHighlight(target.bx, target.by, target.bz);

#ifdef RENDER_STATS
    while (GFX_STATUS & (1 << 27)) {}
    gfxPolys = GFX_POLYGON_RAM_USAGE;
    gfxVerts = GFX_VERTEX_RAM_USAGE;
#endif

    glFlush(0);
}