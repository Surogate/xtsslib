
#include <iostream>
#include <cstddef>
#include <array>
#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>
#include <utility>
#include <queue>
#include <chrono>
#include <random>
#include <memory>
#include <iterator>
#include <numeric>
#include <future>

#include "catch.hpp"
#include "2Dgrid.hpp"
#include "astar.hpp"
#include "operation_type.hpp"

//compatibility between vs2017 and gcc
#ifndef _WIN32

namespace stdext
{
	template <typename T>
	inline T checked_array_iterator(T val, std::size_t)
	{
		return val;
	}
}

#endif

typedef grid2d::grid_view<unsigned char> grid_type;
typedef grid2d::coord					coord_type;

struct square_valid
{
	bool operator()(const grid_type& grid, grid_type::index_type pos)
	{
		return invoke(grid, pos);
	}

	static bool invoke(const grid_type& grid, grid_type::index_type pos)
	{
		return grid[pos] == 1;
	}
};

struct movement_cost_to
{
	static constexpr xts::operation_type op_type = xts::operation_type::commutative;
	typedef int value_type;

	value_type operator()(const grid_type& grid, grid_type::index_type lval, grid_type::index_type rval)
	{
		return invoke(grid, lval, rval);
	}

	static value_type invoke(const grid_type& grid, grid_type::index_type lval, grid_type::index_type rval)
	{
		return 1u;
	}
};

using nearby_square_functor = grid2d::nearby_square_non_diag<square_valid>;
using heuristic = grid2d::mahattan;

template <typename Graph, typename Cost_T>
bool a_star_search
(const Graph& graph, typename Graph::index_type start, typename Graph::index_type goal,
	std::unordered_map<int, typename Graph::index_type>& came_from,
	std::unordered_map<int, Cost_T>& cost_so_far
)
{
	typedef a_star::_impl::index_data<heuristic::value> index_data;
	typedef square_valid square_validation;
	typedef std::priority_queue<index_data, std::vector<index_data>, a_star::_impl::coord_data_greater_cost> opened_deck;
	opened_deck frontier;
	frontier.push({ start, 0 });

	came_from[start] = start;
	cost_so_far[start] = 0;
	while (!frontier.empty())
	{
		auto current = frontier.top();
		frontier.pop();
		if (current.map_index == goal) {
			return true;
		}

		for (auto next : nearby_square_functor::invoke(graph, current.map_index))
		{
			Cost_T new_cost = cost_so_far[current.map_index] + 1;
			if (!cost_so_far.count(next) || new_cost < cost_so_far[next])
			{
				cost_so_far[next] = new_cost;

				int priority = new_cost + graph.invoke_heuristic<heuristic>(current.map_index, next);
				frontier.push(index_data{ next, priority });
				came_from[next] = current.map_index;
			}
		}
	}
	return false;
}

template <typename index_type>
std::vector<index_type> reconstruction_path(
	index_type start, index_type goal,
	std::unordered_map<int, index_type> came_from
)
{
	std::vector<index_type> result;
	int current = goal;
	while (current != start)
	{
		result.push_back(current);
		current = came_from[current];
	}
	std::reverse(result.begin(), result.end());
	return result;
}

//basic a* to test conformity of the optimized one
int test_find_path(
	const grid_type::index_type nStartX, const grid_type::index_type nStartY,
	const grid_type::index_type nTargetX, const grid_type::index_type nTargetY,
	grid_type grid,
	int* pOutBuffer, const int nOutBufferSize)
{
	std::unordered_map<int, grid_type::index_type> came_from;
	std::unordered_map<int, int> cost_so_far;
	auto start = grid.index_from_coord({ nStartX, nStartY });
	auto goal = grid.index_from_coord({ nTargetX, nTargetY });

	if (grid[start] == 0 || grid[goal] == 0) return -1;

	bool found = a_star_search(grid, start, goal,
		came_from, cost_so_far);
	if (found)
	{
		auto path = reconstruction_path(start, goal, came_from);
		for (int out_index = 0; out_index < nOutBufferSize && std::size_t(out_index) < path.size(); out_index++)
		{
			pOutBuffer[out_index] = path[out_index];
		}
		return int(path.size());
	}
	return -1;
}

