#ifndef TEST_URL_HPP
#define TEST_URL_HPP

#include <string>
#include <vector>
#include <cstdint>

struct test_url
{
   std::string url;
   std::string scheme;
   std::string authority;
   std::string user_info;
   std::string user;
   std::string password;
   std::string hostname;
   uint32_t port;
   std::string paths;
   std::string queries;
   std::string fragments;
};

std::vector<test_url> create_uri_view_samples();
void dump(const std::string& url);

#endif //!TEST_URL_HPP
