#include <cmath>

#include "gtest/gtest.h"

#include "geometry/bounds.hpp"

using std::isnan;
using geometry::Bounds;

TEST(BoundsTest, ConstructDefault) {
    Bounds bounds;
    EXPECT_TRUE( isnan(bounds.center.x) );
    EXPECT_TRUE( isnan(bounds.center.y) );
    EXPECT_TRUE( isnan(bounds.half_height) );
    EXPECT_TRUE( isnan(bounds.half_width) );

    EXPECT_TRUE( isnan(bounds.get_x_max() ));
    EXPECT_TRUE( isnan(bounds.get_x_min() ));
    EXPECT_TRUE( isnan(bounds.get_y_min() ));
    EXPECT_TRUE( isnan(bounds.get_y_max() ));
}

TEST(BoundsTest, ConstructByCenterAndSize) {
    Bounds bounds({1.0, 2.0}, 3.0, 5.0);
    
    ASSERT_DOUBLE_EQ(bounds.center.x,     1.0);
    ASSERT_DOUBLE_EQ(bounds.center.y,     2.0);
    ASSERT_DOUBLE_EQ(bounds.half_height,  1.5);
    ASSERT_DOUBLE_EQ(bounds.half_width,   2.5);

    EXPECT_DOUBLE_EQ(bounds.get_x_max(),  3.5);
    EXPECT_DOUBLE_EQ(bounds.get_x_min(), -1.5);
    EXPECT_DOUBLE_EQ(bounds.get_x_max(),  3.5);
    EXPECT_DOUBLE_EQ(bounds.get_y_min(),  0.5);
}

TEST(BoundsTest, ConstructByExtents) {
    Bounds bounds;
    
    bounds.clear();
    EXPECT_TRUE( isnan(bounds.center.x) );
    EXPECT_TRUE( isnan(bounds.center.y) );
    EXPECT_TRUE( isnan(bounds.half_height) );
    EXPECT_TRUE( isnan(bounds.half_width) );

    bounds.extend(1,1);
    ASSERT_DOUBLE_EQ(bounds.center.x,     1.0);
    ASSERT_DOUBLE_EQ(bounds.center.y,     1.0);
    ASSERT_DOUBLE_EQ(bounds.half_height,  0.);
    ASSERT_DOUBLE_EQ(bounds.half_width,   0.);

    bounds.extend(3, 5);
    ASSERT_DOUBLE_EQ(bounds.center.x,     2.0);
    ASSERT_DOUBLE_EQ(bounds.half_width,   1.);
    ASSERT_DOUBLE_EQ(bounds.center.y,     3.0);
    ASSERT_DOUBLE_EQ(bounds.half_height,  2.);

    bounds.extend(-1, -1);
    ASSERT_DOUBLE_EQ(bounds.center.x,     1.0);
    ASSERT_DOUBLE_EQ(bounds.half_width,   2.);
    ASSERT_DOUBLE_EQ(bounds.center.y,     2.0);
    ASSERT_DOUBLE_EQ(bounds.half_height,  3.);

    bounds.clear();
    bounds.extend( 3, 4);
    ASSERT_DOUBLE_EQ(bounds.center.x,     3.0);
    ASSERT_DOUBLE_EQ(bounds.half_width,   0.);
    ASSERT_DOUBLE_EQ(bounds.center.y,     4.0);
    ASSERT_DOUBLE_EQ(bounds.half_height,  0.);

    bounds.extend( 5,11);
    ASSERT_DOUBLE_EQ(bounds.center.x,     4.0);
    ASSERT_DOUBLE_EQ(bounds.half_width,   1.);
    ASSERT_DOUBLE_EQ(bounds.center.y,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_height,  3.5);

    bounds.extend(12, 8);
    ASSERT_DOUBLE_EQ(bounds.center.x,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_width,   4.5);
    ASSERT_DOUBLE_EQ(bounds.center.y,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_height,  3.5);

    bounds.extend( 9, 5);
    ASSERT_DOUBLE_EQ(bounds.center.x,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_width,   4.5);
    ASSERT_DOUBLE_EQ(bounds.center.y,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_height,  3.5);

    bounds.extend( 5, 6);
    ASSERT_DOUBLE_EQ(bounds.center.x,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_width,   4.5);
    ASSERT_DOUBLE_EQ(bounds.center.y,     7.5);
    ASSERT_DOUBLE_EQ(bounds.half_height,  3.5);
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
