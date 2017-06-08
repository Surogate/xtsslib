#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <array>
#include <algorithm>
#include <cstddef>
#include <cassert>
#include <numeric>
#include "aarray_view.hpp"

namespace xts
{
	template <typename T, std::size_t width, std::size_t height>
	class matrix {
		typedef std::array<T, width * height> data_holder;
		data_holder data;

	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef typename data_holder::iterator iterator;
		typedef typename data_holder::const_iterator const_iterator;
		typedef std::size_t size_type;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		
		enum {
			WIDTH = width,
			HEIGHT = height
		};

		matrix() = default;
		matrix(const matrix&) = default;
		matrix& operator=(const matrix&) = default;
		matrix(matrix&&) noexcept = default;
		matrix& operator=(matrix&&) noexcept = default;
		matrix(std::initializer_list<T> arr)
		{
			assert(arr.size() <= data.size());
			std::copy(arr.begin(), arr.end(), data.begin());
		}
		matrix(const astd::array_view<T>& arr)
		{
			assert(arr.size() <= data.size());
			std::copy(arr.begin(), arr.end(), data.begin());
		}
		
		iterator begin() { return data.begin(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator cbegin() const { return data.cbegin(); }
		iterator end() { return data.end(); }
		const_iterator end() const { return data.end(); }
		const_iterator cend() const { return data.cend(); }
		reverse_iterator rbegin() { return data.rbegin(); }
		reverse_iterator rend() { return data.rend(); }
		const_reverse_iterator rbegin() const { return data.rbegin(); }
		const_reverse_iterator rend() const { return data.rend(); }
		const_reverse_iterator crbegin() const { return data.crbegin(); }
		const_reverse_iterator crend() const { return data.crend(); }
		
		constexpr auto size() const noexcept { return data.size(); }
		constexpr auto max_size() const noexcept { return data.max_size(); }
		constexpr auto capacity() const noexcept { return data.capacity(); }

		T& at(std::size_t index) { return data.at(index); }
		const T& at(std::size_t index) const { return data.at(index); }
		T& operator[](std::size_t index) { return data[index]; }
		const T& operator[](std::size_t index) const { return data[index]; }
	};

	template <typename T, std::size_t width, std::size_t height>
	bool operator==(const matrix<T, width, height>& lval, const matrix<T, width, height>& rval)
	{
		return std::equal(lval.begin(), lval.end(), rval.begin(), rval.end());
	}

	template <typename T, std::size_t size>
	using vec = matrix<T, size, 1>;

	template <typename T>
	using vec2 = vec<T, 2>;

	template <typename T>
	using vec3 = vec<T, 3>;

	template <typename T>
	using vec4 = vec<T, 4>;

	template <typename T>
	using mat2 = matrix<T, 2, 2>;

	template <typename T>
	using mat3 = matrix<T, 3, 3>;

	template <typename T>
	using mat4 = matrix<T, 4, 4>;

	template <typename T, std::size_t width, std::size_t height>
	struct coordinate_ref
	{
		coordinate_ref(matrix<T, width, height>& vec) : _vec(vec) 
		{
			static_assert(width * height >= 3, "invalid array");
		}

		T& x() { return _vec[0]; }
		T& y() { return _vec[1]; }
		T& z() { return _vec[2]; }

		matrix<T, width, height>& _vec;
	};

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, column, row> operator+(const matrix<T, column, row>& lval, const matrix<T, column, row>& rval)
	{
		matrix<T, column, row> result;
		std::transform(lval.begin(), lval.end(), rval.begin(), result.begin(), [](const auto& inner_lval, const auto& inner_rval)
		{
			return inner_lval + inner_rval;
		});
		return result;
	}

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, column, row> operator-(const matrix<T, column, row>& lval, const matrix<T, column, row>& rval)
	{
		matrix<T, column, row> result;
		std::transform(lval.begin(), lval.end(), rval.begin(), result.begin(), [](const auto& inner_lval, const auto& inner_rval)
		{
			return inner_lval - inner_rval;
		});
		return result;
	}

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, column, row>& operator+=(matrix<T, column, row>& lval, const matrix<T, column, row>& rval)
	{
		std::transform(lval.begin(), lval.end(), rval.begin(), lval.begin(), [](const auto& inner_lval, const auto& inner_rval)
		{
			return inner_lval + inner_rval;
		});
		return lval;
	}

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, column, row>& operator-=(matrix<T, column, row>& lval, const matrix<T, column, row>& rval)
	{
		std::transform(lval.begin(), lval.end(), rval.begin(), lval.begin(), [](const auto& inner_lval, const auto& inner_rval)
		{
			return inner_lval - inner_rval;
		});
		return lval;
	}

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, column, row>& operator-(matrix<T, column, row>& mat)
	{
		std::for_each(lval.begin(), lval.end(), [](auto& val)
		{
			return val = -val;
		});	
		return mat;
	}

	template <typename T, std::size_t column, std::size_t row>
	matrix<T, row, column> transpose(const matrix<T, column, row>& source)
	{
		matrix<T, row, column> result;

		for (int i = 0; i < row; i++)
			for (int j = 0; j < column; j++)
				result[j + i*row] = source[i + j*column];
		return result;
	}

	template <typename T, std::size_t column>
	inline T dot_product(const matrix<T, column, 1>& lval, const matrix<T, column, 1>& rval)
	{
		return std::inner_product(lval.begin(), lval.end(), rval.begin(), T(0));
	}

	template <typename T>
	inline vec4<T> dot_product(const mat4<T>& transformation, const vec4<T>& vertex)
	{
		return {
			transformation[0] * vertex[0] + transformation[4] * vertex[1] + transformation[8]	* vertex[2] + transformation[0xC] * vertex[3],
			transformation[1] * vertex[0] + transformation[5] * vertex[1] + transformation[9]	* vertex[2] + transformation[0xD] * vertex[3],
			transformation[2] * vertex[0] + transformation[6] * vertex[1] + transformation[0xA] * vertex[2] + transformation[0xE] * vertex[3],
			transformation[3] * vertex[0] + transformation[7] * vertex[1] + transformation[0xB] * vertex[2] + transformation[0xF] * vertex[3]
		};
	}

	template <typename T>
	inline mat4<T> dot_product(const mat4<T>& lval, const mat4<T>& rval)
	{
		return {
			lval[0] * rval[0]	+ lval[4] * rval[1]		+ lval[8]	* rval[2]	+ lval[0xC] * rval[3], //0
			lval[1] * rval[0]	+ lval[5] * rval[1]		+ lval[9]	* rval[2]	+ lval[0xD] * rval[3], //1
			lval[2] * rval[0]	+ lval[6] * rval[1]		+ lval[0xA] * rval[2]	+ lval[0xE] * rval[3], //2
			lval[3] * rval[0]	+ lval[7] * rval[1]		+ lval[0xB] * rval[2]	+ lval[0xF] * rval[3], //3
			lval[0] * rval[4]	+ lval[4] * rval[5]		+ lval[8]	* rval[6]	+ lval[0xC] * rval[7], //4
			lval[1] * rval[4]	+ lval[5] * rval[5]		+ lval[9]	* rval[6]	+ lval[0xD] * rval[7], //5
			lval[2] * rval[4]	+ lval[6] * rval[5]		+ lval[0xA] * rval[6]	+ lval[0xE] * rval[7], //6
			lval[3] * rval[4]	+ lval[7] * rval[5]		+ lval[0xB] * rval[6]	+ lval[0xF] * rval[7], //7
			lval[0] * rval[8]	+ lval[4] * rval[9]		+ lval[8]	* rval[0xA]	+ lval[0xC] * rval[0xB], //8
			lval[1] * rval[8]	+ lval[5] * rval[9]		+ lval[9]	* rval[0xA]	+ lval[0xD] * rval[0xB], //9
			lval[2] * rval[8]	+ lval[6] * rval[9]		+ lval[0xA] * rval[0xA]	+ lval[0xE] * rval[0xB], //10
			lval[3] * rval[8]	+ lval[7] * rval[9]		+ lval[0xB] * rval[0xA]	+ lval[0xF] * rval[0xB], //11
			lval[0] * rval[0xC] + lval[4] * rval[0xD]	+ lval[8]	* rval[0xE]	+ lval[0xC] * rval[0xF], //12
			lval[1] * rval[0xC] + lval[5] * rval[0xD]	+ lval[9]	* rval[0xE]	+ lval[0xD] * rval[0xF], //13
			lval[2] * rval[0xC] + lval[6] * rval[0xD]	+ lval[0xA] * rval[0xE]	+ lval[0xE] * rval[0xF], //14
			lval[3] * rval[0xC] + lval[7] * rval[0xD]	+ lval[0xB] * rval[0xE]	+ lval[0xF] * rval[0xF], //15
		};
	}

	template <typename T, std::size_t column>
	matrix<T, column, 1> cross_product(const matrix<T, column, 1>& lval, const matrix<T, column, 1>& rval)
	{
		matrix<T, column, 1> result;

		for (std::size_t i = 0; i < column; i++)
		{
			result[i] = lval[(i + 1) % column] * rval[(i + 2) % column] - lval[(i + 2) % column] * rval[(i + 1) % column];
		}
		return result;
	}
	
	template <typename T, std::size_t column>
	inline T length(const matrix<T, column, 1>& mat)
	{
		return std::sqrt(std::accumulate(mat.begin(), mat.end(), T(0), [](const auto& init, const auto& rval) {
			return init + rval * rval;
		}));
	}

	template <typename T, std::size_t size>
	matrix<T, size, size> identity() {
		matrix<T, size, size> result;
		for (std::size_t i = 0; i < result.size(); i++)
		{
			if (!(i % (size + 1)))
			{
				result[i] = T(1);
			}
			else
			{
				result[i] = T(0);
			}
		}
		return result;
	}

	template <typename T>
	matrix<T, 4, 4> identity4()
	{
		return {
			T(1), T(0), T(0), T(0),
			T(0), T(1), T(0), T(0),
			T(0), T(0), T(1), T(0),
			T(0), T(0), T(0), T(1)
		};
	}

	template <typename T>
	matrix<T, 4, 4> translation_transf(const T& x, const T& y, const T& z = T(0))
	{
		auto result = identity<T, 4, 4>();
		result[12] = x;
		result[13] = y;
		result[14] = z;
		return result;
	}

	template <typename T>
	inline matrix<T, 4, 4> scale_transf(const T& x, const T& y = T(1), const T& z = T(1))
	{
		return {
			x,		T(0),	T(0),	T(0),
			T(0),	y,		T(0),	T(0),
			T(0),	T(0),	z,		T(0),
			T(0),	T(0),	T(0),	T(1)
		};
	}

}


#endif //!MATRIX_HPP