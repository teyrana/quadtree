#include <cmath>

#include "gtest/gtest.h"

#include <nlohmann/json/json.hpp>

#include "geometry/layout.hpp"

using std::isnan;
using terrain::geometry::Layout;

TEST(LayoutTest, ConstructDefault) {
    Layout layout;

    EXPECT_DOUBLE_EQ(layout.get_precision(), 1.0);
    EXPECT_DOUBLE_EQ(layout.get_x(), 0);
    EXPECT_DOUBLE_EQ(layout.get_y(), 0);
    EXPECT_DOUBLE_EQ(layout.get_width(), 1.0);

    EXPECT_EQ( layout.get_padding(),         64);
    EXPECT_EQ( layout.get_dimension(),        1);
    EXPECT_DOUBLE_EQ(layout.get_half_width(), 0.5);
    EXPECT_EQ( layout.get_size(),             1);

    EXPECT_DOUBLE_EQ(layout.get_center()[0],  0);
    EXPECT_DOUBLE_EQ(layout.get_center()[1],  0);

    EXPECT_DOUBLE_EQ(layout.get_anchor()[0], -0.5);
    EXPECT_DOUBLE_EQ(layout.get_anchor()[1], -0.5);

    EXPECT_DOUBLE_EQ(layout.get_x_max(),      0.5);
    EXPECT_DOUBLE_EQ(layout.get_x_min(),     -0.5);
    EXPECT_DOUBLE_EQ(layout.get_y_max(),      0.5);
    EXPECT_DOUBLE_EQ(layout.get_y_min(),     -0.5);

    // i.e. memory size:
    EXPECT_EQ( sizeof(layout), 64);
}

TEST(LayoutTest, ConstructExplicitValues) {
    Layout layout(1.,   // precision
                  3,    // center.x
                  1,    // center.y
                  2.);  // width

    EXPECT_DOUBLE_EQ(layout.get_precision(),   1.);
    EXPECT_DOUBLE_EQ(layout.get_x(),           3.);
    EXPECT_DOUBLE_EQ(layout.get_y(),           1.);
    EXPECT_DOUBLE_EQ(layout.get_width(),       2.);

    EXPECT_EQ(layout.get_padding(),           62);
    EXPECT_EQ( layout.get_dimension(),         2);
    EXPECT_DOUBLE_EQ(layout.get_half_width(),  1.);
    EXPECT_EQ( layout.get_size(),              4);
    EXPECT_DOUBLE_EQ(layout.get_center().x(),  3.);
    EXPECT_DOUBLE_EQ(layout.get_center().y(),  1.);
    EXPECT_DOUBLE_EQ(layout.get_anchor().x(),  2.);
    EXPECT_DOUBLE_EQ(layout.get_anchor().y(),  0.);
}

TEST(LayoutTest, ConstructExplicitEvenNumbers) {
    Layout layout(2., 0, 0, 8);

    EXPECT_DOUBLE_EQ(layout.get_precision(), 2.0);
    EXPECT_DOUBLE_EQ(layout.get_x(), 0);
    EXPECT_DOUBLE_EQ(layout.get_y(), 0);
    EXPECT_DOUBLE_EQ(layout.get_width(), 8.0);
}

