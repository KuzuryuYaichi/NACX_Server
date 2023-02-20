#ifndef _STRUCT_DEVICE_WIN_H
#define _STRUCT_DEVICE_WIN_H

#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_WIN_LIB

#include <stdexcept>
#include <string>
#include "xdma_public_win.h"

struct device_file {
    HANDLE h;
    device_file();
    device_file(const std::wstring& path, DWORD accessFlags);
    ~device_file();
};

class xdma_device {
public:
    xdma_device();
    xdma_device(const std::wstring& device_path);
    size_t write_to_engine(void* buffer, size_t size, int channel);
    size_t read_from_engine(void* buffer, size_t size, int channel);
    void write_user_register(long addr, uint32_t value);
    uint32_t read_user_register(long addr);
private:
    static constexpr int MAX_CHANNEL = 2;
    device_file control;
    device_file h2c[MAX_CHANNEL];
    device_file c2h[MAX_CHANNEL];
    device_file user;
    uint32_t read_control_register(long addr);
};

#endif

#endif
