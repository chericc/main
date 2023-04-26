#include <gtest/gtest.h>

#include <iostream>
#include <random>
#include <algorithm>

#include "rbtree.hpp"

typedef std::list<KeyType> RBOrder;

TEST(RBTEST, COPY_CONSTRUCT)
{
    {
        std::vector<KeyType> vec = {1,2,3,4,5,6,7,8,9};
        std::default_random_engine re;
        
        for (int i = 0; i < 100; ++i)
        {
            std::shuffle(vec.begin(), vec.end(), re);

            {
                RBTree tree;
                RBOrder preorder, midorder, suborder;
                RBOrder preorder2, midorder2, suborder2;

                for (auto &it : vec)
                {
                    tree.Insert(it);
                }

                RBTree tree2(tree);

                tree.GetOrders(preorder, midorder, suborder);
                tree2.GetOrders(preorder2, midorder2, suborder2);

                ASSERT_TRUE(preorder == preorder2);
                ASSERT_TRUE(midorder == midorder2);
                ASSERT_TRUE(suborder == suborder2);
            }
        }
    }
}

TEST(RBTEST, COPY)
{
    {
        std::vector<KeyType> vec = {1,2,3,4,5,6,7,8,9};
        std::default_random_engine re;
        
        for (int i = 0; i < 100; ++i)
        {
            std::shuffle(vec.begin(), vec.end(), re);

            {
                RBTree tree;
                RBTree tree2;
                RBOrder preorder, midorder, suborder;
                RBOrder preorder2, midorder2, suborder2;

                for (auto &it : vec)
                {
                    tree.Insert(it);
                }

                tree2 = tree;

                tree.GetOrders(preorder, midorder, suborder);
                tree2.GetOrders(preorder2, midorder2, suborder2);

                ASSERT_TRUE(preorder == preorder2);
                ASSERT_TRUE(midorder == midorder2);
                ASSERT_TRUE(suborder == suborder2);
            }
        }
    }
}

