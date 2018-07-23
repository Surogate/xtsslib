#ifndef XTS_LINUX_PROCESS_HPP
#define XTS_LINUX_PROCESS_HPP

#ifndef WIN32

#include <cerrno>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace xts
{

   class linux_process
   {
      std::system_error run_process(const std::filesystem::path &filename, const std::vector<std::string> &argv,
                                    const std::vector<std::string> &envp)
      {
         cpid = fork();
         if (cpid == -1)
         {
            return std::system_error(std::make_error_code(std::errc(std::errno)));
         }
         if (cpid == 0)
         {
            std::vector<char *> arg;
            arg.push_back(filename.c_str());
            for (const auto &v : argv)
            {
               arg.push_back(v.c_str());
            }
            arg.push_back(nullptr);

            std::vector<char *> env;
            for (auto &e : envp)
            {
               env.push_back(e.c_str());
            }
            env.push_back(nullptr);

            auto exec_error = execve(filename.c_str(), arg.data(), env.data());
            perror("execve");
            exit(EXIT_FAILURE);
         }
         else
         {
            attached = true;
         }
         return std::system_error(std::make_error_code(std::errc(0)));
      }

    public:
      linux_process(const std::filesystem::path &filename, const std::vector<std::string> &argv = {},
                    const std::vector<std::string> &envp = {})
      {
         std::system_error ec = run_process(filename, argv, envp);
         if (ec.code().value() != 0)
         {
            throw ec;
         }
      }

      linux_process(const std::filesystem::path &filename, const std::vector<std::string> &argv,
                    const std::vector<std::string> &envp, std::system_error &ec)
      {
         ec = run_process(filename, argv, envp);
      }

      bool join() 
      {
         int status;
         bool result = ::waitpid(cpid, &status, 0) >= 0;
         if(result)
            atached = false;
         return result;
      }

      bool kill() const
      {
         return ::kill(cpid, SIGKILL) == 0;
      }

      void detach()
      {
         attached = false;
      }

	  int pid() const
      {
		 return cpid;
	  }

      ~linux_process()
      {
         if (attached)
         {
            kill();
         }
      }

    private:
      int cpid;
      bool attached;
   };
}
#endif //!WIN32

#endif //!XTS_LINUX_PROCESS_HPP