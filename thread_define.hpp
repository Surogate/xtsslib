
#ifndef thread_define_h__
#define thread_define_h__

#include <boost/thread.hpp>

namespace ts 
{
	typedef boost::thread std_thread;
	typedef boost::shared_mutex std_shared_mutex;
	typedef boost::lock_guard<std_shared_mutex> std_lock_guard;
	typedef boost::shared_lock<std_shared_mutex> std_shared_lock_guard;
}


#endif // thread_define_h__

