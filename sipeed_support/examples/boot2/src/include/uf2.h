#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

// All entries are little endian.

#define UF2_MAGIC_START0                0x0A324655UL // "UF2\n"
#define UF2_MAGIC_START1                0x9E5D5157UL // Randomly selected
#define UF2_MAGIC_END                   0x0AB16F30UL // Ditto

#define UF2_FLAG_NOT_MAIN_FLASH         0x00000001UL
#define UF2_FLAG_FILE_CONTAINER         0x00001000UL
#define UF2_FLAG_FAMILY_ID_PRESENT      0x00002000UL
#define UF2_FLAG_MD5_CHECKSUM_PRESENT   0x00004000UL
#define UF2_FLAG_EXTENSION_TAGS_PRESENT 0x00008000UL

#define FAMILY_ID_M0SENSE               0x11de784a
#define FAMILY_ID_MAIXPLAYU4            0x4b684d71

typedef struct uf2_block {
    // 32 byte header
    uint32_t magic_start0;
    uint32_t magic_start1;
    uint32_t flags;
    uint32_t target_addr;
    uint32_t payload_size;
    uint32_t block_no;
    uint32_t num_blocks;
    union {
        uint32_t family_id;
        uint32_t file_size;
    };

    // raw data;
    uint8_t data[476];

    // store magic also at the end to limit damage from partial block reads
    uint32_t magic_end;
} uf2_block_t;

// static_assert(sizeof(struct uf2_block) == 512, "uf2_block not sector sized");

static inline bool uf2_is_uf2_block(uf2_block_t *puf2_blk)
{
    return UF2_MAGIC_START0 == puf2_blk->magic_start0 &&
           UF2_MAGIC_START1 == puf2_blk->magic_start1 &&
           UF2_MAGIC_END == puf2_blk->magic_end;
}
