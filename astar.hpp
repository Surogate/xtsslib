#ifndef ASTAR_HPP
#define ASTAR_HPP

#include "aarray_view.hpp"
#include "operation_type.hpp"
#include <vector>

namespace a_star {
	namespace _impl {
		template <typename cost_t>
		struct index_data
		{
			index_data(int index = 0, cost_t c = std::numeric_limits<cost_t>::max()) : map_index(index), cost(c) {}
			index_data(const index_data&) = default;
			index_data& operator=(const index_data&) = default;
			index_data(index_data&&) noexcept = default;
			index_data& operator=(index_data&&) noexcept = default;

			int map_index;
			cost_t	cost;
		};

		struct coord_data_greater_cost
		{
			template <typename T>
			bool operator()(const T& lval, const T& rval)
			{
				return lval.cost > rval.cost;
			}
		};

		template <typename visited_dictionary>
		int fill_output(const visited_dictionary& visited,
			int local_start_index, typename visited_dictionary::const_iterator it_end_node,
			astd::array_ref<int>& out_buffer)
		{
			int result = 0;
			auto visited_end = visited.end();
			while (it_end_node != visited_end && it_end_node->first != local_start_index)
			{
				if (out_buffer.size() > 0)
				{
					out_buffer[result % out_buffer.size()] = it_end_node->first;
				}
				result++;

				it_end_node = visited.find(it_end_node->second.map_index);
			}
			if (out_buffer.size())
			{
				if (std::size_t(result) > out_buffer.size())
				{
					std::size_t pivot = result % out_buffer.size();
					std::rotate(out_buffer.begin(), out_buffer.begin() + pivot, out_buffer.end());
				}
				std::reverse(out_buffer.begin(), out_buffer.begin() + std::min(std::size_t(result), out_buffer.size()));
			}
			return int(result);
		}

		template <typename visited_dictionary, std::size_t static_size>
		int fill_output(const visited_dictionary& visited,
			int local_start_index, typename visited_dictionary::const_iterator it_end_node,
			fixed::vector<int, static_size>& out_buffer)
		{
			out_buffer.clear();
			int result = 0;
			auto visited_end = visited.end();
			while (it_end_node != visited_end && it_end_node->first != local_start_index)
			{
				if (out_buffer.size() < out_buffer.max_size())
				{
					out_buffer.push_back(it_end_node->first);
				}
				else
				{
					out_buffer[result % out_buffer.size()] = it_end_node->first;
				}
				result++;

				it_end_node = visited.find(it_end_node->second.map_index);
			}
			if (out_buffer.size())
			{
				if (std::size_t(result) > out_buffer.size())
				{
					std::size_t pivot = result % out_buffer.size();
					std::rotate(out_buffer.begin(), out_buffer.begin() + pivot, out_buffer.end());
				}
				std::reverse(out_buffer.begin(), out_buffer.begin() + std::min(std::size_t(result), out_buffer.size()));
			}
			return int(result);
		}

		template <typename visited_dictionary>
		int fill_output(const visited_dictionary& visited,
			int local_start_index, typename visited_dictionary::const_iterator it_end_node,
			std::vector<int>& out_buffer)
		{
			out_buffer.clear();
			auto visited_end = visited.end();
			while (it_end_node != visited_end && it_end_node->first != local_start_index)
			{
				out_buffer.push_back(it_end_node->first);
				it_end_node = visited.find(it_end_node->second.map_index);
			}
			if (out_buffer.size())
			{
				std::reverse(out_buffer.begin(), out_buffer.end());
			}
			return int(out_buffer.size());
		}

		template <typename visited_dictionary>
		int fill_output_inverted(const visited_dictionary& visited,
			int local_end_index,
			typename visited_dictionary::const_iterator it_end_node,
			astd::array_ref<int>& out_buffer)
		{
			int result = 0;
			auto visited_end = visited.end();
			it_end_node = visited.find(it_end_node->second.map_index);

			while (it_end_node != visited_end)
			{
				if (out_buffer.size() > 0 && std::size_t(result) < out_buffer.size())
				{
					out_buffer[result] = it_end_node->first;
				}
				result++;
				if (it_end_node->first != local_end_index)
					it_end_node = visited.find(it_end_node->second.map_index);
				else
					it_end_node = visited.end();
			}
			return int(result);
		}

