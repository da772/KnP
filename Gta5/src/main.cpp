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
#include <thread>
#include <zlib.h>

#include "offsets.h"

using namespace IEVector;

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

	DWORD pID = KnP::memory::get_process_id(L"GTA5.exe");;

	if (!pID)
	{
		return -1;
	}
	KnP::memory::ModuleInfo ac_client = KnP::hook::memory::get_module_information("GTA5.exe", pID);
	
	std::cout << pID << std::endl;
	std::cout << ac_client.base << std::endl;
	std::cout << ac_client.size << std::endl;

	constexpr char nativeHandlersSig[] = "48 8D 0D ?? ?? ?? ?? 48 8B 14 FA E8 ?? ?? ?? ?? 48 85 C0 75 0A";
	uintptr_t nativeHandlers = KnP::hook::memory::scan_signature(ac_client.base, ac_client.size, nativeHandlersSig, pID);

	std::cout << "NATIVE HANDLER: " << nativeHandlers << std::endl;

	uintptr_t world = KnP::hook::memory::read_memory<uintptr_t>(ac_client.base+ 0x0256A878, pID);
	uintptr_t player_ptr = KnP::hook::memory::read_memory<uintptr_t>(world + OFFSET_PLAYER, pID);
	uintptr_t player_info_ptr = KnP::hook::memory::read_memory<uintptr_t>(player_ptr + OFFSET_PLAYER_INFO, pID);

	float health = KnP::hook::memory::read_memory<float>(player_ptr+ OFFSET_ENTITY_HEALTH, pID);
	

	while (true)
	{
		health = KnP::hook::memory::read_memory<float>(player_ptr + OFFSET_ENTITY_HEALTH, pID); // player health
		std::cout << "Health: " << health << std::endl;
		KnP::hook::memory::write_memory<float>(player_ptr + OFFSET_ENTITY_HEALTH, 200.f, pID); // player health
		float speed = KnP::hook::memory::read_memory<float>(player_info_ptr + OFFSET_PLAYER_INFO_RUN_SPD, pID);// player speed
		std::cout << "Speed: " << speed << std::endl;
		KnP::hook::memory::write_memory<float>(player_info_ptr + OFFSET_PLAYER_INFO_RUN_SPD, 2.f, pID);// Player speed
		std::cout << "Player wanted: " << KnP::hook::memory::read_memory<int>(player_info_ptr + OFFSET_PLAYER_INFO_WANTED, pID) << std::endl;
		KnP::hook::memory::write_memory<int>(player_info_ptr + OFFSET_PLAYER_INFO_WANTED, 0, pID);
		uintptr_t vehicle_ptr = KnP::hook::memory::read_memory<uintptr_t>(player_ptr + OFFSET_PLAYER_VEHICLE, pID); // vehicle
		
		KnP::hook::memory::write_memory<float>(KnP::hook::memory::read_memory<uintptr_t>(vehicle_ptr + OFFSET_VEHICLE_HANDLING, pID) + OFFSET_VEHICLE_HANDLING_ACCELERATION, 1000.f, pID); // vehicle acceleration
		//KnP::hook::memory::write_memory<float>(KnP::hook::memory::read_memory<uintptr_t>(vehicle_ptr + OFFSET_VEHICLE_HANDLING, pID) + OFFSET_VEHICLE_HANDLING_MASS, 10.f, pID); // vehicle mass

		Sleep(1500);
	}


	return 0;
}