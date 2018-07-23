#ifndef XTS_WINDOWS_PROCESS_HPP
#define XTS_WINDOWS_PROCESS_HPP
#ifdef WIN32

#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>

namespace xts
{
   class windows_process
   {
      std::string quote_this(std::string_view arg)
      {
         if (arg.size() && arg[0] != '"' && arg.find(' ') != std::string::npos)
         {
            return '"' + std::string(arg) + '"';
         }
         return std::string(arg);
      }

      std::system_error run_process(const std::filesystem::path& filename, const std::vector<std::string>& argv,
                                    const std::vector<std::string>& envp)
      {
         std::memset(&si, 0, sizeof(si));
         std::memset(&pi, 0, sizeof(pi));

         command_line = filename.string();
         for (const auto& s : argv)
         {
            command_line += " " + quote_this(s);
         }
         std::vector<char> env_block;
         env_block.reserve(env.size() * 128);
         for (const auto& keys : envp)
         {
            env_block.insert(env_block.end(), keys.begin(), keys.end());
            env_block.push_back('\0');
         }
         env_block.push_back('\0');

#ifdef _DEBUG
         std::cout << command_line << std::endl;
#endif

		 char* env_block_data = nullptr;
         if(envp.size())
            env_block_data = env_block.data();

         auto result = ::CreateProcessA(NULL, command_line.data(), NULL, NULL,
             TRUE, NULL, env_block_data, running_dir.data(), &si, &pi);
         if (result > 0)
         {
            attached = true;
         }
         else
         {
            return std::system_error(std::error_code(GetLastError(), std::system_category()));
         }
         return std::system_error(std::make_error_code(std::errc(0)));
      }

    public:
      windows_process(const std::filesystem::path& filename, const std::vector<std::string>& argv = {},
                      const std::vector<std::string>& envp = {})
      {
         std::system_error ec = run_process(filename, argv, envp);
         if (ec.code().value() != 0)
         {
            throw ec;
         }
      }

      windows_process(const std::filesystem::path& filename, const std::vector<std::string>& argv,
                      const std::vector<std::string>& envp, std::system_error& ec)
      {
         ec = run_process(filename, argv, envp);
      }

      bool join()
      {
         if(pi.hProcess)
         {
            bool result = ::WaitForSingleObject(
                       pi.hProcess, std::numeric_limits<DWORD>::max())
                != WAIT_FAILED; 
			if(result)
               attached = false;
		 }
         return false;
      }

      bool kill() const
      {
         if (pi.hProcess)
            return ::TerminateProcess(pi.hProcess, -1);
         return false;
      }

      void detach()
      {
         attached = false;
      }

	  DWORD pid() const
	  { 
		  return pi.dwProcessId;
	  }

      ~windows_process()
      {
         if (attached)
         {
            kill();
         }
         if (pi.hProcess)
         {
            CloseHandle(pi.hProcess);
         }
         if (pi.hThread)
         {
            CloseHandle(pi.hThread);
         }
      }

    private:
      std::string command_line;
      std::string env;
      std::string running_dir = std::filesystem::current_path().string();
      bool attached;
      PROCESS_INFORMATION pi;
      STARTUPINFO si;
   };
}
#endif
#endif // !XTS_WINDOWS_PROCESS_HPP