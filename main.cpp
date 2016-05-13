
#include <iostream>
#include "aarray_view.hpp"

int main(void)
{
	std::cout << "hello world" << std::endl;
   std::vector<int> vec = { 1, 2, 3, 4, 5 };
   astd::array_view<int> view = vec;

   for (auto& integer : view)
   {
      std::cout << integer << std::endl;
   }

	return 0;
}