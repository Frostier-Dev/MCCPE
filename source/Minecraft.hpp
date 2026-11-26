#ifndef MINECRAFT_HPP
#define MINECRAFT_HPP
#include <stdint.h>
#include <string>
#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 32
struct Block {
    char name[32];
    uint8_t id;
    uint8_t textureid;
    uint8_t w;
    uint8_t h;
};

struct Texture {
    bool uniqueface;
    uint8_t topimg;
    uint8_t bottomimg;
    uint8_t side0img;
    uint8_t side1img;
};
struct Chunk {
    uint8_t blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
    uint8_t light[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];
    int32_t chunkX;
    int32_t chunkZ;
    int32_t displayListId;
    bool loaded;
    bool active;
    bool modified;
    bool dirty;
};
#define LOADED_X 3
#define LOADED_Z 3
extern Chunk loadedChunks[LOADED_X][LOADED_Z];
extern Chunk swapChunk;
struct GhostChunk {
    int32_t chunkX;
    int32_t chunkZ;
    int32_t displayListId;
    bool active;
};
extern GhostChunk ghostChunks[16]; //260 bytes
struct Player {
    float x, y, z;
    float vx, vy, vz;
    float width = 0.6f;
    float height = 1.8f;
    bool is_on_ground;
};
struct RaycastHit {
    bool hit;
    int bx, by, bz;
    int prevX, prevY, prevZ;
};
RaycastHit raycastBlock(float maxDist = 4.5f);
extern Chunk loadedChunks[LOADED_X][LOADED_Z];
extern Chunk swapChunk;
extern GhostChunk ghostChunks[16];
extern Player local_player;
extern Texture textureregister[4];
extern Block blockregister[4];
extern bool crouch;

void RegisterSetup(void);
void GenerateWorld(std::string type);
bool isSolidAt(float x, float y, float z);
Block& getBlockAt(int wx, int wy, int wz);
void rebuildChunkList(Chunk& chunk);
#endif