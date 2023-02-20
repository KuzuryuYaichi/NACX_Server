#ifndef _STREAM_DMA_UNIX_HPP
#define _STREAM_DMA_UNIX_HPP

#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_UNIX_LIB

#include <vector>
#include <system_error>

std::vector<std::string> get_device_paths() {
    std::vector<std::string> res;
    res.push_back("/dev/xdma0_");
    return res;
}

#endif

#endif
