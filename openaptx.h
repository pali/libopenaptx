/*
 * Open Source implementation of Audio Processing Technology codec (aptX)
 * Copyright (C) 2018-2020  Pali Rohár <pali.rohar@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef OPENAPTX_H
#define OPENAPTX_H

#define OPENAPTX_MAJOR 0
#define OPENAPTX_MINOR 3
#define OPENAPTX_PATCH 0

#include <stddef.h>
#include <stdint.h>

extern const int aptx_major;
extern const int aptx_minor;
extern const int aptx_patch;

struct aptx_context;

/*
 * Initialize context for aptX codec and reset it.
 * hd = 0 process aptX codec
 * hd = 1 process aptX HD codec
 */
struct aptx_context *aptx_init(int hd);

/*
 * Reset internal state, predictor and parity sync of aptX context.
 * It is needed when going to encode or decode a new stream.
 */
void aptx_reset(struct aptx_context *ctx);

/*
 * Free aptX context initialized by aptx_init().
 */
void aptx_finish(struct aptx_context *ctx);

/*
 * Encodes sequence of 4 raw 24bit signed stereo samples from input buffer with
 * size input_size to aptX audio samples into output buffer with output_size.
 * Return value indicates processed length from input buffer and to written
 * pointer is stored length of encoded aptX audio samples in output buffer.
 * Therefore input buffer must contain sequence of the 24 bytes in format
 * LLLRRRLLLRRRLLLRRRLLLRRR (L-left, R-right) and output buffer would contain
 * encoded sequence of either four bytes (LLRR) of aptX or six bytes (LLLRRR)
 * of aptX HD.
 */
size_t aptx_encode(struct aptx_context *ctx,
                   const unsigned char *input,
                   size_t input_size,
                   unsigned char *output,
                   size_t output_size,
                   size_t *written);

/*
 * Finish encoding of current stream and reset internal state to be ready for
 * encoding or decoding a new stream. Due to aptX latency, last 90 samples
 * (rounded to 92) will be filled by this finish function. When output buffer is
 * too small, this function returns zero, fills buffer only partially, does not
 * reset internal state and subsequent calls continue filling output buffer.
 * When output buffer is large enough, then function returns non-zero value.
 * In both cases into written pointer is stored length of encoded samples.
 */
int aptx_encode_finish(struct aptx_context *ctx,
                       unsigned char *output,
                       size_t output_size,
                       size_t *written);

/*
 * Decodes aptX audio samples in input buffer with size input_size to sequence
 * of raw 24bit signed stereo samples into output buffer with size output_size.
 * Return value indicates processed length from input buffer and to written
 * pointer is stored length of decoded output samples in output buffer.
 * Input buffer must contain seqeunce of four bytes (LLRR) of aptX or six
 * bytes (LLLRRR) of aptX HD samples and output buffer would contain decoded
 * sequence of 24 bytes in format LLLRRRLLLRRRLLLRRRLLLRRR (L-left, R-right)
 * for one aptX sample. Due to aptX latency, output buffer starts filling
 * after 90 samples. When parity check fails then this function stops decoding
 * and returns processed length of input buffer. To detect such failure it is
 * needed to compare return value and input_size. Note that if you have a
 * finite stream then the last two decoded samples from the last decode call
 * does not contain any meaningful value. They are present just because aptX
 * samples are rounded to the multiple by four and latency is 90 samples so
 * last 2 samples are just padding.
 */
size_t aptx_decode(struct aptx_context *ctx,
                   const unsigned char *input,
                   size_t input_size,
                   unsigned char *output,
                   size_t output_size,
                   size_t *written);

/*
 * Auto synchronization variant of aptx_decode() function suitable for partially
 * corrupted continuous stream in which some bytes are missing. All arguments,
 * including return value have same meaning as for aptx_decode() function. The
 * only difference is that there is no restriction for size of input buffer,
 * output buffer must have space for decoding whole input buffer plus space for
 * one additional decoded sample (24 bytes) and the last difference is that this
 * function continue to decode even when parity check fails. When decoding fails
 * this function starts searching for next bytes from the input buffer which
 * have valid parity check (to be synchronized) and then starts decoding again.
 * Into synced pointer is stored 1 if at the end of processing is decoder fully
 * synchronized (in non-error state, with valid parity check) or is stored 0 if
 * decoder is unsynchronized (in error state, without valid parity check). Into
 * dropped pointer is stored number of dropped (not decoded) bytes which were
 * already processed. Functions aptx_decode() and aptx_decode_sync() should not
 * be mixed together.
 */
size_t aptx_decode_sync(struct aptx_context *ctx,
                        const unsigned char *input,
                        size_t input_size,
                        unsigned char *output,
                        size_t output_size,
                        size_t *written,
                        int *synced,
                        size_t *dropped);

/*
 * Finish decoding of current auto synchronization stream and reset internal
 * state to be ready for encoding or decoding a new stream. This function
 * returns number of unprocessed cached bytes which would have been processed
 * by next aptx_decode_sync() call, therefore in time of calling this function
 * it is number of dropped input bytes.
 */
size_t aptx_decode_sync_finish(struct aptx_context *ctx);

#if ENABLE_QUALCOMM_API

/*
 * The following definitions are provided for API compatibility with Qualcomm
 * apt-X encoder library, so libopenaptx can be used as a drop-in replacement
 * for bt-aptX-4.2.2 or aptXHD-1.0.0 libraries found of various Android-based
 * smartphones.
 *
 * Use this API only if you want to be compatible with Qualcomm apt-X library.
 * Otherwise, please, use API provided by libopenaptx library defined above.
 */

typedef struct aptx_context * APTXENC;

APTXENC NewAptxEnc(int swap);
APTXENC NewAptxhdEnc(int swap);

size_t SizeofAptxbtenc(void);
size_t SizeofAptxhdbtenc(void);

int aptxbtenc_init(APTXENC enc, int swap);
int aptxhdbtenc_init(APTXENC enc, int swap);

void aptxbtenc_destroy(APTXENC enc);
void aptxhdbtenc_destroy(APTXENC enc);

int aptxbtenc_encodestereo(APTXENC enc,
                           const int32_t pcmL[4],
                           const int32_t pcmR[4],
                           uint16_t code[2]);
int aptxhdbtenc_encodestereo(APTXENC enc,
                             const int32_t pcmL[4],
                             const int32_t pcmR[4],
                             uint32_t code[2]);

const char *aptxbtenc_build(void);
const char *aptxhdbtenc_build(void);

const char *aptxbtenc_version(void);
const char *aptxhdbtenc_version(void);

#endif

#endif
