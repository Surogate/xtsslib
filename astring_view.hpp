#ifndef ASTRING_VIEW_HEADER_HPP
#define ASTRING_VIEW_HEADER_HPP


#if 0 //full standard
namespace astd
{
   typedef std::string_view string_view;
}
#elif _LIBCPP_VERSION || __GLIBCXX__ //experimental
#include <experimental/string_view>

namespace astd
{
   typedef std::experimental::string_view string_view;
}

#else //boost implementation
#include "boost/utility/string_ref.hpp"
#include <functional>

namespace astd
{
   
   template<typename T>
   using basic_string_view=boost::basic_string_ref<T>;

   typedef basic_string_view<char> string_view;
   typedef basic_string_view<char16_t> u16string_view;
   typedef basic_string_view<char32_t> u32string_view;
   typedef basic_string_view<wchar_t> wstring_view;
}

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
         for (auto& c : arg)
         {	// fold in another byte
            Val ^= (size_t)c;
            Val *= FNV_prime;
         }
         return (Val);
      }
   };
}
#endif //if standard implementation exist

#endif //!ASTRING_VIEW_HEADER_HPP