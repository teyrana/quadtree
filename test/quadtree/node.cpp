#include <cmath>

#include <gtest/gtest.h>

#include "quadtree/node.hpp"

using std::isnan;

namespace quadtree {

TEST(NodeTest, ConstructDefault) {
    Node n;

    EXPECT_TRUE( isnan(n.bounds.center.x));
    EXPECT_TRUE( isnan(n.bounds.center.y));
    EXPECT_TRUE( isnan(n.bounds.half_height));
    EXPECT_TRUE( isnan(n.bounds.half_width));

    ASSERT_EQ( n.northeast.get(), nullptr);
    ASSERT_EQ( n.quadrants.northwest.get(), nullptr);
    ASSERT_EQ( n.quadrants.southeast.get(), nullptr);
    ASSERT_EQ( n.quadrants.southwest.get(), nullptr);
    
    ASSERT_TRUE( n.is_leaf() );
}

TEST(NodeTest, ConstructByCenterAndSize) {
    Node n({1.0, 2.0}, 3.0, 5.0);
    
    ASSERT_DOUBLE_EQ(n.bounds.center.x,    1.0);
    ASSERT_DOUBLE_EQ(n.bounds.center.y,    2.0);
    ASSERT_DOUBLE_EQ(n.bounds.half_height, 1.5);
    ASSERT_DOUBLE_EQ(n.bounds.half_width,  2.5);
    ASSERT_TRUE( n.is_leaf() );
}

TEST(NodeTest, SetGet) {
    Node n({1.0, 2.0}, 5.0, 5.0);
    
    n.set_value(22);
    
    ASSERT_TRUE( n.is_leaf() );
    ASSERT_EQ(n.get_value(), 22);
    
}

TEST(NodeTest, SplitNode){
    Node n({10.0, 20.0}, 4.0, 4.4);

    ASSERT_TRUE(n.is_leaf());
    n.split();
    ASSERT_FALSE(n.is_leaf());

    Node& neq = *n.northeast.get();
    ASSERT_DOUBLE_EQ(neq.bounds.center.x,    11.1);
    ASSERT_DOUBLE_EQ(neq.bounds.center.y,    21.0);
    ASSERT_DOUBLE_EQ(neq.bounds.half_height,  1.0);
    ASSERT_DOUBLE_EQ(neq.bounds.half_width,   1.1);
    ASSERT_TRUE( neq.is_leaf() );

    Node& nwq = *n.quadrants.northwest.get();
    ASSERT_DOUBLE_EQ(nwq.bounds.center.x,     8.9);
    ASSERT_DOUBLE_EQ(nwq.bounds.center.y,    21.0);
    ASSERT_DOUBLE_EQ(nwq.bounds.half_height,  1.0);
    ASSERT_DOUBLE_EQ(nwq.bounds.half_width,   1.1);
    ASSERT_TRUE( nwq.is_leaf() );

    Node& swq = *n.quadrants.southwest.get();
    ASSERT_DOUBLE_EQ(swq.bounds.center.x,     8.9);
    ASSERT_DOUBLE_EQ(swq.bounds.center.y,    19.0);
    ASSERT_DOUBLE_EQ(swq.bounds.half_height,  1.0);
    ASSERT_DOUBLE_EQ(swq.bounds.half_width,   1.1);
    ASSERT_TRUE( swq.is_leaf() );

    Node& seq = *n.quadrants.southeast.get();
    ASSERT_DOUBLE_EQ(seq.bounds.center.x,    11.1);
    ASSERT_DOUBLE_EQ(seq.bounds.center.y,    19.0);
    ASSERT_DOUBLE_EQ(seq.bounds.half_height,  1.0);
    ASSERT_DOUBLE_EQ(seq.bounds.half_width,   1.1);
    ASSERT_TRUE( seq.is_leaf() );
}

TEST(NodeTest, InterpolateValue) {
    Node n1({0.0, 0.0},   5.0, 5.0);
    n1.set_value(0);
    Node n2({10.0, 10.0}, 5.0, 5.0);
    n2.set_value(50);
    
    ASSERT_TRUE( n1.is_leaf() );
    ASSERT_TRUE( n2.is_leaf() );

    ASSERT_EQ(n1.interpolate(n2, {-1, -1}),  0);
    ASSERT_EQ(n1.interpolate(n2, { 0,  0}),  0);
    ASSERT_EQ(n1.interpolate(n2, { 1,  1}),  5);
    ASSERT_EQ(n1.interpolate(n2, { 2,  2}), 10);
    ASSERT_EQ(n1.interpolate(n2, { 3,  3}), 15);
    ASSERT_EQ(n1.interpolate(n2, { 4,  4}), 20);
    ASSERT_EQ(n1.interpolate(n2, { 5,  5}), 25);
    ASSERT_EQ(n1.interpolate(n2, { 6,  6}), 30);
    ASSERT_EQ(n1.interpolate(n2, { 7,  7}), 35);
    ASSERT_EQ(n1.interpolate(n2, { 8,  8}), 40);
    ASSERT_EQ(n1.interpolate(n2, { 9,  9}), 45);
    ASSERT_EQ(n1.interpolate(n2, {10, 10}), 50);
    ASSERT_EQ(n1.interpolate(n2, {11, 11}), 50);
}

} // namespace quadtree
