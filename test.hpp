#ifndef TEST_HPP
#define TEST_HPP

template <typename T, typename Z>
bool equal(const T& lhs, const Z& rhs)
{
   if (lhs != rhs)
   {
      std::cerr << "error " << lhs << " != " << rhs << std::endl;
      return false;
   }
   return true;
}

#endif //!TEST_HPP
