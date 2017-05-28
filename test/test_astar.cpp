
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
#include "static_vector.hpp"
#include "2Dgrid.hpp"
#include "astar.hpp"

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

typedef xts::grid2d_ref<unsigned char> grid_type;

struct square_valid
{
	bool operator()(const grid_type& grid, std::size_t pos)
	{
		return invoke(grid, pos);
	}

	static bool invoke(const grid_type& grid, std::size_t pos)
	{
		return grid[pos] == 1;
	}
};

struct movement_cost_to
{
	typedef std::size_t value_type;

	value_type operator()(const grid_type& grid, std::size_t lval, std::size_t rval)
	{
		return invoke(grid, lval, rval);
	}

	static value_type invoke(const grid_type& grid, std::size_t lval, std::size_t rval)
	{
		return 1u;
	}
};

typedef xts::nearby_square_non_diag<square_valid> nearby_square_functor;
typedef xts::mahattan heuristic;

template <typename Graph, typename Cost_T>
bool a_star_search
(const Graph& graph, std::size_t start, std::size_t goal,
	std::unordered_map<std::size_t, xts::index_t>& came_from,
	std::unordered_map<std::size_t, Cost_T>& cost_so_far
)
{
	typedef a_star::index_data<heuristic::value> index_data;
	typedef square_valid square_validation;
	typedef std::priority_queue<index_data, std::vector<index_data>, a_star::coord_data_greater_cost> opened_deck;
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

				std::size_t priority = new_cost + graph.invoke_heuristic<heuristic>(current.map_index, next);
				frontier.push(index_data{ next, priority });
				came_from[next] = current.map_index;
			}
		}
	}
	return false;
}

std::vector<xts::index_t> reconstruction_path(
	std::size_t start, std::size_t goal,
	std::unordered_map<std::size_t, xts::index_t> came_from
)
{
	std::vector<xts::index_t> result;
	std::size_t current = goal;
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
	const xts::index_t nStartX, const xts::index_t nStartY,
	const xts::index_t nTargetX, const xts::index_t nTargetY,
	grid_type grid,
	int* pOutBuffer, const int nOutBufferSize)
{
	std::unordered_map<std::size_t, xts::index_t> came_from;
	std::unordered_map<std::size_t, std::size_t> cost_so_far;
	auto start = grid.index_from_coord({ nStartX, nStartY });
	auto goal = grid.index_from_coord({ nTargetX, nTargetY });

	if (grid[start] == 0 || grid[goal] == 0) return -1;

	bool found = a_star_search(grid, start, goal,
		came_from, cost_so_far);
	if (found)
	{
		auto path = reconstruction_path(start, goal, came_from);
		for (std::size_t out_index = 0; out_index < std::size_t(nOutBufferSize) && out_index < path.size(); out_index++)
		{
			pOutBuffer[out_index] = path[out_index];
		}
		return int(path.size());
	}
	return -1;
}

void display_map(const unsigned char* map, std::size_t width, std::size_t height)
{
	std::cout << "map size: " << width << 'x' << height << std::endl;
	std::size_t size = width * height;
	std::cout << '{';
	for (std::size_t i = 0; i < size; i++)
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
		for (std::size_t i = 0; i < std::size_t(size); i++)
			std::cout << buffer[i] << ", ";
	std::cout << "}\n";
}

void display_indexes(const std::size_t* buffer, int size)
{

	std::cout << "size: " << size << " : { ";
	if (size > 0)
		for (std::size_t i = 0; i < std::size_t(size); i++)
			std::cout << buffer[i] << ", ";
	std::cout << "}\n";
}

void display_coord(const xts::coord2d& value)
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

	std::size_t nMapWidth = map_size_distribution(generator);
	std::size_t nMapHeight = map_size_distribution(generator);
	map.resize(nMapWidth * nMapHeight);

	std::vector<std::size_t> result_1;
	std::vector<int> result_2;
	result_1.reserve(nOutBufferSize);
	result_2.resize(nOutBufferSize);

	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	for (std::size_t i = 0; i < map.size(); i++)
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
		xts::coord2d{ nStartX, nStartY }, xts::coord2d{ nTargetX, nTargetY }, grid_type{ map, nMapWidth, nMapHeight }, std::ref(result_1));
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
		for (std::size_t i = 0; i < std::size_t(std::min(size_1, nOutBufferSize)); i++)
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