TEST(RBTEST, INSERT)
{
    /* insert test steps */
    {
        RBOrder preorder, midorder, suborder;
        RBTree t;
        
        t.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder.empty());
        ASSERT_TRUE(midorder.empty());

        /**
         * B(10)
         */
        t.Insert(10);
        t.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({10}));
        ASSERT_TRUE(midorder == RBOrder({10}));

        /* 2 items */
        {
            /**
             *    B(10)
             *         R(15)
             */
            RBTree t2 = t;
            t2.Insert(15);
            t2.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({10,15}));
            ASSERT_TRUE(midorder == RBOrder({10,15}));

            /* 3 items */
            {
                /**
                 *         B(10)
                 *  R(5)           R(15)
                 */
                RBTree t3 = t2;
                t3.Insert(5);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({10,5,15}));
                ASSERT_TRUE(midorder == RBOrder({5,10,15}));

                /* 4 items */
                {
                    /**
                     * conditon 1-1
                     *           B(10)
                     *      R(5)        R(15)
                     *   R(2)
                     * ---------------------->
                     *           R(10)
                     *      B(5)         B(15)
                     *   R(2)
                     * ---------------------->
                     *            B(10)
                     *      B(5)         B(15)
                     *   R(2)
                     */
                    RBTree t4 = t3;
                    t4.Insert(2);
                    t4.GetOrders(preorder, midorder, suborder);
                    ASSERT_TRUE(preorder == RBOrder({10,5,2,15}));
                    ASSERT_TRUE(midorder == RBOrder({2,5,10,15}));
                }
                {
                    /**
                     * condition 1-1
                     *              B(10)
                     *      R(5)            R(15)
                     *          R(7)
                     * ------------------------->
                     *              R(10)
                     *      B(5)            B(15)
                     *          R(7)
                     * ------------------------->
                     *              B(10)
                     *      B(5)            B(15)
                     *          R(7)      
                     */
                    RBTree t4 = t3;
                    t4.Insert(7);
                    t4.GetOrders(preorder, midorder, suborder);
                    ASSERT_TRUE(preorder == RBOrder({10,5,7,15}));
                    ASSERT_TRUE(midorder == RBOrder({5,7,10,15}));
                }
                {
                    /**
                     * condition 2-1
                     *              B(10)
                     *      R(5)            R(15)
                     *                   R(13)
                     * ------------------------->
                     *              R(10)
                     *      B(5)            B(15)
                     *                   R(13)
                     * ------------------------->
                     *              B(10)
                     *      B(5)            B(15)
                     *                   R(13) 
                     */
                    RBTree t4 = t3;
                    t4.Insert(13);
                    t4.GetOrders(preorder, midorder, suborder);
                    ASSERT_TRUE(preorder == RBOrder({10,5,15,13}));
                    ASSERT_TRUE(midorder == RBOrder({5,10,13,15}));
                }
                {
                    /**
                     * condition 2-1
                     *              B(10)
                     *      R(5)            R(15)
                     *                          R(17)
                     * ------------------------->
                     *              R(10)
                     *      B(5)            B(15)
                     *                          R(17)
                     * ------------------------->
                     *              B(10)
                     *      B(5)            B(15)
                     *                          R(17)
                     */
                    RBTree t4 = t3;
                    t4.Insert(17);
                    t4.GetOrders(preorder, midorder, suborder);
                    ASSERT_TRUE(preorder == RBOrder({10,5,15,17}));
                    ASSERT_TRUE(midorder == RBOrder({5,10,15,17}));
                }
            }
            {
                /**
                 * conditon 2-2
                 *        B(10)
                 *                    R(15)
                 *               R(12)
                 * ------------------------>
                 *         B(10)
                 *                R(12)
                 *                     R(15)
                 * ------------------------>
                 * conditon 2-3
                 *         R(10)
                 *               B(12)
                 *                    R(15)
                 * ----------------------->
                 *            B(12)
                 *     R(10)         R(15)
                 * 
                 */
                RBTree t3 = t2;
                t3.Insert(12);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({12,10,15}));
                ASSERT_TRUE(midorder == RBOrder({10,12,15}));
            }
            {
                /**
                 * condition 2-3
                 *        B(10)
                 *               R(15)
                 *                    R(18)
                 * --------------------->
                 *        R(10)
                 *               B(15)
                 *                    R(18)
                 * --------------------->
                 *        B(15)
                 *   R(10)       R(18)
                 */
                RBTree t3 = t2;
                t3.Insert(18);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({15,10,18}));
                ASSERT_TRUE(midorder == RBOrder({10,15,18}));
            }
        }

        {
            /**
             *       B(10)
             * R(5)
             */
            RBTree t2 = t;
            t2.Insert(5);
            t2.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({10,5}));
            ASSERT_TRUE(midorder == RBOrder({5,10}));


            {
                /**
                 * conditon 1-3
                 *           B(10)
                 *      R(5)
                 *   R(2)
                 * ------------------>
                 *           R(10)
                 *      B(5)
                 *   R(2)
                 * ------------------>
                 *         B(5)
                 *   R(2)        R(10)
                 */
                RBTree t3 = t2;
                t3.Insert(2);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({5,2,10}));
                ASSERT_TRUE(midorder == RBOrder({2,5,10}));
            }
            {
                /**
                 * condition 1-2
                 *             B(10)
                 *      R(5)
                 *        R(7)
                 * ------------------>
                 *             B(10)
                 *         R(7)
                 *      R(5)
                 * ------------------>
                 *             R(10)
                 *         B(7)
                 *      R(5)
                 * ------------------>
                 *         B(7)
                 *    R(5)      R(10)
                 */
                RBTree t3 = t2;
                t3.Insert(7);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({7,5,10}));
                ASSERT_TRUE(midorder == RBOrder({5,7,10}));
            }
            {
                /**
                 *           B(10)
                 *      R(5)      R(15)
                 */
                RBTree t3 = t2;
                t3.Insert(15);
                t3.GetOrders(preorder, midorder, suborder);
                ASSERT_TRUE(preorder == RBOrder({10,5,15}));
                ASSERT_TRUE(midorder == RBOrder({5,10,15}));
            }
        }
    }

    /* long insert */
    {
        RBTree tree;
        RBOrder preorder, midorder, suborder;

        /**
         *         B(1)
         */
        tree.Insert(1);

        /**
         *         B(1)
         *              R(2)
         */
        tree.Insert(2);

        /**
         *         B(1)
         *              R(2)
         *                 R(3)
         * ---------------------
         *         B(2)
         *     R(1)    R(3)
         */
        tree.Insert(3);

        /**
         *         B(2)
         *    R(1)        R(3)
         *                    R(4)
         * -----------------------
         *         B(2)
         *    B(1)        B(3)
         *                    R(4)
         */
        tree.Insert(4);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({2,1,3,4}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4}));

        /**
         *         B(2)
         *    B(1)        B(3)
         *                   R(4)
         *                     R(5)
         * ------------------------
         *         B(2)
         *    B(1)        B(4)
         *              R(3)  R(5)
         */
        tree.Insert(5);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({2,1,4,3,5}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5}));

        /**
         *       B(2)
         *   B(1)          B(4)
         *               R(3)  R(5)
         *                        R(6)
         * ---------------------------
         *        B(2)
         *    B(1)         R(4)
         *               B(3)   B(5)
         *                         R(6)
         */
        tree.Insert(6);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({2,1,4,3,5,6}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6}));

        /**
         *      B(2)
         *  B(1)            R(4)
         *               B(3)   B(5)
         *                         R(6)
         *                            R(7)
         * -------------------------------
         *      B(2)
         *  B(1)           R(4)
         *              B(3)      B(6)
         *                      R(5) R(7)
         */
        tree.Insert(7);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({2,1,4,3,6,5,7}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,7}));

        /**
         *      B(2)
         *  B(1)           R(4)
         *              B(3)      B(6)
         *                      R(5) R(7)
         *                              R(8)
         * ---------------------------------
         *      B(2)
         *  B(1)           R(4)
         *              B(3)      R(6)
         *                      B(5) B(7)
         *                              R(8)
         * ---------------------------------
         *      R(2)
         *  B(1)           B(4)
         *              B(3)      R(6)
         *                      B(5) B(7)
         *                              R(8)
         * ---------------------------------
         *            B(4)
         *     R(2)           R(6)
         *  B(1)  B(3)    B(5)   B(7)
         *                            R(8)
         * 
         */
        tree.Insert(8);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({4,2,1,3,6,5,7,8}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,7,8}));

        /**
         *            B(4)
         *     R(2)           R(6)
         *  B(1)  B(3)    B(5)   B(7)
         *                            R(8)
         *                               R(9)
         * ------------------------------------
         *            B(4)
         *     R(2)           R(6)
         *  B(1)  B(3)    B(5)      B(8)
         *                       R(7)  R(9)
         */
        tree.Insert(9);
        tree.GetOrders(preorder, midorder, suborder);
        ASSERT_TRUE(preorder == RBOrder({4,2,1,3,6,5,8,7,9}));
        ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,7,8,9}));
    }
}

