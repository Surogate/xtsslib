#ifndef XTS_STATIC_HASHMAP_HPP
#define XTS_STATIC_HASHMAP_HPP

#include <cstddef>
#include <utility>
#include <array>
#include <functional>
#include <numeric>
#include <iterator>
#include <algorithm>

#include "static_vector.hpp"

namespace xts
{
	template <typename KEY, typename VALUE, std::size_t SIZE, class HASH = std::hash<KEY>, class PRED = std::equal_to<KEY>>
	class static_hashmap
	{
	public:
		typedef KEY key_type;
		typedef VALUE mapped_type;
		typedef std::pair<key_type, mapped_type> inner_value_type;
		typedef std::pair<const key_type, mapped_type> value_type;
		typedef HASH hasher;
		typedef PRED key_equal;

		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

	private:
		typedef xts::static_vector<inner_value_type, SIZE> value_container;

		value_container _data;
		hasher _hash;
		key_equal _equal;

	public:
		typedef typename value_container::iterator local_iterator;

	private:
		typedef std::pair<local_iterator, local_iterator> block_type;
		typedef std::array<block_type, SIZE> block_container;
		block_container _block;

	public:
		struct iterator : public value_container::iterator
		{
		private:
			typedef typename value_container::iterator base;
		public:
			iterator() = default;
			iterator(const iterator&) = default;
			iterator(iterator&&) = default;
			iterator(const base& val) : base(val) {}

			iterator& operator=(const iterator&) = default;
			iterator& operator=(iterator&&) = default;

			value_type& operator*() { return base::operator*(); }
			value_type* operator->() { return base::operator->(); }
		};

		struct const_iterator : public value_container::const_iterator
		{
		private:
			typedef typename value_container::const_iterator base;
		public:
			const_iterator() = default;
			const_iterator(const const_iterator&) = default;
			const_iterator(const_iterator&&) = default;
			const_iterator(const base& val) : base(val) {}
			const_iterator(const typename value_container::iterator& val) : base(val) {}

			const_iterator& operator=(const const_iterator&) = default;
			const_iterator& operator=(const_iterator&&) = default;

			const value_type& operator*() { return base::operator*(); }
			const value_type* operator->() { return base::operator->(); }
		};
	
	private:
		inline std::size_t get_index(const value_type& pair) const
		{
			return bucket(pair.first);
		}

		inline typename block_container::iterator get_blk(const key_type& key)
		{
			return _block.begin() + bucket(key);
		}
		inline typename block_container::iterator get_blk(const local_iterator& pos)
		{
			auto blk_it = get_blk(pos->first);
			if (find_in_block(blk, pos->first) != blk.second)
				return _block.end();
			return blk_it;
		}

		inline typename local_iterator find_in_block(const block_type& blk, const key_type& key_value)
		{
			return std::find_if(blk.first, blk.second, [&key_value, this](const auto& pair_value) {
				return _equal(key_value, pair_value.first);
			});
		}

		static constexpr block_type default_block() { return std::make_pair(local_iterator(), local_iterator()); }

		void rehash(typename value_container::iterator begin)
		{
			std::for_each(begin, _data.end(), [this](const auto& value) {
				_block[bucket(value.first)] = default_block();
			});

			auto end = _data.end();
			while (begin != end)
			{
				auto& blk = _block[bucket(begin->first)];
				if (blk == default_block())
				{
					blk.first = begin;
					blk.second = begin + 1;
				}
				else
				{
					blk.second++;
				}
				++begin;
			}
		}

	public:
		//constructor
		static_hashmap() : _data(), _block()
		{
			std::fill(_block.begin(), _block.end(), default_block());
		}
		static_hashmap(const static_hashmap&) = default;
		static_hashmap(static_hashmap&&) noexcept = default;
		static_hashmap& operator=(const static_hashmap&) = default;
		static_hashmap& operator=(static_hashmap&&) noexcept = default;

		//capacity
		bool empty() const {
			return size() == 0;
		}

		std::size_t size() const
		{
			return _data.size();
		}

		constexpr std::size_t max_size()
		{
			return SIZE;
		}

		//iterators
		iterator begin()
		{
			return _data.begin();
		}

		auto begin() const
		{
			return cbegin();
		}

		const_iterator cbegin() const
		{
			return _data.cbegin();
		}

		iterator end()
		{
			return _data.end();
		}

		auto end() const
		{
			return cend();
		}

		const_iterator cend() const
		{
			return _data.cend();
		}

		//Element lookup
		mapped_type& operator[](const key_type& key_value)
		{
			return insert(value_type(key_value, mapped_type())).first->second;
		}

		mapped_type& operator[](key_type&& key_value)
		{
			return insert(value_type(key_value, mapped_type())).first->second;
		}

