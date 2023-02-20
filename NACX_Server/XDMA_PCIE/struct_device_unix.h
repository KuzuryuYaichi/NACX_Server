#ifndef _STRUCT_DEVICE_UNIX_H
#define _STRUCT_DEVICE_UNIX_H

#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_UNIX_LIB

#include <string>
#include <stdexcept>
#include "xdma_public_unix.h"

struct device_file {
    int fd;
    device_file();
    device_file(const std::string path, int flag);
    device_file(const device_file& device_path) noexcept = delete;
    device_file& operator=(const device_file& device_path) noexcept = delete;
    device_file(device_file&& device_path) noexcept = default;
    device_file& operator=(device_file&& device_path) noexcept = default;
    ~device_file();
};

class xdma_device {
public:
    xdma_device();
    xdma_device(const std::string& device_path);
    ssize_t write_to_engine(void* buffer, size_t size, int channel);
    ssize_t read_from_engine(void* buffer, size_t size, int channel);
    void write_user_register(long addr, uint32_t value);
    uint32_t read_user_register(long addr);

    device_file c2h[3];
    std::string device_path;

protected:
    device_file control;
    device_file h2c[3];
    device_file user;

    uint32_t read_control_register(long addr);
};

#endif

#endif
