#include "hook.h"

namespace hook
{
	int wingdi::draw_box(int x, int y, int w, int h, int t, int r, int g, int b)
	{
		::memory::MEMORY instruction;
		instruction.r = r;
		instruction.g = g;
		instruction.b = b;

		instruction.x = x;
		instruction.y = y;
		instruction.w = w;
		instruction.h = h;
		instruction.t = t;
		
		instruction.action = ACTION_DRAWBOX;

		::hook::call_hook(&instruction);

		return 0;
	}

	int wingdi::draw_text(const char* str, int x, int y, int w, int h)
	{
		::memory::MEMORY instruction;

		instruction.x = x;
		instruction.y = y;
		instruction.w = w;
		instruction.h = h;
		
		instruction.text = (char*)str;

		instruction.action = ACTION_DRAWTEXT;
		instruction.output = malloc(sizeof(int));
		::hook::call_hook(&instruction);
		int result = *(int*)instruction.output;
		free(instruction.output);

		return result;
	}

	uint32_t get_process_id(std::string process_name)
	{
		PROCESSENTRY32 processEntry;

		const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL));

		if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}

		processEntry.dwSize = sizeof(MODULEENTRY32);

		while (Process32Next(snapshot_handle.get(), &processEntry) == TRUE)
		{
			if (process_name.compare(processEntry.szExeFile) == NULL)
			{
				return processEntry.th32ProcessID;
			}
		}

		return NULL;
	}

	std::string memory::read_memory_string(UINT_PTR readAddr, ULONG pid)
	{
		std::string result = "";
		char ch = -1;
		size_t counter = 0;

		while (ch != 0 && counter < 1024)
		{
			ch = read_memory<char>(readAddr+counter, pid);
			if (ch != 0) result += ch;
			counter += sizeof(char);
		}

		return result;
	}

	std::wstring memory::read_memory_wstring(UINT_PTR readAddr, ULONG pid)
	{
		std::wstring result = L"";
		wchar_t ch = -1;
		size_t counter = 0;

		while (ch != 0 && counter < 1024)
		{
			ch = read_memory<wchar_t>(readAddr+counter, pid);
			if (ch != 0) result += ch;
			counter += sizeof(wchar_t);
		}

		return result;
	}

	void memory::read_memory(UINT_PTR readAddr, void* output, size_t size, ULONG pid)
	{
		::memory::MEMORY instructions;
		instructions.pid = pid;
		instructions.size = size;

		instructions.addr = readAddr;
		instructions.action = ACTION_READ_KERNEL;
		instructions.output = output;
		::hook::call_hook(&instructions);
	}

	UINT_PTR memory::scan_signature(UINT_PTR readAddr, ULONG size, const byte sig[], const char* mask, ULONG pid)
	{
		ULONG sigSize = strlen(mask);
		byte* buffer = (byte*)malloc(sizeof(byte) * sigSize);
		for (ULONG i = 0; i < size; i++)
		{
			memory::read_memory(readAddr+i, buffer, sigSize * sizeof(byte), pid);

			for (ULONG j = 0; j < sigSize; j++)
			{
				if (mask[j] == '?')
				{
					continue;
				}

				if (buffer[j] != sig[j])
				{
					break;
				}

				if (j == sigSize-1) return readAddr + i;
			}

		}

		return NULL;
	}

}