TEST(LayoutTest, ConstructExplicitUneven_p2_w5) {
    Layout layout(2., 0, 0, 5.);

    EXPECT_DOUBLE_EQ(layout.get_precision(),  1.25);
    EXPECT_DOUBLE_EQ(layout.get_x(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_width(),      5.0);

    EXPECT_EQ(layout.get_padding(),          60);
    EXPECT_EQ(layout.get_dimension(),         4);
    EXPECT_EQ(layout.get_size(),             16);
}

TEST(LayoutTest, ConstructExplictUneven_p04_w16) {
    Layout layout(0.4, 8,8, 16.);

    EXPECT_DOUBLE_EQ(layout.get_precision(),  0.25);
    EXPECT_DOUBLE_EQ(layout.get_x(),          8.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          8.);
    EXPECT_DOUBLE_EQ(layout.get_width(),     16.0);

    EXPECT_EQ(layout.get_padding(),          52);
    EXPECT_EQ( layout.get_dimension(),       64);
    EXPECT_EQ( layout.get_size(),          4096);
}

TEST(LayoutTest, ConstructExplictUneven_p7_w32) {
    Layout layout(7, 0,0, 32.);

    EXPECT_DOUBLE_EQ(layout.get_precision(),  4.0);
    EXPECT_DOUBLE_EQ(layout.get_x(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_width(),     32.0);

    EXPECT_EQ(layout.get_padding(),         58);
    EXPECT_EQ(layout.get_dimension(),        8);
    EXPECT_EQ(layout.get_size(),            64);
}

TEST(LayoutTest, ConstructExplictUneven_p32_w4096) {
    Layout layout(32, 0, 0, 4096);

    EXPECT_DOUBLE_EQ(layout.get_precision(), 32.0);
    EXPECT_DOUBLE_EQ(layout.get_x(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_width(),   4096.0);

    EXPECT_EQ(layout.get_padding(),          50);
    EXPECT_EQ(layout.get_dimension(),       128);
    EXPECT_EQ(layout.get_size(),          16384);
}

TEST(LayoutTest, ConstructExplictUneven_p40_w4096) {
    Layout layout(40, 0, 0, 4096);

    EXPECT_DOUBLE_EQ(layout.get_precision(), 32.0);
    EXPECT_DOUBLE_EQ(layout.get_x(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          0.);
    EXPECT_DOUBLE_EQ(layout.get_width(),   4096.0);

    EXPECT_EQ(layout.get_padding(),          50);
    EXPECT_EQ(layout.get_dimension(),       128);
    EXPECT_EQ(layout.get_size(),          16384);
}

TEST(LayoutTest, ContainPoints) {
    Layout l(1.,   // precision
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

TEST(LayoutTest, ManufactureFromJSON) {
    nlohmann::json source = {{"precision", 1},
                             {"x", 7.},
                             {"y", 8.},
                             {"width", 2.}};

    // vvvv Target vvvv
    auto opt = Layout::make_from_json(source);
    // ^^^^ Target ^^^^

    ASSERT_TRUE(opt);

    Layout& layout = *opt;
    EXPECT_DOUBLE_EQ(layout.get_precision(),  1.);
    EXPECT_DOUBLE_EQ(layout.get_x(),          7.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          8.);
    EXPECT_DOUBLE_EQ(layout.get_width(),      2.);

    EXPECT_EQ(layout.get_padding(),          62);
    EXPECT_EQ(layout.get_dimension(),         2);
    EXPECT_EQ(layout.get_size(),              4);
}

TEST(LayoutTest, HashByRowMajor) {
    ASSERT_EQ( sizeof(uint64_t), 8);  // should be guaranteed, by spec
    ASSERT_EQ( sizeof(size_t), 8);    // not strictly necessary

    Layout layout(1.,  2.,  2., 4);

    EXPECT_DOUBLE_EQ(layout.get_precision(),  1.);
    EXPECT_DOUBLE_EQ(layout.get_x(),          2.);
    EXPECT_DOUBLE_EQ(layout.get_y(),          2.);
    EXPECT_DOUBLE_EQ(layout.get_width(),      4.);
    EXPECT_EQ(layout.get_dimension(),         4.);

    EXPECT_EQ( layout.rhash(0.5, 0.5),  0);
    EXPECT_EQ( layout.rhash(1.5, 0.5),  1);
    EXPECT_EQ( layout.rhash(2.5, 0.5),  2);
    EXPECT_EQ( layout.rhash(3.5, 0.5),  3);

    ASSERT_EQ( layout.rhash(0.5, 1.5),  4);
    ASSERT_EQ( layout.rhash(1.5, 1.5),  5);
    ASSERT_EQ( layout.rhash(2.5, 1.5),  6);
    ASSERT_EQ( layout.rhash(3.5, 1.5),  7);

    ASSERT_EQ( layout.rhash(0.5, 2.5),  8);
    ASSERT_EQ( layout.rhash(1.5, 2.5),  9);
    ASSERT_EQ( layout.rhash(2.5, 2.5), 10);
    ASSERT_EQ( layout.rhash(3.5, 2.5), 11);

    ASSERT_EQ( layout.rhash(0.5, 3.5), 12);
    ASSERT_EQ( layout.rhash(1.5, 3.5), 13);
    ASSERT_EQ( layout.rhash(2.5, 3.5), 14);
    ASSERT_EQ( layout.rhash(3.5, 3.5), 15);
}

TEST(LayoutTest, HashByZOrderCurve) {
    ASSERT_EQ( sizeof(uint64_t), 8);  // should be guaranteed, by spec
    ASSERT_EQ( sizeof(size_t), 8);    // not strictly necessary

    Layout layout(1,  2,  2, 4);

    EXPECT_EQ(layout.get_padding(),             60);
    EXPECT_DOUBLE_EQ(layout.get_precision(),     1.);
    EXPECT_DOUBLE_EQ(layout.get_x(),             2.);
    EXPECT_DOUBLE_EQ(layout.get_y(),             2.);
    EXPECT_DOUBLE_EQ(layout.get_width(),         4.);
    EXPECT_EQ(layout.get_dimension(),            4);

    EXPECT_EQ( layout.zhash(0.5, 0.5), 0x0000000000000000);
    EXPECT_EQ( layout.zhash(1.5, 0.5), 0x1000000000000000);
    ASSERT_EQ( layout.zhash(0.5, 1.5), 0x2000000000000000);
    ASSERT_EQ( layout.zhash(1.5, 1.5), 0x3000000000000000);
    
    EXPECT_EQ( layout.zhash(2.5, 0.5), 0x4000000000000000);
    EXPECT_EQ( layout.zhash(3.5, 0.5), 0x5000000000000000);
    ASSERT_EQ( layout.zhash(2.5, 1.5), 0x6000000000000000);
    ASSERT_EQ( layout.zhash(3.5, 1.5), 0x7000000000000000);

    ASSERT_EQ( layout.zhash(0.5, 2.5), 0x8000000000000000);
    ASSERT_EQ( layout.zhash(1.5, 2.5), 0x9000000000000000);
    ASSERT_EQ( layout.zhash(0.5, 3.5), 0xa000000000000000);
    ASSERT_EQ( layout.zhash(1.5, 3.5), 0xb000000000000000);

    ASSERT_EQ( layout.zhash(2.5, 2.5), 0xc000000000000000);
    ASSERT_EQ( layout.zhash(3.5, 2.5), 0xd000000000000000);
    ASSERT_EQ( layout.zhash(2.5, 3.5), 0xe000000000000000);
    ASSERT_EQ( layout.zhash(3.5, 3.5), 0xf000000000000000);
}
