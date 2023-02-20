#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_UNIX_LIB

#include "struct_device_unix.h"

device_file::device_file() {}

device_file::device_file(const std::string path, int flag) {
    fd = open(path.c_str(), flag);
	if (fd < 0) {
		perror("open device");
	}
}

device_file::~device_file() {
    if (fd > 0)
        close(fd);
}

inline static uint32_t bit(uint32_t n) {
    return (1 << n);
}

inline static bool is_bit_set(uint32_t x, uint32_t n) {
    return (x & bit(n)) == bit(n);
}

xdma_device::xdma_device() {}

xdma_device::xdma_device(const std::string& device_path) :
    //user(device_path + XDMA_FILE_USER, O_RDWR),
    c2h {
        device_file(device_path + XDMA_FILE_C2H_0, O_RDONLY),
        device_file(device_path + XDMA_FILE_C2H_1, O_RDONLY)
    },
    device_path(device_path),
    control(device_path + XDMA_FILE_CONTROL, O_RDWR)
{
//    if (!is_bit_set(read_control_register(0x0), 15) || !is_bit_set(read_control_register(0x1000), 15)) {
//        throw std::runtime_error("XDMA engines h2c_0 and/or c2h_0 are not streaming engines!");
//    }
}

uint32_t xdma_device::read_control_register(long addr) {
    uint32_t value = 0;
    if (lseek(control.fd, addr, SEEK_SET) < 0) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(errno));
    }
    if (read(control.fd, &value, 4) < 0) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(errno));
    }
    return value;
}

uint32_t xdma_device::read_user_register(long addr) {
    uint32_t value = 0;
    if (lseek(user.fd, addr, SEEK_SET) < 0) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(errno));
    }
    if (read(user.fd, &value, 4) < 0) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(errno));
    }
    return value;
}

void xdma_device::write_user_register(long addr, uint32_t value) {
    if (lseek(user.fd, addr, SEEK_SET) < 0) {
        throw std::runtime_error("SetFilePointer failed: " + std::to_string(errno));
    }
    if (write(user.fd, &value, 4) < 0) {
        throw std::runtime_error("ReadFile failed:" + std::to_string(errno));
    }
}

ssize_t xdma_device::write_to_engine(void* buffer, size_t size, int channel) {
    ssize_t num_bytes_written = write(h2c[channel].fd, buffer, size);
	if (num_bytes_written < 0) {
		return -EIO;
	}
    return num_bytes_written;
}

ssize_t xdma_device::read_from_engine(void* buffer, size_t size, int channel) {
    ssize_t num_bytes_read = read(c2h[channel].fd, buffer, size);
    return num_bytes_read;
}

#endif
