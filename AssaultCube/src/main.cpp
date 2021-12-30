#include <windows.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>

#include <KnPCore/IEVector.hpp>
#include <KnPCore/memory.h>
#include <KnPCore/hook.h>
#include <KnPCore/utility.h>
#include <KnPCore/maths.h>
#include <KnPCore/data.h>

#include <tchar.h>

#include <zlib.h>

using namespace IEVector;

v3 screen;
v4 clipCoords;
v4 NDC;

int main()
{
	KnP::data::knpdriver driver = KnP::data::create_driver();
	KnP::data::kdmapper mapper = KnP::data::create_mapper();

	unsigned char first[] = { 0x49, 0xBA }; // mov r10
	unsigned char last[] = { 0x90, 0x90, 0x41, 0xFF, 0xE2 }; // nop nop jmp r10
	
	bool mod = KnP::data::modify_driver_bytecode(&driver, first, sizeof(first), last, sizeof(last)) &&
		KnP::data::modify_driver_module_name(&driver, "NtDxgkGetTrackedWorkloadStatistics") &&
		KnP::data::modify_driver_module_location(&driver, "\\SystemRoot\\System32\\drivers\\dxgkrnl.sys");

	
	if (!mod)
	{
		return -1;
	}

	KnP::utility::exec_info info = KnP::data::finalize_data(&driver, &mapper);

	HWND hwnd = FindWindowA(NULL, "AssaultCube");
	
	if (!hwnd)
	{
		return -1;
	}
	
	RECT rect;
	GetWindowRect(hwnd, &rect);
	DWORD pID = KnP::memory::get_process_id(L"ac_client.exe");;

	if (!pID)
	{
		return -1;
	}

	KnP::memory::ModuleInfo ac_client = KnP::hook::memory::get_module_information("ac_client.exe", pID);
	DWORD baseAddr = 0x00400000+ 0x0010F4F4;
	std::cout << pID << std::endl;
	std::cout << ac_client.base << std::endl;
	std::cout << ac_client.size << std::endl;

	constexpr uint64_t ofHealth = 0xF8ULL;
	constexpr uint64_t ofName = 0x225ULL;
	constexpr uint64_t ofSpeed = 0x08ULL;
	constexpr uint64_t ofViewMat = 0x501AE8ULL;
	constexpr uint64_t ofEntity = 0x4ULL;
	constexpr uint64_t ofPlayer = 0x509B74ULL;
	constexpr uint64_t ofXPos = 0x4ULL;
	constexpr uint64_t ofTeam = 0x32CULL;
	constexpr uint64_t ofAmmo = 0x0150ULL;
	constexpr uint64_t ofPlayerCount = 0xCULL;

	const uint32_t playerCount = KnP::hook::memory::read_memory<uint32_t>(baseAddr + ofPlayerCount, pID);

	DWORD clientAddr = KnP::hook::memory::read_memory<DWORD>(baseAddr, pID);
	DWORD entityAddr = KnP::hook::memory::read_memory<DWORD>(baseAddr+ofEntity, pID);

	//UINT_PTR sig = ::hook::memory::scan_signature(ac_client.base, ac_client.size, "AE ?? AB ??", pID);

	const uint32_t clientTeam = KnP::hook::memory::read_memory<uint32_t>(clientAddr + ofTeam, pID);

	while (true)
	{
		float matrix[16];
		KnP::hook::memory::read_memory(ofViewMat, matrix, sizeof(matrix), pID);

		v3 playerPos = KnP::hook::memory::read_memory<v3>(clientAddr + ofEntity + ofXPos, pID);
		int health = KnP::hook::memory::read_memory<int>(clientAddr + ofHealth, pID);

		if (health < 100)
		{
			KnP::hook::memory::write_memory<int>(clientAddr + ofHealth, 100, pID);
		}

		int ammo = KnP::hook::memory::read_memory<int>(clientAddr + ofAmmo, pID);

		if (ammo < 100)
		{
			KnP::hook::memory::write_memory<int>(clientAddr + ofAmmo, 100, pID);
		}

		std::cout << "HEALTH: " << health << std::endl;

		for (uint32_t i = 0; i < playerCount; i++)
		{
			DWORD ent = KnP::hook::memory::read_memory<DWORD>(entityAddr + (ofEntity * i), pID);
			const int hp = KnP::hook::memory::read_memory<int>(ent + ofHealth, pID);
			if (hp <= 0)
			{
				continue;
			}

			const v3 pos = KnP::hook::memory::read_memory<v3>(ent + ofXPos, pID);
			const uint32_t team = KnP::hook::memory::read_memory<uint32_t>(ent + ofTeam, pID);
			v3 color = {255.f, 0.f ,0.f};

			if (team == clientTeam)
			{
				color.x = 0.f;
				color.y = 255.f;
			}

			if (KnP::math::WorldToScreen(pos, clipCoords, NDC, screen, matrix, 720, 480))
			{
				float xPos = screen.x + rect.left - (50 / 2);
				float yPos = screen.y + rect.top + (50 / 2);
				//if (screen.x >= rect.left && screen.x <= rect.right && screen.y <= rect.bottom && screen.y >= rect.top)
				KnP::hook::wingdi::draw_box((int)xPos, (int)yPos, 50, 50, 2, (int)color.x, (int)color.y, (int)color.z);
			}
		}
		//Sleep(16);
	}

	
	KnP::hook::memory::write_memory<char>(clientAddr + ofSpeed, 10, pID);




	return 0;
}