void display_map(const unsigned char* map, int width, int height)
{
	std::cout << "map size: " << width << 'x' << height << std::endl;
	int size = width * height;
	std::cout << '{';
	for (int i = 0; i < size; i++)
	{
		if (!(i % width))
			std::cout << "\n";
		std::cout << char('0' + map[i]) << ", ";
	}
	std::cout << '\n' << '}' << std::endl;
}

void display_indexes(const int* buffer, int size)
{
	std::cout << "size: " << size << " : { ";
	if (size > 0)
		for (int i = 0; i < int(size); i++)
			std::cout << buffer[i] << ", ";
	std::cout << "}\n";
}

void display_coord(const grid2d::coord& value)
{
	std::cout << "x: " << value[0] << " y: " << value[1] << std::endl;
}

int randomized_test_a_star()
{
	static thread_local auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	static thread_local std::default_random_engine generator(static_cast<unsigned int>(seed));
	static thread_local std::vector<unsigned char> map;

	constexpr int max_map_side = 1000;
	constexpr float chances_to_get_valid_square = 0.8f;

	std::uniform_int_distribution<int> map_size_distribution(1, max_map_side);
	int nOutBufferSize = int(std::sqrt(max_map_side *max_map_side * 2));

	int nMapWidth = map_size_distribution(generator);
	int nMapHeight = map_size_distribution(generator);
	map.resize(nMapWidth * nMapHeight);

	std::vector<grid_type::index_type> result_1;
	std::vector<int> result_2;
	result_1.reserve(nOutBufferSize);
	result_2.resize(nOutBufferSize);

	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	for (int i = 0; std::size_t(i) < map.size(); i++)
	{
		map[i] = square_content_distribution(generator) ? 1 : 0;
	}

	std::uniform_int_distribution<int> X_value_distrib(0, nMapWidth - 1);
	std::uniform_int_distribution<int> Y_value_distrib(0, nMapHeight - 1);

	const int nStartX = X_value_distrib(generator);
	const int nStartY = Y_value_distrib(generator);
	const int nTargetX = X_value_distrib(generator);
	const int nTargetY = Y_value_distrib(generator);

	auto future1 = std::async(a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to, decltype(result_1)>,
		grid2d::coord{ nStartX, nStartY }, grid2d::coord{ nTargetX, nTargetY }, grid_type{ map, nMapWidth, nMapHeight }, std::ref(result_1));
	auto future2 = std::async(test_find_path, nStartX, nStartY, nTargetX, nTargetY, grid_type{ map, nMapWidth, nMapHeight }, &result_2[0], nOutBufferSize);

	int size_1 = future1.get();
	int size_2 = future2.get();

	if (size_1 != size_2)
	{
		display_coord({ nStartX, nStartY });
		display_coord({ nTargetX, nTargetY });
		display_map(map.data(), nMapWidth, nMapHeight);
		display_indexes(result_1.data(), size_1);
		display_indexes(result_2.data(), size_2);
	}


	CHECK(size_1 == size_2);
	if (size_1 != -1)
	{
		REQUIRE(size_1 == result_1.size());
		auto same = true;
		for (int i = 0; i < std::min(size_1, nOutBufferSize); i++)
		{
			same = same && result_1[i] == result_2[i];
			CHECK(result_1[i] >= 0);
			CHECK(result_1[i] < int(map.size()));
			CHECK(result_2[i] >= 0);
			CHECK(result_2[i] < int(map.size()));

			REQUIRE(map[result_1[i]] == 1);
			REQUIRE(map[result_2[i]] == 1);
		}
	}

	return 0;
}

std::pair<int, double> max_a_star(int nMapWidth, int nMapHeight)
{
	static thread_local auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	static thread_local std::default_random_engine generator(static_cast<unsigned int>(seed));
	static thread_local std::vector<unsigned char> map;
	static thread_local std::vector<int> result;

	constexpr float chances_to_get_valid_square = 0.8f;

	map.resize(nMapWidth * nMapHeight);
	int max_result = nMapWidth * nMapHeight / 2;
	result.reserve(max_result);


	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	for (int i = 0; i < nMapWidth * nMapHeight; i++)
	{
		map[i] = square_content_distribution(generator) ? 1 : 0;
	}

	map.front() = 1;
	map.back() = 1;

	const int nStartX = 0;
	const int nStartY = 0;
	const int nTargetX = nMapWidth - 1;
	const int nTargetY = nMapHeight - 1;

	auto start = std::chrono::system_clock::now();
	int success = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ nStartX, nStartY },
		{ nTargetX, nTargetY },
		{ map, nMapWidth, nMapHeight },
		result);
	std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;

	if (success > 0)
	{
		for (int i = 0; i < std::min(success, max_result); i++)
		{
			REQUIRE(std::size_t(result[i]) < map.size());
			CHECK(map[result[i]] == 1);
		}
	}

	return { success, diff .count() };
}

