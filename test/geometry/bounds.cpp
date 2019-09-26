#include <cmath>

#include "gtest/gtest.h"

#include "geometry/bounds.hpp"

using std::isnan;
using terrain::geometry::Bounds;

TEST(BoundsTest, ConstructDefault) {
    Bounds bounds;
    EXPECT_TRUE( isnan(bounds.center.x) );
    EXPECT_TRUE( isnan(bounds.center.y) );
    EXPECT_TRUE( isnan(bounds.half_width) );

    EXPECT_TRUE( isnan(bounds.get_x_max() ));
    EXPECT_TRUE( isnan(bounds.get_x_min() ));
    EXPECT_TRUE( isnan(bounds.get_y_min() ));
    EXPECT_TRUE( isnan(bounds.get_y_max() ));
}

TEST(BoundsTest, ConstructByCenterAndSize) {
    Bounds bounds({1.0, 2.0}, 5.0);
    
    ASSERT_DOUBLE_EQ(bounds.center.x,     1.0);
    ASSERT_DOUBLE_EQ(bounds.center.y,     2.0);
    ASSERT_DOUBLE_EQ(bounds.half_width,   2.5);

    EXPECT_DOUBLE_EQ(bounds.get_x_max(),  3.5);
    EXPECT_DOUBLE_EQ(bounds.get_x_min(), -1.5);
    EXPECT_DOUBLE_EQ(bounds.get_y_max(),  4.5);
    EXPECT_DOUBLE_EQ(bounds.get_y_min(), -0.5);
}

TEST(BoundsTest, ContainPoints) {
    Bounds b({2.0, 3.0}, 2.0);
    // x bounds -> [1, 3]
    // y bounds -> [2, 4]

    EXPECT_TRUE(  b.contains({  2  , 3  }));
    EXPECT_TRUE(  b.contains({  1.5, 2.5}));
    EXPECT_TRUE(  b.contains({  2.5, 2.5}));
    EXPECT_TRUE(  b.contains({  1.5, 3.5}));
    EXPECT_TRUE(  b.contains({  2.5, 3.5}));
    
    // borders contain points _on_ the borders
    EXPECT_TRUE( b.contains({2.0, 2.0}));
    EXPECT_TRUE( b.contains({1.0, 3.0}));

    EXPECT_FALSE( b.contains({0  , 0  }));
    EXPECT_FALSE( b.contains({1.0, 1.0}));
    
    EXPECT_FALSE( b.contains({2.0, 1.5}));
    EXPECT_FALSE( b.contains({2.0, 4.5}));
    EXPECT_FALSE( b.contains({0.5, 3.0}));
    EXPECT_FALSE( b.contains({3.5, 3.0}));

}
