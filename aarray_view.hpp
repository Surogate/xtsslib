#ifndef AARRAY_VIEW
#define AARRAY_VIEW

#include <cstddef>
#include <vector>
#include <array>

namespace astd
{
   template< typename T >
   class array_view
   {
   public:
      //Defines
      typedef T            value_type;
      typedef std::size_t  size_type;
      typedef T&           reference;
      typedef const T&     const_reference;
      typedef T*           pointer;
      typedef const T*     const_pointer;

      typedef const T*        const_iterator;
      typedef const_iterator  iterator;

      typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
      typedef const_reverse_iterator                  reverse_iterator;

      //Constructors / Dtors
      array_view() = default;
      array_view& operator=(const array_view& view) = default;
      array_view(const array_view& view) = default;
      array_view(array_view&& view) = default;

      template< std::size_t SIZE>
      array_view(T(&static_ptr)[SIZE])
         : _data(static_ptr), _size(SIZE)
      {}

      array_view(const std::vector<T>& vec)
         : _data(vec.data()), _size(vec.size())
      {}

      template < std::size_t SIZE >
      array_view(const std::array<T, SIZE>& arr)
         : _data(arr.data()), _size(SIZE)
      {}

      array_view(const T* val, std::size_t val_size)
         : _data(val), _size(val_size)
      {}

      //Element Access
      T& at(std::size_t value)
      {
#ifdef _DEBUG
         if (value > _size)
            throw std::out_of_range();
#endif
         return _data[value];
      }

      const T& at(std::size_t value) const
      {
#ifdef _DEBUG
         if (value > _size)
            throw std::out_of_range();
#endif
         return _data[value];

      }

      T& operator[](std::size_t value)
      {
         return at(value);
      }

      const T& operator[](std::size_t value) const
      {
         return at(value);
      }

      T& front()
      {
         return at(0);
      }

      const T& front() const
      {
         return at(0);
      }

      T& back()
      {
#ifdef _DEBUG
         if (!size)
            throw std::out_of_range();
#endif
         return at(size - 1);
      }

      const T& back() const
      {
#ifdef  _DEBUG
         if (!size)
            throw std::out_of_range();
#endif
         return at(size - 1);
      }

      T* data() { return _data; }
      const T* data() const { return _data; }

      std::vector<T> to_vector() const
      {
#ifdef _DEBUG
         if (!size)
            throw std::runtime_error("std::array_view is empty")
#endif
            return std::vector<T>(_data, _data(size));
      }

      //iterator
      const_iterator begin() { return _data; }
      const_iterator end() { return _data + _size; }
      const_iterator cbegin() const { return _data; }
      const_iterator cend() const { return _data + _size; }

      const_reverse_iterator rbegin() { return const_reverse_iterator(begin()); }
      const_reverse_iterator crbegin() const { return const_reverse_iterator(cbegin()); }
      const_reverse_iterator rend() { return const_reverse_iterator(end()); }
      const_reverse_iterator crend() const { return const_reverse_iterator(cend()); }

      //Capacity
      bool empty() const { return _data == nullptr || _size == 0; }
      std::size_t max_size() const { return size(); }
      std::size_t size() const { return _size; }
      std::size_t capacity() { return size(); }
      
      //Modifier
      void clear() { _data = nullptr; _size = 0; }
      void remove_prefix(std::size_t value) 
      { 
#ifdef _DEBUG
         if (value > _size)
            throw std::out_of_range();
#endif
         _data += value;
         _size -= value;
      }
      void remove_suffix(std::size_t value)
      {
#ifdef _DEBUG
         if (value > _size)
            throw std::out_of_range();
#endif
         _size -= value;
      }
      constexpr void swap(array_view& view) const noexcept
      {
         std::swap(_data, view._data);
         std::swap(_size, view._size);
      }  
   private:
      const T* _data;
      std::size_t _size;
   };

   template <typename T>
   bool operator==(const array_view<T>& lv, const array_view<T>& rv)
   {
      return lv.size() == rv.size() && std::equal(lv.begin(), lv.end(), rv.begin(), rv.end());
   }

   template <typename T>
   bool operator!=(const array_view<T>& lv, const array_view<T>& rv)
   {
      return !operator==(lv, rv);
   }

   template <typename T>
   bool operator<(const array_view<T>& lv, const array_view<T>& rv)
   {
      return std::lexicographical_compare(lv.begin(), lv.end(), rv.begin(), rv.end());
   }

   template <typename T>
   bool operator<=(const array_view<T>& lv, const array_view<T>& rv)
   {
      return operator==(lv, rv) || operator<(lv, rv);
   }

   template <typename T>
   bool operator>(const array_view<T>& lv, const array_view<T>& rv)
   {
      return !operator<(lv, rv);
   }

   template <typename T>
   bool operator>=(const array_view<T>& lv, const array_view<T>& rv)
   {
      return operator==(lv, rv) || operator>(lv, rv);
   }

}

#endif //!AARRAY_VIEW