std::pair<int, double> max_a_star(std::size_t nMapWidth, std::size_t nMapHeight)
{
	static thread_local auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	static thread_local std::default_random_engine generator(static_cast<unsigned int>(seed));
	static thread_local std::vector<unsigned char> map;
	static thread_local std::vector<std::size_t> result;

	constexpr float chances_to_get_valid_square = 0.8f;

	map.resize(std::size_t(nMapWidth) * std::size_t(nMapHeight));
	std::size_t max_result = std::size_t(nMapWidth) * std::size_t(nMapHeight) / 2;
	result.reserve(max_result);


	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	for (std::size_t i = 0; i < nMapWidth * nMapHeight; i++)
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
		for (std::size_t i = 0; i < std::size_t(std::min(std::size_t(success), max_result)); i++)
		{
			REQUIRE(result[i] < map.size());
			CHECK(map[result[i]] == 1);
		}
	}

	return { success, diff.count() };
}

template <std::size_t side_map_size>
void test_map_hash()
{
	static_assert(side_map_size > 0, "pass a valid map size");
	static thread_local auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	static thread_local std::default_random_engine generator(static_cast<unsigned int>(seed));
	static thread_local std::vector<unsigned char> map;

	constexpr std::size_t map_size = side_map_size * side_map_size;
	map.resize(map_size);
	float chances_to_get_valid_square = 0.8f;
	constexpr std::size_t sample_size = 10000u > map_size ? map_size : 10000u;
	std::array<std::uint64_t, sample_size> hash;
	std::size_t collision_num = 0;
	std::bernoulli_distribution square_content_distribution(chances_to_get_valid_square);

	xts::grid2d_ref<unsigned char> grid(map, side_map_size, side_map_size);
	std::hash<xts::grid2d_ref<unsigned char>> hasher;
	for (std::size_t sample_num = 0; sample_num < sample_size; sample_num++)
	{
		for (std::size_t map_index = 0; map_index < map_size; map_index++)
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
		auto result = max_a_star(i, i);
		std::cout << i << 'x' << i << " | success " << (result.first != -1) << "  time " << result.second << " s" << std::endl;
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

		std::array<xts::static_vector<std::size_t, 4>, 5 * 5> valid_squares =
		{
		xts::static_vector<std::size_t, 4>
		{1u, 5u},{ 2u, 6u, 0u },{ 3u, 7u, 1u },{ 4u, 2u },{ 9u, 3u },
		{ 6u, 10u, 0u },{ 7u, 11u, 5u, 1u },{ 12u, 6u, 2u },{ 9u, 7u, 3u },{ 14u, 4u },
		{ 11u, 15u, 5u },{ 12u, 10u, 6u },{ 11u, 7u },{ 14u, 12u },{ 19u, 9u },
		{ 20u, 10u },{ 21u, 15u, 11u },{ 22u, 12u },{ 19u, 23u },{ 24u, 14u },
		{ 21u, 15u },{ 22u, 20u },{ 23u, 21u },{ 24u, 22u },{ 23u, 19u }
		};

		for (std::size_t i = 0; i < 5 * 5; i++)
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
		std::size_t pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>(xts::coord2d{ 2, 0 }, xts::coord2d{ 0, 2 }, { pMap, 3, 3 }, astd::array_ref<std::size_t>(pOutBuffer));
		auto value2 = test_find_path(2, 0, 0, 2, { pMap, 3, 3 }, pOutBuffer2, 7);
		REQUIRE(value == -1);
		REQUIRE(value == value2);
	}

	{
		unsigned char pMap[] =
		{ 1, 1,
		0, 1 };
		std::size_t pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 1 }, { pMap, 2, 2 }, astd::array_ref<std::size_t>(pOutBuffer));
		auto value2 = test_find_path(0, 0, 1, 1, { pMap, 2, 2 }, pOutBuffer2, 7);
		REQUIRE(value == 2);
		REQUIRE(value == value2);
		std::size_t output_index = 0;
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
		std::size_t pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 1 }, { pMap, 2, 2 }, pOutBuffer);
		auto value2 = test_find_path(0, 0, 1, 1, { pMap, 2, 2 }, pOutBuffer2, 7);
		REQUIRE(value == -1);
		REQUIRE(value == value2);
	}

	{
		unsigned char pMap[] =
		{ 1, 1, 1, 1, 1, 1 };
		std::size_t pOutBuffer[7];
		int pOutBuffer2[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 5, 0 }, { pMap, 6, 1 }, pOutBuffer);
		auto value2 = test_find_path(0, 0, 5, 0, { pMap, 6, 1 }, pOutBuffer2, 7);
		REQUIRE(value == 5);
		REQUIRE(value2 == value);
		std::size_t output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 5 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] =
		{ 1, 1, 1, 1, 1, 1 };
		std::size_t pOutBuffer[7];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 0, 5 }, { pMap, 1, 6 }, pOutBuffer);
		REQUIRE(value == 5);
		std::size_t output_index = 0;
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
		std::size_t pOutBuffer[12];
		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 1, 2 }, { pMap, 4, 3 }, pOutBuffer);
		REQUIRE(value == 3);
		std::size_t output_index = 0;
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

		std::size_t pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 4, 4 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 8);
		std::size_t output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 9, 14, 19, 24 })
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
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 1, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		std::size_t pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 2, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		assert(value == 10);
		std::size_t output_index = 0;
		for (auto expected_index : { 3, 4, 9, 14, 19, 24, 23, 22, 17, 12 })
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
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 1, 1,
			//15 16 17 18 19
			1, 0, 1, 1, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		std::size_t pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 8);
		std::size_t output_index = 0;
		for (auto expected_index : { 1, 2, 3, 4, 9, 14, 13, 12 })
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
			1, 0, 0, 0, 1,
			//10 11 12 13 14
			1, 0, 1, 0, 1,
			//15 16 17 18 19
			1, 0, 0, 0, 1,
			//20 21 22 23 24
			1, 1, 1, 1, 1
		};

		std::size_t pOutBuffer[12];

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

		std::size_t pOutBuffer[12];

		auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 2, 2 }, { pMap, 5, 5 }, pOutBuffer);
		REQUIRE(value == 12);
		std::size_t output_index = 0;
		for (auto expected_index : { 5, 10, 15, 20, 21, 22, 23, 24, 19, 14, 13, 12 })
		{
			REQUIRE(pOutBuffer[output_index] == expected_index);
			++output_index;
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
		1, 1, 1, 1, 0, 1, 1, 0, 1, 1, //5
		1, 0, 0, 1, 1, 1, 0, 1, 1, 0, //6
		1, 0, 0, 1, 1, 1, 0, 1, 1, 0, //7
		1, 0, 0, 1, 0, 1, 0, 1, 1, 1, //8
		1, 1, 0, 1, 0, 1, 1, 1, 0, 1 //9
		};

		std::size_t pOutBuffer[30];
		std::array<int, 20> expected_result = { 10, 20, 30, 40, 41, 42, 52, 53, 63, 73, 74, 75, 85, 95, 96, 97, 87, 88, 89, 99 };

		for (std::size_t i = 0; i < 20; i++)
		{
			auto value = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 0, 0 }, { 9, 9 }, { pMap, 10, 10 }, astd::array_ref<std::size_t>(pOutBuffer, i));
			REQUIRE(value == 20);
			for (std::size_t expected_result_index = 0; expected_result_index < i; expected_result_index++)
			{
				REQUIRE(pOutBuffer[expected_result_index] == expected_result[expected_result_index]);
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

		std::size_t pOutBuffer1[32];
		std::size_t pOutBuffer2[16];

		auto path1 = std::async(a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to, astd::array_ref<std::size_t>>, xts::coord2d{ 0, 0 }, xts::coord2d{ 4, 4 }, grid_type{ pMap, 10, 6 }, astd::array_ref<std::size_t>{pOutBuffer1});
		auto path2 = std::async(a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to, astd::array_ref<std::size_t>>, xts::coord2d{ 4, 4 }, xts::coord2d{ 9, 0 }, grid_type{ pMap, 10, 6 }, astd::array_ref<std::size_t>{pOutBuffer2});
		int size_out_1 = path1.get();
		int size_out_2 = path2.get();
		REQUIRE(size_out_1 == 8);
		REQUIRE(size_out_2 == 11);
		std::move(pOutBuffer2, pOutBuffer2 + size_out_2,
			stdext::checked_array_iterator<std::size_t*>(pOutBuffer1 + size_out_1, std::size_t(32 - size_out_1)));
		std::size_t output_index = 0;
		for (auto expected_index : { 10, 20, 30, 40, 41, 42, 43, 44,
		45, 55, 56, 57, 58, 59, 49, 39, 29, 19, 9 })
		{
			REQUIRE(pOutBuffer1[output_index] == expected_index);
			++output_index;
		}
	}

	{
		unsigned char pMap[] = {
			//  0  1  2  3  4  5  6  7  8  9
			0, 0, 1, 1, 1, 1, 1, 1, 0, 1,	//0
			0, 1, 1, 0, 1, 1, 1, 1, 1, 1,	//1
			1, 0, 0, 0, 1, 1, 1, 1, 1, 1,	//2
			1, 1, 1, 1, 1, 1, 0, 1, 1, 1,	//3
			1, 1, 1, 0, 0, 1, 1, 1, 1, 0	//4
		};

		std::size_t pOutBuffer1[16];
		std::size_t pOutBuffer2[16];

		auto size1 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 8, 4 }, { 6, 2 }, { pMap, 10, 5 }, pOutBuffer1);
		auto size2 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 8, 4 }, { 6, 2 }, { pMap, 10, 5 }, pOutBuffer2);

		auto expected = { 47, 37, 27, 26 };
		REQUIRE(std::size_t(size1) == expected.size());
		REQUIRE(std::size_t(size2) == expected.size());
		auto i = 0;
		for (auto& value : expected)
		{
			REQUIRE(pOutBuffer1[i] == value);
			REQUIRE(pOutBuffer2[i] == value);
			++i;
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

		std::size_t pOutBuffer[15];
		xts::static_vector<std::size_t, 15> static_output;
		std::vector<std::size_t> dyn_output;

		auto value1 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, pOutBuffer);
		auto value2 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, static_output);
		auto value3 = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 1, 4 }, { 8, 2 }, { pMap, 10, 5 }, dyn_output);
		auto expected = { 40, 30, 20, 10, 11, 1, 2, 3, 4, 5, 6, 7, 17, 27, 28 };
		REQUIRE(std::size_t(value1) == expected.size());
		CHECK(value1 == value2);
		CHECK(value2 == value3);
		CHECK(std::size_t(value3) == dyn_output.size());
		CHECK(std::size_t(value2) == static_output.size());
		std::size_t result_index = 0;
		for (auto& ex_value : expected)
		{
			REQUIRE(pOutBuffer[result_index] == ex_value);
			REQUIRE(static_output[result_index] == ex_value);
			REQUIRE(dyn_output[result_index] == ex_value);
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
		std::size_t pOutBuffer[70];

		auto size = a_star::find_path<nearby_square_functor, heuristic, grid_type, movement_cost_to>({ 32, 35 }, { 31, 76 }, { pMap, 33, 95 }, pOutBuffer);
		REQUIRE(size == 60);
		std::size_t result_index = 0;
		//testing::display_indexes(pOutBuffer, std::min(size, 70));
		for (auto& ex_value : { 1220, 1253, 1286, 1319, 1352, 1351, 1350, 1349, 1348, 1381, 1380, 1413, 1446, 1479, 1512,
		1545, 1544, 1577, 1576, 1609, 1642, 1675, 1708, 1707, 1740, 1773, 1806, 1839, 1838, 1837, 1870, 1903, 1936,
		1969, 2002, 2003, 2036, 2069, 2070, 2103, 2136, 2169, 2170, 2203, 2236, 2269, 2302, 2335, 2368, 2401, 2402,
		2403, 2404, 2405, 2406, 2407, 2440, 2473, 2506, 2539 })
		{
			REQUIRE(pMap[pOutBuffer[result_index]] == 1);
			REQUIRE(pOutBuffer[result_index] == ex_value);
			result_index++;
		}
	}

	std::cout << "basic_test_a_star ok" << std::endl;
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
	std::cout << "test hash ok" << std::endl;
}

void conformity_test()
{
	for (int i = 0; i < 1000; i++)
		randomized_test_a_star();
	std::cout << "conformity test ok !" << std::endl;
}

TEST_CASE("test a star", "[astart]")
{
	basic_test_a_star();
	conformity_test();
	profiling_performance();
	test_hash();
}