template <grid_type::size_type side_map_size>
void test_map_hash()
{
	static_assert(side_map_size > 0, "pass a valid map size");
	static thread_local auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	static thread_local std::default_random_engine generator(static_cast<unsigned int>(seed));
	static thread_local std::vector<unsigned char> map;

	constexpr int map_size = side_map_size * side_map_size;
	map.resize(map_size);
	float chances_to_get_valid_square = 0.8f;
	constexpr int sample_size = 10000u > map_size ? map_size : 10000u;
	std::array<std::uint64_t, sample_size> hash;
	int collision_num = 0;
	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	grid_type grid(map, side_map_size, side_map_size);
	std::hash<grid_type> hasher;
	for (int sample_num = 0; sample_num < sample_size; sample_num++)
	{
		for (int map_index = 0; map_index < map_size; map_index++)
		{
			map[map_index] = square_content_distribution(generator) ? 1 : 0;
		}
		
		hash[sample_num] = hasher(grid);
	}

	std::sort(hash.begin(), hash.end());
	auto beg = hash.begin();
	auto end = hash.end();

	while (beg != end)
	{
		auto diff = std::find_if(beg, end, [beg](const std::uint64_t& val) {return val != *beg; });
		collision_num += diff - beg - 1;
		beg = diff;
	}

	std::cout << "collision probability on " << map_size << " grid: " << (float(collision_num) / float(sample_size)) * 100 << std::endl;
}

void profiling_performance()
{
	int success = 100;
	for (int iter = 0; iter < success; iter++)
	{
		if (max_a_star(1000, 1000).first == -1)
			success--;
	}
	std::cout << "success rate " << float(success) << std::endl;

	std::cout << "a* on big values" << std::endl;
	for (int i = 500; i < 5000; i += 500)
	{
		int success_percent = 0;
		double total_time = 0;
		for (int num = 0; num < 100; num++)
		{
			auto result = max_a_star(i, i);
			if (result.first != -1)
			{
				success_percent++;
				total_time += result.second;
			}
		}
		std::cout << i << 'x' << i << " | success " << success_percent << "  time " << total_time / success_percent << " s" << std::endl;
	}
}

