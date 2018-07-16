#ifndef URL_HPP
#define URL_HPP

#include <string_view>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace xts
{
// Read only container to handle the path, query & fragment part of uri
template <typename CHAR_CONTAINER, CHAR_CONTAINER DELIMITOR>
class basic_uri_tokenizer
{
public:
   typedef std::basic_string<CHAR_CONTAINER> string_type;
   typedef std::basic_string_view<CHAR_CONTAINER> string_view_type;
   typedef std::vector<string_view_type> cutted_vector_type;
   typedef string_view_type value_type;
   typedef string_view_type* pointer;
   typedef string_view_type& reference;
   typedef const string_view_type& const_reference;
   typedef typename cutted_vector_type::iterator iterator;
   typedef typename cutted_vector_type::const_iterator const_iterator;

private:
   const string_view_type _data;
   const cutted_vector_type _cutted;

   static cutted_vector_type cut_data(const string_view_type& data)
   {
      cutted_vector_type result;
      auto beg = data.begin();
      auto end = data.end();
      while (beg != end)
      {
         if (*beg == DELIMITOR)
            ++beg;

         auto finded = std::find(beg, end, DELIMITOR);
         if (finded != end)
         {
            result.push_back(data.substr(std::distance(data.begin(), beg), std::distance(beg, finded)));
         }
         else
         {
            result.push_back(data.substr(std::distance(data.begin(), beg)));
         }
         beg = finded;
      }
      return result;
   }

public:
   basic_uri_tokenizer() = default;
   basic_uri_tokenizer(const basic_uri_tokenizer&) = default;
   basic_uri_tokenizer(basic_uri_tokenizer&&) = default;
   ~basic_uri_tokenizer() = default;
   basic_uri_tokenizer& operator=(const basic_uri_tokenizer&) = default;
   basic_uri_tokenizer& operator=(basic_uri_tokenizer&&) = default;
   basic_uri_tokenizer(const string_view_type& view)
       : _data(view), _cutted(cut_data(view))
   {
   }

   const string_view_type& data() const { return _data; }
   std::size_t data_size() const { return _data.size(); }
   std::size_t size() const { return _cutted.size(); }
   const_iterator begin() const { return _cutted.begin(); }
   const_iterator end() const { return _cutted.end(); }

   bool operator==(const basic_uri_tokenizer& rhs) const
   {
      return std::equal(begin(), end(), rhs.begin(), rhs.end());
   }
};

// Read only implementation of URI | URL | URN, as per defined in RFC 3986

template <typename CHAR_CONTAINER> class basic_uri
{
   public:
   typedef std::basic_string<CHAR_CONTAINER> string_type;
   typedef std::basic_string_view<CHAR_CONTAINER> string_view_type;
   typedef basic_uri_tokenizer<CHAR_CONTAINER, '/'> path_tokenizer;
   typedef basic_uri_tokenizer<CHAR_CONTAINER, '?'> query_tokenizer;
   typedef basic_uri_tokenizer<CHAR_CONTAINER, '#'> fragment_tokenizer;

   basic_uri() = default;
   basic_uri(const basic_uri&) = default;
   basic_uri(basic_uri&&) = default;
   basic_uri& operator=(const basic_uri&) = default;
   basic_uri& operator=(basic_uri&&) = default;
   ~basic_uri() = default;

   bool operator==(const basic_uri& rhs) const
   {
      return scheme() == rhs.scheme() && authority() == rhs.authority()
          && userinfo() == rhs.userinfo() && user() == rhs.user()
          && password() == rhs.password() && hostname() == rhs.hostname()
          && port() == rhs.port() && paths() == rhs.paths()
          && queries() == rhs.queries() && fragments() == rhs.fragments();
   }

   bool operator!=(const basic_uri& rhs) const { return !operator==(rhs); }

   bool operator>(const basic_uri& rhs) const { return _data > rhs._data; }

   bool operator<(const basic_uri& rhs) const { return _data < rhs._data; }

   basic_uri(const string_view_type& view)
       : _data(view)
   {
   }

   basic_uri(string_type&& str)
       : _data(str)
   {
   }

   bool absolute() const
   {
      return scheme().size() > 0 && authority().size() > 0;
   }

   const string_type& data() const { return _data; }
   const std::size_t size() const { return _data.size(); }

   string_view_type scheme() const
   {
      auto pos = _data.find_first_of(':');
      if(pos != string_type::npos)
         return string_view_type(_data.c_str(), pos);
      return {};
   }

   string_view_type authority() const
   {
      std::size_t start = _data.find("//");
      if(start != string_type::npos)
      {
         start += 2;

         std::size_t end = (std::min)(
             _data.find_first_of('/', start), _data.find_first_of('?', start));
         end = (std::min)(end, _data.find_first_of('#', start));
         if(end != string_type::npos)
            return {_data.c_str() + start, end - start};
         return {_data.c_str() + start};
      }
      return {};
   }

   string_view_type userinfo() const
   {
      auto autho = authority();
      std::size_t pos = autho.find_first_of('@');
      if(pos != string_type::npos)
      {
         return autho.substr(0, pos);
      }
      return {};
   }

   string_view_type user() const
   {
      auto unp = userinfo();
      auto pos = unp.find_first_of(':');
      return unp.substr(0, pos);
   }

   string_view_type password() const
   {
      auto unp = userinfo();
      auto pos = unp.find_first_of(':');
      if(pos != string_view_type::npos)
         return unp.substr(pos + DELIMITOR_SIZE);
      return {};
   }

   string_view_type hostname() const
   {
      auto autho = authority();
      auto user_info_pos = userinfo().size();
      if(user_info_pos > 0)
         user_info_pos += DELIMITOR_SIZE;
      auto host_port = autho.substr(user_info_pos);
      auto end = host_port.find_first_of(':');
      return host_port.substr(0, end);
   }

   uint32_t port() const
   {
      auto autho = authority();
      auto user_info_pos = userinfo().size();
      if(user_info_pos > 0)
         user_info_pos += DELIMITOR_SIZE;
      auto host_port = autho.substr(user_info_pos);
      auto delimitor = host_port.find_first_of(':');
      if(delimitor != string_view_type::npos)
      {
         auto port = host_port.substr(delimitor + 1);
         if(port.size())
         {
            return std::stoul(std::string(port));
         }
      }
      return {};
   }

   path_tokenizer paths() const
   {
      auto scheme_size = scheme().size();
      auto authority_size = authority().size();
      std::size_t start = 0;
      if(authority_size > 0)
         start = _data.find_first_of('/', scheme_size + authority_size);
      else if(scheme_size > 0)
         start = scheme_size + DELIMITOR_SIZE;

      if(start != string_type::npos)
      {
         auto end = (std::min)(
             _data.find_first_of('?', start), _data.find_first_of('#', start));
         if(end != string_type::npos)
         {
            return path_tokenizer(
                string_view_type{_data.c_str() + start, end - start});
         }
         else
         {
            return {_data.c_str() + start};
         }
      }
      return {};
   }

   query_tokenizer queries() const
   {
      auto start = _data.find_first_of('?');
      auto end = _data.find_first_of('#');
      if(start != string_type::npos)
      {
         if(end != string_type::npos)
         {
            return string_view_type{_data.c_str() + start, end - start};
         }
         return {_data.c_str() + start};
      }
      return {};
   }

   fragment_tokenizer fragments() const
   {
      auto start = _data.find_first_of('#');
      if(start != string_type::npos)
      {
         return string_view_type{_data.c_str() + start};
      }
      return {};
   }

   private:
   string_type _data;

   enum
   {
      SCHEME_DELIMITOR_SIZE = 3,
      DELIMITOR_SIZE = 1,
   };
};

typedef basic_uri<char> uri;
typedef basic_uri<wchar_t> wuri;
}

template <typename CHAR>
std::basic_ostream<CHAR>& operator<<(
    std::basic_ostream<CHAR>& stream, const xts::basic_uri<CHAR>& uri)
{
   stream << uri.data();
   return stream;
}

template <typename CHAR, CHAR DELIMITOR>
std::basic_ostream<CHAR>& operator<<(std::basic_ostream<CHAR>& stream,
    const xts::basic_uri_tokenizer<CHAR, DELIMITOR>& token)
{
   stream << token.data();
   return stream;
}

#endif // !URL_HPP
