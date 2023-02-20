#ifndef _DLL_IMPORT_H
#define _DLL_IMPORT_H

#include "XDMA_PCIE/XDMA_PCIE_global.h"

void Q_DECL_STDCALL DataCX(std::shared_ptr<Struct_Datas<StructDataCX>>&);

void Q_DECL_STDCALL DataZC(std::shared_ptr<Struct_Datas<StructDataZC>>&);

#endif
