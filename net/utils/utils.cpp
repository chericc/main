#include "utils.hpp"

#include <string.h>

UrlSplitResult url_split(const std::string &url)
{
    // scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
    // proto://host path

    UrlSplitResult result;
    std::size_t pos = 0;
    std::size_t pos_host = 0;
    std::size_t pos_path = 0;

    /* parse protocol */
    pos = url.find(":");
    if (pos != url.npos)
    {
        result.proto = std::string(url, 0, pos);

        // skip '://'
        ++pos;
        for (int i = 0; i < 2; ++i)
        {
            if (url.at(pos) == '/')
            {
                ++pos;
            }
        }
    }
    else 
    {
        result.path = url;
        return result;
    }

    pos_host = pos;

    /* seperate path from hostname */
    pos = url.find_first_of("/?#", pos_host);
    if (pos != url.npos)
    {
        pos_path = pos;
    }
    else 
    {
        pos_path = url.size();
    }
    result.path = std::string(url, pos_path);

    // host is not empty
    if (pos_path != pos_host)
    {
        std::size_t pos_at = 0;
        pos = pos_host;
        do 
        {
            pos_at = url.find('@', pos);
            if (pos_at != url.npos && pos_at < pos_path)
            {
                result.authorization = std::string(url, pos_host, pos_at - pos_host);
                pos = pos_at + 1; // skip '@'
            }
            else 
            {
                break;
            }
        }
        while (true);

        std::size_t pos_brk = 0;
        std::size_t pos_col = 0;
        if ( (url.at(pos) == '[') && 
             ((pos_brk = url.find("]", pos)) != url.npos) &&
             (pos_brk < pos_path))
        {
            result.hostname = std::string(url, pos + 1, pos_brk - pos);
            if (url.at(pos_brk + 1) == ':')
            {
                result.port = std::string(url, pos_brk + 1, pos_path - pos_brk - 1);
            }
        }
        else if (((pos_col = url.find(":", pos)) != url.npos) &&
                    pos_col < pos_path)
        {
            result.hostname = std::string(url, pos, pos_col - pos);
            result.port = std::string(url, pos_col + 1, pos_path - pos_col - 1);
        }
        else 
        {
            result.hostname = std::string(url, pos, pos_path - pos);
        }
    }

    return result;
}