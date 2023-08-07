#ifndef GLOBAL_H
#define GLOBAL_H

static constexpr int BAND_WIDTH_MHZ = 20;
static constexpr int BAND_WIDTH_KHZ = BAND_WIDTH_MHZ * 1e3;
static constexpr int BAND_WIDTH_HZ = BAND_WIDTH_KHZ * 1e3;
static constexpr int HALF_BAND_WIDTH_MHZ = BAND_WIDTH_MHZ / 2;
static constexpr int HALF_BAND_WIDTH_KHZ = BAND_WIDTH_KHZ / 2;
static constexpr int HALF_BAND_WIDTH_HZ = BAND_WIDTH_HZ / 2;

static constexpr long long MIN_FREQ_MHZ = 200;
static constexpr long long MAX_FREQ_MHZ = 500;
static constexpr long long MIN_FREQ_KHZ = MIN_FREQ_MHZ * 1e3;
static constexpr long long MAX_FREQ_KHZ = MAX_FREQ_MHZ * 1e3;
static constexpr long long MIN_FREQ_HZ = MIN_FREQ_KHZ * 1e3;
static constexpr long long MAX_FREQ_HZ = MAX_FREQ_KHZ * 1e3;
static constexpr long long CENTER_FREQ_MHZ = (MIN_FREQ_MHZ + MAX_FREQ_MHZ) / 2;
static constexpr long long CENTER_FREQ_KHZ = (MIN_FREQ_KHZ + MAX_FREQ_KHZ) / 2;
static constexpr long long CENTER_FREQ_HZ = (MIN_FREQ_HZ + MAX_FREQ_HZ) / 2;

static constexpr int CHANNEL_NUM = 4;
static constexpr int ZC_NB_CHANNEL_NUM = 4;
static constexpr int ZC_NB_CHANNEL_PAGE = 4;
static constexpr int ZC_NB_CHANNEL_NUMS = ZC_NB_CHANNEL_NUM * ZC_NB_CHANNEL_PAGE;
static constexpr short AMPL_OFFSET = -160;
static constexpr short BASE_DIRECTION = 130 * 10;
static constexpr int MIN_AMPL = -160;
static constexpr int MAX_AMPL = 0;
static constexpr int MIN_PHASE = -180;
static constexpr int MAX_PHASE = 180;
static constexpr int MARKER_MIN_DIRECTION = 0;
static constexpr int MARKER_MAX_DIRECTION = 360;
static constexpr int WATERFALL_DEPTH = 50;
static constexpr int CONFIDENCE_MINIMUM = 50;
static constexpr int CONFIDENCE_MAXIMUM = 100;
static constexpr int POINTS_ANALYZE_MINIMUM = 10;
static constexpr int DECIMALS_PRECISION = 6;

#endif