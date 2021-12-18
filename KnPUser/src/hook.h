#pragma once
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <tlhelp32.h>
#include <string_view>
#include <iostream>
#include "memory.h"

namespace hook {


	template<typename ... Args>
	inline uint64_t call_hook(const Args ... args)
	{
		LoadLibraryA("user32.dll");
		void* hooked_func = GetProcAddress(LoadLibrary("win32u.dll"), "NtDxgkGetTrackedWorkloadStatistics");

		const auto func = static_cast<uint64_t(__stdcall*)(Args...)>(hooked_func);

		return func(args ...);
	}

	struct HandleDisposer
	{
		using pointer = HANDLE;

		void operator()(HANDLE handle)const
		{
			if (handle != NULL && handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(handle);
			}
		}
	};

	using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

	uint32_t get_process_id(std::string process_name);

	namespace wingdi
	{
		int draw_box(int x, int y, int w, int h, int t, int r, int g, int b);
		int draw_text(const char* str, int x, int y, int w, int h);
	}

	namespace memory
	{

		std::string read_memory_string(UINT_PTR readAddr, ULONG pid);
		std::wstring read_memory_wstring(UINT_PTR readAddr, ULONG pid);
		void read_memory(UINT_PTR readAddr, void* output, size_t size, ULONG pid);
		UINT_PTR scan_signature(UINT_PTR readAddr, ULONG size, const byte sig[], const char* mask, ULONG pid);

		template<class T>
		inline T read_memory(UINT_PTR readAddr, ULONG pid)
		{
			T result;

			::memory::MEMORY instructions;
			instructions.pid = pid;
			instructions.size = sizeof(T);

			instructions.addr = readAddr;
			instructions.action = ACTION_READ_KERNEL;
			instructions.output = &result;
			::hook::call_hook(&instructions);

			return result;
		}

		
		template<typename T>
		inline void write_memory(UINT_PTR writeAddr, const T& obj, ULONG pid)
		{
			void* buffer = malloc(sizeof(T));
			memcpy(buffer, &obj, sizeof(T));

			::memory::MEMORY instructions;
			instructions.pid = pid;
			instructions.size = sizeof(T);

			instructions.addr = writeAddr;
			instructions.action = ACTION_WRITE_KERNEL;
			instructions.buffer_addr = buffer;
			instructions.size = sizeof(T);
			::hook::call_hook(&instructions);

			free(buffer);
		}
	}

}