void basic_test_a_star()
{
	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 1, 1, 0, 1,
			//10 11 12 13 14
			1, 1, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 0, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		std::array<fixed::vector<grid2d::index_t, 4>, 5 * 5> valid_squares =
		{
			fixed::vector<grid2d::index_t, 4>
		{1u, 5u},{ 2u, 6u, 0u },{ 3u, 7u, 1u },{ 4u, 2u },{ 9u, 3u },
		{ 6u, 10u, 0u },{ 7u, 11u, 5u, 1u },{ 12u, 6u, 2u },{ 9u, 7u, 3u },{ 14u, 4u },
		{ 11u, 15u, 5u },{ 12u, 10u, 6u },{ 11u, 7u },{ 14u, 12u },{ 19u, 9u },
		{ 20u, 10u },{ 21u, 15u, 11u },{ 22u, 12u },{ 19u, 23u },{ 24u, 14u },
		{ 21u, 15u },{ 22u, 20u },{ 23u, 21u },{ 24u, 22u },{ 23u, 19u }
		};

		for (int i = 0; i < 5 * 5; i++)
		{
			auto square_found = nearby_square_functor::invoke(grid_type{ pMap, 5, 5 }, i);
			REQUIRE(square_found == valid_squares[i]);
		}
	}

	{
		unsigned char pMap[] =
		{ 0, 0, 1,
		0, 1, 1,
		1, 0, 1 };
		int pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>(grid2d::coord{ 2, 0 }, grid2d::coord{ 0, 2 }, { pMap, 3, 3 }, astd::array_ref<int>(pOutBuffer));
		auto value2 = test_find_path(2, 0, 0, 2, { pMap, 3, 3 }, pOutBuffer2, 7);
		REQUIRE(value == -1);
		REQUIRE(value == value2);
	}

	{
		unsigned char pMap[] =
		{ 1, 1,
		0, 1 };
		int pOutBuffer[7];
		int pOutBuffer2[7];
		astd::array_ref<int> output(pOutBuffer);
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 1 }, { pMap, 2, 2 }, output);
		auto value2 = test_find_path(0, 0, 1, 1, { pMap, 2, 2 }, pOutBuffer2, 7);
		REQUIRE(value == 2);
		REQUIRE(value == value2);
		int output_index = 0;
		for (auto expected_index : { 1, 3 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			REQUIRE(pOutBuffer[output_index] == pOutBuffer2[output_index]);

			++output_index;
		}
	}

	{
		unsigned char pMap[] =
		{ 1, 5,
		0, 1 };
		int pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 1 }, { pMap, 2, 2 }, pOutBuffer);
		auto value2 = test_find_path(0, 0, 1, 1, { pMap, 2, 2 }, pOutBuffer2, 7);
		REQUIRE(value == -1);
		REQUIRE(value == value2);
	}

	{
		unsigned char pMap[] =
		{ 1, 1, 1, 1, 1, 1 };
		int pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 5, 0 }, { pMap, 6, 1 }, pOutBuffer);
		auto value2 = test_find_path(0, 0, 5, 0, { pMap, 6, 1 }, pOutBuffer2, 7);
		REQUIRE(value == 5);
		REQUIRE(value2 == value);
		int output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 5 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
		{ 1, 1, 1, 1, 1, 1 };
		int pOutBuffer[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 0, 5 }, { pMap, 1, 6 }, pOutBuffer);
		REQUIRE(value == 5);
		int output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 5 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
		{ 1, 1, 1, 1,
		0, 1, 0, 1,
		0, 1, 1, 1 };
		int pOutBuffer[12];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 2 }, { pMap, 4, 3 }, pOutBuffer);
		REQUIRE(value == 3);
		int output_index = 0;
		for (auto expected_index : { 1, 5, 9 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 1, 1, 0, 1,
			//10 11 12 13 14
			1, 1, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 0, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		int pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 4, 4 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 8);
		std::array<int, 8> result1 = { 1, 2, 3, 4, 9, 14, 19, 24 };
		std::array<int, 8> result2 = { 5, 10, 15, 20, 21, 22, 23, 24 };
		for (int i = 0; i < 8; i++)
		{
			bool ok = pOutBuffer[i] == result1[i] || pOutBuffer[i] == result2[i];
			CHECK(ok == true);
		}
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 1, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		int pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 2, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		assert(value == 10);
		int output_index = 0;
		for (auto expected_index : { 3, 4, 9, 14, 19, 24, 23, 22, 17, 12 })
		{
			CHECK(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 1, 1,
			//15 16 17 18 19
			1, 0, 1, 1, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		int pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 8);
		int output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 9, 14, 13, 12 })
		{
			CHECK(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 0, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		int pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == -1);
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4
		{ 1, 1, 1, 1, 1,
			//5  6  7  8  9
			1, 0, 0, 0, 0,
			//10 11 12 13 14
			1, 0, 1, 1, 1,
			//15 16 17 18 19
			1, 0, 0, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		int pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 12);
		int output_index = 0;
		for (auto expected_index : { 5, 10, 15, 20, 21, 22, 23, 24, 19, 14, 13, 12 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
		  //0  1  2  3  4  5  6  7  8  9
		{ 
			1, 1, 1, 1, 0, 1, 1, 1, 1, 1, //0
			1, 0, 0, 1, 0, 1, 0, 1, 0, 1, //1
			1, 0, 1, 1, 0, 1, 1, 1, 0, 1, //2
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1, //3
			1, 1, 1, 1, 1, 1, 0, 1, 0, 1, //4
			1, 1, 1, 1, 0, 1, 1, 0, 1, 1, //5
			1, 0, 0, 1, 1, 1, 0, 1, 1, 0, //6
			1, 0, 0, 1, 1, 1, 0, 1, 1, 0, //7
			1, 0, 0, 1, 0, 1, 0, 1, 1, 1, //8
			1, 1, 0, 1, 0, 1, 1, 1, 0, 1  //9
		};

		int pOutBuffer[30];
		std::array<int, 20> expected_result		= { 10, 20, 30, 40, 41, 42, 52, 53, 63, 73, 74, 75, 85, 95, 96, 97, 87, 88, 89, 99 };
		std::array<int, 20> expected_result2	= { 10, 20, 30, 40, 41, 42, 43, 53, 63, 73, 74, 75, 85, 95, 96, 97, 87, 88, 89, 99 };

		for (int i = 0; i < 20; i++)
		{
			astd::array_ref<int> output(pOutBuffer, i);
			auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 9, 9 }, { pMap, 10, 10 }, output);
			REQUIRE(value == 20);
			for (int expected_result_index = 0; expected_result_index < i; expected_result_index++)
			{
				bool ok = pOutBuffer[expected_result_index] == expected_result[expected_result_index] ||
					pOutBuffer[expected_result_index] == expected_result2[expected_result_index];
				CHECK(ok == true);
			}
		}
	}

	{
		unsigned char pMap[] =
			//0  1  2  3  4  5  6  7  8  9
		{ 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, //0
		1, 0, 0, 1, 0, 1, 0, 1, 0, 1, //1
		1, 0, 1, 1, 0, 1, 1, 1, 0, 1, //2
		1, 0, 1, 0, 0, 0, 0, 1, 0, 1, //3
		1, 1, 1, 1, 1, 1, 0, 1, 0, 1, //4
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1  //5
		};

		int pOutBuffer1[32];
		int pOutBuffer2[16];

		astd::array_ref<int> output1{ pOutBuffer1 };
		astd::array_ref<int> output2{ pOutBuffer2 };

		auto path1 = std::async(a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to, astd::array_ref<int>>, grid2d::coord{ 0, 0 }, grid2d::coord{ 4, 4 }, grid_type{ pMap, 10, 6 }, output1);
		auto path2 = std::async(a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to, astd::array_ref<int>>, grid2d::coord{ 4, 4 }, grid2d::coord{ 9, 0 }, grid_type{ pMap, 10, 6 }, output2);
		int size_out_1 = path1.get();
		int size_out_2 = path2.get();
		REQUIRE(size_out_1 == 8);
		REQUIRE(size_out_2 == 11);
		std::move(pOutBuffer2, pOutBuffer2 + size_out_2,
			stdext::checked_array_iterator<int*>(pOutBuffer1 + size_out_1, int(32 - size_out_1)));
		int output_index = 0;
		std::array<int, 19> result1 = { 10, 20, 30, 40, 41, 42, 43, 44,
			45, 55, 56, 57, 58, 59, 49, 39, 29, 19, 9 };
		std::array<int, 19> result2 = { 10, 20, 30, 40, 41, 42, 43, 44,
			45, 55, 56, 57, 47, 37, 27, 17, 7, 8, 9 };

		for (int i = 0; i < 19; i++)
		{
			bool ok = pOutBuffer1[i] == result1[i] || pOutBuffer1[i] == result2[i];
			CHECK(ok == true);
		}
	}

	{
		unsigned char pMap[] = 
		{
		//  0  1  2  3  4  5  6  7  8  9
			0, 0, 1, 1, 1, 1, 1, 1, 0, 1,	//0
			0, 1, 1, 0, 1, 1, 1, 1, 1, 1,	//1
			1, 0, 0, 0, 1, 1, 1, 1, 1, 1,	//2
			1, 1, 1, 1, 1, 1, 0, 1, 1, 1,	//3
			1, 1, 1, 0, 0, 1, 1, 1, 1, 0	//4
		};

		int pOutBuffer1[16];
		int pOutBuffer2[16];

		auto size1 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 8, 4 }, { 6, 2 }, { pMap, 10, 5 }, pOutBuffer1);
		auto size2 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 8, 4 }, { 6, 2 }, { pMap, 10, 5 }, pOutBuffer2);

		std::array<int, 4> expected = { 47, 37, 27, 26 };
		std::array<int, 4> expected2 = { 38, 28, 27, 26 };
		REQUIRE(int(size1) == expected.size());
		REQUIRE(int(size2) == expected.size());
		for (int i = 0; std::size_t(i) < expected.size(); i++)
		{
			auto value1 = pOutBuffer1[i] == expected[i] || pOutBuffer1[i] == expected2[i];
			auto value2 = pOutBuffer2[i] == expected[i] || pOutBuffer2[i] == expected2[i];
			CHECK(value1);
			CHECK(value2);
		}
	}

	{
		unsigned char pMap[] =
		{
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 1, 1, 1, 1, 1, 0,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 1, 0, 0, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
		};

		int pOutBuffer[15];
		fixed::vector<int, 15> static_output;
		std::vector<int> dyn_output;

		auto value1 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, pOutBuffer);
		auto value2 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, static_output);
		auto value3 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, dyn_output);
		auto expected = { 40, 30, 20, 10, 11, 1, 2, 3, 4, 5, 6, 7, 17, 27, 28 };
		REQUIRE(int(value1) == expected.size());
		CHECK(value1 == value2);
		CHECK(value2 == value3);
		CHECK(int(value3) == dyn_output.size());
		CHECK(int(value2) == static_output.size());
		for (int result_index = 0; result_index < 15; result_index++)
		{
			CHECK(pOutBuffer[result_index] == static_output[result_index]);
			CHECK(static_output[result_index] == dyn_output[result_index]);
			result_index++;
		}
	}

	{
		unsigned char pMap[] = {
		1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1,
		0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1,
		1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1,
		1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1,
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1,
		1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1,
		1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
		1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
		1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
		1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0,
		1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0,
		1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
		1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1,
		1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1,
		1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1,
		1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
		1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0,
		0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1,
		0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,
		1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1,
		1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,
		1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,
		1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1
		};

		int pOutBuffer[70];

		auto size = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 32, 35 }, { 31, 76 }, { pMap, 33, 95 }, pOutBuffer);
		REQUIRE(size == 60);
		std::array<int, 70> expected_result = { 1220, 1253, 1286, 1319, 1318, 1317, 1350, 1349, 1348, 1381, 1380, 1413, 1446, 1479, 1512, 
			1545, 1544, 1577, 1576, 1609, 1642, 1675, 1708, 1707, 1740, 1773, 1806, 1839, 1838, 1837, 1870, 1903, 1936, 
			1969, 2002, 2003, 2036, 2069, 2070, 2103, 2136, 2169, 2170, 2203, 2236, 2269, 2302, 2335, 2368, 2401, 2402, 
			2403, 2404, 2405, 2406, 2407, 2440, 2473, 2506, 2539 };
		std::array<std::size_t, 70> expected_result2 = { 1220, 1253, 1286, 1319, 1352, 1351, 1350, 1349, 1348, 1381, 1380, 1413, 1446, 1479, 1512,
			1545, 1544, 1577, 1576, 1609, 1642, 1675, 1708, 1707, 1740, 1773, 1806, 1839, 1838, 1837, 1870, 1903, 1936,
			1969, 2002, 2003, 2036, 2069, 2070, 2103, 2136, 2169, 2170, 2203, 2236, 2269, 2302, 2335, 2368, 2401, 2402,
			2403, 2404, 2405, 2406, 2407, 2440, 2473, 2506, 2539 };
		
		//display_indexes(pOutBuffer, std::min(size, 70));
		for (int i = 0; i < std::min(size, 70); i++)
		{
			CHECK(pMap[pOutBuffer[i]] == 1);
			bool valid = pOutBuffer[i] == expected_result[i] || pOutBuffer[i] == expected_result2[i];
			CHECK(valid);
		}
	}
}

void test_hash()
{
	test_map_hash<2>();
	test_map_hash<4>();
	test_map_hash<8>();
	test_map_hash<16>();
	test_map_hash<32>();
	test_map_hash<64>();
	test_map_hash<128>();
}

void conformity_test()
{
	for (int i = 0; i < 1000; i++)
		randomized_test_a_star();
}

TEST_CASE("test a star", "[astar]")
{
	basic_test_a_star();
	conformity_test();
	profiling_performance();
//	test_hash();
}
