#ifndef SOUNDSYS_HPP
#define SOUNDSYS_HPP

#include <stdint.h>
#include <cstdlib>
#include <nds.h>
#include <maxmod9.h>
#include "soundbank.h"

class SoundSYS {
public:
    enum Material { MAT_NONE, MAT_GRASS, MAT_STONE };

    static Material materialOf(uint8_t blockId) {
        switch (blockId) {
            case 1:
            case 2:
                return MAT_GRASS;
            case 3:
                return MAT_STONE;
            default:
                return MAT_NONE;
        }
    }

    static void PlayBlock(uint8_t blockId) {
        breakplace(materialOf(blockId));
    }

    static void breakplace(Material m) {
        static const mm_word grass[4] = { SFX_GRASS_DIG1, SFX_GRASS_DIG2, SFX_GRASS_DIG3, SFX_GRASS_DIG4 };
        static const mm_word stone[4] = { SFX_STONE_DIG1, SFX_STONE_DIG2, SFX_STONE_DIG3, SFX_STONE_DIG4 };
        static mm_word lastLoaded = 0xFFFF;

        const mm_word* set = nullptr;
        if (m == MAT_GRASS)      set = grass;
        else if (m == MAT_STONE) set = stone;
        else return;

        const mm_word id = set[rand() % 4];

        if (lastLoaded != 0xFFFF) mmUnloadEffect(lastLoaded);
        mmLoadEffect(id);
        mmEffect(id);
        lastLoaded = id;
    }
};

#endif