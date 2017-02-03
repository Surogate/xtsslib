# xtsslib
Handy header only c++ library
Offer several utilities to extend the C++ standard library
Header only
May introduce a dependancy on boost/assert, boost/config, boost/throw_exception, boost/utility, boost/filesystem, boost/interprocess

## astd namespace
  * aarray_view
    - Provide a read only view of an array of type T
    - boost independant
  * afilesystem
    - Provide a filesystem abstraction mirroring std filesystem (http://en.cppreference.com/w/cpp/experimental/fs)
    - May use boost/filesystem if no current filesystem implementation is in standard library
  * astring_view
    - Provide a read only view of an array of char or an std::string, mirroring std string_view (http://en.cppreference.com/w/cpp/string/basic_string_view)
    - May use boost/assert, boost/config, boost/throw_exception & boost/utility

## xts namespace
  * fast_convert.hpp
    - function to convert from string to uint
  * file_operation.hpp
    - functions to fetch all the content of files
  * trim.hpp
    - heapless trim functions
    - depend on astring_view
  * interprocess
    - Offer cross platform, interprocess robust mutexes
    - dependancy on boost/interprocess
    
## cmake
  * utility.cmake : functions to find boost and SDL2 library
