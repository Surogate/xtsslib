#ifndef TWODGRID_HPP
#define TWODGRID_HPP

#include <cstddef>
#include <cstdlib>
#include "matrix.hpp"
#include "aarray_view.hpp"
#include <array>
#include <functional>
#include "operation_type.hpp"
#include "fixed/vector.hpp"

namespace grid2d {
	using index_t = int;
	using coord = std::array<index_t, 2>;

	struct mahattan {
		typedef index_t value;
		static constexpr xts::operation_type op_type = xts::operation_type::commutative;

		value operator()(const coord& lval, const coord& rval)
		{
			return invoke(lval, rval);
		}

		static value invoke(const coord& lval, const coord& rval)
		{
			return std::abs(rval[0] - lval[0]) + std::abs(rval[1] - lval[1]);
		}
	};

	struct euclidean {
		typedef float value;
		static constexpr xts::operation_type op_type = xts::operation_type::commutative;

		static value invoke(const coord& lval, const coord& rval)
		{
			int x_diff = rval[0] - lval[0];
			int y_diff = rval[1] - lval[1];
			return std::sqrt(
				float(x_diff * x_diff + y_diff * y_diff)
			);
		}

		value operator()(const coord& lval, const coord& rval)
		{
			return invoke(lval, rval);
		}
	};

	bool operator<(const coord& lval, const coord& rval)
	{
		bool result = lval[0] < rval[0];
		if (lval[0] == rval[0])
			result = lval[1] < rval[1];
		return result;
	}

	template <typename T>
	class grid_view
	{
	public:
		typedef T			value_type;
		typedef index_t		index_type;
		typedef index_type	size_type;
		typedef coord		coord_type;
		typedef typename astd::array_view<T>::iterator			iterator;
		typedef typename astd::array_view<T>::const_iterator	const_iterator;

		grid_view(astd::array_view<T> memory, size_type width, size_type height)
			: _memory(memory), _width(width), _height(height), _size(width * height)
		{
			assert(width > 0);
			assert(height > 0);
		}

		grid_view(const grid_view&) = default;
		grid_view& operator=(const grid_view&) = default;
		grid_view(grid_view&&) noexcept = default;
		grid_view& operator=(grid_view&&) noexcept = default;

		index_type index_from_coord(const coord& val) const
		{
			return val[0] + val[1] * _width;
		}

		coord coord_from_index(const size_type& index) const
		{
			return { index_t(index % _width), index_t(index / _width) };
		}

		bool coordinate_valid(const coord& pos) const {
			return pos[0] >= 0 && pos[0] < _width && pos[1] >= 0 && pos[1] < _height;
		}

		bool coordinate_valid(const index_type& pos) const {
			return pos < _memory.size();
		}
		
		template <typename Heuristic_T>
		typename Heuristic_T::value invoke_heuristic(const index_type& lval, const index_type& rval) const
		{
			return Heuristic_T::invoke(coord_from_index(lval), coord_from_index(rval));
		}

		const T& operator[](index_type value) const { return _memory[value]; }
		const T& operator[](const coord& value) const { return operator[](index_from_coord(value)); }

		size_type width() const { return _width; }
		size_type height() const { return _height; }
		size_type size() const { return _size; }
		const T* data() const { return _memory.data(); }
		const astd::array_view<T>& view() const { return _memory; }

		auto begin() const { return _memory.cbegin(); }
		auto end() const { return _memory.cend(); }
	private:
		astd::array_view<T> _memory;
		size_type _width;
		size_type _height;
		size_type _size;
	};

	template <typename square_validation>
	struct nearby_square_non_diag
	{
		template <typename T>
		fixed::vector<index_t, 4> operator()(const grid_view<T>& grid,
			index_t pos) const
		{
			return invoke(grid, pos);
		}

		template <typename T>
		static fixed::vector<index_t, 4> invoke(const grid_view<T>& grid,
			index_t pos)
		{
			fixed::vector<index_t, 4> result;
			coord xy = grid.coord_from_index(pos);

			if (typename grid_view<T>::size_type(xy[0] + 1) < grid.width() && square_validation::invoke(grid, pos + 1))
			{
				result.push_back(pos + 1);
			}
			if (typename grid_view<T>::size_type(xy[1] + 1) < grid.height() && square_validation::invoke(grid, pos + grid.width()))
			{
				result.push_back(pos + grid.width());
			}
			if (xy[0] >= 1 && square_validation::invoke(grid, pos - 1))
			{
				result.push_back(pos - 1);
			}
			if (xy[1] >= 1 && square_validation::invoke(grid, pos - grid.width()))
			{
				result.push_back(pos - grid.width());
			}
			return result;
		}
	};
}

namespace std {
	template <typename T>
	struct hash <grid2d::grid_view<T>>
	{
		typedef grid2d::grid_view<T> argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const argument_type& arg) const noexcept
		{
			const std::size_t FNV_offset_basis = 2166136261U;
			const std::size_t FNV_prime = 16777619U;

			std::size_t result = FNV_offset_basis;
			for (const T& v : arg)
			{
				result ^= (size_t)v;
				result *= FNV_prime;
			}
			result ^= arg.width();
			result ^= arg.height();
			return result;
		}
	};
}


#endif //!TWODGRID_HPP