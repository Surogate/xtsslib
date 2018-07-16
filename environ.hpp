#ifndef XTS_ENVIRON_HPP
#define XTS_ENVIRON_HPP

#ifdef WIN32
#include "windows_environ.hpp"
#else
#include "linux_environ.hpp"
#endif //WIN32

namespace xts::env
{
   std::unordered_map<std::string, std::vector<std::string>> merge_environ(
       const std::unordered_map<std::string, std::vector<std::string>>& lval,
       const std::unordered_map<std::string, std::vector<std::string>>& rval)
   {
      std::unordered_map<std::string, std::vector<std::string>> result;

      for (const auto& values : lval)
      {
         auto& keys = result[values.first];

         for (const auto& key_to_insert : values.second)
         {
            if (std::find(keys.begin(), keys.end(), key_to_insert) == keys.end())
            {
               keys.push_back(key_to_insert);
            }
         }
      }

      for (const auto& values : rval)
      {
         auto& keys = result[values.first];

         for (const auto& key_to_insert : values.second)
         {
            if (std::find(keys.begin(), keys.end(), key_to_insert) == keys.end())
            {
               keys.push_back(key_to_insert);
            }
         }
      }
      return result;
   }
}

#endif //!XTS_ENVIRON_HPP