TEST(RBTEST, DELETE)
{
    {
        RBTree t;
        RBOrder preorder, midorder, suborder;
        t.Insert(10);
        t.Erase(10);
        t.GetOrders(preorder, midorder, suborder);

        ASSERT_TRUE(preorder == RBOrder({}));
        ASSERT_TRUE(midorder == RBOrder({}));
    }

    {
        RBOrder preorder, midorder, suborder;

        /* fixup test */

        {
            RBTree tree;

            /**
             *        B(2)
             *    B(1)         R(4)
             *               B(3)   B(5)
             *                         R(6)
             */
            for (int i = 1; i <= 6; ++i)
            {
                tree.Insert(i);
            }

            /**
             * case erasefixup::1-1
             *        B(2)
             *   x             R(4,w)
             *               B(3)   B(5)
             *                         R(6)
             * -----------------------------
             *                B(4)
             *        R(2)            B(5)
             *      x    B(3)             R(6)
             * -----------------------------
             *                B(4)
             *        B(2)            B(5)
             *      x    B(3,w)             R(6)
             * 
             * -----------------------------
             * case erasefixup::1-2
             *                B(4)
             *        B(x,2)            B(5)
             *          R(3,w)             R(6)
             */    
            tree.Erase(1);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,3,5,6}));
            ASSERT_TRUE(midorder == RBOrder({2,3,4,5,6}));
        }

        {
            RBTree tree;

            /**
             *         B(2)
             *    B(1)        B(4)
             *              R(3)  R(5)
             */
            for (int i = 1; i <= 5; ++i)
            {
                tree.Insert(i);
            }

            /**
             * case erasefixup::1-4
             *         B(2)
             *    B(1)        B(4)
             *              R(3)  R(5)
             * ----------------------------------
             * y_original_color = B
             *              B(2)
             *       x              B(4)
             *                   R(3)  R(5)
             * ----------------------------------
             *              B(2)
             *       x              B(4)
             *                   R(3)  B(5)
             * ----------------------------------
             *              B(4)
             *       B(2)        B(5)
             *    x    R(3)
             * ----------------------------------
             *              B(4)(x)
             *       B(2)            B(5)
             *         R(3)
             */
            tree.Erase(1);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,3,5}));
            ASSERT_TRUE(midorder == RBOrder({2,3,4,5}));
        }
    }

    /* all position */
    {
        RBTree tree_base;
        RBOrder preorder, midorder, suborder;

        /**
         *            B(4)
         *     R(2)           R(6)
         *  B(1)  B(3)    B(5)      B(8)
         *                       R(7)  R(9)
         */
        for (int i = 1; i <= 9; ++i)
        {
            tree_base.Insert(i);
        }

        {
            RBTree tree = tree_base;
            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             * case erasefixup::1-2
             *            B(4)
             *     R(2)           R(6)
             *  x     B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             * break
             *            B(4)
             *     B(2,x)           R(6)
             *         R(3)    B(5)      B(8)
             *                       R(7)  R(9)
             */
            tree.Erase(1);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,3,6,5,8,7,9}));
            ASSERT_TRUE(midorder == RBOrder({2,3,4,5,6,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     B(3,x)         R(6)
             *  B(1)          B(5)      B(8)
             *                       R(7)  R(9)
             */
            tree.Erase(2);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,3,1,6,5,8,7,9}));
            ASSERT_TRUE(midorder == RBOrder({1,3,4,5,6,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             * case erasefixup::2-2
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             * break
             *            B(4)
             *     R(2,x)         R(6)
             *  R(1)          B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     B(2)           R(6)
             *  R(1)          B(5)      B(8)
             *                       R(7)  R(9)
             */
            tree.Erase(3);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,6,5,8,7,9}));
            ASSERT_TRUE(midorder == RBOrder({1,2,4,5,6,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             * y_original_color=Black
             * case erasefixup::1-4
             *            B(5)
             *     R(2)           R(6)
             *  B(1)  B(3)     x        B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(5)
             *     R(2)           B(6)
             *  B(1)  B(3)     x        R(8)
             *                       R(7) B(9)
             * ---------------------------------
             * break
             *            B(5,x)
             *     R(2)           R(8)
             *  B(1)  B(3)     B(6)     B(9)
             *                    R(7)
             */
            tree.Erase(4);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({5,2,1,3,8,6,7,9}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,5,6,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             * case erasefixup::1-4
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     R(2)           B(6)
             *  B(1)  B(3)      x       R(8)
             *                       R(7)  B(9)
             * ---------------------------------
             *            B(4)
             *     R(2)               R(8)
             *  B(1)  B(3)      B(6)       B(9)
             *                    R(7)
             */
            tree.Erase(5);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,3,8,6,7,9}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,4,6,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     R(2)           R(7)
             *  B(1)  B(3)    B(5)      B(8)
             *                             R(9)
             */
            tree.Erase(6);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,3,7,5,8,9}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,7,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                             R(9)
             */
            tree.Erase(7);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,3,6,5,8,9}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,8,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(9)
             *                       R(7)  
             */
            tree.Erase(8);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,3,6,5,9,7}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,7,9}));
        }

        {
            RBTree tree = tree_base;

            /**
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  R(9)
             * ---------------------------------
             *            B(4)
             *     R(2)           R(6)
             *  B(1)  B(3)    B(5)      B(8)
             *                       R(7)  
             */
            tree.Erase(9);
            tree.GetOrders(preorder, midorder, suborder);
            ASSERT_TRUE(preorder == RBOrder({4,2,1,3,6,5,8,7}));
            ASSERT_TRUE(midorder == RBOrder({1,2,3,4,5,6,7,8}));
        }
    }
}