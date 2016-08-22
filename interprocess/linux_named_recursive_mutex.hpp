#ifndef LINUX_NAMED_RECURSIVE_MUTEX_H
#define LINUX_NAMED_RECURSIVE_MUTEX_H

#ifndef _MSC_VER //only on linux

#include "boost/interprocess/creation_tags.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/interprocess/exceptions.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/sync/posix/ptime_to_timespec.hpp"

#include <pthread.h>
#include <cstring>
#include <mutex>

#ifdef PTHREAD_MUTEX_ROBUST
namespace xts
{
   // Y
   // Design to only handle an interprocess race.
   // Do not use it for interthread competition.
   // Offer to properly handle the abandonned state
   //
   class linux_named_recursive_mutex
   {
      class linux_recursive_mutex
      {
      public:
         enum mutex_status
         {
            LOCK_ACQUIRED = 0,
            LOCK_FROM_ABANDONNED = EOWNERDEAD,
            LOCK_TIMEOUT,
            LOCK_ERROR,
            MOVED
         };

      private:
         pthread_mutex_t handler;
         pthread_mutexattr_t attribute;
         mutex_status res;

      public:
         linux_recursive_mutex() = delete;
         linux_recursive_mutex(const linux_named_recursive_mutex& orig) = delete;
         linux_recursive_mutex& operator=(const linux_named_recursive_mutex&) = delete;

         linux_recursive_mutex(linux_recursive_mutex&& orig) noexcept
         {
            handler = orig.handler;
            attribute = orig.attribute;
            res = orig.res;
            orig.res = MOVED;
         }

         linux_recursive_mutex& operator=(linux_recursive_mutex&& orig) noexcept
         {
            handler = orig.handler;
            attribute = orig.attribute;
            res = orig.res;
            orig.res = MOVED;
            return *this;
         }

         bool initialize_attr_mutex()
         {
            bool result = true;
            std::memset(&handler, 0, sizeof(handler));
            std::memset(&attribute, 0, sizeof(attribute));

            result = result && pthread_mutexattr_init(&attribute) == 0;
            result = result && pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_RECURSIVE_NP) == 0;
            result = result && pthread_mutexattr_setrobust(&attribute, PTHREAD_MUTEX_ROBUST) == 0;
            return result;
         }

         void destroy_attr()
         {
            pthread_mutexattr_destroy(&attribute);
         }

         bool handle_lock_result(int result)
         {
            if (result != LOCK_ACQUIRED && result != LOCK_FROM_ABANDONNED)
            {
               res = LOCK_ERROR;
               return false;
            }
            if (result == LOCK_FROM_ABANDONNED)
            {
               pthread_mutex_consistent(&handler);
            }
            return true;
         }

      public:
         linux_recursive_mutex(boost::interprocess::create_only_t)
         {
            if (!initialize_attr_mutex())
            {
               throw boost::interprocess::interprocess_exception(boost::interprocess::error_info(errno), "error at initializing the mutex attribute");
            }
            pthread_mutex_init(&handler, &attribute);
         }

         ~linux_recursive_mutex()
         {
            if (res != MOVED)
            {
               destroy_attr();
               pthread_mutex_destroy(&handler);
            }
         }

         void unlock()
         {
            auto result = pthread_mutex_unlock(&handler);
            if (result != 0)
            {
               res = LOCK_ERROR;
               throw boost::interprocess::interprocess_exception(boost::interprocess::error_info(result));
            }
         }

         void lock()
         {
            auto result = pthread_mutex_lock(&handler);
            if (!handle_lock_result(result))
               throw boost::interprocess::interprocess_exception(boost::interprocess::error_info(result));
         }

         bool try_lock()
         {
            auto result = pthread_mutex_trylock(&handler);
            return handle_lock_result(result);
         }

         bool timed_lock(const boost::posix_time::ptime& time)
         {
            timespec abstime = boost::interprocess::ipcdetail::ptime_to_timespec(time);
            auto result = pthread_mutex_timedlock(&handler, &abstime);
            return handle_lock_result(result);
         }

         mutex_status status()
         {
            return res;
         }
      };

      struct shared_structure
      {
         linux_recursive_mutex mutex;
         uint32_t reference_counter;

