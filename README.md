# xtsslib
Handy header only c++ library
Offer several utilities to extend the C++ standard library
Header only
Require c++17 compliant compiler

## astd namespace
  * aarray_view
    - Provide a read only view of an array of type T
    - boost independant

## xts namespace
  * fast_convert.hpp
    - function to convert from string to uint
  * file_operation.hpp
    - functions to fetch all the content of files
  * trim.hpp
    - heapless trim functions
    - depend on astd::astring_view
  * interprocess
    - Offer cross platform, interprocess robust mutexes
    - dependancy on boost/interprocess
  * uri.hpp & uri_builder.hpp
    - Offer tools to parse and manipulate uri formatted data 
  * return_status.hpp
    - return_status structure designed to provide return types and status information.
    
## cmake
  * utility.cmake : functions to find boost and SDL2 library