		mapped_type& at(const key_type& key_value)
		{
			auto it = find(key_value);
			if (it == end())
				throw std::out_of_range("key not found");
			return it->second;
		}

		const mapped_type& at(const key_type& key_value) const
		{
			auto it = find(key_value);
			if (it == end())
				throw std::out_of_range("key not found");
			return it->second;
		}

		iterator find(const key_type& key_value)
		{
			auto result = end();
			if (size())
			{
				auto index = bucket(key_value);
				if (_block[index] != default_block())
				{
					auto it = find_in_block(_block[index], key_value);
					if (it != _block[index].second)
					{
						result = it;
					}
				}
			}
			return result;
		}

		const_iterator find(const key_type& key_value) const
		{
			auto result = end();
			if (size() > 0)
			{
				auto index = bucket(key_value);
				if (_block[index] != default_block())
				{
					auto it = find_in_block(_block[index], key_value);
					if (it != _block[index].second)
					{
						result = it;
					}
				}
			}
			return result;
		}

		size_type count(const key_type& key_value) const
		{
			return std::count_if(begin(), end(), [&key_value, this](const auto& pair_value) {return _equal(pair_value.first, key_value); });
		}

		std::pair<iterator, iterator>
			equal_range(const key_type& key_value)
		{
			auto it = find(key_value);
			if (it != end())
				return std::make_pair(it, it + 1);
			return std::make_pair(end(), end());
		}

		std::pair<const_iterator, const_iterator>
			equal_range(const key_type& key_value) const
		{
			auto it = find(key_value);
			if (it != end())
				return std::make_pair(it, it + 1);
			return std::make_pair(end(), end());
		}

		//Bucket interface
		local_iterator begin(size_type n)
		{
			return _blk[n].first;
		}

		local_iterator end(size_type n)
		{
			return _blk[n].second;
		}

		inline size_type bucket(const KEY& key)
		{
			return _hash(key) % max_size();
		}

		inline size_type bucket_size(size_type n) const
		{
			return std::distance(_block[n].first, _block[n].second);
		}

		inline constexpr size_type max_bucket_count() const
		{
			return SIZE;
		}

		inline size_type bucket_count() const
		{
			return max_bucket_count() - std::count(_block.begin(), _block.end(), default_block());
		}

		//Modifiers
		std::pair<iterator, bool> insert(const value_type& value)
		{
			std::pair<iterator, bool> result = { end(), false };

			auto index = bucket(value.first);
			if (_block[index] == default_block())
			{
				auto previous_size = size();
				_data.push_back(value);
				_block[index].first = _data.begin() + previous_size;
				_block[index].second = _data.begin() + previous_size + 1;
				result.first = _block[index].first;
				result.second = true;
			}
			else
			{
				auto already_existing_it = find_in_block(_block[index], value.first);
				if (already_existing_it != _block[index].second)
				{
					result.first = already_existing_it;
				}
				else
				{
					_data.insert(_block[index].second, value);
					for (auto& blk : _block)
					{
						if (blk.first >= _block[index].second)
						{
							blk.first++;
							blk.second++;
						}
					}
					result.first = _block[index].second;
					result.second = true;
					_block[index].second++;
				}
			}

			return result;
		}

		std::pair<iterator, bool> insert(value_type&& value)
		{
			std::pair<iterator, bool> result = { end(), false };

			auto index = bucket(value.first);
			if (_block[index] == default_block())
			{
				auto previous_size = size();
				_data.push_back(value);
				_block[index].first = _data.begin() + previous_size;
				_block[index].second = _data.begin() + previous_size + 1;
				result.first = iterator(_block[index].first);
				result.second = true;
			}
			else
			{
				auto already_existing_it = find_in_block(_block[index], value.first);
				if (already_existing_it != _block[index].second)
				{
					result.first = already_existing_it;
				}
				else
				{
					_data.insert(_block[index].second, value);
					for (auto& blk : _block)
					{
						if (blk != default_block() 
							&& blk.first >= _block[index].second)
						{
							blk.first++;
							blk.second++;
						}
					}
					result.first = _block[index].second;
					result.second = true;
					_block[index].second++;
				}
			}

			return result;
		}

		iterator insert(const_iterator hint, const value_type& value)
		{
			return insert(value).first;
		}

		iterator insert(const_iterator hint, value_type&& value)
		{
			return insert(value).first;
		}

		template < class InputIt >
		void insert(InputIt first, InputIt last)
		{
			std::for_each(first, last, [this](const auto& value)
			{
				insert(value);
			});
		}

		void insert(std::initializer_list<value_type> ilist)
		{
			insert(std::begin(ilist), std::end(ilist));
		}

		template <class M>
		std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj)
		{
			auto result = std::make_pair(find(k), false);
			if (result.first != end())
				result.first->second = std::forward<M>(obj);
			else
				result = insert(value_type(k, std::forward<M>(obj)));
			return result;
		}

