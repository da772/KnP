#include "data.h"
#include "generated/kdmapper.h"
#include "generated/knpdriver.h"

#include <zlib.h>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

namespace KnP {

	namespace data
	{
		knpdriver create_driver()
		{

			knpdriver result;
			result.data = (unsigned char*)malloc(::knpdriver::size * sizeof(unsigned char));
			assert(result.data != NULL);
			result.size = ::knpdriver::size;

			uncompress(result.data, (uLongf*)&result.size, (unsigned char*)::knpdriver::data, sizeof(::knpdriver::data));

			return result;
		}

		kdmapper create_mapper()
		{
			kdmapper result;
			result.data = (unsigned char*)malloc(::kdmapper::size * sizeof(unsigned char));
			assert(result.data != NULL);
			result.size = ::kdmapper::size;

			uncompress(result.data, (uLongf*)&result.size, (unsigned char*)::kdmapper::data, sizeof(::kdmapper::data));

			return result;
		}


		void modify_driver(knpdriver* driver)
		{

		}

		utility::exec_info finalize_data(knpdriver* driver, kdmapper* mapper)
		{
			const std::wstring kdmap_path = utility::get_exe_dir() + L"\\kdcompressed.exe";

			std::ofstream wf(kdmap_path, std::ios::out | std::ios::binary);
			wf.write((const char*)mapper->data, mapper->size);
			wf.close();
			free(mapper->data);
			mapper->size = 0;

			const std::wstring driver_path = KnP::utility::get_exe_dir() + L"\\driver.sys";

			wf = std::ofstream(driver_path, std::ios::out | std::ios::binary);
			wf.write((const char*)driver->data, driver->size);
			wf.close();

			free(driver->data);
			driver->size = 0;

			std::wstring command = kdmap_path + L" " + driver_path;

			KnP::utility::exec_info info;
			KnP::utility::exec_command(command, &info);

			_wremove(driver_path.c_str());
			_wremove(kdmap_path.c_str());

			return info;
		}
	}
}