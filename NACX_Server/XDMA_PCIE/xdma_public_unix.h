#ifndef _XDMA_PUBLIC_UNIX_H
#define _XDMA_PUBLIC_UNIX_H

#include "XDMA_PCIE_global.h"

#ifdef XDMA_PCIE_UNIX_LIB

#define	XDMA_FILE_USER		"user"
#define	XDMA_FILE_CONTROL	"control"
#define XDMA_FILE_BYPASS	"bypass"

#define	XDMA_FILE_EVENT_0	"event_0"
#define	XDMA_FILE_EVENT_1	"event_1"
#define	XDMA_FILE_EVENT_2	"event_2"
#define	XDMA_FILE_EVENT_3	"event_3"
#define	XDMA_FILE_EVENT_4	"event_4"
#define	XDMA_FILE_EVENT_5	"event_5"
#define	XDMA_FILE_EVENT_6	"event_6"
#define	XDMA_FILE_EVENT_7	"event_7"
#define	XDMA_FILE_EVENT_8	"event_8"
#define	XDMA_FILE_EVENT_9	"event_9"
#define	XDMA_FILE_EVENT_10	"event_10"
#define	XDMA_FILE_EVENT_11	"event_11"
#define	XDMA_FILE_EVENT_12	"event_12"
#define	XDMA_FILE_EVENT_13	"event_13"
#define	XDMA_FILE_EVENT_14	"event_14"
#define	XDMA_FILE_EVENT_15	"event_15"

#define	XDMA_FILE_H2C_0		"h2c_0"
#define	XDMA_FILE_H2C_1		"h2c_1"
#define	XDMA_FILE_H2C_2		"h2c_2"
#define	XDMA_FILE_H2C_3		"h2c_3"

#define	XDMA_FILE_C2H_0		"c2h_0"
#define	XDMA_FILE_C2H_1		"c2h_1"
#define	XDMA_FILE_C2H_2		"c2h_2"
#define	XDMA_FILE_C2H_3		"/c2h_3"

#define XDMA_IOCTL(index) CTL_CODE(FILE_DEVICE_UNKNOWN, index, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_XDMA_GET_VERSION			XDMA_IOCTL(0x0)
#define IOCTL_XDMA_PERF_START			XDMA_IOCTL(0x1)
#define IOCTL_XDMA_PERF_STOP			XDMA_IOCTL(0x2)
#define IOCTL_XDMA_PERF_GET				XDMA_IOCTL(0x3)
#define IOCTL_XDMA_ADDRMODE_GET			XDMA_IOCTL(0x4)
#define IOCTL_XDMA_ADDRMODE_SET			XDMA_IOCTL(0x5)
#define IOCTL_WRITE_KEYHOLE_REGISTER	XDMA_IOCTL(0x6)
#define IOCTL_MAP_BAR					XDMA_IOCTL(0x7)

#endif

#endif

