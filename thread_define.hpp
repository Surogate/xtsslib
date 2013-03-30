
#ifndef thread_define_h__
#define thread_define_h__

#include <thread>
#include <mutex>
#include <memory>

namespace ts 
{
	typedef std::thread std_thread;
	typedef std::mutex std_shared_mutex;
	typedef std::lock_guard<std_shared_mutex> std_lock_guard;
}


#endif // thread_define_h__

