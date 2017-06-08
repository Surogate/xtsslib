
#include "matrix.hpp"
#include "catch.hpp"

void basic_test()
{
	xts::matrix<int, 4, 1> mat2 = { 0, 1, 2, 3 };
	xts::coordinate_ref<int, 4, 1> coord(mat2);
	CHECK(coord.x() == 0);
	CHECK(coord.y() == 1);
	CHECK(coord.z() == 2);
}

void operation_test()
{
	{
		xts::matrix<int, 4, 1> value_1 = { 0, 1, 2, 3 };
		xts::matrix<int, 4, 1> value_2 = { 1, 1, 1, 1 };
		xts::matrix<int, 4, 1> result = value_1 + value_2;
		REQUIRE(result.size() == 4);
		CHECK(result[0] == 1);
		CHECK(result[1] == 2);
		CHECK(result[2] == 3);
		CHECK(result[3] == 4);
	}

	{
		xts::matrix<int, 4, 1> value_1 = { 0, 1, 2, 3 };
		xts::matrix<int, 4, 1> value_2 = { 1, 1, 1, 1 };
		xts::matrix<int, 4, 1> result = value_1 - value_2;
		REQUIRE(result.size() == 4);
		CHECK(result[0] == -1);
		CHECK(result[1] == 0);
		CHECK(result[2] == 1);
		CHECK(result[3] == 2);
	}

	{
		xts::mat4<int> mat{
			0x0, 0x1, 0x2, 0x3,
			0x4, 0x5, 0x6, 0x7,
			0x8, 0x9, 0xA, 0xB,
			0xC, 0xD, 0xE, 0xF
		};

		auto trans = xts::transpose(mat);
		auto expected = {
			0x0, 0x4, 0x8, 0xC,
			0x1, 0x5, 0x9, 0xD,
			0x2, 0x6, 0xA, 0xE,
			0x3, 0x7, 0xB, 0xF
		};
		REQUIRE(trans.size() == expected.size());
		std::size_t trans_index = 0;
		for (auto& ex : expected)
		{
			CHECK(trans[trans_index] == ex);
			++trans_index;
		}
	}

	{
		xts::vec3<int> lval{ 1, 3, -5 };
		xts::vec3<int> rval{ 4, -2, -1 };
		auto result = xts::dot_product(lval, rval);
		CHECK(result == 3);
	}

	{
		xts::mat4<int> trans{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			1, 1, 1, 1
		};

		xts::vec4<int> pos{
			1, 2, 3, 1
		};

		auto result = xts::dot_product(trans, pos);
		REQUIRE(result.size() == 4);
		std::size_t i = 0;
		for (auto n : { 2, 3, 4, 1 })
		{
			CHECK(result[i] == n);
			i++;
		}
	}

	{
		xts::mat4<int> ltrans{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			1, 1, 1, 1
		};

		xts::mat4<int> rtrans{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			1, 2, 3, 1
		};

		auto result = xts::dot_product(ltrans, rtrans);
		REQUIRE(result.size() == 16);
		std::size_t i = 0;
		for (auto n : { 1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						2, 3, 4, 1})
		{
			CHECK(result[i] == n);
			i++;
		}
	}

	{
		auto m1 = xts::identity<int, 1>();
		REQUIRE(m1.size() == 1);
		CHECK(m1[0] == 1);
	}

	{
		auto m1 = xts::identity<int, 2>();
		REQUIRE(m1.size() == 4);
		CHECK(m1[0] == 1);
		CHECK(m1[3] == 1);
	}

	{
		auto m1 = xts::identity<int, 3>();
		REQUIRE(m1.size() == 9);
		CHECK(m1[0] == 1);
		CHECK(m1[4] == 1);
		CHECK(m1[8] == 1);
	}

	{
		auto m1 = xts::identity<int, 4>();
		REQUIRE(m1.size() == 16);
		CHECK(m1[0] == 1);
		CHECK(m1[5] == 1);
		CHECK(m1[10] == 1);
		CHECK(m1[15] == 1);
	}
}


TEST_CASE("test matrix class", "[matrix]")
{
	basic_test();
	operation_test();
}

