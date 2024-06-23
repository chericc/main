#include <gtest/gtest.h>

#include "matrix.hpp"

TEST(MATRIX, CONSTRUCTOR) {
    // default constructor
    {
        Matrix matrix;
        EXPECT_EQ(matrix.Order().Width(), 0);
        EXPECT_EQ(matrix.Order().Height(), 0);
    }

    // construct with order
    {
        int w = 5;
        int h = 10;
        MatrixOrder order(w, h);
        Matrix matrix(order);
        EXPECT_EQ(matrix.Order().Width(), w);
        EXPECT_EQ(matrix.Order().Height(), h);
    }

    // construct with order and initializer
    {
        int w = 2;
        int h = 3;
        MatrixOrder order(w, h);
        Matrix matrix(order, {0, 1, 2, 3, 4});

        EXPECT_EQ(matrix.Order().Width(), w);
        EXPECT_EQ(matrix.Order().Height(), h);

        EXPECT_EQ(matrix.Element(0, 0), 0);
        EXPECT_EQ(matrix.Element(1, 0), 1);
        EXPECT_EQ(matrix.Element(0, 1), 2);
        EXPECT_EQ(matrix.Element(1, 1), 3);
        EXPECT_EQ(matrix.Element(0, 2), 4);
        EXPECT_EQ(matrix.Element(1, 2), 0);
    }

    // construct with order and data
    {
        int w = 3;
        int h = 2;
        MatrixOrder order(w, h);
        std::vector<MatrixDataType> data = {0, 1, 2, 3, 4};
        Matrix matrix(order, data);

        EXPECT_EQ(matrix.Order().Width(), w);
        EXPECT_EQ(matrix.Order().Height(), h);

        EXPECT_EQ(matrix.Element(0, 0), 0);
        EXPECT_EQ(matrix.Element(1, 0), 1);
        EXPECT_EQ(matrix.Element(2, 0), 2);
        EXPECT_EQ(matrix.Element(0, 1), 3);
        EXPECT_EQ(matrix.Element(1, 1), 4);
        EXPECT_EQ(matrix.Element(2, 1), 0);
    }

    // copy constructor
    {
        int w = 2;
        int h = 3;
        MatrixOrder order(w, h);
        Matrix matrix_1(order, {0, 1, 2, 3, 4});
        Matrix matrix_2(matrix_1);

        EXPECT_TRUE(matrix_1 == matrix_2);

        matrix_1.SetElement(0, 0, 1);
        EXPECT_TRUE(matrix_1 != matrix_2);
    }
}

TEST(MATRIX, OPERATIONS) {
    // -A
    {
        Matrix matrix({2, 3}, {1, 2, 3});
        Matrix matrix_neg(-matrix);
        Matrix matrix2({2, 3}, {-1, -2, -3});

        EXPECT_TRUE(matrix_neg == matrix2);
    }

    // A + B
    {
        Matrix matrix_a({2, 3}, {1, 2, 3});
        Matrix matrix_b({2, 3}, {3, 2, 1});
        Matrix matrix_result(matrix_a + matrix_b);
        Matrix matrix_o({2, 3}, {4, 4, 4});

        EXPECT_TRUE(matrix_result == matrix_o);
    }

    // A - B
    {
        Matrix matrix_a({2, 3}, {1, 2, 3});
        Matrix matrix_b({2, 3}, {3, 2, 1});
        Matrix matrix_result(matrix_a - matrix_b);
        Matrix matrix_o({2, 3}, {-2, 0, 2});

        EXPECT_TRUE(matrix_result == matrix_o);
    }

    // A * B
    {
        Matrix matrix_a({4, 2}, {1, 0, 3, -1, 2, 1, 0, 2});
        Matrix matrix_b({3, 4}, {4, 1, 0, -1, 1, 3, 2, 0, 1, 1, 3, 4});

        EXPECT_TRUE(matrix_a.Order().Joinable(matrix_b.Order()));

        Matrix matrix_result(matrix_a * matrix_b);
        Matrix matrix_o({3, 2}, {9, -2, -1, 9, 9, 11});

        EXPECT_TRUE(matrix_result == matrix_o);
    }

    // A * k
    {
        Matrix matrix({2, 3}, {1, 2, 3, 4, 5, 6});
        Matrix matrix_k = matrix * 2;
        Matrix matrix_o({2, 3}, {2, 4, 6, 8, 10, 12});

        EXPECT_TRUE(matrix_k.Order() == matrix.Order());
        EXPECT_TRUE(matrix_k == matrix_o);
    }

    // transpose
    {
        Matrix matrix_a({2, 3}, {1, 2, 3, 4, 5});
        Matrix matrix_transpose = matrix_a.MakeTranspose();

        /**
         * 1 2
         * 3 4
         * 5 0
         * -->
         * 1 3 5
         * 2 4 0
         */

        EXPECT_TRUE(matrix_transpose.Order() == MatrixOrder(3, 2));
        Matrix matrix_o({3, 2}, {1, 3, 5, 2, 4, 0});
        EXPECT_TRUE(matrix_transpose == matrix_o);
    }
}