#pragma once

#include <string>

struct UrlSplitResult
{
    std::string proto;
    std::string authorization;
    std::string hostname;
    std::string port;
    std::string path;
};

/*

URL:

scheme:[//[user[:password]@host][:port]][/path][?query][#fragment]

*/

UrlSplitResult url_split(const std::string &url);