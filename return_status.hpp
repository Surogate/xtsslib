#ifndef RETURN_STATUS_HPP
#define RETURN_STATUS_HPP

#include <cstdlib>
#include <tuple>

template <typename STATUS_TYPE, typename RETURN_TYPE>
struct basic_return_status
{
   typedef bool(*test_status_function)(const STATUS_TYPE&);

private:
   STATUS_TYPE _status;
   RETURN_TYPE rtype;
   test_status_function status_test;

public:
   basic_return_status() : _status(), rtype(), status_test(nullptr) {}
   basic_return_status(const basic_return_status&) = default;
   basic_return_status(basic_return_status&&) = default;
   ~basic_return_status() = default;

   basic_return_status& operator=(const basic_return_status&) = default;
   basic_return_status& operator=(basic_return_status&&) = default;

   basic_return_status(const STATUS_TYPE& st, const RETURN_TYPE& rt, test_status_function status_test)
      : _status(st), rtype(rt), status_test(status_test) {}

   basic_return_status(const STATUS_TYPE& st, RETURN_TYPE&& rt, test_status_function status_test)
      : _status(st), rtype(rt), status_test(status_test) {}

   operator bool() const { return valid(); }
   operator STATUS_TYPE() const { return status(); }
   bool valid() const { return status_test ? status_test(_status) : false; }

   const STATUS_TYPE& status() const { return _status; }
   STATUS_TYPE& status() { return _status; }
   const RETURN_TYPE& value() const { return rtype; }
   RETURN_TYPE& value() { return rtype; }
};

//template <typename STATUS_TYPE, typename RETURN_TYPE, typename... OTHER>
//struct return_status : return_status<STATUS_TYPE, std::tuple<RETURN_TYPE, OTHER...>>
//{
//   return_status(const STATUS_TYPE& st, RETURN_TYPE&& rt, OTHER...&& rts)
//       : status(st), rtype(rt, rts...)
//   {}
//};

template <typename RETURN_TYPE>
struct return_status : basic_return_status<int, RETURN_TYPE>
{
   return_status() 
      : return_status(EXIT_FAILURE)
   {}

   return_status(int sta)
      : return_status(sta, RETURN_TYPE())
   {}

   return_status(int sta, const RETURN_TYPE& rt)
     : basic_return_status<int, RETURN_TYPE>(sta, rt, [](const int& st) { return st == EXIT_SUCCESS; })
   {}

   return_status(int sta, RETURN_TYPE&& rt)
     : basic_return_status<int, RETURN_TYPE>(sta, rt, [](const int& st) { return st == EXIT_SUCCESS; })
   {}

   return_status(const return_status&) = default;
   return_status(return_status&&) = default;
};

#endif //!RETURN_STATUS_HPP
