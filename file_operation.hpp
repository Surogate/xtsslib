#ifndef FILE_OPERATION_HPP
#define FILE_OPERATION_HPP

#include <fstream>
#include "afilesystem.hpp"

namespace xts
{
	std::vector<char> get_file_content(const astd::filesystem::path& path)
	{
		std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
		std::vector<char> result;

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(std::size_t(in.tellg()));
			in.seekg(0, std::ios::beg);
			in.read(result.data(), result.size());
		}
		return result;
	}
	
	template<std::size_t SIZE>
	std::pair<std::array<char, SIZE>, std::size_t> get_file_content_fixed(const astd::filesystem::path& path)
	{
		std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
		std::array<char, SIZE> result;
		std::size_t read = 0;

		if (in)
		{
			in.seekg(0, std::ios::end);
			read = in.tellg();
			assert(SIZE >= read);
			in.seekg(0, std::ios::beg);
			in.read(result.data(), result.size());
		}
		return std::make_pair(result, read);
	}

}

#endif //!FILE_OPERATION_HPP