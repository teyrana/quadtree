#include <cmath>

#include "gtest/gtest.h"

#include "geometry/bounds.hpp"

using std::isnan;
using geometry::Bounds;

TEST(BoundsTest, ConstructDefault) {
    Bounds bd;
    EXPECT_TRUE( isnan(bd.center.x));
    EXPECT_TRUE( isnan(bd.center.y));
    EXPECT_TRUE( isnan(bd.half_height));
    EXPECT_TRUE( isnan(bd.half_width));
}

TEST(BoundsTest, ConstructByCenterAndSize) {
    Bounds bd({1.0, 2.0}, 3.0, 5.0);
    
    ASSERT_DOUBLE_EQ(bd.center.x,    1.0);
    ASSERT_DOUBLE_EQ(bd.center.y,    2.0);
    ASSERT_DOUBLE_EQ(bd.half_height, 1.5);
    ASSERT_DOUBLE_EQ(bd.half_width,  2.5);
}

TEST(BoundsTest, ContainPoints) {
    Bounds b({2.0, 3.0}, 2.0, 2.0);
    // x bounds -> [1, 3]
    // y bounds -> [2, 4]

    EXPECT_TRUE(  b.contains(  2  , 3  ));
    EXPECT_TRUE(  b.contains(  1.5, 2.5));
    EXPECT_TRUE(  b.contains(  2.5, 2.5));
    EXPECT_TRUE(  b.contains(  1.5, 3.5));
    EXPECT_TRUE(  b.contains(  2.5, 3.5));
    
    EXPECT_FALSE( b.contains(  2.0, 2.0));
    EXPECT_FALSE( b.contains(  1.0, 3.0));

    EXPECT_FALSE( b.contains(  0  , 0  ));
    EXPECT_FALSE( b.contains(  1.0, 1.0));
    
    EXPECT_FALSE( b.contains(  2.0, 1.5));
    EXPECT_FALSE( b.contains(  2.0, 4.5));
    EXPECT_FALSE( b.contains(  0.5, 3.0));
    EXPECT_FALSE( b.contains(  3.5, 3.0));

}
