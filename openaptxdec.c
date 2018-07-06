/*
 * aptX decoder utility
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

static unsigned char input_buffer[512*8*6];
static unsigned char output_buffer[512*8*3*2*4*6/4];

int main(int argc, char *argv[])
{
    int i;
    int hd;
    size_t length;
    size_t offset;
    size_t sample_size;
    size_t process_size;
    size_t processed;
    size_t written;
    struct aptx_context *ctx;
    unsigned int failed;

    hd = 0;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            fprintf(stderr, "aptX decoder utility\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "This utility decodes aptX or aptX HD audio stream\n");
            fprintf(stderr, "from stdin to a raw 24 bit signed stereo on stdout\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "When input is damaged it tries to synchronize and recover\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Usage:\n");
            fprintf(stderr, "        %s [options]\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "        -h, --help   Display this help\n");
            fprintf(stderr, "        --hd         Decode from aptX HD\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Examples:\n");
            fprintf(stderr, "        %s < sample.aptx > sample.s24\n", argv[0]);
            fprintf(stderr, "        %s --hd < sample.aptxhd > sample.s24\n", argv[0]);
            fprintf(stderr, "        %s < sample.aptx | play -t raw -r 44.1k -s -3 -c 2 -\n", argv[0]);
            return 1;
        } else if (strcmp(argv[i], "--hd") == 0) {
            hd = 1;
        } else {
            fprintf(stderr, "%s: Invalid option %s\n", argv[0], argv[i]);
            return 1;
        }
    }

    /* every eight sample contains synchronization parity check */
    sample_size = 8 * (hd ? 6 : 4);

    ctx = aptx_init(hd);
    if (!ctx) {
        fprintf(stderr, "%s: Cannot initialize aptX encoder\n", argv[0]);
        return 1;
    }

    failed = 0;
    offset = 0;

    /* Try to guess type of input stream based on the first six bytes
     * Encoder produces fixed first sample because aptX predictor has fixed values */
    length = fread(input_buffer, 1, 6, stdin);
    if (length >= 4 && memcmp(input_buffer, "\x4b\xbf\x4b\xbf", 4) == 0) {
        if (hd)
            fprintf(stderr, "%s: Input looks like aptX audio stream (not aptX HD), try without --hd\n", argv[0]);
    } else if (length >= 6 && memcmp(input_buffer, "\x73\xbe\xff\x73\xbe\xff", 6) == 0) {
        if (!hd)
            fprintf(stderr, "%s: Input looks like aptX HD audio stream, try with --hd\n", argv[0]);
    } else {
        fprintf(stderr, "%s: Input does not look like aptX nor aptX HD audio stream\n", argv[0]);
    }

    while (length > 0 || !feof(stdin)) {
        /* For decoding we need at least eight samples for synchronization */
        if (length < sample_size && !feof(stdin)) {
            if (length > 0)
                memmove(input_buffer, input_buffer + offset, length);
            offset = 0;
            length += fread(input_buffer + length, 1, sizeof(input_buffer) - length, stdin);
            if (ferror(stdin))
                fprintf(stderr, "%s: aptX encoding failed to read input data\n", argv[0]);
        }

        process_size = length;

        /* Always process multiple of the 8 samples (expect last) for synchronization support */
        if (length >= sample_size)
            process_size -= process_size % sample_size;

        /* When decoding previous samples failed, reset internal state, predictor and state of the synchronization parity */
        if (failed > 0)
            aptx_reset(ctx);

        processed = aptx_decode(ctx, input_buffer + offset, process_size, output_buffer, sizeof(output_buffer), &written);

        if (processed > sample_size && failed > 0) {
            fprintf(stderr, "%s: ... synchronization successful, dropped %u bytes\n", argv[0], failed);
            failed = 0;
        }

        /* If we have not decoded all supplied samples then decoding failed */
        if (processed != process_size) {
            if (failed == 0) {
                if (length < sample_size)
                    fprintf(stderr, "%s: aptX decoding stopped in the middle of the sample, dropped %u bytes\n", argv[0], (unsigned int)(length-processed));
                else
                    fprintf(stderr, "%s: aptX decoding failed, trying to synchronize ...\n", argv[0]);
            }
            if (length >= sample_size)
                failed++;
            else if (failed > 0)
                failed += length;
            if (processed <= sample_size) {
                /* If we have not decoded at least 8 samples (with proper parity check)
                 * drop decoded buffer and try decoding again on next byte */
                processed = 1;
                written = 0;
            }
        } else if (length < sample_size) {
            fprintf(stderr, "%s: aptX decoding stopped in the middle of the sample\n", argv[0]);
        }

        if (written > 0) {
            if (fwrite(output_buffer, 1, written, stdout) != written) {
                fprintf(stderr, "%s: aptX decoding failed to write decoded data\n", argv[0]);
                failed = 0;
                length = 0;
                break;
            }
        }

        if (length < sample_size)
            break;

        length -= processed;
        offset += processed;
    }

    if (failed > 0)
        fprintf(stderr, "%s ... synchronization failed, dropped %u bytes\n", argv[0], failed);

    aptx_finish(ctx);
    return 0;
}
