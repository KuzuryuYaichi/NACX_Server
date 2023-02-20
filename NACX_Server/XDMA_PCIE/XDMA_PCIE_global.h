#ifndef XDMA_PCIE_GLOBAL_H
#define XDMA_PCIE_GLOBAL_H

#include <memory>
#include "../StructData.h"
#include "../StructDatas.h"

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#define Q_DECL_EXPORT __declspec(dllexport)
#define Q_DECL_IMPORT __declspec(dllimport)
#define Q_DECL_STDCALL __stdcall
#define Q_DECL_CDECL __cdecl

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define XDMA_PCIE_WIN_LIB

#else

#define Q_DECL_EXPORT __attribute__((visibility("default")))
#define Q_DECL_IMPORT __attribute__((visibility("default")))
#define Q_DECL_STDCALL //__attribute__((__stdcall__))
#define Q_DECL_CDECL __attribute__((__cdecl__))

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define XDMA_PCIE_UNIX_LIB

#endif

typedef void (Q_DECL_STDCALL *P_CXDATA_CALLBACK)(std::shared_ptr<Struct_Datas<StructDataCX>>&);
typedef void (Q_DECL_STDCALL *P_ZCDATA_CALLBACK)(std::shared_ptr<Struct_Datas<StructDataZC>>&);

#if defined(XDMA_PCIE_LIBRARY)
#  define XDMA_PCIE_EXPORT Q_DECL_EXPORT
#else
#  define XDMA_PCIE_EXPORT Q_DECL_IMPORT
#endif

#endif // XDMA_PCIE_GLOBAL_H
