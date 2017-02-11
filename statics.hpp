#ifndef XTS_STATICS_HPP
#define XTS_STATICS_HPP

#include "static_vector.hpp"
#include "boost/container/flat_map.hpp"
#include "static_hashmap.hpp"

namespace xts
{
	template <typename KEY, typename VALUE, std::size_t SIZE, Pred = std::less<std::pair<KEY, VALUE>>>
	using static_map = boost::container::flat_map<KEY, VALUE, Pred, static_allocator<std::pair<KEY, VALUE>, SIZE>>;
}

#endif //!XTS_STATICS_HPP
