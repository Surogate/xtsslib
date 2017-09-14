#ifndef XTS_COMPATIBILITY_HPP
#define XTS_COMPATIBILITY_HPP

#if (defined _MSC_VER && _MSC_VER < 1910) or not defined _MSC_VER
namespace stdext
{
	template <typename T>
	inline T checked_array_iterator(T val, int)
	{
		return val;
	}
}
#endif

#ifndef _MSC_VER

namespace std
{
	template<class T, class... Args> inline
		typename enable_if<!is_array<T>::value, unique_ptr<T>>::type
		make_unique(Args&&... _Args)
	{
		return (unique_ptr<T>(new T(std::forward<Args>(_Args)...)));
	}

	template<class T> inline
		typename enable_if<is_array<T>::value && extent<T>::value == 0, unique_ptr<T> >::type
		make_unique(size_t size)
	{
		typedef typename remove_extent<T>::type Clean_T;
		return (unique_ptr<T>(new Clean_T[size]()));
	}

	template<class T, class... Args>
	typename enable_if<extent<T>::value != 0, void>::type
		make_unique(Args&&...) = delete;
}

#endif


#endif //!XTS_COMPATIBILITY_HPP