#ifndef XTSS_STATIC_VECTOR_HPP
#define XTSS_STATIC_VECTOR_HPP

#include <vector>
#include "boost/container/static_vector.hpp"

namespace xts
{
	template <typename T, std::size_t SIZE>
	using static_allocator = boost::container::container_detail::static_storage_allocator<T, SIZE>;

	template <typename T, std::size_t SIZE>
	using static_vector = boost::container::static_vector<T, SIZE>;
}

#endif //!XTSS_STATIC_VECTOR_HPP