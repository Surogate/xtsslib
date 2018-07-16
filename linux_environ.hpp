#ifndef XTS_LINUX_ENVIRON_HPP
#define XTS_LINUX_ENVIRON_HPP

#ifndef WIN32

#include <string>
#include <unordered_map>
#include <vector>

#include <stdlib.h>
#include <unistd.h>

namespace xts::env
{
   char delim = ':';

   std::unordered_map<std::string, std::vector<std::string>> get_current_environ()
   {
      std::unordered_map<std::string, std::vector<std::string>> result;

      char** tmp_environ = environ;

      while (tmp_environ && *tmp_environ)
      {
         std::string_view current_value(*tmp_environ);

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
         ++tmp_environ;
      }

      return result;
   }

   bool set_current_environ(const std::unordered_map<std::string, std::vector<std::string>>& env)
   {
      int error = 0;
      for (const auto& e : env)
      {
         std::string value;
         if (e.second.size())
         {
            for (const auto& v : e.second)
            {
               value += v + delim;
            }
         }
         error |= setenv(e.first.c_str(), value.c_str(), 1);
      }
      return error != 0;
   }
}

#endif //!WIN32

#endif //!XTS_LINUX_ENVIRON_HPP