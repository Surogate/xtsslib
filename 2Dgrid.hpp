#ifndef TWODGRID_HPP
#define TWODGRID_HPP

#include <cstddef>
#include <cstdlib>
#include "matrix.hpp"
#include "static_vector.hpp"
#include "aarray_view.hpp"
#include <array>
#include <functional>

namespace xts {
	using index_t = int;
	using coord2d = std::array<index_t, 2>;

	struct mahattan {
		typedef std::size_t value;

		value operator()(const coord2d& lval, const coord2d& rval)
		{
			return invoke(lval, rval);
		}

		static value invoke(const coord2d& lval, const coord2d& rval)
		{
			return std::abs(rval[0] - lval[0]) + std::abs(rval[1] - lval[1]);
		}
	};

	struct euclidean {
		typedef float value;

		static value invoke(const coord2d& lval, const coord2d& rval)
		{
			int x_diff = rval[0] - lval[0];
			int y_diff = rval[1] - lval[1];
			return std::sqrtf(
				float(x_diff * x_diff + y_diff * y_diff)
			);
		}

		value operator()(const coord2d& lval, const coord2d& rval)
		{
			return invoke(lval, rval);
		}
	};

	bool operator<(const coord2d& lval, const coord2d& rval)
	{
		bool result = lval[0] < rval[0];
		if (lval[0] == rval[0])
			result = lval[1] < rval[1];
		return result;
	}

	template <typename T>
	class grid2d_ref
	{
	public:
		typedef T value_type;
		typedef std::size_t index_type;
		typedef coord2d coord_type;

		grid2d_ref(astd::array_view<T> memory, std::size_t width, std::size_t height)
			: _memory(memory), _width(width), _height(height), _size(width * height)
		{
			assert(width > 0);
			assert(height > 0);
		}

		grid2d_ref(const grid2d_ref&) = default;
		grid2d_ref& operator=(const grid2d_ref&) = default;
		grid2d_ref(grid2d_ref&&) noexcept = default;
		grid2d_ref& operator=(grid2d_ref&&) noexcept = default;

		std::size_t index_from_coord(const coord2d& val) const
		{
			return val[0] + val[1] * _width;
		}

		coord2d coord_from_index(const std::size_t& index) const
		{
			return { index_t(index % _width), index_t(index / _width) };
		}

		bool coordinate_valid(const coord2d& pos) const {
			return pos[0] >= 0 && std::size_t(pos[0]) < _width && pos[1] >= 0 && std::size_t(pos[1]) < _height;
		}

		bool coordinate_valid(const std::size_t& pos) const {
			return pos < _memory.size();
		}
		
		template <typename Heuristic_T>
		typename Heuristic_T::value invoke_heuristic(const std::size_t& lval, const std::size_t& rval) const
		{
			return Heuristic_T::invoke(coord_from_index(lval), coord_from_index(rval));
		}

		const T& operator[](std::size_t value) const { return _memory[value]; }
		const T& operator[](const coord2d& value) const { return operator[](index_from_coord(value)); }

		std::size_t width() const { return _width; }
		std::size_t height() const { return _height; }
		std::size_t size() const { return _size; }
		const T* data() const { return _memory.data(); }

		auto begin() const { return _memory.cbegin(); }
		auto end() const { return _memory.cend(); }
	private:
		astd::array_view<T> _memory;
		std::size_t _width;
		std::size_t _height;
		std::size_t _size;
	};

	template <typename square_validation>
	struct nearby_square_non_diag
	{
		template <typename T>
		xts::static_vector<std::size_t, 4> operator()(const xts::grid2d_ref<T>& grid,
			std::size_t pos) const
		{
			return invoke(grid, pos);
		}

		template <typename T>
		static xts::static_vector<std::size_t, 4> invoke(const xts::grid2d_ref<T>& grid,
			std::size_t pos)
		{
			xts::static_vector<std::size_t, 4> result;
			xts::coord2d xy = grid.coord_from_index(pos);

			if (std::size_t(xy[0] + 1) < grid.width() && square_validation::invoke(grid, pos + 1))
			{
				result.push_back(pos + 1);
			}
			if (std::size_t(xy[1] + 1) < grid.height() && square_validation::invoke(grid, pos + grid.width()))
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
	struct hash <xts::grid2d_ref<T>>
	{
		typedef xts::grid2d_ref<T> argument_type;
		typedef std::size_t result_type;
		std::size_t operator()(const argument_type& arg) const noexcept
		{
			const size_t FNV_offset_basis = 2166136261U;
			const size_t FNV_prime = 16777619U;

			size_t Val = FNV_offset_basis;
			size_t index = 0;
			for (const T& c : arg)
			{	// fold in another byte
				Val ^= (size_t)c << (index % (sizeof(size_t) * 8));
				Val *= FNV_prime;
			}
			return (Val);
		}
	};
}


#endif //!TWODGRID_HPP