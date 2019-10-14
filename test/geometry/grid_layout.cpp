#include <cmath>

#include "gtest/gtest.h"

#include <nlohmann/json/json.hpp>

#include "geometry/grid_layout.hpp"

using std::isnan;
using terrain::geometry::GridLayout;

TEST(GridLayoutTest, ConstructDefault) {
    GridLayout layout;

    EXPECT_DOUBLE_EQ(layout.precision, 1.0);
    EXPECT_DOUBLE_EQ(layout.x, 0);
    EXPECT_DOUBLE_EQ(layout.y, 0);
    EXPECT_DOUBLE_EQ(layout.width, 1.0);
    
    EXPECT_DOUBLE_EQ(layout.center()[0],  0);
    EXPECT_DOUBLE_EQ(layout.center()[1],  0);

    EXPECT_DOUBLE_EQ(layout.anchor()[0], -0.5);
    EXPECT_DOUBLE_EQ(layout.anchor()[1], -0.5);

    EXPECT_DOUBLE_EQ(layout.get_x_max(),  0.5);
    EXPECT_DOUBLE_EQ(layout.get_x_min(), -0.5);
    EXPECT_DOUBLE_EQ(layout.get_y_max(),  0.5);
    EXPECT_DOUBLE_EQ(layout.get_y_min(), -0.5);

    EXPECT_EQ( layout.dimension, 1);
    EXPECT_EQ( layout.size, 1);

    // EXPECT_EQ( sizeof(layout), 32);
    EXPECT_EQ( sizeof(layout), 56);
}

TEST(GridLayoutTest, ConstructExplicitValues) {
    GridLayout layout(2.,   // precision
                      3,    // center.x
                      1,    // center.y
                      2.);  // width

    EXPECT_DOUBLE_EQ(layout.precision, 2.0);
    EXPECT_DOUBLE_EQ(layout.x, 3);
    EXPECT_DOUBLE_EQ(layout.y, 1);
    EXPECT_DOUBLE_EQ(layout.width, 2.0);
    
    EXPECT_DOUBLE_EQ(layout.dimension,    1.);
    EXPECT_DOUBLE_EQ(layout.size,         1.);

    EXPECT_DOUBLE_EQ(layout.center()[0],  3.);
    EXPECT_DOUBLE_EQ(layout.center()[1],  1.);

    EXPECT_DOUBLE_EQ(layout.anchor()[0],  2.);
    EXPECT_DOUBLE_EQ(layout.anchor()[1],  0.);
}

TEST(GridLayoutTest, ConstructExplicitEvenNumbers) {
    GridLayout layout(2., 0, 0, 8);

    EXPECT_DOUBLE_EQ(layout.precision, 2.0);
    EXPECT_DOUBLE_EQ(layout.x, 0);
    EXPECT_DOUBLE_EQ(layout.y, 0);
    EXPECT_DOUBLE_EQ(layout.width, 8.0);
}

TEST(GridLayoutTest, ConstructExplicitUneven_p2_w5) {
    GridLayout layout(2., 0, 0, 5.);

    EXPECT_DOUBLE_EQ(layout.dimension,    4.);
    EXPECT_DOUBLE_EQ(layout.precision,    1.25);
    EXPECT_DOUBLE_EQ(layout.size,        16.);
    EXPECT_DOUBLE_EQ(layout.x,            0.);
    EXPECT_DOUBLE_EQ(layout.y,            0.);
    EXPECT_DOUBLE_EQ(layout.width,        5.0);
}

TEST(GridLayoutTest, ConstructExplictUneven_p04_w16) {
    GridLayout layout(0.4, 8,8, 16.);

    EXPECT_DOUBLE_EQ(layout.dimension,    64.);
    EXPECT_DOUBLE_EQ(layout.precision,     0.25);
    EXPECT_DOUBLE_EQ(layout.size,       4096.);
    EXPECT_DOUBLE_EQ(layout.x,             8.);
    EXPECT_DOUBLE_EQ(layout.y,             8.);
    EXPECT_DOUBLE_EQ(layout.width,        16.0);

    // EXPECT_TRUE(layout.bounds == Bounds({8,8}, 16));
    // EXPECT_EQ(layout.dimension,                64);
    // EXPECT_DOUBLE_EQ(layout.precision,          0.25);
    // EXPECT_EQ(layout.size,                   4096);
}

