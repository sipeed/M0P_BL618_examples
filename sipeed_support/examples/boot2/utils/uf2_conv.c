#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uf2.h"

#define BLOCK_SIZE 0x100UL

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s file.bin [file.uf2] [is_pika]\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "No such file: %s\n", argv[1]);
        return 1;
    }

    fseek(f, 0L, SEEK_END);
    uint32_t sz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    printf("Opened file: %s, size: %u\n", argv[1], sz);

    const char *outname = argc > 2 ? argv[2] : "flash.uf2";

    FILE *fout = fopen(outname, "wb");

    uf2_block_t blk;
    memset(&blk, 0xff, sizeof(blk));

    blk.magic_start0 = UF2_MAGIC_START0;
    blk.magic_start1 = UF2_MAGIC_START1;
    blk.magic_end = UF2_MAGIC_END;
    blk.flags = UF2_FLAG_FAMILY_ID_PRESENT;
    blk.family_id = FAMILY_ID_MAIXPLAYU4;
    blk.num_blocks = (sz + (sizeof(blk.data) - 1)) / sizeof(blk.data);

    blk.target_addr = (argc > 3) ? 0x40000 : 0;
    printf("Starting from 0x%x\n", blk.target_addr);
    for (blk.block_no = 0; blk.block_no < blk.num_blocks; blk.block_no++) {
        blk.payload_size = fread(blk.data, 1, sizeof(blk.data), f);
        fwrite(&blk, sizeof(blk), 1, fout);
        blk.target_addr += blk.payload_size;
        // clear for next iteration, in case we get a short read
        memset(blk.data, 0xff, sizeof(blk.data));
    }

    fclose(fout);
    fclose(f);

    printf("Wrote %d blocks[%dx%lu+%u] to %s\n", blk.block_no, blk.block_no - 1, sizeof(blk.data), blk.payload_size, outname);
    return 0;
}