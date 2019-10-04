#include <cmath>

#include "gtest/gtest.h"

#include "geometry/layout.hpp"

using std::isnan;
using terrain::geometry::Bounds;
using terrain::geometry::Layout;

TEST(LayoutTest, ConstructDefault) {
    Layout layout;

    EXPECT_TRUE(layout.bounds ==       Bounds({0,0}, 1.));

    EXPECT_DOUBLE_EQ(layout.precision, 1.0);
    EXPECT_DOUBLE_EQ(layout.dimension, 1.0);
    EXPECT_DOUBLE_EQ(layout.size,      1.0);
}

TEST(LayoutTest, ConstructDefaultValues) {
    Layout layout(Bounds({0,0}, 1), 1.);

    EXPECT_TRUE(layout.bounds ==       Bounds({0,0}, 1.));

    EXPECT_DOUBLE_EQ(layout.precision, 1.0);
    EXPECT_DOUBLE_EQ(layout.dimension, 1.0);
    EXPECT_DOUBLE_EQ(layout.size,      1.0);
}

TEST(LayoutTest, ConstructExplictEvenNumbers) {
    Layout layout(Bounds({0,0}, 8), 2.0);

    EXPECT_TRUE(layout.bounds == Bounds({0,0}, 8.));
    EXPECT_EQ(layout.dimension,                4);
    EXPECT_DOUBLE_EQ(layout.precision,         2.0);
    EXPECT_EQ(layout.size,                    16);
}

TEST(LayoutTest, ConstructExplictUneven_w5_p2) {
    Layout layout(Bounds({0,0}, 5), 2);

    EXPECT_TRUE(layout.bounds == Bounds({0,0}, 5.));
    EXPECT_EQ(layout.dimension,                4);
    EXPECT_DOUBLE_EQ(layout.precision,         1.25);
    EXPECT_EQ(layout.size,                    16);
}

TEST(LayoutTest, ConstructExplictUneven_w16_p0_4) {
    Layout layout(Bounds({8,8}, 16.), 0.4);

    EXPECT_TRUE(layout.bounds == Bounds({8,8}, 16));
    EXPECT_EQ(layout.dimension,                64);
    EXPECT_DOUBLE_EQ(layout.precision,          0.25);
    EXPECT_EQ(layout.size,                   4096);
}

TEST(LayoutTest, ConstructExplictUneven_w32_p7) {
    Layout layout(Bounds({0,0}, 32.), 7);

    EXPECT_TRUE(layout.bounds == Bounds({0,0}, 32.));
    EXPECT_EQ(layout.dimension,                 8);
    EXPECT_DOUBLE_EQ(layout.precision,          4.0);
    EXPECT_EQ(layout.size,                     64);
}

TEST(LayoutTest, ConstructExplictUneven_w4096_p32) {
    Layout layout(Bounds({0,0}, 4096), 32);

    EXPECT_TRUE(layout.bounds == Bounds({0,0}, 4096));
    EXPECT_EQ(layout.dimension,       128);
    EXPECT_DOUBLE_EQ(layout.precision, 32.0);
    EXPECT_EQ(layout.size,           16384);
}

TEST(LayoutTest, ConstructExplictUneven_w4096_p40) {
    Layout layout(Bounds({0,0}, 4096), 40);

    EXPECT_TRUE(layout.bounds == Bounds({0,0}, 4096));
    EXPECT_EQ(layout.dimension,       128);
    EXPECT_DOUBLE_EQ(layout.precision, 32.0);
    EXPECT_EQ(layout.size,           16384);
}
