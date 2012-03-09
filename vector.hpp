
#ifndef vector_h__
#define vector_h__

#include <vector>
#include <cstddef>

#include "thread_define.hpp"

namespace ts
{
	template < typename T, class Allocator = std::allocator< T > >
	class vector
	{
	public:
		typedef typename std::vector< T, Allocator > vector_implem;
	private:
		mutable std_shared_mutex _mut;
		vector_implem _implem;
	public:
		typedef typename vector_implem::reference reference;
		typedef typename vector_implem::const_reference const_reference;
		typedef typename vector_implem::value_type value_type;
		typedef typename vector_implem::allocator_type allocator_type;
		typedef typename vector_implem::size_type size_type;
		typedef typename vector_implem::difference_type difference_type;
		typedef typename vector_implem::pointer pointer;
		typedef typename vector_implem::const_pointer const_pointer;

		class iterator : public vector_implem::iterator
		{
		public:
			iterator()
				: vector_implem::iterator(), _guard()
			{}

			iterator(std_shared_mutex& mut, typename const vector_implem::iterator& it)
				: vector_implem::iterator(it), _guard(_mut) 
			{}

			iterator(const iterator& orig)
				: vector_implem::iterator(orig), _guard(orig._guard)
			{}

			iterator(iterator&& orig)
				: vector_implem::iterator(orig), _guard(orig._guard)
			{}

			~iterator()
			{}

			iterator& operator=(const iterator& orig)
			{
				if (this != &orig)
				{
					_guard = orig._guard;
					vector_implem::iterator::operator=(orig);
				}
				return *this;
			}

			iterator& operator=(iterator&& orig)
			{
				if (this != &orig)
				{
					_guard = orig._guard;
					vector_implem::iterator::operator=(orig);
				}
				return *this;
			}

		private:
			std_shared_lock_guard _guard;
		};

		class const_iterator : public vector_implem::const_iterator
		{
		public:
			const_iterator()
				: vector_implem::const_iterator(), _guard()
			{}

			const_iterator(std_shared_mutex& mut, typename const vector_implem::const_iterator& it)
				: vector_implem::const_iterator(it), _guard(_mut) 
			{}

			const_iterator(const iterator& orig)
				: vector_implem::const_iterator(orig), _guard(orig._guard)
			{}

			const_iterator(iterator&& orig)
				: vector_implem::const_iterator(orig), _guard(orig._guard)
			{}

			~const_iterator()
			{}

			const_iterator& operator=(const const_iterator& orig)
			{
				if (this != &orig)
				{
					_guard = orig._guard;
					vector_implem::const_iterator::operator=(orig);
				}
				return *this;
			}

			const_iterator& operator=(const_iterator&& orig)
			{
				if (this != &orig)
				{
					_guard = orig._guard;
					vector_implem::const_iterator::operator=(orig);
				}
				return *this;
			}

		private:
			std_shared_lock_guard _guard;
		};

		typedef typename std::reverse_iterator< iterator > reverse_iterator;
		typedef typename std::reverse_iterator< const_iterator > const_reverse_iterator;

		explicit vector(const allocator_type& al = allocator_type())
			 : _implem(al), _mut()
		{}

		explicit vector(std::size_t n, const value_type& value = value_type(), const allocator_type& al = allocator_type())
			: _implem(n, value, al), _mut()
		{}

		template <class InputIterator>
		vector(InputIterator first, InputIterator last, const allocator_type& al = allocator_type())
			: _implem(first, last, al), _mut()
		{}

		vector(const vector& orig)
			: _mut()
		{
			std_shared_lock_guard _(orig._mut);
			_implem = orig._implem;
		}

		vector(vector&& orig)
			: _mut()
		{
			std_shared_lock_guard _(orig._mut);
			_implem = orig._implem;
		}

		~vector()
		{}

		vector& operator=(const vector& orig)
		{
			if (this != &orig)
			{
				std_shared_lock_guard orig_guard(orig._mut);
				std_lock_guard guard(orig._mut);				
				_implem = orig._implem;
			}
			return *this;
		}

		vector& operator=(vector&& orig)
		{
			if (this != &orig)
			{
				std_lock_guard orig_guard(orig._mut);
				std_lock_guard guard(orig._mut);				
				_implem = orig._implem;
			}
			return *this;
		}

		allocator_type get_allocator() const
		{
			return _implem.get_allocator();
		}

		std::size_t size() const
		{
			std_shared_lock_guard _(_mut);
			return _implem.size();
		}

		std::size_t max_size() const
		{
			return _implem.max_size();
		}

		void resize(size_type sz, T c = T())
		{
			std_lock_guard _(_mut);
			_implem.resize(sz, c);
		}

		std::size_t capacity() const
		{
			std_shared_lock_guard _(_mut);
			return _implem.capacity();
		}

		bool empty() const
		{
			std_shared_lock_guard _(_mut);
			return _implem.empty();
		}

		void reserve(size_type n)
		{
			std_lock_guard _(_mut);
			_implem.reserve(n);
		}

		void shrink_to_fit()
		{
			std_lock_guard _(_mut);
			_implem.shrink_to_fit();
		}

		template < class InputIterator >
		void assign(InputIterator first, InputIterator last)
		{
			std_lock_guard _(_mut);
			_implem.assign(first, last);
		}

		void assign( size_type n, const T& u)
		{
			std_lock_guard _(_mut);
			_implem.assign(n, u);
		}

		iterator at(size_type i)
		{
			return iterator(_mut, _implem.begin() + i);
		}

		const_iterator at(size_type i) const
		{
			return const_iterator(_mut, _implem.begin() + i);
		}

		iterator operator[](size_type i)
		{
			return iterator(_mut, _implem.begin() + i);
		}

		const_iterator operator[](size_type i) const
		{
			return const_iterator(_mut, _implem.begin() + i);
		}

		iterator front()
		{
			return begin();
		}

		const_iterator front() const
		{
			return begin();
		}

		iterator back()
		{
			return end();
		}

		const_iterator back() const
		{
			return end();
		}

		iterator begin()
		{
			return iterator(_mut, _implem.begin());
		}

		const_iterator begin() const
		{
			return const_iterator(_mut, _implem.begin());
		}

		const_iterator cbegin() const
		{
			return const_iterator(_mut, _implem.cbegin());
		}

		iterator end()
		{
			return iterator(_mut, _implem.end());
		}

		const_iterator end() const
		{
			return const_iterator(_mut, _implem.end());
		}

		const_iterator cend() const
		{
			return const_iterator(_mut, _implem.cend());
		}

		reverse_iterator rbegin()
		{
			return reverse_iterator(begin());
		}

		const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(begin());
		}

		void push_back(const T& x)
		{
			std_lock_guard _(_mut);
			_implem.push_back(x)
		}


	};
}



#endif // vector_h__

