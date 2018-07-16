#ifndef FILE_OPERATION_HPP
#define FILE_OPERATION_HPP

#include <filesystem>
#include <fstream>

namespace xts
{
   inline std::vector<char> get_file_content(std::ifstream& in)
   {
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

   inline std::vector<char> get_file_content(const std::filesystem::path& path)
   {
      std::ifstream stream(path.c_str(), std::ios::in | std::ios::binary);
      return get_file_content(stream);
   }

   template <std::size_t SIZE>
   std::pair<std::array<char, SIZE>, std::size_t> get_file_content_fixed(std::ifstream& in)
   {
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

   template <std::size_t SIZE>
   std::pair<std::array<char, SIZE>, std::size_t> get_file_content_fixed(const std::filesystem::path& path)
   {
      return get_file_content_fixed(std::ifstream(path.c_str(), std::ios::in | std::ios::binary));
   }
}

#endif //!FILE_OPERATION_HPP