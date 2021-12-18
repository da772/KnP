#include <windows.h>
#include "memory.h"
#include "hook.h"
#include <iostream>
#include <assert.h>

typedef struct
{
	float x, y;
} vec2d_f;

typedef struct
{
	float x, y, z;
} vec3d_f;

typedef struct
{
	float x, y, z, w;
} vec4d_f;

vec3d_f screen;
vec4d_f clipCoords;
vec4d_f NDC;

bool WorldToScreen(vec3d_f pos, float matrix[16], int windowWidth, int windowHeight)
{

	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
	clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
	clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
	clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

	if (clipCoords.w < 0.1f)
	{
		return false;
	}

	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

	return true;
}

float GetDistance3D(vec3d_f m_pos, vec3d_f en_pos)
{
	return (float)(sqrt(((en_pos.x - m_pos.x) * (en_pos.x - m_pos.x)) + ((en_pos.y - m_pos.y) * (en_pos.y - m_pos.y)) + ((en_pos.z - m_pos.z) * (en_pos.z - m_pos.z))));
}

int main()
{
	HWND hwnd = FindWindow(NULL, "AssaultCube");
	
	if (!hwnd)
	{
		return -1;
	}
	
	RECT rect;
	GetWindowRect(hwnd, &rect);

	DWORD pID = hook::get_process_id("ac_client.exe");

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

	const uint32_t playerCount = ::hook::memory::read_memory<uint32_t>(baseAddr + 0xCULL, pID);

	DWORD clientAddr = ::hook::memory::read_memory<DWORD>(baseAddr, pID);
	DWORD entityAddr = ::hook::memory::read_memory<DWORD>(baseAddr+ofEntity, pID);

	UINT_PTR sig = ::hook::memory::scan_signature(ac_client.base, ac_client.size, { 0x00 }, "", pID);

	const uint32_t clientTeam = ::hook::memory::read_memory<uint32_t>(clientAddr + ofTeam, pID);

	while (true)
	{
		float matrix[16];
		::hook::memory::read_memory(ofViewMat, matrix, sizeof(matrix), pID);

		vec3d_f playerPos = ::hook::memory::read_memory<vec3d_f>(clientAddr + ofEntity + ofXPos, pID);

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

			if (WorldToScreen(pos, matrix, 720, 480))
			{
				float xPos = screen.x + rect.left - (50 / 2);
				float yPos = screen.y + rect.top + (50 / 2);
				//if (screen.x >= rect.left && screen.x <= rect.right && screen.y <= rect.bottom && screen.y >= rect.top)
				::hook::wingdi::draw_box((int)xPos, (int)yPos, 50, 50, 2, (int)color.x, (int)color.y, (int)color.z);
			}
		}
		//Sleep(5);
	}

	
	::hook::memory::write_memory<char>(clientAddr + ofSpeed, 10, pID);




	return 0;
}



