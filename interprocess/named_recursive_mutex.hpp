#ifndef NAMED_RECURSIVE_MUTEX
#define NAMED_RECURSIVE_MUTEX

#include "windows_named_recursive_mutex.hpp"
#include "linux_named_recursive_mutex.hpp"

namespace xts
{
   //typedef to abstract the underlying implementation of mutex
#ifdef _MSC_VER
   typedef windows_named_recursive_mutex named_recursive_mutex;
#else
   typedef linux_named_recursive_mutex named_recursive_mutex;
#endif
}

#endif //!NAMED_RECURSIVE_MUTEX