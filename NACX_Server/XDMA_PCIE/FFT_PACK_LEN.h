#ifndef FFT_PACK_LEN_H
#define FFT_PACK_LEN_H

static constexpr int FFT_NUM_MAP[] = { 400, 800, 1600, 3200, 200 };
static constexpr int FFT_PACK_NUM[] = { 8, 4, 2, 1, 16 };
static constexpr int TOTAL_CHANNEL = 57;

static constexpr int FFT_TOTAL_PACK[] = { TOTAL_CHANNEL * FFT_PACK_NUM[0],
                                          TOTAL_CHANNEL * FFT_PACK_NUM[1],
                                          TOTAL_CHANNEL * FFT_PACK_NUM[2],
                                          TOTAL_CHANNEL * FFT_PACK_NUM[3],
                                          TOTAL_CHANNEL * FFT_PACK_NUM[4] };

static constexpr int PACK_LEN_PER_PACK[] = { FFT_NUM_MAP[0] * 2 + 10 * 8,
                                             FFT_NUM_MAP[1] * 2 + 10 * 8,
                                             FFT_NUM_MAP[2] * 2 + 10 * 8,
                                             FFT_NUM_MAP[3] * 2 + 10 * 8,
                                             FFT_NUM_MAP[4] * 2 + 10 * 8 };

static constexpr int LEN_PER_PACK[] = { TOTAL_CHANNEL * PACK_LEN_PER_PACK[0],
                                        TOTAL_CHANNEL * PACK_LEN_PER_PACK[1],
                                        TOTAL_CHANNEL * PACK_LEN_PER_PACK[2],
                                        TOTAL_CHANNEL * PACK_LEN_PER_PACK[3],
                                        TOTAL_CHANNEL * PACK_LEN_PER_PACK[4] };

static constexpr int BLOCK_LEN_PER_PACK[] = { LEN_PER_PACK[0] * FFT_PACK_NUM[0],
                                              LEN_PER_PACK[1] * FFT_PACK_NUM[1],
                                              LEN_PER_PACK[2] * FFT_PACK_NUM[2],
                                              LEN_PER_PACK[3] * FFT_PACK_NUM[3],
                                              LEN_PER_PACK[4] * FFT_PACK_NUM[4] };

#endif // FFT_PACK_LEN_H
