#pragma once

#include "utility.h"

namespace KnP {

	namespace data
	{
		typedef struct {
			unsigned char* data;
			size_t size;
		} kdmapper, knpdriver;

		knpdriver create_driver();
		kdmapper create_mapper();

		void modify_driver(knpdriver* driver);

		utility::exec_info finalize_data(knpdriver* driver, kdmapper* mapper);
	}

}