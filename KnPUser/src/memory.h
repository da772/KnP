#pragma once
#include <Windows.h>

#define ACTION_WRITE_KERNEL 0x01
#define ACTION_READ_KERNEL 0x02
#define ACTION_REQBASE 0x03
#define ACTION_DRAWBOX 0x04
#define ACTION_DRAWTEXT 0x05

namespace memory
{
	typedef struct _moduleInfo
	{
		ULONG64 base;
		ULONG size;
	} ModuleInfo;

	typedef struct _MEMORY
	{
		void* buffer_addr;
		UINT_PTR addr;
		ULONGLONG size;
		ULONG pid;
		ULONG64 action;
		ModuleInfo module_info;
		void* output;
		const char* module_name;
		char* text;
		int r, g, b, x, y, w, h, t;

	} MEMORY;

	ModuleInfo get_module_information(const char* module_name, ULONG pid);
}
