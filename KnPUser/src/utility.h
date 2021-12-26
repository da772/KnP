#pragma once
#include <windows.h>
#include <string>

#define EXEC_FAILURE_CODE 1235678;

namespace utility {

	typedef struct {
		std::wstring out;
		std::wstring err;
		DWORD exit_code;
	} exec_info ;

	void exec_command(const std::wstring& cmd, exec_info* info);
	std::wstring get_exe_dir();
}