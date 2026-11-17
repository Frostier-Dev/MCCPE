#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <stdint.h>
#include <nds.h>
#include "Minecraft.hpp"

#define MAX_DL_WORDS 8192

#define ATLAS_SIZE_PX 32
#define TILE_SIZE_PX  16
#define TILES_PER_ROW (ATLAS_SIZE_PX / TILE_SIZE_PX)

extern uint32_t chunkDisplayLists[LOADED_X * LOADED_Z][MAX_DL_WORDS];
extern uint32_t swapDisplayList[MAX_DL_WORDS];

void Renderer_Init(void);
void Renderer_LoadTexture(const void* textureData);
void Renderer_SetupCamera(void);
void Renderer_RenderWorld(void);
void rebuildChunkList(Chunk& chunk, int listIndex);
void setBlockAt(int wx, int wy, int wz, uint8_t newBlockId);

#endif