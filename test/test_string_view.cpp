#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <string_view>
#include "compatibility.hpp"
#include "catch.hpp"

static void test_constructor()
{
   std::string cppstr = "Foo";
   char array[3] = { 'B', 'a', 'r' };

   std::string_view cppstr_v(cppstr);
   std::string_view array_v(array, 3);
   std::wstring_view wcstr_v = L"xyzzy";

   CHECK(cppstr == cppstr_v);
   CHECK(array_v == "Bar");
   CHECK(wcstr_v == L"xyzzy");
}

static void test_access()
{
   std::string ref = "Hello, world";
   std::string_view v = "Hello, world";
   for (std::size_t i = 0; i < v.size(); ++i)
   {
      CHECK(v[i] == ref[i]);
   }

   for (std::size_t i = 0; i < v.size(); ++i)
   {
      char ref_c = ref.at(i);
      char v_c = v.at(i);
      CHECK(ref_c == v_c);
   }

   {
      std::size_t i = 0;
      for (auto& c : v)
      {
         CHECK(c == ref[i]);
         ++i;
      }
   }
   
   CHECK(ref.front() == v.front());
   CHECK(ref.back() == v.back());
   CHECK(v.data() == &v[0]);
}

static void test_capacity()
{
   std::string_view str("Test string");
   CHECK(str.size() == 11);
   CHECK(str.length() == 11);
   CHECK(str.max_size());
   CHECK(str.empty() == false);
   CHECK(std::string_view().empty() == true);
}

static void test_modifier()
{
   {
      std::string str = "   trim me";
      std::string_view v = str;
      v.remove_prefix(std::min(v.find_first_not_of(" "), v.size()));
      CHECK(v == "trim me");
   }
   
   {
      char arr[] = { 'a', 'b', 'c', 'd', '\0', '\0', '\0' };
      std::string_view v(arr, sizeof arr);
      auto trim_pos = v.find('\0');
      if (trim_pos != std::string::npos)
         v.remove_suffix(v.size() - trim_pos);
      CHECK(v.size() == 4);
   }

   {
      std::string_view origin = "test";
      std::string_view target;
      target.swap(origin);
      CHECK(target == "test");
      CHECK(origin.empty());
   }
}

static void test_operation()
{
   std::string_view v = "Hello, world";

   {
	  std::string str = std::string(v);
      CHECK(str == v);
   }

   {
      std::vector<char> str;
      str.resize(6);
      v.copy(str.data(), 5, 0);
      CHECK(std::equal(str.begin(), str.end(), "Hello"));
   }

   {
      auto sub1 = v.substr(7);
      CHECK(sub1 == "world");
      CHECK(sub1.size() == 5);
      auto sub2 = v.substr(0, 5);
      CHECK(sub2 == "Hello");
      CHECK(sub2.size() == 5);
   }

   {
      auto result = v.find(',');
      CHECK(result == 5);
      result = v.find(',', 6);
      CHECK(result == std::string::npos);

      std::string_view str("There are two needles in this haystack with needles.");
      std::string_view str2("needle");

      // different member versions of find in the same order as above:
      std::size_t found = str.find(str2);
      CHECK(found == 14);

      found = str.find("needles are small", found + 1, 6);
      CHECK(found == 44);

      found = str.find("haystack");
      CHECK(found == 30);

      found = str.find('.');
      CHECK(found == 51);
   }

   {
      std::string_view str("The sixth sick sheik's sixth sheep's sick.");
      std::string_view key("sixth");

      std::size_t found = str.rfind(key);
      CHECK(found == 23);
   }

   {
      std::string_view str("/usr/bin/man");
      auto found = str.find_last_of("/\\");
      CHECK(str.substr(0, found) == "/usr/bin");
      CHECK(str.substr(found + 1) == "man");
   }

   {
      std::string_view str("Please, replace the vowels in this sentence by asterisks.");
      std::size_t found = str.find_first_of("aeiou");
      CHECK(found == 2);
   }
}

TEST_CASE("testing string_view", "[astring_view]")
{
   test_constructor();
   test_operation();
   test_capacity();
   test_access();
}
