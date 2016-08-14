#ifndef ASTRING_VIEW_HEADER_HPP
#define ASTRING_VIEW_HEADER_HPP

#if 0 //full standard
#define STD_STRING_VIEW
namespace astd
{
   template<typename T>
   using basic_string_view=std::basic_string_ref<T>;
}
#elif _LIBCPP_VERSION || __GLIBCXX__ //experimental
#define EXPERIMENTAL_STRING_VIEW
#include <experimental/string_view>
namespace astd
{
   template<typename T>
   using basic_string_view=std::experimental::basic_string_view<T>;
}
#else //boost implementation
#define BOOST_STRING_VIEW
#include "boost/utility/string_view.hpp"
namespace astd
{
   template<typename T>
   using basic_string_view=boost::basic_string_view<T>;
}
#endif //if standard implementation exist

namespace astd
{
   typedef basic_string_view<char> string_view;
   typedef basic_string_view<char16_t> u16string_view;
   typedef basic_string_view<char32_t> u32string_view;
   typedef basic_string_view<wchar_t> wstring_view;
}

#ifdef BOOST_STRING_VIEW
#include <functional>

namespace std
{
   template<>
   struct hash<astd::string_view>
   {
      typedef astd::string_view argument_type;
      typedef std::size_t result_type;
      std::size_t operator()(const argument_type& arg) const noexcept
      {
         const size_t FNV_offset_basis = 2166136261U;
         const size_t FNV_prime = 16777619U;

         size_t Val = FNV_offset_basis;
         for (char c : arg)
         {	// fold in another byte
            Val ^= (size_t)c;
            Val *= FNV_prime;
         }
         return (Val);
      }
   };
}

#include <string>

inline std::string operator+(const std::string& lhs, const astd::string_view& rhs)
{
   std::string result = lhs;
   result.append(rhs.data(), rhs.size());
   return result;
}

#endif //!BOOST_STRING_VIEW

#endif //!ASTRING_VIEW_HEADER_HPP
