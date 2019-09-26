#include <cmath>

#include <gtest/gtest.h>

#include "quadtree/node.hpp"

using std::isnan;

namespace terrain::quadtree {

TEST(NodeTest, ConstructByCenterAndSize) {
    Node n({{1.0, 2.0}, 3.0}, 0);
    
    ASSERT_DOUBLE_EQ(n.bounds.center.x,    1.0);
    ASSERT_DOUBLE_EQ(n.bounds.center.y,    2.0);
    ASSERT_DOUBLE_EQ(n.bounds.half_width,  1.5);

    ASSERT_TRUE( n.is_leaf() );
    ASSERT_EQ( n.northeast.get(), nullptr);
    ASSERT_EQ( n.northwest.get(), nullptr);
    ASSERT_EQ( n.southwest.get(), nullptr);
    ASSERT_EQ( n.southeast.get(), nullptr);

    ASSERT_EQ( n.get_value(), 0);
}

TEST(NodeTest, SetGet) {
    Node n({{1.0, 2.0}, 5.0}, 22);
    ASSERT_TRUE( n.is_leaf() );
    ASSERT_EQ(n.get_value(), 22);

    ASSERT_TRUE( n.is_leaf() );
    ASSERT_EQ( n.northeast.get(), nullptr);
    ASSERT_EQ( n.northwest.get(), nullptr);
    ASSERT_EQ( n.southwest.get(), nullptr);
    ASSERT_EQ( n.southeast.get(), nullptr);
    
    n.set_value(24);

    ASSERT_TRUE( n.is_leaf() );
    ASSERT_EQ(n.get_value(), 24);
}

TEST(NodeTest, SplitNode){
    Node n({{10.0, 20.0}, 4.0}, NAN);

    ASSERT_TRUE(n.is_leaf());
    n.split();
    ASSERT_FALSE(n.is_leaf());

    Node& neq = *n.northeast.get();
    ASSERT_DOUBLE_EQ(neq.bounds.center.x,    11.0);
    ASSERT_DOUBLE_EQ(neq.bounds.center.y,    21.0);
    ASSERT_DOUBLE_EQ(neq.bounds.half_width,   1.0);
    ASSERT_TRUE( neq.is_leaf() );

    Node& nwq = *n.northwest.get();
    ASSERT_DOUBLE_EQ(nwq.bounds.center.x,     9.0);
    ASSERT_DOUBLE_EQ(nwq.bounds.center.y,    21.0);
    ASSERT_DOUBLE_EQ(nwq.bounds.half_width,   1.0);
    ASSERT_TRUE( nwq.is_leaf() );

    Node& swq = *n.southwest.get();
    ASSERT_DOUBLE_EQ(swq.bounds.center.x,     9.0);
    ASSERT_DOUBLE_EQ(swq.bounds.center.y,    19.0);
    ASSERT_DOUBLE_EQ(swq.bounds.half_width,   1.0);
    ASSERT_TRUE( swq.is_leaf() );

    Node& seq = *n.southeast.get();
    ASSERT_DOUBLE_EQ(seq.bounds.center.x,    11.0);
    ASSERT_DOUBLE_EQ(seq.bounds.center.y,    19.0);
    ASSERT_DOUBLE_EQ(seq.bounds.half_width,   1.0);
    ASSERT_TRUE( seq.is_leaf() );
}

TEST(NodeTest, InterpolateValueBetween) {
    Node n1({{0.0, 0.0},   5.0},  0);
    Node n2({{10.0, 10.0}, 5.0}, 50);
    
    ASSERT_TRUE( n1.is_leaf() );
    ASSERT_TRUE( n2.is_leaf() );

    ASSERT_EQ(n1.interpolate_linear({-1, -1}, n2),  0);
    ASSERT_EQ(n1.interpolate_linear({ 0,  0}, n2),  0);
    ASSERT_EQ(n1.interpolate_linear({ 1,  1}, n2),  5);
    ASSERT_EQ(n1.interpolate_linear({ 2,  2}, n2), 10);
    ASSERT_EQ(n1.interpolate_linear({ 3,  3}, n2), 15);
    ASSERT_EQ(n1.interpolate_linear({ 4,  4}, n2), 20);
    ASSERT_EQ(n1.interpolate_linear({ 5,  5}, n2), 25);
    ASSERT_EQ(n1.interpolate_linear({ 6,  6}, n2), 30);
    ASSERT_EQ(n1.interpolate_linear({ 7,  7}, n2), 35);
    ASSERT_EQ(n1.interpolate_linear({ 8,  8}, n2), 40);
    ASSERT_EQ(n1.interpolate_linear({ 9,  9}, n2), 45);
    ASSERT_EQ(n1.interpolate_linear({10, 10}, n2), 50);
    ASSERT_EQ(n1.interpolate_linear({11, 11}, n2), 50);
}

TEST(NodeTest, InterpolateValueOffset) {
    Node n1({{-15.0, -15.0}, 16.0}, 100);
    Node n2({{ 17.0, -15.0}, 16.0},  50);
    
    ASSERT_TRUE( n1.is_leaf() );
    ASSERT_TRUE( n2.is_leaf() );

    ASSERT_EQ(n1.interpolate_linear({-15.1, -15.0}, n2), 100);
    ASSERT_EQ(n1.interpolate_linear({-15.0, -15.0}, n2), 100);
    ASSERT_EQ(n1.interpolate_linear({-14.9, -15.0}, n2), 100);
    ASSERT_EQ(n1.interpolate_linear({-14.8, -15.0}, n2), 100);
    ASSERT_EQ(n1.interpolate_linear({-14.0, -15.0}, n2),  98);
    ASSERT_EQ(n1.interpolate_linear({-10.0, -15.0}, n2),  92);
    ASSERT_EQ(n1.interpolate_linear({ -5.0, -15.0}, n2),  84);
    ASSERT_EQ(n1.interpolate_linear({  0.0, -15.0}, n2),  77);
    ASSERT_EQ(n1.interpolate_linear({  1.0, -15.0}, n2),  75);
    ASSERT_EQ(n1.interpolate_linear({  2.0, -15.0}, n2),  73);
    ASSERT_EQ(n1.interpolate_linear({  5.0, -15.0}, n2),  69);
    ASSERT_EQ(n1.interpolate_linear({ 10.0, -15.0}, n2),  61);
    ASSERT_EQ(n1.interpolate_linear({ 11.0, -15.0}, n2),  59);
    ASSERT_EQ(n1.interpolate_linear({ 12.0, -15.0}, n2),  58);
    ASSERT_EQ(n1.interpolate_linear({ 13.0, -15.0}, n2),  56);
    ASSERT_EQ(n1.interpolate_linear({ 14.0, -15.0}, n2),  55);
    ASSERT_EQ(n1.interpolate_linear({ 15.0, -15.0}, n2),  53);
    ASSERT_EQ(n1.interpolate_linear({ 16.0, -15.0}, n2),  52);
    ASSERT_EQ(n1.interpolate_linear({ 16.9, -15.0}, n2),  50);
    ASSERT_EQ(n1.interpolate_linear({ 17.0, -15.0}, n2),  50);
    ASSERT_EQ(n1.interpolate_linear({ 17.1, -15.0}, n2),  50);
}

} // namespace quadtree
