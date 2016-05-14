#include <cstdlib>
#include <iostream>
#include <vector>
#include "aarray_view.hpp"
#include "astring_view.hpp"
//#include <experimental/string_view>


//template <typename T>
//using basic = std::experimental::basic_string_view<T>;


int main(void)
{
   std::cout << "hello world" << std::endl;
   std::vector<int> vec = { 1, 2, 3, 4, 5 };
   astd::string_view test = "toto";
   // astd::array_view<int> view = vec;

   //basic<char> tutu = "tata";
   //  std::experimental::basic_string_view<char> basic_test = "titi";
   
   //for (auto& integer : view)
   {
     // std::cout << integer << std::endl;
   }

  return EXIT_SUCCESS;
}
