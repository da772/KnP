#include <windows.h>
#include "memory.h"
#include "hook.h"
#include <iostream>
#include <assert.h>
#include <kdmapper.hpp>
#include <sstream>
#include <zlib.h>
#include "KdMapperData.h"

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

std::wstring GetExeDir()
{
	wchar_t NPath[MAX_PATH];
	GetModuleFileNameW(NULL, NPath, MAX_PATH);
	std::wstring path(NPath);
	size_t slash = path.find_last_of(L"\\");
	return path.substr(0, slash);
}


bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	UNREFERENCED_PARAMETER(mdlptr);
	Log("[+] Callback example called" << std::endl);

	/*
	This callback occurs before call driver entry and
	can be usefull to pass more customized params in
	the last step of the mapping procedure since you
	know now the mapping address and other things
	*/
	return true;
}

std::vector<unsigned char> GetBytes(std::wstring filename)
{
	std::ifstream file(filename, std::ios::binary);

	// Stop eating new lines in binary mode!!!
	file.unsetf(std::ios::skipws);

	// get its size:
	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	std::vector<BYTE> vec;
	vec.reserve(fileSize);

	// read the data:
	vec.insert(vec.begin(),
		std::istream_iterator<BYTE>(file),
		std::istream_iterator<BYTE>());

	file.close();

	return vec;
}

int main()
{
	const std::wstring exePath = GetExeDir() + L"\\dasdasvasd.exe";

	unsigned char* cpy = kdmapper::data::ckdmapper;
	unsigned char* result = (unsigned char*)malloc(sizeof(kdmapper::data::ckdmapper)*3);
	uint64_t size = sizeof(kdmapper::data::ckdmapper)/sizeof(unsigned char)*3;
	std::cout << "UnCompressing " << uncompress(result, (uLongf*)&size, (unsigned char*)cpy, sizeof(kdmapper::data::ckdmapper)/sizeof(unsigned char)) << std::endl;

	/*
	std::stringstream ss;

	ss << "#include <vector>\n";
	ss << "namespace KdMapper {\n";
	ss << "std::vector<unsigned char> data = {\n";
	for (size_t i = 0; i < size - 1; i++)
	{
		ss << std::hex << "0x" << (0xFF & result[i]) << ", ";
		if (i % 50 == 0 && i != 0)
		{
			ss << "\n";
		}
	}

	ss << std::hex << "0x" << (0xFF & result[size-1]);
	ss << " };\n";
	ss << "}";

	std::ofstream out(GetExeDir() + L"outputCOmpressed.txt");
	out << ss.str();
	out.close();
	free(result);
	*/
	
	std::ofstream wf(GetExeDir() + L"outputBinCompressed.exe", std::ios::out | std::ios::binary);
	wf.write((const char*)result, size);
	wf.close();
	/*
	std::ofstream out(GetExeDir()+L"outputBin.exe");
	out << ss.str();
	out.close();
	wf.close();
	*/

	return 0;
	const std::wstring driver_path = GetExeDir() + L"\\KnPDriver.sys";

	HANDLE iqvw64e_device_handle;
	iqvw64e_device_handle = intel_driver::Load();

	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE)
		return -1;

	std::vector<uint8_t> raw_image = { 0 };
	if (!utils::ReadFileToMemory(driver_path, &raw_image)) {
		Log(L"[-] Failed to read image to memory" << std::endl);
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}

	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(iqvw64e_device_handle, raw_image.data(), 0, 0, free, true, false, false, callbackExample, &exitCode)) {
		Log(L"[-] Failed to map " << driver_path << std::endl);
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		Log(L"[-] Warning failed to fully unload vulnerable driver " << std::endl);
	}
	Log(L"[+] success" << std::endl);

	while (1)
	{

	}

	return 0;

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