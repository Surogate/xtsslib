

#include <iostream>
#include "catch.hpp"
#include "vector_deck.hpp"

TEST_CASE("test vector deck", "[container]")
{
	vector_deck<std::size_t, 3> deck;

	for (std::size_t i = 0; i < 10; i++)
	{
		deck.push_back(i);
	}

	for (std::size_t i = 0; i < 10; i++)
	{
		CHECK(deck[i] == i);
	}
}