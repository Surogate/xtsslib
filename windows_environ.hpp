#ifndef XTS_WINDOWS_ENVIRON_HPP
#define XTS_WINDOWS_ENVIRON_HPP

#ifdef WIN32

#include <Windows.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace xts::env
{
   char delim = ';';

   std::unordered_map<std::string, std::vector<std::string>> get_current_environ()
   {
      LPCH orig_env = GetEnvironmentStrings();
      LPCH env = orig_env;
      std::unordered_map<std::string, std::vector<std::string>> result;

      while (env && *env)
      {
         std::string_view current_value(env);

         if (std::size_t found = current_value.find('=');
             found != std::string_view::npos && found != 0) //environment variable starting by '=' are to be ignored
         {
            std::string key(current_value.substr(0, found));
            if (key.size())
            {
               auto& values = result[key];
               std::size_t end = 0;
               do
               {
                  end = current_value.find(delim, found + 1);
                  if (end != std::string::npos)
                  {
                     if ((end - found - 1) > 0)
                        values.push_back(std::string(current_value.substr(found + 1, end - found - 1)));
                     found = end;
                  }
               } while (end != std::string::npos);
               if (found + 1 < current_value.size())
                  values.push_back(std::string(current_value.substr(found + 1)));
            }
         }
         env += current_value.size() + 1;
      }
      
	  FreeEnvironmentStringsA(orig_env);
	  return result;
   }

   bool set_current_environ(const std::unordered_map<std::string, std::vector<std::string>>& env)
   {
      std::vector<char> env_block;
      env_block.reserve(env.size() * 128);
      for (const auto& keys : env)
      {
         std::string env_line = keys.first + '=';
         for (const auto& values : keys.second)
         {
            env_line += values + ';';
         }
         env_block.insert(env_block.end(), env_line.begin(), env_line.end());
         env_block.push_back('\0');
      }
      env_block.push_back('\0');
      return SetEnvironmentStringsA(env_block.data()) == TRUE;
   }
}

#endif //WIN32

#endif //!XTS_WINDOWS_ENVIRON_HPP