#ifndef NULL_NAMED_RECURSIVE_MUTEX
#define NULL_NAMED_RECURSIVE_MUTEX

#include <boost/interprocess/creation_tags.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace xts
{

   // Y
   // designed to do nothing but keep the recursive mutex interfaces
   // useful if you have to pass it to template function that expect this interfaces
   //
   class null_named_recursive_mutex
   {
   public:
      enum mutex_status
      {
         LOCK_ACQUIRED,
         LOCK_FROM_ABANDONNED,
         LOCK_TIMEOUT,
         LOCK_ERROR,
         MOVED
      };

      null_named_recursive_mutex() = delete;
      null_named_recursive_mutex(const null_named_recursive_mutex&) = delete;
      null_named_recursive_mutex& operator=(const null_named_recursive_mutex&) = delete;

      // Constructor
      null_named_recursive_mutex(
         boost::interprocess::create_only_t, const char*) {}

      null_named_recursive_mutex(
         boost::interprocess::open_only_t, const char*) {}

      null_named_recursive_mutex(
         boost::interprocess::open_or_create_t, const char*) {}

      ~null_named_recursive_mutex() = default;

      // Methods

      void unlock()
      {}

      void lock()
      {}

      bool try_lock()
      {
         return true;
      }

      bool timed_lock(const boost::posix_time::ptime&)
      {
         return true;
      }

      mutex_status status() { return mutex_status(); }

      // Nothing here because we use windows object life over posix
      static bool remove(const char*) { return true; }
   };

}
#endif //! WINDOWS_NAMED_RECURSIVE_MUTEX
