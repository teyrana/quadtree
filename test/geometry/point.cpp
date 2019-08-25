#include <cmath>

#include "gtest/gtest.h"

#include "geometry/point.hpp"

using std::isnan;
using geometry::Point;


TEST(PointTest, ConstructDefault) {
    Point p;
    EXPECT_TRUE( isnan(p.x));
    EXPECT_TRUE( isnan(p.y));
}

TEST(PointTest, ConstructByCenterAndSize) {
    Point p(1.1, 2.2);

    ASSERT_DOUBLE_EQ(p.x, 1.1);
    ASSERT_DOUBLE_EQ(p.y, 2.2);
}

TEST(PointTest, Magnitude) {
    Point p(1.1, 2.2);
    ASSERT_DOUBLE_EQ(p.norm(), 2.459674775249769);
}


TEST(PointTest, VectorSubtract) {
    Point p1(1.0, 1.0);
    Point p2(2.0, 2.0);

    const Point diff = p2.subtract(&p1);

    ASSERT_DOUBLE_EQ(diff.x, 1.0);
    ASSERT_DOUBLE_EQ(diff.y, 1.0);
    ASSERT_DOUBLE_EQ(diff.norm(), M_SQRT2);
}

TEST(PointTest, VectorAngle) {
    {  // case 0:
        Point p1;
        Point p2;
        ASSERT_TRUE( isnan(p1.angle(&p2)));
        ASSERT_TRUE( isnan(p2.angle(&p1)));
    }{ // case 1:
        Point p1(0.0, 0.0);
        Point p2(0.0, 0.0);
        ASSERT_TRUE( isnan(p1.angle(&p2)));
        ASSERT_TRUE( isnan(p2.angle(&p1)));
    }{ // case 2:
        Point p1(1.0, 0.0);
        Point p2(0.0, 1.0);
        ASSERT_DOUBLE_EQ(p1.angle(&p2), M_PI_2);
        ASSERT_DOUBLE_EQ(p2.angle(&p1), M_PI_2);
    }{ // case 3:
        Point p1(2.0, 0.0);
        Point p2(0.0, 2.0);
        ASSERT_DOUBLE_EQ(p1.angle(&p2), M_PI_2);
        ASSERT_DOUBLE_EQ(p2.angle(&p1), M_PI_2);
    }{ // case 4:
        Point p1(1.0, 1.0);
        Point p2(0.0, 2.0);
        ASSERT_DOUBLE_EQ(p1.angle(&p2), M_PI_4);
        ASSERT_DOUBLE_EQ(p2.angle(&p1), M_PI_4);
    }
}

TEST(PointTest, CrossProduct) {
    {  // case 0:
        Point p1;
        Point p2;
        ASSERT_TRUE( isnan(p1.cross(&p2)));
        ASSERT_DOUBLE_EQ(p1.is_right_handed(&p2), 1.0);
        ASSERT_TRUE( isnan(p2.cross(&p1)));
        ASSERT_DOUBLE_EQ(p2.is_right_handed(&p1), 1.0);
    }{ // case 1:
        Point p1(0.0, 0.0);
        Point p2(0.0, 0.0);
        ASSERT_DOUBLE_EQ(p1.cross(&p2), 0.0);
        ASSERT_DOUBLE_EQ(p1.is_right_handed(&p2), 1.0);
        ASSERT_DOUBLE_EQ(p2.cross(&p1), 0.0);
        ASSERT_DOUBLE_EQ(p2.is_right_handed(&p1), 1.0);
    }{ // case 2:
        Point p1(1.0, 0.0);
        Point p2(0.0, 1.0);
        ASSERT_DOUBLE_EQ(p1.cross(&p2), 1.0);
        ASSERT_DOUBLE_EQ(p1.is_right_handed(&p2), 1.0);
        ASSERT_DOUBLE_EQ(p2.cross(&p1), -1.0);
        ASSERT_DOUBLE_EQ(p2.is_right_handed(&p1), -1.0);
    }{ // case 3:
        Point p1(2.0, 0.0);
        Point p2(0.0, 2.0);
        ASSERT_DOUBLE_EQ(p1.cross(&p2), 4.0);
        ASSERT_DOUBLE_EQ(p1.is_right_handed(&p2), 1.0);
        ASSERT_DOUBLE_EQ(p2.cross(&p1), -4.0);
        ASSERT_DOUBLE_EQ(p2.is_right_handed(&p1), -1.0);
    }
}

TEST(PointTest, DotProduct) {
    { // case 1:
        Point p1; // default constructed to NANs
        Point p2; // default constructed to NANS
        ASSERT_TRUE( isnan(p1.dot(&p2)));
    }{ // case 1:
        Point p1(1.0, 0.0);
        Point p2(0.0, 1.0);
        ASSERT_DOUBLE_EQ(p1.dot(&p2), 0);
    }{ // case 2: 
        Point p1(1.0, 0.0);
        Point p2(0.0, 1.0);
        ASSERT_DOUBLE_EQ(p1.dot(&p2), 0);
    }{ // case 3:
        Point p1(1.0, 0.0);
        Point p2(1.0, 0.0);
        ASSERT_DOUBLE_EQ(p1.dot(&p2), 1.0);
    }{ // case 4:
        Point p1(1.0, 0.0);
        Point p2(1.0, 1.0);
        ASSERT_DOUBLE_EQ(p1.dot(&p2), 1.0);
    }{ // case 5:

    }
}


