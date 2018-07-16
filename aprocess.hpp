#ifndef XTS_APROCESS_HPP
#define XTS_APROCESS_HPP

#ifdef WIN32
#include "windows_process.hpp"
#else
#include "linux_process.hpp"
#endif

namespace xts
{
#ifdef WIN32
   using aprocess = windows_process;
#else
   using aprocess = linux_process;
#endif
}

#endif //!XTS_APROCESS_HPP