#ifndef URI_BUILDER_HPP
#define URI_BUILDER_HPP

#include <string>
#include <vector>
#include "uri.hpp"

namespace xts
{
   template <typename CHAR>
   struct basic_uri_builder
   {
      typedef basic_uri<CHAR> uri;
      typedef typename uri::string_type string_type;
      typedef typename uri::string_view_type string_view_type;

      basic_uri_builder() = default;
      basic_uri_builder(const basic_uri_builder&) = default;
      basic_uri_builder(basic_uri_builder&&) = default;
      basic_uri_builder& operator=(basic_uri_builder&&) = default;
      basic_uri_builder& operator=(const basic_uri_builder&) = default;
      ~basic_uri_builder() = default;

      basic_uri_builder(const uri& uri)
         : scheme(uri.scheme())
         , username(uri.user())
         , password(uri.password())
         , hostname(uri.hostname())
         , port(uri.port())
      {
         auto uri_paths = uri.paths();
         paths = { uri_paths.begin(), uri_paths.end() };
         auto uri_queries = uri.queries();
         queries = { uri_queries.begin(), uri_queries.end() };
         auto uri_fragments = uri.fragments();
         fragments = { uri_fragments.begin(), uri_fragments.end() };
      }

      uri assemble() const
      {
         string_type result;

         if (scheme.size())
            result += scheme + ':';
         result += assemble_authority();
         for (auto& p : paths)
         {
            result += '/' + p;
         }
         for (auto& q : queries)
         {
            result += '?' + q;
         }
         for (auto& f : fragments)
         {
            result += '#' + f;
         }
         return uri{result};
      }

      void set_path_from_string(string_view_type string_view)
      {
         typename uri::path_tokenizer token(string_view);
         
         paths.clear();
         for (auto& s : token)
         {
            paths.emplace_back(s.to_string());
         }
      }

      void set_queries_from_string(string_view_type string_view)
      {
         typename uri::query_tokenizer token(string_view);

         queries.clear();
         for (auto& s : token)
         {
            queries.emplace_back(s.to_string());
         }
      }

      void set_fragments_from_string(string_view_type string_view)
      {
         typename uri::fragment_tokenizer token(string_view);

         fragments.clear();
         for (auto& s : token)
         {
            fragments.emplace_back(s.to_string());
         }
      }


      string_type scheme;
      string_type username;
      string_type password;
      string_type hostname;
      uint32_t port;

      std::vector<string_type> paths;
      std::vector<string_type> queries;
      std::vector<string_type> fragments;

   private:
      std::string assemble_authority() const
      {
         std::string hostinfo;
         
         if (hostname.size())
         {
            hostinfo = hostname;
            if (port > 0)
               hostinfo += ':' + std::to_string(port);

            std::string userinfo;

            if (username.size())
            {
               userinfo = username;
               if (password.size())
                  userinfo += ':' + password;
               userinfo += '@';
            }
            return "//" + userinfo + hostinfo;
         }        
         return {};
      }
   };

   typedef basic_uri_builder<char> uri_builder;
   typedef basic_uri_builder<wchar_t> wuri_builder;
}

#endif //!URI_BUILDER_HPP
