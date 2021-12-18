#include "memory.h"
#include "hook.h"

namespace memory
{
	ModuleInfo get_module_information (const char* module_name, ULONG pid)
	{
		memory::MEMORY instructions = { 0 };
		instructions.pid = pid;
		instructions.action = ACTION_REQBASE;
		instructions.module_name = module_name;
		hook::call_hook(&instructions);

		return instructions.module_info;
	}

}