         shared_structure(boost::interprocess::create_only_t)
            : mutex(boost::interprocess::create_only)
            , reference_counter(1)
         {
         }
      };

      std::string name;
      boost::interprocess::shared_memory_object handler;
      boost::interprocess::mapped_region region;
      shared_structure* sh_mem_mapped;

   public:
      linux_named_recursive_mutex(boost::interprocess::create_only_t, const char* _name)
         : name(_name)
         , handler(boost::interprocess::create_only, name.data(), boost::interprocess::read_write)
         , region()
         , sh_mem_mapped(nullptr)
      {
         handler.truncate(sizeof(shared_structure));
         region = std::move(boost::interprocess::mapped_region(handler, boost::interprocess::read_write));
         sh_mem_mapped = static_cast<shared_structure*>(region.get_address());
         new(sh_mem_mapped) shared_structure(boost::interprocess::create_only);
      }

      linux_named_recursive_mutex(boost::interprocess::open_only_t, const char* _name)
         : name(_name)
         , handler(boost::interprocess::open_only, name.data(), boost::interprocess::read_write)
         , region()
         , sh_mem_mapped(nullptr)
      {
         region = boost::interprocess::mapped_region(handler, boost::interprocess::read_write);
         sh_mem_mapped = static_cast<shared_structure*>(region.get_address());
         std::lock_guard<linux_recursive_mutex> g(sh_mem_mapped->mutex);
         sh_mem_mapped->reference_counter++;
      }

      linux_named_recursive_mutex(boost::interprocess::open_or_create_t, const char* _name)
         : name(_name)
         , handler()
         , region()
         , sh_mem_mapped(nullptr)
      {

         bool exist = false;
         try
         {
            handler = boost::interprocess::shared_memory_object(boost::interprocess::open_only, name.data(), boost::interprocess::read_write);
            exist = true;
         }
         catch (...)
         {
            handler = boost::interprocess::shared_memory_object(boost::interprocess::create_only, name.data(), boost::interprocess::read_write);
         }

         if (!exist)
            handler.truncate(sizeof(shared_structure));
         region = boost::interprocess::mapped_region(handler, boost::interprocess::read_write);
         sh_mem_mapped = static_cast<shared_structure*>(region.get_address());
         if (!exist)
         {
            new(sh_mem_mapped) shared_structure(boost::interprocess::create_only);
         }
         else
         {
            std::lock_guard<linux_recursive_mutex> g(sh_mem_mapped->mutex);
            sh_mem_mapped->reference_counter++;
         }
      }

      linux_named_recursive_mutex(linux_named_recursive_mutex&& orig)
      {
         name = std::move(orig.name);
         sh_mem_mapped = orig.sh_mem_mapped;
         orig.sh_mem_mapped = nullptr;
         region = std::move(orig.region);
         handler = std::move(orig.handler);
      }

      linux_named_recursive_mutex& operator=(linux_named_recursive_mutex&& orig)
      {
         if (this != &orig)
         {
            name = std::move(orig.name);
            sh_mem_mapped = orig.sh_mem_mapped;
            orig.sh_mem_mapped = nullptr;
            region = std::move(orig.region);
            handler = std::move(orig.handler);
         }
         return *this;
      }

      ~linux_named_recursive_mutex()
      {
         uint32_t ref = 1;
         {
            std::lock_guard<linux_recursive_mutex> g(sh_mem_mapped->mutex);
            sh_mem_mapped->reference_counter--;
            ref = sh_mem_mapped->reference_counter;
         }
         if (ref == 0)
         {
            sh_mem_mapped->~shared_structure();
            boost::interprocess::shared_memory_object::remove(name.data());
         }
      }

      void unlock()
      {
         sh_mem_mapped->mutex.unlock();
      }

      void lock()
      {
         sh_mem_mapped->mutex.lock();
      }

      bool try_lock()
      {
         return sh_mem_mapped->mutex.try_lock();
      }
      bool timed_lock(const boost::posix_time::ptime& time)
      {
         return sh_mem_mapped->mutex.timed_lock(time);
      }

      //existence tied to reference counter, no manual remove required
      static bool remove(const char*)
      {
         return true;
      }
   };

}
#endif //!PTHREAD_MUTEX_ROBUST

#endif //!_MSC_VER

#endif //! LINUX_NAMED_RECURSIVE_MUTEX_H
