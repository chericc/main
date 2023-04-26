#include <gtest/gtest.h>

#include "matrix_order.hpp"

TEST(MATRIX_ORDER, CONSTRUCTOR)
{
    // default constructor
    {
        MatrixOrder order;
        EXPECT_EQ(order.Width(), 0);
        EXPECT_EQ(order.Height(), 0);
        EXPECT_EQ(order.Size(), 0);
    }

    // w & h constructor
    {
        int w = 2;
        int h = 3;
        MatrixOrder order(w,h);
        EXPECT_EQ(order.Width(), w);
        EXPECT_EQ(order.Height(), h);
        EXPECT_EQ(order.Size(), w * h);
    }

    // copy constructor
    {
        int w = 2;
        int h = 3;
        MatrixOrder order1(w,h);
        MatrixOrder order2(order1);
        EXPECT_TRUE(order1 == order2);
    }

    // copy operator
    {
        int w = 2;
        int h = 3;
        MatrixOrder order1(w,h);
        MatrixOrder order2;

        EXPECT_TRUE(order1 != order2);

        order2 = order1;
        EXPECT_TRUE(order1 == order2);
    }
}

TEST(MATRIX_ORDER, OPERATIONS)
{
    /* join */
    {
        int w = 2;
        int h = 3;
        MatrixOrder order1(w,h+1);
        MatrixOrder order2(h-1,w);

        EXPECT_TRUE(order1.Joinable(order2));
        EXPECT_FALSE(order2.Joinable(order1));

        MatrixOrder order3 = order1.MakeJoined(order2);
        EXPECT_EQ(order3.Width(), order2.Width());
        EXPECT_EQ(order3.Height(), order1.Height());
    }

    /* transpose */
    {
        int w = 2;
        int h = 3;
        MatrixOrder order1(w,h);
        MatrixOrder order2 = order1.MakeTranspose();
        EXPECT_EQ(order2.Width(), h);
        EXPECT_EQ(order2.Height(), 2);
        EXPECT_TRUE(order2.MakeTranspose() == order1); 
    }
}