		template <class M>
		std::pair<iterator, bool> insert_or_assign(const_iterator hint, const key_type& k, M&& obj)
		{
			auto result = std::make_pair(find(k), false);
			if (result.first != end())
				result.first->second = std::forward<M>(obj);
			else
				result = insert(hint, value_type(k, std::forward<M>(obj)));
			return result;
		}

		template <class M>
		std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj)
		{
			auto result = std::make_pair(find(k), false);
			if (result.first != end())
				result.first->second = std::forward<M>(obj);
			else
				result = insert(value_type(k, std::forward<M>(obj)));
			return result;
		}

		template <class M>
		std::pair<iterator, bool> insert_or_assign(const_iterator hint, key_type&& k, M&& obj)
		{
			auto result = std::make_pair(find(k), false);
			if (result.first != end())
				result.first->second = std::forward<M>(obj);
			else
				result = insert(hint, value_type(k, std::forward<M>(obj)));
			return result;
		}

		template <class... Args>
		std::pair<iterator, bool> emplace(Args&&... args)
		{
			return insert(value_type(args...));
		}

		template <class... Args>
		std::pair<iterator, bool> emplace(const_iterator hint, Args&&... args)
		{
			return insert(hint, value_type(args...));
		}

		iterator erase(const const_iterator it_to_delete)
		{
			return erase(it_to_delete, it_to_delete + 1);
		}

		size_type erase(const key_type& key_value)
		{
			size_type result = 0;
			auto it = find(key_value);
			if (it != end())
			{
				erase(it);
				result++;
			}
			return result;
		}

		iterator erase(const_iterator first, const_iterator second)
		{
			auto size_to_erase = std::distance(first, second);
			auto it = get_blk(first->first);
			auto new_element = _data.erase(first, second);
			auto block_size = std::distance(it->first, it->second);

			if (block_size > size_to_erase)
			{
				rehash(new_element);
			}
			else
			{
				if (block_size == size_to_erase)
				{
					*it = default_block();
				}
				for (auto& blk : _block)
				{
					if (blk != default_block()
						&& blk.first >= second)
					{
						blk.first -= size_to_erase;
						blk.second -= size_to_erase;
					}
				}
			}
			return new_element;
		}

		void clear() noexcept
		{
			_data.clear();
			std::fill(_block.begin(), _block.end(), default_block());
		}

		void swap(static_hashmap& dest)
		{
			std::swap(_data, dest._data);

			rehash();
			dest.rehash();
		}

		//Observers
		hasher hash_function() const { return _hash; }
		key_equal key_eq() const { return _equal; }

		//Hash policy
		float load_factor() const
		{
			return std::static_cast<float>(size()) / std::static_cast<float>(bucket_count());
		}

		constexpr float max_load_factor() const
		{
			return max_size();
		}

		//!Warning : does nothing, no reallocation possible
		constexpr float max_load_factor(float) const
		{
			return max_load_factor();
		}

		//!Warning : there is no point to call this function in a static allocated container
		void rehash()
		{
			std::fill(_block.begin(), _block.end(), default_block());

			auto beg = _data.begin();
			auto end = _data.end();
			while (beg != end)
			{
				auto index = get_index(*beg);
				if (_block[index] == default_block())
				{
					_block[index].first = beg;
					_block[index].second = beg + 1;
				}
				else
				{
					_block[index].second++;
				}
				++beg;
			}
		}

		//!Warning : does nothing, no reallocation possible
		constexpr void reserve(size_type)
		{}
	};

	template <typename KEY, typename VALUE
		, typename HASHER1, typename HASHER2
		, typename EQUAL1, typename EQUAL2
		, std::size_t SIZE1, std::size_t SIZE2>
		bool operator==(const static_hashmap<KEY, VALUE, SIZE1, HASHER1, EQUAL1>& lval
			, const static_hashmap<KEY, VALUE, SIZE2, HASHER2, EQUAL2>& rval)
	{
		auto result = lval.size() == rval.size() &&
			std::all(lval.begin(), lval.end(), [&rval](const auto& lval_pair)
		{
			return std::find(rval.begin(), rval.end(), lval_pair) != rval.end();
		});
	}

	template <typename KEY, typename VALUE
		, typename HASHER1, typename HASHER2
		, typename EQUAL1, typename EQUAL2
		, std::size_t SIZE1, std::size_t SIZE2>
		bool operator!=(const static_hashmap<KEY, VALUE, SIZE1, HASHER1, EQUAL1>& lval
			, const static_hashmap<KEY, VALUE, SIZE2, HASHER2, EQUAL2>& rval)
	{
		return !operator==(lval, rval);
	}


}

#endif //!XTS_STATIC_HASHMAP_HPP