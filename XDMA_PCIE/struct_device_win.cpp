#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_WIN_LIB

#include "struct_device_win.h"

device_file::device_file() {}

device_file::device_file(const std::wstring& path, DWORD accessFlags) {
    h = CreateFile(path.c_str(), accessFlags, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("CreateFile control failed: " + std::to_string(GetLastError()));
    }
}

device_file::~device_file() {
    CloseHandle(h);
}

inline static uint32_t bit(uint32_t n) {
    return (1 << n);
}

inline static bool is_bit_set(uint32_t x, uint32_t n) {
    return (x & bit(n)) == bit(n);
}

xdma_device::xdma_device() {}

xdma_device::xdma_device(const std::wstring& device_path) :
    control(device_path + XDMA_FILE_CONTROL, GENERIC_READ | GENERIC_WRITE),
    //user(device_path + XDMA_FILE_USER, GENERIC_READ | GENERIC_WRITE),
    h2c{ device_file(device_path + XDMA_FILE_H2C_0, GENERIC_WRITE),
        device_file(device_path + XDMA_FILE_H2C_1, GENERIC_WRITE) },
    c2h{ device_file(device_path + XDMA_FILE_C2H_0, GENERIC_READ),
        device_file(device_path + XDMA_FILE_C2H_1, GENERIC_READ) }
{
    if (!is_bit_set(read_control_register(0x0), 15) || !is_bit_set(read_control_register(0x1000), 15)) {
        throw std::runtime_error("XDMA engines h2c_0 and/or c2h_0 are not streaming engines!");
    }
}

uint32_t xdma_device::read_control_register(long addr) {
    uint32_t value = 0;
    size_t num_bytes_read;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(control.h, addr, NULL, FILE_BEGIN)) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(GetLastError()));
    }
    if (!ReadFile(control.h, (LPVOID)&value, 4, (LPDWORD)&num_bytes_read, NULL)) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(GetLastError()));
    }
    return value;
}

uint32_t xdma_device::read_user_register(long addr) {
    uint32_t value = 0;
    size_t num_bytes_read;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(user.h, addr, NULL, FILE_BEGIN)) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(GetLastError()));
    }
    if (!ReadFile(user.h, (LPVOID)&value, 4, (LPDWORD)&num_bytes_read, NULL)) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(GetLastError()));
    }
    return value;
}

void xdma_device::write_user_register(long addr, uint32_t value) {
    size_t num_bytes_read;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(user.h, addr, NULL, FILE_BEGIN)) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(GetLastError()));
    }
    if (!WriteFile(user.h, (LPVOID)&value, 4, (LPDWORD)&num_bytes_read, NULL)) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(GetLastError()));
    }
}

size_t xdma_device::write_to_engine(void* buffer, size_t size, int channel) {
    unsigned long num_bytes_written;
    if (!WriteFile(h2c[channel].h, buffer, (DWORD)size, &num_bytes_written, NULL)) {
        throw std::runtime_error("Failed to write to stream! " + std::to_string(GetLastError()));
    }
    return num_bytes_written;
}

size_t xdma_device::read_from_engine(void* buffer, size_t size, int channel) {
    unsigned long num_bytes_read;
    if (!ReadFile(c2h[channel].h, buffer, (DWORD)size, &num_bytes_read, NULL)) {
        throw std::runtime_error("Failed to read from stream! " + std::to_string(GetLastError()));
    }
    return num_bytes_read;
}

#endif
