#include <cmath>

#include <gtest/gtest.h>

#include "geometry/interpolate.hpp"

using std::isnan;

namespace terrain::geometry {

TEST( InterpolateTest, InterpolateValueBetween) {
    Sample s1({0.0, 0.0}  0);
    Sample s2({10.0, 10.0}, 50);
    
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

TEST( InterpolateTest, InterpolateValueOffset) {
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

    
struct TestPoint{
    const double x;
    const double y;
    const cell_value_t value;

    constexpr TestPoint(const double _x, const double _y, const cell_value_t _value): x(_x), y(_y), value(_value) {}
};

TEST( InterpolateTest, InterpolateTree){
    Tree tree({{1,1}, 64}, 1.0);
    Terrain terrain(tree);
    tree.root->split();

    // Set Quadrant I:
    tree.root->get_northeast()->set_value(0);
    // Set Quadrdant II:
    tree.root->get_northwest()->set_value(50);
    // Set Quadrant III:
    tree.root->get_southwest()->set_value(100);
    // Set Quadrant IV:
    tree.root->get_southeast()->set_value(50);


    vector<TestPoint> test_cases;
    test_cases.emplace_back( -35,    4, cell_default_value);  // Start out of bounds
    test_cases.emplace_back( -33,    4, cell_default_value);
    test_cases.emplace_back( -32,    4, cell_default_value);
    test_cases.emplace_back( -31,    4,   70);  // border of tree
    test_cases.emplace_back( -30.9,  4,   70);
    test_cases.emplace_back( -30,    4,   70);
    test_cases.emplace_back( -20,    4,   70);
    test_cases.emplace_back( -17,    4,   70);
    test_cases.emplace_back( -16,    4,   70);
    test_cases.emplace_back( -15.1,  4,   70);
    test_cases.emplace_back( -15.0,  4,   70);  // breakpoint - center of outer cell
    test_cases.emplace_back( -14.9,  4,   70);
    test_cases.emplace_back( -10,    4,   62);
    test_cases.emplace_back( - 5,    4,   54);
    test_cases.emplace_back(   0,    4,   47);
    test_cases.emplace_back(   1,    4,   45);  // midpoint
    test_cases.emplace_back(   2,    4,   43);
    test_cases.emplace_back(  10,    4,   31);
    test_cases.emplace_back(  14,    4,   25);
    test_cases.emplace_back(  15,    4,   23);
    test_cases.emplace_back(  16,    4,   22);  // breakpoint - center of outer cell
    test_cases.emplace_back(  17,    4,   20);
    test_cases.emplace_back(  20,    4,   20);
    test_cases.emplace_back(  30,    4,   20);
    test_cases.emplace_back(  31,    4,   20);
    test_cases.emplace_back(  32,    4,   20);
    test_cases.emplace_back(  33,    4,   20);  // border of tree
    test_cases.emplace_back(  34,    4, cell_default_value);
    test_cases.emplace_back(  35,    4, cell_default_value);

    // =====================================
    for( const TestPoint& expect : test_cases){
        const auto& actual_value = tree.interp({expect.x, expect.y});

        std::ostringstream buf;
        buf << "@@  x=" << expect.x << "  y=" << expect.y << "  v=" << expect.value << endl;

        ASSERT_EQ(actual_value, expect.value) << buf.str();
    }
}


} // namespace terrain::geometry
