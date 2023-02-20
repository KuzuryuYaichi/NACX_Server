#ifndef _STRUCT_DATAS_H
#define _STRUCT_DATAS_H

#include <memory>

template<typename T>
struct Struct_Datas
{
	unsigned int PACK_NUM;
	T* ptr = nullptr;

	Struct_Datas(unsigned int PACK_NUM): ptr(new T[PACK_NUM]), PACK_NUM(PACK_NUM) {}

	~Struct_Datas()
	{
		if (ptr != nullptr)
			delete[] ptr;
	}
};

#endif
