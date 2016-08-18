#ifndef WINDOWS_NAMED_RECURSIVE_MUTEX
#define WINDOWS_NAMED_RECURSIVE_MUTEX
#ifdef _MSC_VER

#include <windows.h>

#include <boost/interprocess/creation_tags.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <utility>

namespace xts
{
   // Y
   // Design to only handle an interprocess race.
   // Do not use it for interthread competition.
   // Offer to properly handle the abandonned state
   //
   class windows_named_recursive_mutex
   {
   public:
      enum mutex_status
      {
         LOCK_ACQUIRED = WAIT_OBJECT_0,
         LOCK_FROM_ABANDONNED = WAIT_ABANDONED,
         LOCK_TIMEOUT = WAIT_TIMEOUT,
         LOCK_ERROR = WAIT_FAILED,
         MOVED
      };

      windows_named_recursive_mutex() = delete;
      windows_named_recursive_mutex(const windows_named_recursive_mutex&) = delete;
      windows_named_recursive_mutex& operator=(const windows_named_recursive_mutex&) = delete;

      // Constructor
      windows_named_recursive_mutex(
         boost::interprocess::create_only_t, const char* name)
         : _handle(nullptr)
         , _attr()
         , _desc()
         , _name(name)
         , _res()
      {
         init_security_attr();

         HANDLE handle = CreateMutexA(&_attr, FALSE, name);
         if (handle != nullptr)
         {
            if (GetLastError() == NO_ERROR)
            {
               _handle = handle;
            }
            else
            {
               throw boost::interprocess::interprocess_exception(
                  boost::interprocess::error_info(GetLastError()),
                  "mutex already created");
            }
         }
         else
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }

      windows_named_recursive_mutex(
         boost::interprocess::open_only_t, const char* name)
         : _handle(nullptr)
         , _name(name)
      {
         _handle = OpenMutexA(SYNCHRONIZE, FALSE, name);
         if (_handle == nullptr)
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }

      windows_named_recursive_mutex(
         boost::interprocess::open_or_create_t, const char* name)
         : _handle(nullptr)
         , _name(name)
      {
         init_security_attr();
         _handle = CreateMutexA(&_attr, FALSE, name);
         if (_handle == nullptr)
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }

      windows_named_recursive_mutex(
         SECURITY_ATTRIBUTES& sec, bool initialOwner, const char* name)
         : _handle(nullptr)
         , _name(name)
         , _attr(sec)
      {
         if (sec.lpSecurityDescriptor != nullptr)
         {
            _desc = *(static_cast<SECURITY_DESCRIPTOR*>(sec.lpSecurityDescriptor));
         }
         _attr.lpSecurityDescriptor = &_desc;
         _handle = CreateMutexA(&_attr, initialOwner ? TRUE : FALSE, name);
         if (_handle == nullptr)
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }

      ~windows_named_recursive_mutex()
      {
         if (_handle && _res != MOVED)
         {
            CloseHandle(_handle);
            _handle = nullptr;
         }
      }

      windows_named_recursive_mutex(windows_named_recursive_mutex&& orig) noexcept
      {
         _handle = orig._handle;
         _attr = orig._attr;
         _desc = orig._desc;
         _name = orig._name;
         _res = orig._res;
         orig._res = MOVED;
      }

      windows_named_recursive_mutex& operator=(windows_named_recursive_mutex&& orig) noexcept
      {
         _handle = orig._handle;
         _attr = orig._attr;
         _desc = orig._desc;
         _name = orig._name;
         _res = orig._res;
         orig._res = MOVED;
         return *this;
      }

      // Methods

      void unlock()
      {
         _res = (mutex_status)ReleaseMutex(_handle);
         if (_res == FALSE)
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(_res));
         }
      }

      void lock()
      {
         _res = LOCK_ERROR;
         _res = (mutex_status)WaitForSingleObject(_handle, INFINITE);

         if (_res != LOCK_ACQUIRED && _res != LOCK_FROM_ABANDONNED)
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }

      bool try_lock()
      {
         _res = (mutex_status)WaitForSingleObject(_handle, 0);
         return _res == LOCK_ACQUIRED || _res == LOCK_FROM_ABANDONNED;
      }

      bool timed_lock(const boost::posix_time::ptime& time)
      {
         boost::posix_time::time_duration duration
            = time - boost::posix_time::second_clock::local_time();
         _res = (mutex_status)WaitForSingleObject(
            _handle, (DWORD)duration.total_milliseconds());
         return _res == LOCK_ACQUIRED || _res == LOCK_FROM_ABANDONNED;
      }

      mutex_status status() { return _res; }

      // Nothing here because we use windows object life over posix
      static bool remove(const char*) { return true; }

   private:
      // attribute
      HANDLE _handle;
      SECURITY_ATTRIBUTES _attr;
      SECURITY_DESCRIPTOR _desc;

   protected:
      std::string _name;
      mutex_status _res;

      // Constant and static
      enum CONSTANT
      {
         REPEAT_UNTIL_LOCK = 5
      };

      // private subroutine
      std::string get_system_message(DWORD val)
      {
         char* errorText = nullptr;
         std::string result;
         FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, val, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorText, 0, nullptr);

         if (nullptr != errorText)
         {
            result = errorText;
            LocalFree(errorText);
            errorText = nullptr;
         }
         return result;
      }

      void init_security_attr()
      {
         // Set default Security Attributes
         _attr.nLength = sizeof(_attr);
         _attr.lpSecurityDescriptor = &_desc;
         _attr.bInheritHandle = TRUE;

         if (!(InitializeSecurityDescriptor(&_desc,
            SECURITY_DESCRIPTOR_REVISION) // Create a security descriptor
            && SetSecurityDescriptorDacl(&_desc, TRUE, (PACL)nullptr,
               FALSE))) // allow every one to access the mutex
         {
            throw boost::interprocess::interprocess_exception(
               boost::interprocess::error_info(GetLastError()));
         }
      }
   };

}
#endif //!_MSC_VER

#endif //! WINDOWS_NAMED_RECURSIVE_MUTEX