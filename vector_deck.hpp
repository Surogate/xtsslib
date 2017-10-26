#ifndef XTSS_LIB_VECTOR_DECK_HPP
#define XTSS_LIB_VECTOR_DECK_HPP

#include <cstddef>
#include <array>
#include <vector>
#include <cassert>
#include "fixed/vector.hpp"

template <typename T, std::size_t POWER_OF_TWO_SIZE>
struct vector_deck
{
	typedef T value_type;
	typedef std::size_t size_type;
	static constexpr std::size_t block_size_value = 0x1 << POWER_OF_TWO_SIZE;
private:
	typedef fixed::vector<T, block_size_value> block_type;
	typedef std::vector<block_type> data_type;

	data_type _data;
	size_type _size = 0;
public:
	void push_back(const T& value)
	{
		auto index_ptrs = _size >> POWER_OF_TWO_SIZE;
		if (index_ptrs >= _data.size())
			_data.emplace_back();
		_data[index_ptrs].push_back(value);
		_size++;
	}

	T& operator[](size_type index)
	{
		assert(index < _size);
		return _data[index >> POWER_OF_TWO_SIZE][index & (block_size_value - 1)];
	}

	const T& operator[](size_type index) const
	{
		assert(index < _size);
		return _data[index >> block_size_value][index & ((0x1 << block_size_value) - 1)];
	}

};

#endif //!VECTOR_DECK_HPP