		template <typename visited_dictionary, std::size_t static_size>
		int fill_output_inverted(const visited_dictionary& visited, 
			int local_end_index,
			typename visited_dictionary::const_iterator it_end_node,
			fixed::vector<int, static_size>& out_buffer)
		{
			out_buffer.clear();
			int result = 0;
			auto visited_end = visited.end();
			it_end_node = visited.find(it_end_node->second.map_index);

			while (it_end_node != visited_end)
			{
				if (out_buffer.size() < out_buffer.max_size())
				{
					out_buffer.push_back(it_end_node->first);
				}
				result++;

				if (it_end_node->first != local_end_index)
					it_end_node = visited.find(it_end_node->second.map_index);
				else
					it_end_node = visited.end();
			}
			return int(result);
		}

		template <typename visited_dictionary>
		int fill_output_inverted(const visited_dictionary& visited,
			int local_end_index,
			typename visited_dictionary::const_iterator it_end_node,
			std::vector<int>& out_buffer)
		{
			out_buffer.clear();
			auto visited_end = visited.end();
			it_end_node = visited.find(it_end_node->second.map_index);

			while (it_end_node != visited_end)
			{
				out_buffer.push_back(it_end_node->first);
				if (it_end_node->first != local_end_index)
					it_end_node = visited.find(it_end_node->second.map_index);
				else
					it_end_node = visited.end();
			}

			return int(out_buffer.size());
		}


		template <typename grid_type>
		bool can_salvage_previous_find(const grid_type& grid,
			int start, int end)
		{
			static thread_local int static_hash = 0;
			static thread_local int static_start = 0;
			static thread_local int static_end = 0;
			static thread_local int static_width = 0;
			static thread_local int static_height = 0;

			std::hash<grid_type> hasher;
			std::size_t hash = hasher(grid);

			if (start != static_start || end != static_end ||
				static_width != grid.width() || static_height != grid.height()
				|| hash != static_hash)
			{
				static_start = start;
				static_end = end;
				static_width = grid.width();
				static_height = grid.height();
				static_hash = hash;
				return false;
			}
			return true;
		}

		template <typename find_nearby_square, typename heuristic, typename grid_type, typename movement_cost, typename index_data>
		auto find_path(int local_start_index, int local_end_index, std::unordered_map<int, index_data>& visited
						, std::priority_queue<index_data, std::vector<index_data>, a_star::_impl::coord_data_greater_cost>& opened, const grid_type& grid)
		{
			typedef typename std::unordered_map<int, index_data>::value_type visited_value_type;
			bool found = false;
			auto end_node = visited.end();

			opened.push(index_data{ local_start_index, 0 });
			visited[local_start_index] = { {/*no previous coordinate*/ }, 0 };

			while (opened.size() && !found)
			{
				index_data top = opened.top();
				opened.pop();

				auto nearby_square = find_nearby_square::invoke(grid, top.map_index);
				auto nearby_beg = nearby_square.begin();
				auto nearby_end = nearby_square.end();
				while (nearby_beg != nearby_end && !found)
				{
					if (*nearby_beg == local_end_index)
					{
						found = true;
						end_node = visited.insert(visited_value_type{ *nearby_beg, index_data{ top.map_index, 0 } }).first;
					}
					else
					{
						index_data& previous_neighbor_value = visited[*nearby_beg]; //<- performance hog

						int neighbor_actual_cost = visited[top.map_index].cost + movement_cost::invoke(grid, top.map_index, *nearby_beg);
						if (neighbor_actual_cost < previous_neighbor_value.cost)
						{
							previous_neighbor_value.cost = neighbor_actual_cost;
							previous_neighbor_value.map_index = top.map_index;
							auto h_value = heuristic::invoke(grid.coord_from_index(local_end_index),
								grid.coord_from_index(*nearby_beg));
							opened.push(index_data{ *nearby_beg, neighbor_actual_cost + h_value });
						}
						++nearby_beg;
					}
				}
			}
			return std::make_pair(found, end_node);
		}
	}

