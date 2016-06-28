
#ifndef TREE_HPP
#define TREE_HPP

namespace xts
{
   template <typename T>
   struct Tree : public T
   {
      std::vector< Tree<T> > childs;
   };
}

#endif /* !TREE_HPP */