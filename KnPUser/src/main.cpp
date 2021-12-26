#include <windows.h>
#include "memory.h"
#include "hook.h"
#include <iostream>
#include <assert.h>
#include <zlib.h>
#include "generated/kdmapper.h"
#include "generated/knpdriver.h"
#include <fstream>
#include "utility.h"
#include <stdio.h>
#include "math.h"

math::vec3d_f screen;
math::vec4d_f clipCoords;
math::vec4d_f NDC;

int main()
{
	unsigned char* result = (unsigned char*)malloc(kdmapper::size*sizeof(unsigned char));
	assert(result);
	uint64_t size = kdmapper::size;
	std::cout << "UnCompressing " << uncompress(result, (uLongf*)&size, (unsigned char*)kdmapper::data, size) << std::endl;
	
	const std::wstring kdmap_path = utility::get_exe_dir() + L"\\outputBinCompressed.exe";

	std::ofstream wf(kdmap_path, std::ios::out | std::ios::binary);
	wf.write((const char*)result, size);
	wf.close();
	free(result);

	result = (unsigned char*)malloc(knpdriver::size * sizeof(unsigned char));
	size = knpdriver::size;
	std::cout << "UnCompressing " << uncompress(result, (uLongf*)&size, (unsigned char*)knpdriver::data, size) << std::endl;

	const std::wstring driver_path = utility::get_exe_dir() + L"\\KnPDriver.sys";

	wf = std::ofstream(driver_path, std::ios::out | std::ios::binary);
	wf.write((const char*)result, size);
	wf.close();

	free(result);

	std::wstring command = utility::get_exe_dir() + L"\\outputBinCompressed.exe " + driver_path;

	std::wcout << command << std::endl;

	utility::exec_info info;
	utility::exec_command(command, &info);

	if (info.exit_code != 0)
	{
		// Failed
	}

	std::wcout << "EXIT CODE: " << (int)info.exit_code << std::endl;

	_wremove(driver_path.c_str());
	_wremove(kdmap_path.c_str());

	HWND hwnd = FindWindowA(NULL, "AssaultCube");
	
	if (!hwnd)
	{
		return -1;
	}
	
	RECT rect;
	GetWindowRect(hwnd, &rect);

	DWORD pID = hook::get_process_id(L"ac_client.exe");

	memory::ModuleInfo ac_client = memory::get_module_information("ac_client.exe", pID);
	DWORD baseAddr = 0x00400000+ 0x0010F4F4;
	std::cout << pID << std::endl;
	std::cout << ac_client.base << std::endl;
	std::cout << ac_client.size << std::endl;

	const uint64_t ofHealth = 0xF8ULL;
	const uint64_t ofName = 0x225ULL;
	const uint64_t ofSpeed = 0x08ULL;
	const uint64_t ofViewMat = 0x501AE8ULL;
	const uint64_t ofEntity = 0x4ULL;
	const uint64_t ofPlayer = 0x509B74ULL;
	const uint64_t ofXPos = 0x4ULL;
	const uint64_t ofTeam = 0x32CULL;
	const uint64_t ofAmmo = 0x0150ULL;

	const uint32_t playerCount = ::hook::memory::read_memory<uint32_t>(baseAddr + 0xCULL, pID);

	DWORD clientAddr = ::hook::memory::read_memory<DWORD>(baseAddr, pID);
	DWORD entityAddr = ::hook::memory::read_memory<DWORD>(baseAddr+ofEntity, pID);

	UINT_PTR sig = ::hook::memory::scan_signature(ac_client.base, ac_client.size, { 0x00 }, "", pID);

	const uint32_t clientTeam = ::hook::memory::read_memory<uint32_t>(clientAddr + ofTeam, pID);

	using vec3d_f = math::vec3d_f;

	while (true)
	{
		float matrix[16];
		::hook::memory::read_memory(ofViewMat, matrix, sizeof(matrix), pID);

		vec3d_f playerPos = ::hook::memory::read_memory<vec3d_f>(clientAddr + ofEntity + ofXPos, pID);
		int health = ::hook::memory::read_memory<int>(clientAddr + ofHealth, pID);

		if (health < 100)
		{
			::hook::memory::write_memory<int>(clientAddr + ofHealth, 100, pID);
		}

		int ammo = ::hook::memory::read_memory<int>(clientAddr + ofAmmo, pID);

		if (ammo < 100)
		{
			::hook::memory::write_memory<int>(clientAddr + ofAmmo, 100, pID);
		}

		std::cout << "HEALTH: " << health << std::endl;

		for (uint32_t i = 0; i < playerCount; i++)
		{
			DWORD ent = ::hook::memory::read_memory<DWORD>(entityAddr + (ofEntity * i), pID);
			const vec3d_f pos = ::hook::memory::read_memory<vec3d_f>(ent + ofXPos, pID);
			const uint32_t team = ::hook::memory::read_memory<uint32_t>(ent + ofTeam, pID);
			vec3d_f color = {255.f, 0.f ,0.f};

			if (team == clientTeam)
			{
				color.x = 0.f;
				color.y = 255.f;
			}

			if (WorldToScreen(pos, clipCoords, NDC, screen, matrix, 720, 480))
			{
				float xPos = screen.x + rect.left - (50 / 2);
				float yPos = screen.y + rect.top + (50 / 2);
				//if (screen.x >= rect.left && screen.x <= rect.right && screen.y <= rect.bottom && screen.y >= rect.top)
				::hook::wingdi::draw_box((int)xPos, (int)yPos, 50, 50, 2, (int)color.x, (int)color.y, (int)color.z);
			}
		}
		//Sleep(16);
	}

	
	::hook::memory::write_memory<char>(clientAddr + ofSpeed, 10, pID);




	return 0;
}