	//If you can guarrantee a valid waypoint, divide the map in two, and run it in parallel.
	template <typename find_nearby_square, typename heuristic, typename grid_type, typename movement_cost
		, typename output_array_type>
	int find_path(
		const typename grid_type::coord_type& start,
		const typename grid_type::coord_type& end,
		const grid_type& grid,
		output_array_type& output)
	{
		assert(grid.coordinate_valid(start));
		assert(grid.coordinate_valid(end));

		int local_start_index = grid.index_from_coord(start);
		int local_end_index = grid.index_from_coord(end);

		if (grid[local_start_index] == 0
			|| grid[local_end_index] == 0)
		{
			return -1;
		}

		//No well defined behavior for this case, I hope this answer makes sense.
		if (local_start_index == local_end_index)
		{
			return 0;
		}

		typedef a_star::_impl::index_data<heuristic::value> index_data;
		typedef std::unordered_map<int, index_data> visited_dictionnary; //TODO: try later with boost::flat_map
		typedef std::priority_queue<index_data, std::vector<index_data>, a_star::_impl::coord_data_greater_cost> opened_deck;

		static thread_local visited_dictionnary visited;
		static thread_local std::vector<index_data> deck_container;
		static thread_local opened_deck opened;
		static thread_local bool found = false;
		static thread_local typename visited_dictionnary::iterator end_node;

		std::size_t hopeful_preallocation = 0;
		std::uint64_t distance = heuristic::invoke(start, end);

		//sadly I can't debug possible collisions in kattis but my hashing function seems solid
		if (grid.size() < 64 ||
			!a_star::_impl::can_salvage_previous_find(grid, local_start_index, local_end_index))
		{
			std::uint64_t max_prealloc = std::numeric_limits<int>::max() / (sizeof(int) * 70);
			std::uint64_t min_prealloc = 1024;
			hopeful_preallocation = std::size_t(std::min(std::max((distance / 10) * distance * distance, min_prealloc), max_prealloc));
			visited.clear();
			visited.reserve(hopeful_preallocation);
			deck_container.clear();
			deck_container.reserve(hopeful_preallocation);
			opened = opened_deck(a_star::_impl::coord_data_greater_cost(), deck_container);
			found = false;
			end_node = visited.end();
		}

		int result = -1;
		
		/*constexpr*/ if (movement_cost::op_type == xts::operation_type::commutative && heuristic::op_type == xts::operation_type::commutative)
		{
			if (!found)
			{
				/*(found, end_node)*/ auto find_result = _impl::find_path<find_nearby_square, heuristic, grid_type, movement_cost>(local_end_index, local_start_index, visited, opened, grid);
				found = find_result.first;
				end_node = find_result.second;
			}
			if (found)
			{
				result = a_star::_impl::fill_output_inverted(visited, local_end_index, end_node, output);
			}
		}
		else
		{
			if (!found)
			{
				/*(found, end_node)*/ auto find_result = _impl::find_path<find_nearby_square, heuristic, grid_type, movement_cost>(local_start_index, local_end_index, visited, opened, grid);
				found = find_result.first;
				end_node = find_result.second;
			}
			if (found)
			{
				result = a_star::_impl::fill_output(visited, local_start_index, end_node, output);
			}
		}

		return result;

	}


	template <typename find_nearby_square, typename heuristic, typename grid_type, typename movement_cost, std::size_t output_array_size>
	int find_path(
		const typename grid_type::coord_type& start,
		const typename grid_type::coord_type& end,
		const grid_type& grid,
		int(&output_buffer)[output_array_size]) 
	{
		return find_path<find_nearby_square, heuristic, grid_type, movement_cost, astd::array_ref<int>>
			(start, end, grid, astd::array_ref<int>(output_buffer));
	}
}

#endif //!