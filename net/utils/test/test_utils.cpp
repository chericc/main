#include <gtest/gtest.h>

#include "net_utils.hpp"
#include "xlog.hpp"

namespace {

struct UrlTestItem
{
    std::string url;
    UrlSplitResult expected_result;
};

class UrlTest : public testing::TestWithParam<int>
{
public:

    std::vector<UrlTestItem> items;

    void SetUp() override
    {
        if (!items.empty())
        {
            return ;
        }

        { // 0
            UrlTestItem item;
            item.url = "https://example.com";
            item.expected_result.proto = "https";
            item.expected_result.authorization = "";
            item.expected_result.hostname = "example.com";
            item.expected_result.port = "";
            item.expected_result.path = "";
            items.push_back(item);
        }
        { // 1
            UrlTestItem item;
            item.url = "https://example.com:8080";
            item.expected_result.proto = "https";
            item.expected_result.authorization = "";
            item.expected_result.hostname = "example.com";
            item.expected_result.port = "8080";
            item.expected_result.path = "";
            items.push_back(item);
        }
        { // 2
            UrlTestItem item;
            item.url = "https://example.com:8080/main?exec#part1";
            item.expected_result.proto = "https";
            item.expected_result.authorization = "";
            item.expected_result.hostname = "example.com";
            item.expected_result.port = "8080";
            item.expected_result.path = "/main?exec#part1";
            items.push_back(item);
        }
        { // 3
            UrlTestItem item;
            item.url = "https://admin@example.com:8080/main?exec#part1";
            item.expected_result.proto = "https";
            item.expected_result.authorization = "admin";
            item.expected_result.hostname = "example.com";
            item.expected_result.port = "8080";
            item.expected_result.path = "/main?exec#part1";
            items.push_back(item);
        }
        { // 4
            UrlTestItem item;
            item.url = "https://admin:123456@example.com:8080/main?exec#part1";
            item.expected_result.proto = "https";
            item.expected_result.authorization = "admin:123456";
            item.expected_result.hostname = "example.com";
            item.expected_result.port = "8080";
            item.expected_result.path = "/main?exec#part1";
            items.push_back(item);
        }
        { // 5
            UrlTestItem item;
            item.url = "filename.txt";
            item.expected_result.proto = "";
            item.expected_result.authorization = "";
            item.expected_result.hostname = "";
            item.expected_result.port = "";
            item.expected_result.path = "filename.txt";
            items.push_back(item);
        }
    }
};

TEST_P(UrlTest, basetest)
{
    int choice = GetParam();
    const UrlTestItem &item = items.at(choice);

    UrlSplitResult result = url_split(item.url);
    EXPECT_EQ(result.proto, item.expected_result.proto);
    EXPECT_EQ(result.authorization, item.expected_result.authorization);
    EXPECT_EQ(result.hostname, item.expected_result.hostname);
    EXPECT_EQ(result.port, item.expected_result.port);
    EXPECT_EQ(result.path, item.expected_result.path);
}

INSTANTIATE_TEST_SUITE_P(OnUrlTest,
                         UrlTest,
                         testing::Range(0, 6));

}