#include <gtest/gtest.h>

#include <list>

namespace {

class ListTest : public testing::Test
{
protected:
    void SetUp() override
    {
        for (int i = 0; i < 100; ++i)
        {
            list_100.push_back(i);
        }
    }

    std::list<int> list_empty;
    std::list<int> list_100;
};

TEST_F(ListTest, DefaultConstructor)
{
    EXPECT_EQ(0u, list_empty.size());
}

TEST_F(ListTest, ElementValue)
{
    int i = 0;
    for (auto const& item : list_100)
    {
        EXPECT_EQ(item, i);
        ++i;
    }
}

TEST_F(ListTest, Clear)
{
    list_100.clear();

    EXPECT_EQ(0, list_100.size());
    EXPECT_EQ(true, list_100.empty());
}

} // end of namespace ANNOY