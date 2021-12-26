#include <iostream>
#include <vector>
#include <fstream>
#include <zlib.h>
#include <sstream>
#include <assert.h>

std::vector<unsigned char> readFile(const wchar_t* filename);

int wmain(const int argc, wchar_t** argv)
{	
	if (argc < 4)
	{
		std::wcout << L"Usage: KnPWrap.exe {src} {dst} {namespace}" << std::endl;
		return -1;
	}
	

	std::vector<unsigned char> file = readFile(argv[1]);

	if (file.size() < 10)
	{
		std::wcout << L"File does not exist: " << argv[1] << std::endl;
		return -2;
	}
	size_t ogSize = file.size();
	unsigned char* compressed = (unsigned char*)malloc(sizeof(unsigned char)*ogSize);
	assert(compressed);
	uLongf size = static_cast<uLongf>(file.size());

	int result = compress(compressed, &size, file.data(), static_cast<uLongf>(file.size()));
	assert(size > 0);

	unsigned char* newPtr = (unsigned char*)realloc(compressed, size*sizeof(unsigned char));

	assert(newPtr);

	compressed = newPtr;

	std::stringstream ss;

	wchar_t* c = argv[3];
	int arg3 = 0;
	while (*c != 0)
	{
		c++;
		arg3++;
	}
	std::string name = "";
	for (size_t i = 0; i < arg3; i++)
	{
		name += static_cast<char>(argv[3][i]);
	}

	ss << "namespace " << name << " { \n";
	ss << "\tsize_t size = " << ogSize << ";\n";
	ss << "\tunsigned char data[] = {\n\t\t";
	
	for (size_t i = 0; i < size - 1; i++)
	{
		ss << std::hex << "0x" << (int)compressed[i] << ", ";
		if (i % 50 == 0 && i != 0)
		{
			ss << "\n\t\t";
		}
	}

	ss << std::hex << "0x" << (int)compressed[size - 1];
	ss << "\n\t};";
	ss << "\n}";

	std::string res = ss.str();

	std::ofstream out(argv[2]);

	out.write(res.c_str(), res.size());

	out.close();

	std::wcout << L"Compressed \"" << argv[1] << "\" to \"" << argv[2] << "\"" << std::endl;

	return 0;
}


std::vector<unsigned char> readFile(const wchar_t* filename)
{
	// open the file:
	std::ifstream file(filename, std::ios::binary);

	// Stop eating new lines in binary mode!!!
	file.unsetf(std::ios::skipws);

	// get its size:
	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	std::vector<unsigned char> vec;
	vec.reserve(fileSize);

	// read the data:
	vec.insert(vec.begin(),
		std::istream_iterator<unsigned char>(file),
		std::istream_iterator<unsigned char>());

	file.close();

	return vec;
}