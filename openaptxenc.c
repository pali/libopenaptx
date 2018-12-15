/*
 * aptX encoder utility
 * Copyright (C) 2018  Pali Rohár <pali.rohar@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <string.h>

#include <openaptx.h>

static unsigned char input_buffer[512*8*3*2*4];
static unsigned char output_buffer[512*8*6];

int main(int argc, char *argv[])
{
    int i;
    int hd;
    size_t length;
    size_t processed;
    size_t written;
    struct aptx_context *ctx;

    hd = 0;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            fprintf(stderr, "aptX encoder utility %d.%d.%d (using libopenaptx %d.%d.%d)\n", OPENAPTX_MAJOR, OPENAPTX_MINOR, OPENAPTX_PATCH, aptx_major, aptx_minor, aptx_patch);
            fprintf(stderr, "\n");
            fprintf(stderr, "This utility encodes a raw 24 bit signed stereo\n");
            fprintf(stderr, "samples from stdin to aptX or aptX HD on stdout\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Usage:\n");
            fprintf(stderr, "        %s [options]\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "        -h, --help   Display this help\n");
            fprintf(stderr, "        --hd         Encode to aptX HD\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Examples:\n");
            fprintf(stderr, "        %s < sample.s24 > sample.aptx\n", argv[0]);
            fprintf(stderr, "        %s --hd < sample.s24 > sample.aptxhd\n", argv[0]);
            fprintf(stderr, "        sox sample.wav -t raw -r 44.1k -s -3 -c 2 - | %s > sample.aptx\n", argv[0]);
            return 1;
        } else if (strcmp(argv[i], "--hd") == 0) {
            hd = 1;
        } else {
            fprintf(stderr, "%s: Invalid option %s\n", argv[0], argv[i]);
            return 1;
        }
    }

    ctx = aptx_init(hd);
    if (!ctx) {
        fprintf(stderr, "%s: Cannot initialize aptX encoder\n", argv[0]);
        return 1;
    }

    while (!feof(stdin)) {
        length = fread(input_buffer, 1, sizeof(input_buffer), stdin);
        if (ferror(stdin))
            fprintf(stderr, "%s: aptX encoding failed to read input data\n", argv[0]);
        if (length == 0)
            break;
        processed = aptx_encode(ctx, input_buffer, length, output_buffer, sizeof(output_buffer), &written);
        if (processed != length)
            fprintf(stderr, "%s: aptX encoding stopped in the middle of the sample, dropped %u bytes\n", argv[0], (unsigned int)(length-processed));
        if (fwrite(output_buffer, 1, written, stdout) != written) {
            fprintf(stderr, "%s: aptX encoding failed to write encoded data\n", argv[0]);
            break;
        }
        if (processed != length)
            break;
    }

    if (aptx_encode_finish(ctx, output_buffer, sizeof(output_buffer), &written)) {
        if (fwrite(output_buffer, 1, written, stdout) != written)
            fprintf(stderr, "%s: aptX encoding failed to write encoded data\n", argv[0]);
    }

    aptx_finish(ctx);
    return 0;
}
