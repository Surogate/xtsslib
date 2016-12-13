#include <iostream>
#include "uri.hpp"
#include "test_uri.hpp"
#include "test.hpp"
#include "catch.hpp"
#include "uri_builder.hpp"

void dump(const xts::uri& url)
{
   std::cout << "==== url " << url.data() << " ====" << std::endl;
   std::cout << "scheme " << url.scheme() << std::endl;
   std::cout << "authority " << url.authority() << std::endl;
   std::cout << "user_information " << url.userinfo() << std::endl;
   std::cout << "user " << url.user() << std::endl;
   std::cout << "password " << url.password() << std::endl;
   std::cout << "hostname " << url.hostname() << std::endl;
   std::cout << "port " << url.port() << std::endl;

   std::cout << "path " << url.paths() << std::endl;
   for (auto& p : url.paths())
   {
      std::cout << "/ " << p << std::endl;
   }

   std::cout << "query " << url.queries() << std::endl;
   for (auto& q : url.queries())
   {
      std::cout << "? " << q << std::endl;
   }

   std::cout << "fragment " << url.fragments() << std::endl;
   for (auto& f : url.fragments())
   {
      std::cout << "# " << f << std::endl;
   }
}

void dump(const xts::uri_builder& builder)
{
   std::cout << "======= builder =======" << std::endl;
   std::cout << "scheme " << builder.scheme << std::endl;
   std::cout << "username " << builder.username << std::endl;
   std::cout << "password " << builder.password << std::endl;
   std::cout << "hostname " << builder.hostname << std::endl;
   std::cout << "port " << builder.port << std::endl;
   std::cout << "path" << std::endl;
   for (auto& str : builder.paths)
   {
      std::cout << "/ " << str << std::endl;
   }

   std::cout << "queries" << std::endl;
   for (auto& str : builder.queries)
   {
      std::cout << "/ " << str << std::endl;
   }

   std::cout << "fragments" << std::endl;
   for (auto& str : builder.fragments)
   {
      std::cout << "/ " << str << std::endl;
   }

   std::cout << "to_string " << builder.assemble_to_string() << std::endl;
}

void dump(const std::string& u)
{
   xts::uri url(u);
   dump(url);
}

std::vector<test_url> create_uri_view_samples()
{
   std::vector<test_url> result;

   result.push_back(test_url());
   result.push_back(test_url{ "reddit.com"
      , ""     , "", "", "", "", "" , 0, "reddit.com" });
   result.push_back({ "http://reddit.com"
      , "http" , "reddit.com", "", "", "", "reddit.com" });
   result.push_back({ "http://test@reddit.com"
      , "http" , "test@reddit.com", "test", "test", "", "reddit.com" });
   result.push_back({ "http://test:toto@reddit.com"
      , "http" , "test:toto@reddit.com", "test:toto", "test", "toto", "reddit.com" });
   result.push_back({ "http://reddit.com/tutu"
      , "http" , "reddit.com", "", "", "", "reddit.com", 0, "/tutu" });
   result.push_back({ "http://test:toto@reddit.com/tutu"
      , "http" , "test:toto@reddit.com", "test:toto", "test", "toto", "reddit.com", 0, "/tutu" });
   result.push_back({ "test:toto@reddit.com:80/tutu"
      , "test" , "", "", "", "", "", 0, "toto@reddit.com:80/tutu" });
   result.push_back({ "http://test:toto@reddit.com:80/tutu/titi"
      , "http" , "test:toto@reddit.com:80", "test:toto", "test", "toto", "reddit.com", 80, "/tutu/titi" });
   result.push_back({ "http://test:toto@reddit.com:80/tutu/titi?key=value&key2=value2#fragid1"
      , "http" , "test:toto@reddit.com:80", "test:toto", "test", "toto", "reddit.com", 80, "/tutu/titi", "?key=value&key2=value2", "#fragid1" });
   result.push_back({ "/images/lecture-en-ligne/one-piece/"
      , "" , "", "", "", "", "", 0, "/images/lecture-en-ligne/one-piece/" });
   result.push_back({ "relative/path/to/resource.txt"
      , "" , "", "", "", "", "",0, "relative/path/to/resource.txt" });
   result.push_back({ "./resource.txt#frag01"
      , "" , "", "", "", "", "",0, "./resource.txt", "", "#frag01" });
   result.push_back({ "resource.txt"
      , "" , "", "", "", "", "",0, "resource.txt" });
   result.push_back({ "#frag01"
      , "" , "", "", "", "", "",0, "", "", "#frag01" });

   return result;
}

std::vector<test_url_paths> create_uri_paths_samples()
{
   std::vector<test_url_paths> result;

   result.push_back(test_url_paths());
   result.push_back({ "//reddit.com",{ } });
   result.push_back({ "reddit.com",{ "reddit.com" } });
   result.push_back({ "test:toto@reddit.com",{ "toto@reddit.com" } });
   result.push_back({ "test:toto@reddit.com:80/tutu",{ "toto@reddit.com:80", "tutu" } });
   result.push_back({ "http://test:toto@reddit.com:80/tutu/titi",{ "tutu", "titi" } });
   result.push_back({ "http://test:toto@reddit.com:80/tutu/titi?key=value&key2=value2#fragid1", 
                     {"tutu", "titi" } });
   result.push_back({ "/images/lecture-en-ligne/one-piece/",
   { "images", "lecture-en-ligne", "one-piece", "" } });
   result.push_back({ "relative/path/to/resource.txt",
   { "relative", "path", "to", "resource.txt" } });
   result.push_back({ "./resource.txt#frag01",
   { ".", "resource.txt" } });
   result.push_back({ "resource.txt",
   { "resource.txt" } });

   return result;
}

TEST_CASE("testing uri viewer" , "[uri]")
{
   auto samples = create_uri_view_samples();
   for (auto& sample : samples)
   {
      xts::uri u(sample.url);
      
      CHECK(sample.scheme == u.scheme());
      CHECK(sample.authority == u.authority());
      CHECK(sample.user_info == u.userinfo());
      CHECK(sample.user == u.user());
      CHECK(sample.password == u.password());
      CHECK(sample.hostname == u.hostname());
      CHECK(sample.port == u.port());
      CHECK(sample.paths == u.paths().data());
      CHECK(sample.queries == u.queries().data());
      CHECK(sample.fragments == u.fragments().data());
   }

   std::vector<xts::uri> vec;
   for (auto& s : samples)
   {
      vec.push_back(xts::uri(s.url));
   }
   std::sort(vec.begin(), vec.end());

   auto path_sample = create_uri_paths_samples();

   for (auto& sample : path_sample)
   {
      xts::uri u(sample.url);
      auto paths = u.paths();
      auto size = std::distance(paths.begin(), paths.end());
      CHECK(sample.paths.size() == size);
      if (sample.paths.size() != size)
      {
         std::cerr << sample.url << std::endl;
      }
      if (sample.paths.size())
         CHECK(std::equal(paths.begin(), paths.end(), sample.paths.begin(), sample.paths.end()));
   }
}

TEST_CASE("testing uri builder", "[uri]")
{
   auto samples = create_uri_view_samples();
   for (auto& sample : samples)
   {
      xts::uri u(sample.url);
      xts::uri_builder b(u);

      CHECK(u == b.assemble());
      if (u != b.assemble())
      {
         dump(u);
         dump(b);
         dump(b.assemble());
      }
   }
}