TEST(GridLayoutTest, ConstructExplictUneven_p7_w32) {
    GridLayout layout(7, 0,0, 32.);

    EXPECT_DOUBLE_EQ(layout.dimension,     8.);
    EXPECT_DOUBLE_EQ(layout.precision,     4.0);
    EXPECT_DOUBLE_EQ(layout.size,         64.);
    EXPECT_DOUBLE_EQ(layout.x,             0.);
    EXPECT_DOUBLE_EQ(layout.y,             0.);
    EXPECT_DOUBLE_EQ(layout.width,        32.0);
}

TEST(GridLayoutTest, ConstructExplictUneven_p32_w4096) {
    GridLayout layout(32, 0, 0, 4096);

    EXPECT_DOUBLE_EQ(layout.dimension,   128.);
    EXPECT_DOUBLE_EQ(layout.precision,   32.0);
    EXPECT_DOUBLE_EQ(layout.size,      16384.);
    EXPECT_DOUBLE_EQ(layout.x,             0.);
    EXPECT_DOUBLE_EQ(layout.y,             0.);
    EXPECT_DOUBLE_EQ(layout.width,      4096.0);
}

TEST(GridLayoutTest, ConstructExplictUneven_p40_w4096) {
    GridLayout layout(40, 0, 0, 4096);

    EXPECT_DOUBLE_EQ(layout.dimension,   128.);
    EXPECT_DOUBLE_EQ(layout.precision,    32.0);
    EXPECT_DOUBLE_EQ(layout.size,      16384.);
    EXPECT_DOUBLE_EQ(layout.x,             0.);
    EXPECT_DOUBLE_EQ(layout.y,             0.);
    EXPECT_DOUBLE_EQ(layout.width,      4096.0);
}

TEST(GridLayoutTest, ContainPoints) {
    GridLayout l(1.,   // precision
                 2,    // center.x
                 3,    // center.y
                 2.);  // width
    // x bounds -> [1, 3]
    // y bounds -> [2, 4]

    EXPECT_TRUE(  l.contains({  2  , 3  }));
    EXPECT_TRUE(  l.contains({  1.5, 2.5}));
    EXPECT_TRUE(  l.contains({  2.5, 2.5}));
    EXPECT_TRUE(  l.contains({  1.5, 3.5}));
    EXPECT_TRUE(  l.contains({  2.5, 3.5}));
    
    // borders contain points _on_ the borders
    EXPECT_TRUE( l.contains({2.0, 2.0}));
    EXPECT_TRUE( l.contains({1.0, 3.0}));

    EXPECT_FALSE( l.contains({0  , 0  }));
    EXPECT_FALSE( l.contains({1.0, 1.0}));
    
    EXPECT_FALSE( l.contains({2.0, 1.5}));
    EXPECT_FALSE( l.contains({2.0, 4.5}));
    EXPECT_FALSE( l.contains({0.5, 3.0}));
    EXPECT_FALSE( l.contains({3.5, 3.0}));
}

TEST(GridLayoutTest, ManufactureFromJSON) {
    nlohmann::json source = {{"precision", 1},
                             {"x", 7.},
                             {"y", 8.},
                             {"width", 2.}};

    // vvvv Target vvvv
    auto opt = GridLayout::make(source);
    // ^^^^ Target ^^^^

    ASSERT_TRUE(opt);

    GridLayout& layout = *opt;
    EXPECT_DOUBLE_EQ(layout.dimension,   2.);
    EXPECT_DOUBLE_EQ(layout.precision,   1.);
    EXPECT_DOUBLE_EQ(layout.size,        4.);
    EXPECT_DOUBLE_EQ(layout.x,           7.);
    EXPECT_DOUBLE_EQ(layout.y,           8.);
    EXPECT_DOUBLE_EQ(layout.width,       2.);
};
